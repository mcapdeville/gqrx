/* 
 * Copyright 2020 Marc CAPDEVILLE
 * 
 * fsk/afsk demodulator block implementation header
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

#ifndef _FSK_DEMOD_IMPL_H_
#define _FSK_DEMOD_IMPL_H_

#include "fsk_demod.h"

namespace gr {
	namespace rtty {
		class fsk_demod_impl : public fsk_demod {
			public:
				fsk_demod_impl(float sample_rate,unsigned int decimation, float mark_freq,float space_freq);
				~fsk_demod_impl();

				void forecast(int noutput_items, gr_vector_int& ninput_items_required);

				int general_work(int noutput_items,
					gr_vector_int& ninput_item,
					gr_vector_const_void_star &input_items,
					gr_vector_void_star &output_items);

				void set_sample_rate(float sample_rate);
				float sample_rate() const;

				void set_mark_freq(float mark_freq);
				float mark_freq() const;

				void set_space_freq(float space_freq);
				float space_freq() const;

				void set_decimation(unsigned int decimation);
				int decimation() const;

			private:
				boost::mutex d_mutex;

				std::vector<gr_complex> d_Corr_mark;
				std::vector<gr_complex> d_Corr_space;

				float d_sample_rate;
				float d_mark_freq;
				float d_space_freq;
				float d_mark_div;
				float d_space_div;
				int state;
				unsigned int state_count;
				unsigned int n_dec;
				unsigned int d_decimation;

    };
  } // namespace rtty
} // namespace gr

#endif // _FSK_DEMOD_IMPL_H_

