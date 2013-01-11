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

#ifndef MAPNIK_VECTOR_RENDERER_HPP
#define MAPNIK_VECTOR_RENDERER_HPP

// mapnik
#include <mapnik/config.hpp>            // for MAPNIK_DECL
#include <mapnik/feature_style_processor.hpp>
#include <mapnik/noncopyable.hpp>       // for noncopyable
#include <mapnik/rule.hpp>              // for rule, symbolizers
#include <mapnik/box2d.hpp>     // for box2d
#include <mapnik/ctrans.hpp>    // for CoordTransform

// fwd declarations to speed up compile
namespace mapnik {
  class Map;
  class feature_impl;
  class feature_type_style;
  class layer;
  class proj_transform;
}

namespace mapnik {

template <typename T>
class MAPNIK_DECL vector_renderer : public feature_style_processor<vector_renderer<T> >,
                                    private mapnik::noncopyable
{
    typedef T backend_type;
public:
    typedef vector_renderer<backend_type> processor_impl_type;

    // ctor
    vector_renderer(Map const& m, backend_type & backend, double scale_factor=1.0);
    // dtor
    ~vector_renderer();
    // hooks
    void start_map_processing(Map const& map);
    void end_map_processing(Map const& map);
    void start_layer_processing(layer const& lay, box2d<double> const& query_extent);
    void end_layer_processing(layer const& lay);
    void start_style_processing(feature_type_style const& st);
    void end_style_processing(feature_type_style const& st);
    // impl
    void process(line_symbolizer const& sym,
                 mapnik::feature_impl & feature,
                 proj_transform const& prj_trans);

    void process(polygon_symbolizer const& sym,
                 mapnik::feature_impl & feature,
                 proj_transform const& prj_trans);

    void process(point_symbolizer const& sym,
                 mapnik::feature_impl & feature,
                 proj_transform const& prj_trans);

    void painted(bool painted) {}
    inline bool process(rule::symbolizers const& /*syms*/,
                        mapnik::feature_impl & /*feature*/,
                        proj_transform const& /*prj_trans*/)
    {
        // agg renderer doesn't support processing of multiple symbolizers.
        return false;
    }

    inline eAttributeCollectionPolicy attribute_collection_policy() const
    {
        return COLLECT_ALL;
    }

private:
    backend_type & backend_;
    unsigned width_;
    unsigned height_;
    double scale_factor_;
    CoordTransform t_;
    box2d<double> query_extent_;
};

}

#endif // MAPNIK_VECTOR_RENDERER_HPP
