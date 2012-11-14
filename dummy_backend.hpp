/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2012 Artem Pavlenko
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *****************************************************************************/

#ifndef MAPNIK_DUMMY_BACKEND_HPP
#define MAPNIK_DUMMY_BACKEND_HPP

// mapnik
//#include <mapnik/util/geometry_to_wkb.hpp>
// proto
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
//boost
#include <boost/algorithm/string/trim.hpp>

namespace mapnik
{
struct dummy_backend
{
    template <typename T>
    void add_path(T & path)
    {
	vertex2d vtx(vertex2d::no_init);
	path.rewind(0);
	std::cout << "<Path>" << std::endl;
	std::string output;
	google::protobuf::io::ZeroCopyOutputStream* raw_output = new google::protobuf::io::StringOutputStream(&output);
	google::protobuf::io::CodedOutputStream* coded_output = new google::protobuf::io::CodedOutputStream(raw_output);
	unsigned count = 0;
	int x=0,y=0;
	while ((vtx.cmd = path.vertex(&vtx.x, &vtx.y)) != SEG_END)
	{
	    std::cout << vtx.x << "," << vtx.y << " cmd=" << vtx.cmd << std::endl;
	    int32_t cur_x = static_cast<int32_t>(vtx.x);
	    int32_t cur_y = static_cast<int32_t>(vtx.y);
	    int32_t dx = cur_x - x;
	    int32_t dy = cur_y - y;
	    if (count > 0 && vtx.cmd == SEG_LINETO &&
		std::fabs(dx) < 1.0 &&
		std::fabs(dy) < 1.0)
	    {
		continue;
	    }
	    // zigzag encoding
	    uint32_t xe = (dx << 1) ^ (dx >> 31);
	    uint32_t ye = (dy << 1) ^ (dy >> 31);
	    coded_output->WriteVarint32(xe);
	    coded_output->WriteVarint32(ye);
	    coded_output->WriteTag(vtx.cmd);
	    x = cur_x;
	    y = cur_y;
	    ++count;
	}

	std::cout << "count = " << count << std::endl;
	std::cout << "size = " << (count*(8 + 8 + 4)) << " new_size=" << output.size() << std::endl;
//std::string hex = util::to_hex(output.data(),output.size());
//	boost::algorithm::trim_right_if(hex,is_any_of("0"));
//	std::cout << hex << std::endl;
	std::cout << "</Path>" << std::endl;

	google::protobuf::io::ZeroCopyInputStream* raw_input = new google::protobuf::io::ArrayInputStream(output.c_str(),output.size());
	google::protobuf::io::CodedInputStream * coded_input = new google::protobuf::io::CodedInputStream(raw_input);
	uint32_t px=0,py=0;
	for (int i = 0; i < count ; ++i)
	{
	    uint32_t x0,y0;
	    coded_input->ReadVarint32(&x0);
	    coded_input->ReadVarint32(&y0);
	    int32_t xe = (x0 >> 1) ^ (-(x0 & 1));
	    int32_t ye = (y0 >> 1) ^ (-(y0 & 1));
	    px += xe;
	    py += ye;
	    uint32_t cmd = coded_input->ReadTag();
	    std::cout << px << "," << py << " cmd=" << cmd << std::endl;
	}
	delete coded_output;
	delete raw_output;
    }

};
}

#endif //MAPNIK_DUMMY_BACKEND_HPP
