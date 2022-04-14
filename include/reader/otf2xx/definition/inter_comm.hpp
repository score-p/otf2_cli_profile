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

#ifndef INCLUDE_OTF2XX_DEFINITIONS_INTER_COMM_HPP
#define INCLUDE_OTF2XX_DEFINITIONS_INTER_COMM_HPP

#include <otf2xx/exception.hpp>

#include <otf2xx/fwd.hpp>
#include <otf2xx/reference.hpp>

#include <otf2xx/definition/group.hpp>
#include <otf2xx/definition/string.hpp>

#include <otf2xx/definition/detail/inter_comm_impl.hpp>
#include <otf2xx/definition/detail/referable_base.hpp>

#include <sstream>
#include <variant>

namespace otf2
{
namespace definition
{

    /**
     * \brief class for representing a inter-inter_comm definition
     */
    class inter_comm : public detail::referable_base<inter_comm, detail::inter_comm_impl>
    {
        using base = detail::referable_base<inter_comm, detail::inter_comm_impl>;
        using base::base;

    public:
        using comm_flag_type = impl_type::comm_flag_type;
        using group_type = impl_type::group_type;

        inter_comm(reference_type ref, const otf2::definition::string& name,
                   const group_type& groupA, const group_type& groupB,
                   const otf2::definition::inter_comm& common_communicator, comm_flag_type flags)
        : base(ref, new impl_type(name, groupA, groupB, common_communicator.get(),
                                  common_communicator.ref(), flags))
        {
        }

        inter_comm(reference_type ref, const otf2::definition::string& name,
                   const group_type& groupA, const group_type& groupB,
                   const otf2::definition::comm& common_communicator, comm_flag_type flags)
        : base(ref, new impl_type(name, groupA, groupB, common_communicator.get(),
                                  common_communicator.ref(), flags))
        {
        }

        inter_comm(reference_type ref, const otf2::definition::string& name,
                   const group_type& groupA, const group_type& groupB,
                   const std::variant<otf2::definition::comm, otf2::definition::inter_comm>&
                       common_communicator,
                   comm_flag_type flags)
        : base(ref,
               std::visit(
                   [&](auto&& cc)
                   { return new impl_type(name, groupA, groupB, cc.get(), cc.ref().get(), flags); },
                   common_communicator))
        {
        }

        inter_comm(reference_type ref, const otf2::definition::string& name,
                   const group_type& groupA, const group_type& groupB, comm_flag_type flags)
        : base(ref, new impl_type(name, groupA, groupB, flags))
        {
        }

        inter_comm() = default;

        /**
         * \brief returns the name of the inter_comm definition as a string definition
         *
         * \returns a string definition containing the name
         *
         */
        const otf2::definition::string& name() const
        {
            assert(this->is_valid());
            return data_->name();
        }

        /**
         * \brief sets the name of the inter_comm definition
         *
         * \param a string definition containing the new name
         *
         */
        void name(const otf2::definition::string& name)
        {
            assert(this->is_valid());
            data_->name() = name;
        }

        /**
         * \brief returns the first comm group of this inter_comm
         *
         * \returns a std::variant of a comm group or comm self group
         */
        const group_type& groupA() const
        {
            assert(this->is_valid());
            return data_->groupA();
        }

        /**
         * \brief returns the second comm group of this inter_comm
         *
         * \returns a std::variant of a comm group or comm self group
         */
        const group_type& groupB() const
        {
            assert(this->is_valid());
            return data_->groupB();
        }

        /**
         * \brief returns the common peer MPI communicator used to create this
         * inter-communicator
         * \return returns a std::variant holding either nothing, a otf2::definition::inter_comm
         * definition, or a otf2::definition::comm definition
         */
        std::variant<otf2::definition::comm, otf2::definition::inter_comm>
        common_communicator() const
        {
            assert(this->is_valid());
            auto t = data_->common_communicator();
            if (std::get<0>(t) != nullptr)
            {
                return otf2::definition::inter_comm{ std::get<2>(t), std::get<0>(t) };
            }
            else
            {
                return otf2::definition::comm{ std::get<2>(t), std::get<1>(t) };
            }
        }

        /**
         * \brief returns the flags of the inter_comm definition
         *
         * \returns returns the flags of the inter_comm definition
         *
         */
        comm_flag_type flags() const
        {
            assert(this->is_valid());
            return data_->flags();
        }
    };

} // namespace definition

} // namespace otf2

#endif // INCLUDE_OTF2XX_DEFINITIONS_INTER_COMM_HPP
