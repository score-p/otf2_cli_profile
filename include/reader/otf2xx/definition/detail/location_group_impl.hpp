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

#ifndef INCLUDE_OTF2XX_DEFINITIONS_DETAIL_LOCATION_GROUP_HPP
#define INCLUDE_OTF2XX_DEFINITIONS_DETAIL_LOCATION_GROUP_HPP

#include <otf2xx/common.hpp>
#include <otf2xx/fwd.hpp>

#include <otf2xx/definition/detail/ref_counted.hpp>

#include <otf2xx/definition/location_group.hpp>
#include <otf2xx/definition/string.hpp>
#include <otf2xx/definition/system_tree_node.hpp>

namespace otf2
{
namespace definition
{
    namespace detail
    {

        class location_group_impl : public ref_counted
        {
        public:
            using tag_type = location_group;

            typedef otf2::common::location_group_type location_group_type;

        private:
            using reference_type = otf2::reference_impl<location_group, tag_type>;

        public:
            location_group_impl(const otf2::definition::string& name, location_group_type type,
                                const otf2::definition::system_tree_node& stm,
                                location_group_impl* creating_location_group, reference_type cref,
                                std::int64_t retain_count = 0)
            : ref_counted(retain_count), name_(name), type_(type), stm_(stm),
              creating_location_group_(creating_location_group), cref_(cref)
            {
            }

            location_group_impl(const otf2::definition::string& name, location_group_type type,
                                const otf2::definition::system_tree_node& stm,
                                std::int64_t retain_count = 0)
            : ref_counted(retain_count), name_(name), type_(type), stm_(stm),
              creating_location_group_(nullptr), cref_(reference_type::undefined())
            {
            }

            const otf2::definition::string& name() const
            {
                return name_;
            }

            otf2::definition::string& name()
            {
                return name_;
            }

            location_group_type type() const
            {
                return type_;
            }

            const otf2::definition::system_tree_node& parent() const
            {
                return stm_;
            }

            auto creating_location_group() const
            {
                return std::make_pair(creating_location_group_.get(), cref_);
            }

        private:
            otf2::definition::string name_;
            location_group_type type_;
            otf2::definition::system_tree_node stm_;
            otf2::intrusive_ptr<location_group_impl> creating_location_group_;
            reference_type cref_;
        };
    } // namespace detail
} // namespace definition
} // namespace otf2

#endif // INCLUDE_OTF2XX_DEFINITIONS_DETAIL_LOCATION_GROUP_HPP
