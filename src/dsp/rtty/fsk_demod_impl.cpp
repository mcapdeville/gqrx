/* 
 * fsk/afsk demodulator block implementation
 * 
 * Copyright 2020 Marc CAPDEVILLE
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
#include <boost/math/constants/constants.hpp>
#include <complex>
#include <volk/volk.h>
#include "fsk_demod_impl.h"


namespace gr {
	namespace rtty {
		
		fsk_demod_impl::fsk_demod_impl(float sample_rate, unsigned int decimation, float mark_freq, float space_freq) :
			gr::block("fsk_demod",
					gr::io_signature::make(1, 1, sizeof(gr_complex)),
					gr::io_signature::make(1, 1, sizeof(float))),
			d_sample_rate(sample_rate),
			d_mark_freq(mark_freq),
			d_space_freq(space_freq),
	       		state(0),
	       		state_count(0),
	       		n_dec(0)	{

			set_mark_freq(mark_freq);
			set_space_freq(space_freq);

			set_decimation(decimation);
		}

		fsk_demod::sptr fsk_demod::make(float sample_rate, unsigned int decimation, float mark_freq, float space_freq) {
			return gnuradio::get_initial_sptr (new fsk_demod_impl(sample_rate, decimation, mark_freq,space_freq));
		}

		fsk_demod_impl::~fsk_demod_impl() {
		}	
		
		void fsk_demod_impl::set_sample_rate(float sample_rate) {
			d_sample_rate = sample_rate;

			set_mark_freq(d_mark_freq);
			set_space_freq(d_space_freq);
		}
		
		float fsk_demod_impl::sample_rate() const {
			return d_sample_rate;
		}
		
		void fsk_demod_impl::set_mark_freq(float mark_freq) {
			boost::mutex::scoped_lock lock(d_mutex);
			
			int n;
			
			d_mark_freq = mark_freq;

			if (d_mark_freq == 0)
				d_mark_div = 1;
			else {
				d_mark_div = round(d_sample_rate/abs(d_mark_freq));
				if (d_mark_div == 0) 
					d_mark_div = 1;
			}

			d_Corr_mark.resize(d_mark_div);

			set_history(d_mark_div>d_space_div?d_mark_div:d_space_div);

			for (n=d_mark_div;n>0;n--)
				d_Corr_mark[d_mark_div-n] = std::polar((float)1,boost::math::constants::two_pi<float>()*n*d_mark_freq/d_sample_rate);
		}
		
		float fsk_demod_impl::mark_freq() const {
			return d_mark_freq;
		}

		void fsk_demod_impl::set_space_freq(float space_freq) {
			boost::mutex::scoped_lock lock(d_mutex);
			
			int n;
			
			d_space_freq = space_freq;

			if (d_space_freq == 0)
				d_space_div = 1;
			else {
				d_space_div = round(d_sample_rate/abs(d_space_freq));
				if (d_space_div == 0)
					d_space_div = 1;
			}

			d_Corr_space.resize(d_space_div);

			set_history(d_mark_div>d_space_div?d_mark_div:d_space_div);

			for (n=d_space_div;n>0;n--)
				d_Corr_space[d_space_div-n] = std::polar((float)1,boost::math::constants::two_pi<float>()*n*d_space_freq/d_sample_rate);
		}
		
		float fsk_demod_impl::space_freq() const {
			return d_space_freq;
		}

		void fsk_demod_impl::set_decimation(unsigned int decimation) {
			d_decimation = decimation;
//			set_relative_rate(1, decimation);
		}

		int fsk_demod_impl::decimation() const {
			return d_decimation;
		}

		void fsk_demod_impl::forecast(int noutput_items, gr_vector_int& ninput_items_required){
			ninput_items_required[0] = noutput_items*decimation();
		}

		int fsk_demod_impl::general_work (int noutput_items,
				gr_vector_int& ninput_items,
				gr_vector_const_void_star& input_items,
				gr_vector_void_star& output_items) {

			float * out = reinterpret_cast<float *>(output_items[0]);
			const gr_complex * in = reinterpret_cast<const gr_complex*>(input_items[0]);

			int n_out,n_in;

			gr_complex mark_power,space_power;
			float diff_power;

			for (n_out=0,n_in=0;n_out<noutput_items;n_out++) {

				diff_power = 0;
				n_dec = 0;	
				state = 0;
				do {

					if (d_mark_div>d_space_div) {
						volk_32fc_x2_dot_prod_32fc(&mark_power,&in[n_in+n_dec],
								d_Corr_mark.data(),d_mark_div);

						volk_32fc_x2_dot_prod_32fc(&space_power,&in[((int)d_mark_div-(int)d_space_div)+n_in+n_dec],
								d_Corr_space.data(),d_space_div);
					} else {
						volk_32fc_x2_dot_prod_32fc(&mark_power,&in[((int)d_space_div-(int)d_mark_div)+n_in+n_dec],
								d_Corr_mark.data(),d_mark_div);

						volk_32fc_x2_dot_prod_32fc(&space_power,&in[n_in+n_dec],
								d_Corr_space.data(),d_space_div);
					}

					diff_power = std::abs(mark_power)*d_space_div - std::abs(space_power)*d_mark_div;
					if (diff_power>0)
						state++;
					else
						state--;

					n_dec++;
				} while (n_dec < d_decimation);

				n_in += n_dec;

				out[n_out] = state;

				// fprintf(stderr,"%c",state>0?'1':'0');
			}

			consume_each(n_in);

			return n_out;
		}

	} /* namespace rtty */
} /* namespace gr */
