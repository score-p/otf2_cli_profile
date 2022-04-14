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

#ifndef INCLUDE_OTF2XX_DEFINITIONS_DETAIL_MAPPING_TABLE_HPP
#define INCLUDE_OTF2XX_DEFINITIONS_DETAIL_MAPPING_TABLE_HPP

#include <otf2xx/common.hpp>
#include <otf2xx/exception.hpp>
#include <otf2xx/fwd.hpp>

#include <otf2xx/definition/detail/ref_counted.hpp>

#include <otf2/OTF2_IdMap.h>

#include <memory>

namespace otf2
{
namespace definition
{
    namespace detail
    {

        class mapping_table_impl : public ref_counted
        {
            using id_map_repr_type = OTF2_IdMap;

        public:
            using mapping_type_type = otf2::common::mapping_type_type;
            using id_map_type = const OTF2_IdMap*;

            mapping_table_impl(mapping_type_type mapping_type,
                               OTF2_IdMapMode mode = OTF2_ID_MAP_SPARSE, std::size_t size = 64,
                               std::int64_t retain_count = 0)
            : ref_counted(retain_count), mapping_type_(mapping_type),
              id_map_(OTF2_IdMap_Create(mode, size))
            {
                if (!id_map_)
                {
                    make_exception("Creation of IdMap failed.");
                }
            }

            mapping_table_impl(mapping_type_type mapping_type, std::vector<uint64_t> mappings,
                               bool optimize_size = false, std::int64_t retain_count = 0)
            : ref_counted(retain_count), mapping_type_(mapping_type),
              id_map_(
                  OTF2_IdMap_CreateFromUint64Array(mappings.size(), mappings.data(), optimize_size))
            {
                if (!id_map_)
                {
                    make_exception("Creation of IdMap failed.");
                }
            }

            mapping_table_impl(mapping_type_type mapping_type, std::vector<uint32_t> mappings,
                               bool optimize_size = false, std::int64_t retain_count = 0)
            : ref_counted(retain_count), mapping_type_(mapping_type),
              id_map_(
                  OTF2_IdMap_CreateFromUint32Array(mappings.size(), mappings.data(), optimize_size))
            {
                if (!id_map_)
                {
                    make_exception("Creation of IdMap failed.");
                }
            }

            static mapping_table_impl* undefined()
            {
                static mapping_table_impl undef(mapping_type_type::max, OTF2_ID_MAP_SPARSE, 64, 1);
                return &undef;
            }

            mapping_type_type mapping_type() const
            {
                return mapping_type_;
            }

            id_map_type id_map() const
            {
                return id_map_.get();
            }

        private:
            struct OTF2_IdMap_deleter
            {
                void operator()(id_map_repr_type* ptr) const
                {
                    OTF2_IdMap_Free(ptr);
                }
            };

            mapping_type_type mapping_type_;
            std::unique_ptr<id_map_repr_type, OTF2_IdMap_deleter> id_map_;
        };
    } // namespace detail
} // namespace definition
} // namespace otf2

#endif // INCLUDE_OTF2XX_DEFINITIONS_DETAIL_MAPPING_TABLE_HPP
