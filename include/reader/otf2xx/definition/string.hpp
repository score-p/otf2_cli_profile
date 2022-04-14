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

#ifndef INCLUDE_OTF2XX_DEFINITIONS_STRING_HPP
#define INCLUDE_OTF2XX_DEFINITIONS_STRING_HPP

#include <otf2xx/reference.hpp>

#include <string>

#include <iostream>

#include <otf2xx/definition/detail/referable_base.hpp>
#include <otf2xx/definition/detail/string_impl.hpp>

namespace otf2
{
namespace definition
{

    /**
     * @brief The string definiton class
     *
     * This class represents an OTF2 string definition.
     */
    class string : public detail::referable_base<string, detail::string_impl>
    {
        using base = detail::referable_base<string, detail::string_impl>;
        using base::base;

    public:
        /**
         * @brief value constructor
         *
         * Takes the reference number and value for this definition.
         *
         * @param ref reference number
         * @param str string value
         */
        string(reference_type ref, const std::string& str) : base(ref, new impl_type(str))
        {
        }

        string() = default;

        /**
         * @brief returns string value as const ref
         *
         * @return the value
         */
        const std::string& str() const
        {
            assert(this->is_valid());
            return data_->str();
        }

        /**
         * @brief operator std::string
         *
         * A string definition is explicit convertible to a std::string
         */
        explicit operator std::string() const
        {
            assert(this->is_valid());
            return data_->str();
        }
    };

    /**
     * @brief operator<< for easily printing out string definitions
     */
    inline std::ostream& operator<<(std::ostream& s, const otf2::definition::string& str)
    {
        return s << str.str();
    }

} // namespace definition

} // namespace otf2

#endif // INCLUDE_OTF2XX_DEFINITIONS_STRING_HPP
