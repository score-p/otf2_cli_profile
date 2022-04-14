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

#ifndef INCLUDE_OTF2XX_DEFINITIONS_LOCATION_GROUP_HPP
#define INCLUDE_OTF2XX_DEFINITIONS_LOCATION_GROUP_HPP

#include <otf2xx/common.hpp>
#include <otf2xx/fwd.hpp>
#include <otf2xx/reference.hpp>

#include <otf2xx/definition/location_group.hpp>
#include <otf2xx/definition/string.hpp>
#include <otf2xx/definition/system_tree_node.hpp>

#include <otf2xx/definition/detail/location_group_impl.hpp>
#include <otf2xx/definition/detail/referable_base.hpp>

namespace otf2
{
namespace definition
{

    /**
     * \brief class for representing a location group definition
     * \note Don't mix up with locationS_group, which would be a group definition containing some
     * location definitions.
     *
     */
    class location_group
    : public detail::referable_base<location_group, detail::location_group_impl>
    {
        using base = detail::referable_base<location_group, detail::location_group_impl>;
        using base::base;

    public:
        typedef otf2::common::location_group_type location_group_type;

        location_group(reference_type ref, const otf2::definition::string& name,
                       location_group_type type, const otf2::definition::system_tree_node& stm,
                       const otf2::definition::location_group& creating_location_group)
        : base(ref, new impl_type(name, type, stm, creating_location_group.get(),
                                  creating_location_group.ref()))
        {
        }

        location_group(reference_type ref, const otf2::definition::string& name,
                       location_group_type type, const otf2::definition::system_tree_node& stm)
        : base(ref, new impl_type(name, type, stm))
        {
        }

        location_group() = default;

        /**
         * \brief returns the name of the location group definion as a string definition
         *
         * \returns a string definiton containing the name
         *
         */
        const otf2::definition::string& name() const
        {
            assert(this->is_valid());
            return data_->name();
        }

        void name(const otf2::definition::string& new_name)
        {
            assert(this->is_valid());
            data_->name() = new_name;
        }

        /**
         * \brief returns the type of the location group defintion
         * \see otf2::common::location_group_type
         */
        location_group_type type() const
        {
            assert(this->is_valid());
            return data_->type();
        }

        /**
         * \brief returns the parentof the location group definition
         * \returns otf2::definition::system_tree_node
         */
        const otf2::definition::system_tree_node& parent() const
        {
            assert(this->is_valid());
            return data_->parent();
        }

        /**
         * \brief returns the creating location group of this location group
         * definition \returns otf2::definition::location_group
         */
        otf2::definition::location_group creating_location_group() const
        {
            assert(this->is_valid());
            auto p = data_->creating_location_group();
            if (p.first != nullptr)
            {
                return otf2::definition::location_group{ p.second, p.first };
            }
            else
            {
                return {};
            }
        }
    };
} // namespace definition
} // namespace otf2

#endif // INCLUDE_OTF2XX_DEFINITIONS_LOCATION_GROUP_HPP
