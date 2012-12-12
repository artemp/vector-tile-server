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

#ifndef MAPNIK_TAGS_HPP
#define MAPNIK_TAGS_HPP

#include <mapnik/global.hpp>
#include <boost/optional.hpp>
#include <string>

namespace mapnik { namespace tags {
    typedef uint32_t tag_type;
    typedef std::string name_type;
    boost::optional<tag_type> tag_from_name(name_type const& name);
    boost::optional<std::string> name_from_tag(tag_type tag);
    boost::optional<tag_type> tag_index_from_kv(name_type const& name, std::string const& val);
    boost::optional<std::pair<std::string,std::string> > kv_from_tag(tag_type tag);
}}

#endif // MAPNIK_TAGS_HPP
