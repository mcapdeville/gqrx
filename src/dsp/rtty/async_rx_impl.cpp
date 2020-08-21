/* 
 * asynchronous receiver block implementation
 * 
 * Copyright 2021 Marc CAPDEVILLE
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include <gnuradio/io_signature.h>
#include <volk/volk.h> 
#include "async_rx_impl.h"

#define LOG(x...)	fprintf(stderr,x)

namespace gr {
namespace rtty {

async_rx::sptr async_rx::make(float sample_rate, float bit_rate, char word_len, enum async_rx_parity parity) {
	return gnuradio::get_initial_sptr(new async_rx_impl(sample_rate, bit_rate, word_len, parity));
}

async_rx_impl::async_rx_impl(float sample_rate, float bit_rate, char word_len, enum async_rx_parity parity) :
			gr::block("async_rx",
				gr::io_signature::make(1, 1, sizeof(float)),
				gr::io_signature::make(1, 1, sizeof(unsigned char))),
			d_sample_rate(sample_rate),
			d_bit_rate(bit_rate),
			d_word_len(word_len),
			d_parity(parity) {
	bit_len = (sample_rate / bit_rate);
	state = ASYNC_WAIT_IDLE;
}

async_rx_impl::~async_rx_impl() {
}

void async_rx_impl::set_word_len(char word_len) {
	boost::mutex::scoped_lock lock(d_mutex);
	
	d_word_len = word_len;
}

char async_rx_impl::word_len() const {
	return d_word_len;
}

void async_rx_impl::set_sample_rate(float sample_rate) {
	boost::mutex::scoped_lock lock(d_mutex);
	
	d_sample_rate = sample_rate;
	bit_len = (sample_rate / d_bit_rate);
}

float async_rx_impl::sample_rate() const {
	return d_sample_rate;
}

void async_rx_impl::set_bit_rate(float bit_rate) {
	boost::mutex::scoped_lock lock(d_mutex);
	
	d_bit_rate = bit_rate;
	bit_len = (d_sample_rate / bit_rate);
}

float async_rx_impl::bit_rate() const {
	return d_bit_rate;
}

void async_rx_impl::set_parity(enum async_rx::async_rx_parity parity) {
	boost::mutex::scoped_lock lock(d_mutex);

	d_parity = parity;
}

enum async_rx::async_rx_parity async_rx_impl::parity() const {
	return d_parity;
}

void async_rx_impl::reset() {
	boost::mutex::scoped_lock lock(d_mutex);

	state = ASYNC_WAIT_IDLE;
}

void async_rx_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required) {
	boost::mutex::scoped_lock lock(d_mutex);
	
	ninput_items_required[0] = noutput_items * (d_word_len+2+(d_parity==ASYNC_RX_PARITY_NONE?0:1)) * bit_len;
}

int async_rx_impl::general_work (int noutput_items,
	       gr_vector_int &ninput_items,
	       gr_vector_const_void_star &input_items,
	       gr_vector_void_star &output_items) {
	float in_count = 0;
	int out_count = 0;
	const float *in = reinterpret_cast<const float*>(input_items[0]);
	unsigned char *out = reinterpret_cast<unsigned char*>(output_items[0]);
	float InAcc;

	while( (out_count < noutput_items) && (((int)roundf(in_count)) < (ninput_items[0]-bit_len))) {
		volk_32f_accumulator_s32f(&InAcc,&in[(int)roundf(in_count)],bit_len);
		switch (state) {
			case ASYNC_IDLE:	// Wait for MARK to SPACE transition
				if (InAcc<=0) { // transition detected
					//LOG("\\");
					in_count+=bit_len/2+1;
					state = ASYNC_CHECK_START;
				} else {
					in_count++;
				}
				break;
			case ASYNC_CHECK_START:	// Check start bit
				if (InAcc<=0) { // Start bit verified
					//LOG("^");
					LOG("0");
					in_count += bit_len;
					state = ASYNC_GET_BIT;
					bit_pos = 0;
					bit_count = 0;
					word = 0;
				} else { // Noise detection on start
					in_count -= bit_len/2;
					state = ASYNC_IDLE;
				}
				break;
			case ASYNC_GET_BIT:
				if (InAcc>0) {
					LOG("1");
					word |= 1<<bit_pos;
					bit_count++;
				}	
				else
					LOG("0");
				in_count+=bit_len;
				bit_pos++;
				if (bit_pos == d_word_len) {
					if (d_parity == ASYNC_RX_PARITY_NONE)
						state = ASYNC_CHECK_STOP;
					else
						state = ASYNC_CHECK_PARITY;
				}
				break;
			case ASYNC_CHECK_PARITY: // Check parity bit
				switch (d_parity) {
					default:
					case ASYNC_RX_PARITY_NONE:
						state = ASYNC_CHECK_STOP;
						break;
					case ASYNC_RX_PARITY_ODD:
						if ((InAcc<=0 && (bit_count&1)) || (InAcc>0 && !(bit_count&1))) {
					//		LOG("O");
							in_count += bit_len;
							state = ASYNC_CHECK_STOP;
						}
						else {
					//		LOG("o");
							if (InAcc>=0)
								state = ASYNC_IDLE;
							else
								state = ASYNC_WAIT_IDLE;
							in_count++;
						}
						break;
					case ASYNC_RX_PARITY_EVEN:
						if ((InAcc<=0 && !(bit_count&1)) || (InAcc>0 && (bit_count&1))) {
					//		LOG("E");
							in_count += bit_len;
							state = ASYNC_CHECK_STOP;
						}
						else {
					//		LOG("e");
							if (InAcc>=0)
								state = ASYNC_IDLE;
							else
								state = ASYNC_WAIT_IDLE;
							in_count++;
						}
						break;
					case ASYNC_RX_PARITY_MARK:
						if (InAcc>0) {
					//		LOG("M");
							in_count += bit_len;
							state = ASYNC_CHECK_STOP;
						}
						else {
					//		LOG("m");
							state = ASYNC_WAIT_IDLE;
							in_count++;
						}
						break;
					case ASYNC_RX_PARITY_SPACE:
						if (InAcc<=0) {
					//		LOG("S");
							in_count += bit_len;
							state = ASYNC_CHECK_STOP;
						}
						else {
					//		LOG("s");
							state = ASYNC_IDLE;
							in_count++;
						}
						break;
					case ASYNC_RX_PARITY_DONTCARE:
						//LOG("x");
						in_count += bit_len;
						state = ASYNC_CHECK_STOP;
						break;
				}
				if (InAcc<=0) 
					LOG("0");
				else
					LOG("1");
				break;
			case ASYNC_CHECK_STOP: // Check stop bit
				if (InAcc>0) { // Stop bit verified
					//LOG("$");
					LOG("1");
					*out = word;
					out++;
					out_count++;
					state = ASYNC_IDLE;
				} else { // Framming error
					//LOG("_");
					LOG("0");
					state = ASYNC_WAIT_IDLE;
				}
				in_count+=bit_len;
				break;
			default:
			case ASYNC_WAIT_IDLE:	// Wait for SPACE to MARK transition
				if (InAcc>0) { // transition detected
					//LOG("/");
					in_count+=bit_len/2+1;
					state = ASYNC_CHECK_IDLE;
				} else {
					in_count++;
				}
				break;
			case ASYNC_CHECK_IDLE:	// Check idle
				if (InAcc>0) { // Idle for 1 bit verified
					LOG("1");
					in_count += bit_len;
					state = ASYNC_IDLE;
					bit_pos = 0;
					bit_count = 0;
					word = 0;
				} else { // Noise detection on start
					in_count -= bit_len/2;
					state = ASYNC_WAIT_IDLE;
				}
				break;
		}
	}

	consume_each ((int)roundf(in_count));

	return (out_count);
}

}
}
