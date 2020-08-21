/* 
 * Copyright 2020 Marc CAPDEVILLE
 * 
 * fsk/afsk demodulator block header
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

#ifndef _FSK_DEMOD_H_
#define _FSK_DEMOD_H_

#include <gnuradio/block.h>

namespace gr {
	namespace rtty {
		class fsk_demod : virtual public gr::block {
			public:
				typedef boost::shared_ptr<fsk_demod> sptr;

				static sptr make(float sample_rate,unsigned int decimation,float mark_freq,float space_freq);

				virtual void forecast(int noutput_items, gr_vector_int& ninput_items_required) = 0;

				virtual	int general_work(int noutput_items,
					gr_vector_int& ninput_items,
					gr_vector_const_void_star &input_items,
					gr_vector_void_star &output_items) = 0;

				virtual void set_sample_rate(float sample_rate) = 0;
				virtual float sample_rate() const = 0;

				virtual void set_mark_freq(float mark_freq) = 0;
				virtual float mark_freq() const = 0;

				virtual void set_space_freq(float space_freq) = 0;
				virtual float space_freq() const = 0;

				virtual void set_decimation(unsigned int decimation) = 0;
				virtual int decimation() const = 0;

    };
  } // namespace rtty
} // namespace gr

#endif // _FSK_DEMOD_H_

