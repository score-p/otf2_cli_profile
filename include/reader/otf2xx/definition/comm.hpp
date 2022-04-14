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

#ifndef INCLUDE_OTF2XX_DEFINITIONS_COMM_HPP
#define INCLUDE_OTF2XX_DEFINITIONS_COMM_HPP

#include <otf2xx/exception.hpp>

#include <otf2xx/fwd.hpp>
#include <otf2xx/reference.hpp>

#include <otf2xx/definition/group.hpp>
#include <otf2xx/definition/string.hpp>

#include <otf2xx/definition/detail/comm_impl.hpp>
#include <otf2xx/definition/detail/referable_base.hpp>

#include <optional>
#include <sstream>

namespace otf2
{
namespace definition
{

    namespace detail
    {
        class comm_base
        {
        public:
            using tag_type = comm_base;
        };
    } // namespace detail

    /**
     * \brief class for representing a comm definition
     */
    class comm : public detail::referable_base<comm, detail::comm_impl>
    {
        using base = detail::referable_base<comm, detail::comm_impl>;
        using base::base;

    public:
        using comm_flag_type = impl_type::comm_flag_type;
        using group_type = impl_type::group_type;

        comm(reference_type ref, const otf2::definition::string& name, const group_type& group,
             const otf2::definition::comm& parent, comm_flag_type flags)
        : base(ref, new impl_type(name, group, parent.get(), parent.ref(), flags))
        {
        }

        comm(reference_type ref, const otf2::definition::string& name, const group_type& group,
             comm_flag_type flags)
        : base(ref, new impl_type(name, group, flags))
        {
        }

        comm() = default;

        /**
         * \brief returns the name of the comm definition as a string definition
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
         * \brief sets the name of the comm definition
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
         * \brief returns the comm group of this comm
         *
         * \returns a std::variant of a comm group or a comm_self group
         */
        const group_type& group() const
        {
            assert(this->is_valid());
            return data_->group();
        }

        /**
         * \brief returns the parent of this comm
         * \return returns a otf::definition::comm, which might not be valid, if the comm hasn't got
         * a parent!
         */
        otf2::definition::comm parent() const
        {
            assert(this->is_valid());
            auto p = data_->parent();
            if (p.first != nullptr)
            {
                return otf2::definition::comm{ p.second, p.first };
            }
            else
            {
                return {};
            }
        }

        /**
         * \brief returns the flags of the comm definition
         *
         * \returns returns the flags of the comm definition
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

#endif // INCLUDE_OTF2XX_DEFINITIONS_COMM_HPP
