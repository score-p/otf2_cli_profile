/*
 * This file is part of otf2xx (https://github.com/tud-zih-energy/otf2xx)
 * otf2xx - A wrapper for the Open Trace Format 2 library
 *
 * Copyright (c) 2013-2019, Technische Universität Dresden, Germany
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

#ifndef INCLUDE_OTF2XX_DEFINITIONS_CALLPATH_PARAMETER_HPP
#define INCLUDE_OTF2XX_DEFINITIONS_CALLPATH_PARAMETER_HPP

#include <otf2xx/definition/call_path.hpp>
#include <otf2xx/definition/parameter.hpp>

#include <otf2xx/definition/detail/base.hpp>
#include <otf2xx/definition/detail/call_path_parameter_impl.hpp>

namespace otf2
{
namespace definition
{

    /**
     * \brief class for representing call path parameter definitions
     */
    class call_path_parameter : public detail::base<detail::call_path_parameter_impl>
    {
        using base = typename detail::base<detail::call_path_parameter_impl>;
        using base::base;

    public:
        using impl_type = typename base::impl_type;
        using type_type = typename impl_type::type_type;
        using value_type = typename impl_type::value_type;

        call_path_parameter(const call_path& def, const parameter& param, type_type type,
                            value_type value)
        : base(new impl_type(def, param, type, value))
        {
        }

        call_path_parameter(const call_path& def, const parameter& param,
                            const otf2::attribute_value& value)
        : base(new impl_type(def, param, value))
        {
        }

        call_path_parameter() = default;

        /**
         * \brief returns the parameter of the property
         *
         */
        const otf2::definition::call_path& call_path() const
        {
            assert(this->is_valid());
            return this->data_->call_path();
        }

        /**
         * \brief returns the type of the property
         *
         */
        type_type type() const
        {
            assert(this->is_valid());
            return this->data_->type();
        }

        /**
         * \brief returns the value of the property
         *
         */
        value_type value() const
        {
            assert(this->is_valid());
            return this->data_->value();
        }

        /**
         * \brief returns the referenced call_path definition
         *
         */
        const otf2::definition::parameter& parameter() const
        {
            assert(this->is_valid());
            return this->data_->parameter();
        }
    };
} // namespace definition
} // namespace otf2

#endif // INCLUDE_OTF2XX_DEFINITIONS_CALLPATH_PARAMETER_HPP
