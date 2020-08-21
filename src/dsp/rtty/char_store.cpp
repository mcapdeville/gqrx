/* 
 * character sink with baudot decoder block implementation
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
#include "char_store.h"

static const int MIN_IN = 1;  /* Mininum number of input streams. */
static const int MAX_IN = 1;  /* Maximum number of input streams. */

char_store::sptr char_store::make(int size,bool baudot)
{
    return gnuradio::get_initial_sptr(new char_store(size,baudot));
}

char_store::char_store(int size,bool baudot) : gr::sync_block ("char_store",
                                gr::io_signature::make (MIN_IN, MAX_IN, sizeof(char)),
                                gr::io_signature::make (0, 0, 0)),
	d_data(size),
	d_baudot(baudot),
	d_figures(false)
{
}

char_store::~char_store ()
{

}

void char_store::store(std::string data)
{
    boost::mutex::scoped_lock lock(d_mutex);
    d_data.push_back(data);
}

void char_store::set_baudot(bool baudot) {
	d_baudot = baudot;
	d_figures = false;
}

#define BAUDOT_LETTERS 31
#define BAUDOT_FIGURES 27

static const char Baudot_letters[32] = "\0E\nA SIU\rDRJNFCKTZLWHYPQOBG\0MXV";
static const char Baudot_figures[32] = "\03\n- \a87\r$4',!:(5°)2#6019?&\0./;";

int char_store::work (int noutput_items,
	gr_vector_const_void_star &input_items,
	gr_vector_void_star &output_items) {
	std::string data;
	int i;
	char c;

	for (i=0;i<noutput_items;i++) {
		c = ((char*)input_items[0])[i];
		if (d_baudot) {
			c &= 0x1f;
			switch (c) {
				case BAUDOT_LETTERS:
					d_figures = false;
					continue;
				case BAUDOT_FIGURES:
					d_figures = true;
					continue;
				default:
					if (d_figures)
						data += Baudot_figures[c];
					else
						data += Baudot_letters[c];
			}
		} else
			data += c;
	}

	store(data);

	return noutput_items;
}

int char_store::get_data (std::string &data) {

	boost::mutex::scoped_lock lock(d_mutex);

	if (!d_data.empty()) {
		data=d_data.front();
		d_data.pop_front();
		return d_data.size();
	}

	return -1;
}
