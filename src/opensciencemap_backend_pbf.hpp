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

#ifndef MAPNIK_OPENSCIENCEMAP_BACKEND_PBF_HPP
#define MAPNIK_OPENSCIENCEMAP_BACKEND_PBF_HPP

//boost
#include <boost/algorithm/string/trim.hpp>
#include <boost/foreach.hpp>

////
#include "tags.hpp"
#include "TileData.pb.h"
// stl
#include <vector>

namespace mapnik
{

using org::oscim::database::oscimap::Data;
using org::oscim::database::oscimap::Data_Element;

typedef std::pair<uint32_t,uint32_t> coord_type;
typedef std::vector<coord_type> path_type;
typedef std::vector<uint32_t> tags_type;
typedef std::vector<uint32_t> index_type;

struct opensciencemap_backend_pbf
{
    struct tile_element
    {
        eGeomType type;
        tags_type tags;
        path_type path;
        index_type index;
    };

private:
    const double SCALE;
    const uint32_t TAGS_MAX;
    std::vector<tags::tag_type> tags_;
    std::vector<std::string> custom_values_;
    boost::ptr_vector<tile_element> tile_elements_;
    std::auto_ptr<tile_element> element_;
    std::string & output_;
public:
    explicit opensciencemap_backend_pbf(std::string & output)
        : SCALE(16.0),
          TAGS_MAX(627),
          output_(output)
    {}

    boost::ptr_vector<tile_element> const& tile_elements() const
    {
        return tile_elements_;
    }
    std::vector<tags::tag_type> const& tags() const
    {
        return tags_;
    }

    template <typename T>
    void start_tile_element( T & feature, eGeomType type)
    {
        /////////////////////////////////////////////////////
        element_.reset(new tile_element);
        element_->type = type; // geometry type POLY/LINE/POINT

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
            uint32_t start = element_->path.size();
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

                if (vtx.cmd == SEG_MOVETO && element_->path.size() > start)
                {
                    uint32_t size = element_->path.size() - start;
                    element_->index.push_back(size);
                    start = element_->path.size();
                }

                if (vtx.cmd == SEG_LINETO || vtx.cmd == SEG_MOVETO)
                {
                    element_->path.push_back(coord_type(dx,dy));
                }

                x = cur_x;
                y = cur_y;
                ++count;
            }
            if (element_->path.size() > start)
            {
                uint32_t size = element_->path.size() - start;
                element_->index.push_back(size);
            }
        }
    }

    uint32_t output_vector_tile()
    {
        Data data;
        data.set_num_tags(tags_.size());

        BOOST_FOREACH(tags::tag_type tag, tags_)
        {
            data.add_keys(tag);
        }

        BOOST_FOREACH(std::string const& val, custom_values_)
        {
            data.add_values(val);
        }

        // output tile elements
        BOOST_FOREACH(tile_element const& elem, tile_elements_)
        {
            if (elem.tags.size() > 0 && !elem.path.empty())
            {
                Data_Element * element = 0;
                if (elem.type == Polygon)
                    element = data.add_polygons();
                else if (elem.type == LineString)
                    element = data.add_lines();
                else if (elem.type == Point)
                    element = data.add_points();

                if (element)
                {
                    // optional uint32 num_indices = 1 [default = 1];
                    if (elem.index.size() > 1)
                    {
                        element->set_num_indices(elem.index.size());
                    }

                    // repeated uint32 tags = 11 [packed = true];
                    BOOST_FOREACH( tags_type::value_type tag , elem.tags)
                    {
                        element->add_tags(tag);
                    }

                    if (elem.type != Point)
                    {
                        // repeated uint32 indices = 12 [packed = true];
                        BOOST_FOREACH(index_type::value_type index, elem.index)
                        {
                            element->add_indices(index);
                        }
                    }
                    // repeated sint32 coordinates = 13 [packed = true];
                    BOOST_FOREACH(coord_type c, elem.path)
                    {
                        element->add_coordinates(c.first);
                        element->add_coordinates(c.second);
                    }
                }
            }
        }
        if (data.SerializeToString(&output_))
        {
            return data.ByteSize();
        }
        else
        {
            return 0;
        }
    }
};
}

#endif //MAPNIK_OPENSCIENCEMAP_BACKEND_PBF_HPP
