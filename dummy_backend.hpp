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
#include <boost/foreach.hpp>
#include "tags.hpp"
#include <vector>

namespace mapnik
{

typedef std::pair<uint32_t,uint32_t> coord_type;
typedef std::vector<coord_type> path_type;
typedef std::vector<uint32_t> tags_type;

struct tile_element
{
    eGeomType type;
    tags_type tags;
    path_type path;
};

struct element_writer
{
    google::protobuf::io::StringOutputStream  raw_output_;
    google::protobuf::io::CodedOutputStream   coded_output_;
    explicit element_writer(std::string & buffer)
        : raw_output_(&buffer),
          coded_output_(&raw_output_)
    {}

    void write_varint32(uint32_t val)
    {
        coded_output_.WriteVarint32(val);
    }

    void write_tag(uint32_t val)
    {
        coded_output_.WriteTag(val);
    }
    void write_string(std::string const& str)
    {
        coded_output_.WriteString(str);
    }

    uint32_t bytes_written() const
    {
        return coded_output_.ByteCount();
    }
};

const static double SCALE = 16.0;
const static uint32_t TAGS_MAX = 627;

struct dummy_backend
{
private:
    std::vector<tags::tag_type> tags_;
    std::vector<std::string> custom_values_;
    boost::ptr_vector<tile_element> tile_elements_;
    std::auto_ptr<tile_element> element_;
    google::protobuf::io::ZeroCopyOutputStream * raw_output_;
    google::protobuf::io::CodedOutputStream * coded_output_;

public:
    explicit dummy_backend(std::string & output):
        raw_output_(new google::protobuf::io::StringOutputStream(&output)),
        coded_output_(new google::protobuf::io::CodedOutputStream(raw_output_))
    {}

    ~dummy_backend()
    {
        delete coded_output_;
        delete raw_output_;
    }

    boost::ptr_vector<tile_element> const& tile_elements() const
    {
        return tile_elements_;
    }
    std::vector<tags::tag_type> const& tags() const
    {
        return tags_;
    }

    template <typename T>
    void start_tile_element( T & feature)
    {
        // get geometry type FIXME: pass as an argument
        eGeomType type = Unknown;
        BOOST_FOREACH(geometry_type const& g, feature.paths())
        {
            type = g.type();
            break;
        }
        /////////////////////////////////////////////////////

        element_.reset(new tile_element);
        element_->type = type;

        feature_kv_iterator itr = feature.begin();
        feature_kv_iterator end = feature.end();
        for ( ;itr!=end; ++itr)
        {
            std::string const& name = boost::get<0>(*itr);
            mapnik::value const& val = boost::get<1>(*itr);
            if (!val.is_null())
            {
                boost::optional<tags::tag_type> tag = tags::tag_value_from_name(name,val.to_string());
                if ( tag )
                {
                    std::cout << "TAG = " << tag << std::endl;
                    if (*tag < TAGS_MAX)
                    {
                        element_->tags.push_back(*tag);
                    }
                    else
                    {
                        boost::optional<tags::tag_type> tag_key = tags::tag_from_name(name);
                        if (tag_key)
                        {
                            tags_.push_back(*tag_key);
                            custom_values_.push_back(val.to_string());
                            element_->tags.push_back((1023 + tags_.size()));
                            std::cout << "CUSTOM TAG=" <<*tag << " "
                                      << name <<  "="  << val.to_string()
                                      << " tag_key=" << (tags_.size() + 1023) << std::endl;
                        }
                    }
                }
            }
        }
    }

    void stop_tile_element()
    {
        tile_elements_.push_back(element_);
    }

    template <typename T>
    void add_path(T & path)
    {
        vertex2d vtx(vertex2d::no_init);
        path.rewind(0);
        unsigned count = 0;
        int32_t x=0,y=0;

        if (element_.get())
        {
            while ((vtx.cmd = path.vertex(&vtx.x, &vtx.y)) != SEG_END)
            {
                int32_t cur_x = static_cast<int32_t>(vtx.x * SCALE);
                int32_t cur_y = static_cast<int32_t>(vtx.y * SCALE);
                int32_t dx = cur_x - x;
                int32_t dy = cur_y - y;
                if (count > 0 && vtx.cmd == SEG_LINETO &&
                    std::fabs(dx) < 1.0 &&
                    std::fabs(dy) < 1.0)
                {
                    continue;
                }

                if (vtx.cmd == SEG_LINETO || vtx.cmd == SEG_MOVETO)
                {
                    // zigzag encoding
                    uint32_t xe = (dx << 1) ^ (dx >> 31);
                    uint32_t ye = (dy << 1) ^ (dy >> 31);
                    element_->path.push_back(coord_type(xe,ye));
                }

                x = cur_x;
                y = cur_y;
                ++count;
            }
            std::cout << "count = " << count << std::endl;
        }
    }

