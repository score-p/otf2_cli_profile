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

#ifndef INCLUDE_OTF2XX_DEFINITIONS_GROUP_HPP
#define INCLUDE_OTF2XX_DEFINITIONS_GROUP_HPP

#include <otf2xx/common.hpp>
#include <otf2xx/definition/fwd.hpp>
#include <otf2xx/reference.hpp>

#include <otf2xx/definition/string.hpp>

#include <otf2xx/traits/definition.hpp>

#include <otf2xx/definition/detail/group_impl.hpp>
#include <otf2xx/definition/detail/referable_base.hpp>

namespace otf2
{
namespace definition
{
    namespace detail
    {
        class group_base
        {
        public:
            using tag_type = group_base;
        };
    } // namespace detail

    /**
     * \brief class template for representing groups
     * \see otf2::definition::locations_group
     * \see otf2::definition::regions_group
     * \see otf2::definition::metric_group
     * \see otf2::definition::comm_group
     * \see otf2::definition::comm_locations_group
     * \see otf2::definition::comm_self_group
     */
    template <class MemberType,
              otf2::common::group_type GroupType = otf2::common::group_type::unknown>
    class group : public detail::referable_base<group<MemberType, GroupType>,
                                                detail::group_impl<MemberType, GroupType>>
    {
        using base = detail::referable_base<group<MemberType, GroupType>,
                                            detail::group_impl<MemberType, GroupType>>;

        static_assert(otf2::traits::is_definition<MemberType>::value,
                      "The MemberType has to be a otf2::definition.");

        using base::base;

    public:
        using impl_type = typename base::impl_type;
        using reference_type = typename base::reference_type;

        typedef typename impl_type::group_type group_type;
        typedef typename impl_type::group_flag_type group_flag_type;
        typedef typename impl_type::paradigm_type paradigm_type;
        typedef typename impl_type::value_type value_type;

    public:
        group(reference_type ref, const otf2::definition::string& name, paradigm_type paradigm,
              group_flag_type group_flag)
        : base(ref, new impl_type(name, paradigm, group_flag))
        {
        }

        group() = default;

        /**
         * \brief returns the name of the group definion as a string definition
         *
         * \returns a string definiton containing the name
         *
         */
        const otf2::definition::string& name() const
        {
            assert(this->is_valid());
            return this->data_->name();
        }

        /**
         * \brief sets the name of the group definition
         *
         * \param a string definition containing the new name
         *
         */
        void name(const otf2::definition::string& name)
        {
            assert(this->is_valid());
            this->data_->name() = name;
        }

        /**
         * \brief returns the type of the group definion
         *
         * \see otf2::common::group_type
         *
         */
        group_type type() const
        {
            assert(this->is_valid());
            return this->data_->type();
        }

        /**
         * \brief returns the paradigm of the group definion
         *
         * \see otf2::common::paradigm_type
         *
         */
        paradigm_type paradigm() const
        {
            assert(this->is_valid());
            return this->data_->paradigm();
        }

        /**
         * \brief returns the group flag of the group definion
         *
         * \see otf2::common::group_flag_type
         *
         */
        group_flag_type group_flag() const
        {
            assert(this->is_valid());
            return this->data_->group_flag();
        }

        /**
         * \brief returns the member of the group definion
         *
         * \return std::vector containing reference numbers of defintions
         *
         */
        std::vector<std::uint64_t> members() const
        {
            assert(this->is_valid());
            return this->data_->members();
        }

        /**
         * \brief returns the number of members
         * \returns number of member as std::size_t
         */
        std::size_t size() const
        {
            assert(this->is_valid());
            return this->data_->size();
        }

        /**
         * \brief returns the i-th member in the group
         *
         * If my_group.members() returns {1,5,19}, then my_group[1] will return the definition with
         * reference number 5.
         *
         * \return the i-th definition in the group
         */
        value_type operator[](std::size_t i) const
        {
            assert(this->is_valid());
            return this->data_->operator[](i);
        }

        /**
         * \brief adds a definition to the group
         */
        void add_member(value_type member)
        {
            assert(this->is_valid());
            this->data_->add_member(member);
        }

        void remove_member(const value_type& member)
        {
            assert(this->is_valid());
            this->data_->remove_member(member);
        }
    };

} // namespace definition

} // namespace otf2

#endif // INCLUDE_OTF2XX_DEFINITIONS_GROUP_HPP
