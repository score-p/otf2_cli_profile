/*
 * This file is part of otf2xx (https://github.com/tud-zih-energy/otf2xx)
 * otf2xx - A wrapper for the Open Trace Format 2 library
 *
 * Copyright (c) 2013-2016, Technische Universität Dresden, Germany
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef INCLUDE_OTF2XX_DEFINITIONS_MAPPING_TABLE_HPP
#define INCLUDE_OTF2XX_DEFINITIONS_MAPPING_TABLE_HPP

#include <otf2xx/fwd.hpp>
#include <otf2xx/reference.hpp>

#include <otf2xx/definition/detail/mapping_table_impl.hpp>

namespace otf2
{
namespace definition
{

    /**
     * \brief class for representing metric class definitions
     *
     * A metric class is a collection of metric members.
     *
     * If you don't have a referencing metric instance, then the scope
     * and recorder of this metric is implicitly given by the location,
     * where the referencing metric event occures.
     */
    class mapping_table : public detail::base<detail::mapping_table_impl>
    {
        using base = detail::base<detail::mapping_table_impl>;
        using base::base;

    public:
        using mapping_type_type = impl_type::mapping_type_type;
        using id_map_type = impl_type::id_map_type;

        mapping_table(mapping_type_type mapping_type, OTF2_IdMapMode mode = OTF2_ID_MAP_SPARSE,
                      std::size_t size = 64)
        : base(new impl_type(mapping_type, mode, size))
        {
        }

        mapping_table(mapping_type_type mapping_type, std::vector<uint64_t> mappings,
                      bool optimize_size = false)
        : base(new impl_type(mapping_type, mappings, optimize_size))
        {
        }

        mapping_table(mapping_type_type mapping_type, std::vector<uint32_t> mappings,
                      bool optimize_size = false)
        : base(new impl_type(mapping_type, mappings, optimize_size))
        {
        }

        mapping_table() = default;

        /**
         * \brief returns the mapping_type of the mapping table
         */
        mapping_type_type mapping_type() const
        {
            assert(this->is_valid());
            return this->data_->mapping_type();
        }

        /**
         * \brief returns a handle to an id_map
         */
        id_map_type id_map() const
        {
            assert(this->is_valid());
            return this->data_->id_map();
        }
    };
} // namespace definition
} // namespace otf2

#endif // INCLUDE_OTF2XX_DEFINITIONS_MAPPING_TABLE_HPP