    uint32_t output_vector_tile()
    {
        // tile tags
        // num tags
        coded_output_->WriteTag(0x08);
        coded_output_->WriteVarint32(tags_.size());
        coded_output_->WriteTag(0x12);

        std::string buffer;
        element_writer writer(buffer);

        BOOST_FOREACH(tags::tag_type tag, tags_)
        {
            writer.write_varint32(tag);
            std::cerr << tag << ",";
        }
        std::size_t bytes = writer.bytes_written();

        coded_output_->WriteVarint32(bytes);
        coded_output_->WriteRaw(buffer.data(), bytes);

        std::cerr << "TILE TAGS VALUES [";

        BOOST_FOREACH(std::string const& val, custom_values_)
        {
            coded_output_->WriteTag(0x1a);
            coded_output_->WriteVarint32(val.size());
            coded_output_->WriteRaw(val.data(),val.size());
            std::cerr << '\"' << val << "\",";
        }
        std::cerr << "]" << std::endl;



        BOOST_FOREACH(tile_element const& elem, tile_elements_)
        {
            if (elem.path.empty())
                continue;

            if (elem.type == Polygon)
                coded_output_->WriteTag(0x62);
            else if (elem.type == LineString)
                coded_output_->WriteTag(0x5a);
            else if (elem.type == Point)
                coded_output_->WriteTag(0x6a);

            if (elem.tags.size() > 0)
            {
                // ELEMENT
                std::size_t element_bytes=0;
                std::string element_buffer;

                {
                    // ELEMENT TAGS
                    std::string tags_buffer;
                    element_writer writer(tags_buffer);
                    writer.write_tag(0x5a);
                    std::string element_tags;
                    uint32_t tags_bytes = output_element_tags(element_tags,elem.tags);
                    writer.write_varint32(tags_bytes);
                    writer.write_string(element_tags.substr(0,tags_bytes));
                    element_bytes += writer.bytes_written();
                    element_buffer+=tags_buffer.substr(0,writer.bytes_written());
                }
/*
                {
                    //TAG_ELEM_NUM_INDICES
                    coded_output_->WriteTag(0x08);
                    coded_output_->WriteVarint32(elem.tags.size());
                }
*/
                {
                    // TAG_ELEM_INDEX
                    std::string index_buffer;
                    element_writer writer(index_buffer);
                    writer.write_tag(0x62);
                    writer.write_varint32(0x01);
                    writer.write_varint32(elem.path.size());
                    element_bytes += writer.bytes_written();
                    element_buffer+=index_buffer.substr(0, writer.bytes_written());
                }

                // COORDS
                {
                    std::string coord_buffer;
                    element_writer writer(coord_buffer);
                    writer.write_tag(0x6a);
                    std::string element_paths;
                    uint32_t paths_bytes = output_element_paths(element_paths,elem.path);
                    writer.write_varint32(paths_bytes);
                    writer.write_string(element_paths.substr(0,paths_bytes));
                    element_bytes += writer.bytes_written();
                    element_buffer+=coord_buffer.substr(0,writer.bytes_written());
                }
                // write element
                coded_output_->WriteVarint32(element_bytes);
                coded_output_->WriteRaw(element_buffer.data(), element_bytes);

                std::cerr << "BYTES=" << element_bytes << std::endl;
            }
        }
        return coded_output_->ByteCount();
    }

    template <typename Tags>
    uint32_t output_element_tags(std::string & buffer, Tags const& tags)
    {
        element_writer writer(buffer);
        BOOST_FOREACH(tags::tag_type tag, tags)
        {
            std::cerr << tag << ",";
            writer.write_varint32(tag);
        }
        return writer.bytes_written();
    }

    template <typename Path>
    uint32_t output_element_paths(std::string & buffer, Path const& path)
    {
        element_writer writer(buffer);
        BOOST_FOREACH(coord_type c, path)
        {
            std::cerr << c.first << "," << c.second << "\n";
            writer.write_varint32(c.first);
            writer.write_varint32(c.second);
        }
        return writer.bytes_written();
    }

};
}

#endif //MAPNIK_DUMMY_BACKEND_HPP
