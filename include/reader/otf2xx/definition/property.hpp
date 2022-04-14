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

#ifndef INCLUDE_OTF2XX_DEFINITIONS_PROPERTY_HPP
#define INCLUDE_OTF2XX_DEFINITIONS_PROPERTY_HPP

#include <otf2xx/common.hpp>
#include <otf2xx/fwd.hpp>
#include <otf2xx/reference.hpp>

#include <otf2xx/definition/string.hpp>

#include <otf2xx/definition/detail/base.hpp>
#include <otf2xx/definition/detail/property_impl.hpp>

namespace otf2
{
namespace definition
{

    /**
     * \brief class for representing property definitions
     */
    template <class Definition>
    class property : public detail::base<detail::property_impl<Definition>>
    {
        using base = typename detail::base<detail::property_impl<Definition>>;
        using base::base;

        static_assert(otf2::traits::is_definition<Definition>::value,
                      "The Definition has to be a otf2::definition.");

    public:
        using impl_type = typename base::impl_type;
        using type_type = typename impl_type::type_type;
        using value_type = typename impl_type::value_type;

        property(const Definition& def, string name, type_type type, value_type value)
        : base(new impl_type(def, name, type, value))
        {
        }

        property(const Definition& def, string name, const otf2::attribute_value& value)
        : base(new impl_type(def, name, value))
        {
        }

        property() = default;

        /**
         * \brief returns the name of the property
         *
         * \returns a \ref string definiton containing the name
         *
         */
        const otf2::definition::string& name() const
        {
            assert(this->is_valid());
            return this->data_->name();
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
         * \brief returns the referenced definition record
         *
         */
        const Definition& def() const
        {
            assert(this->is_valid());
            return this->data_->def();
        }
    };
} // namespace definition
} // namespace otf2

#endif // INCLUDE_OTF2XX_DEFINITIONS_PROPERTY_HPP
