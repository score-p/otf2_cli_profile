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

#ifndef INCLUDE_OTF2XX_DEFINITIONS_SOURCE_CODE_LOCATION_HPP
#define INCLUDE_OTF2XX_DEFINITIONS_SOURCE_CODE_LOCATION_HPP

#include <otf2xx/common.hpp>
#include <otf2xx/fwd.hpp>
#include <otf2xx/reference.hpp>

#include <otf2xx/definition/string.hpp>

#include <otf2xx/definition/detail/referable_base.hpp>
#include <otf2xx/definition/detail/source_code_location_impl.hpp>

namespace otf2
{
namespace definition
{

    /**
     * \brief class for representing source_code_location definitions
     */
    class source_code_location
    : public detail::referable_base<source_code_location, detail::source_code_location_impl>
    {
        using base =
            detail::referable_base<source_code_location, detail::source_code_location_impl>;
        using base::base;

    public:
        source_code_location(reference_type ref, string file, std::uint32_t line_number)
        : base(ref, new impl_type(file, line_number))
        {
        }

        source_code_location() = default;

        /**
         * \brief returns the file of the source_code_location definion as a string definition
         *
         * \returns a \ref string definiton containing the file
         */
        const otf2::definition::string& file() const
        {
            assert(this->is_valid());
            return data_->file();
        }

        /**
         * \brief returns the line number of the source_code_location definion as an uint32
         *
         * \returns a \ref std::uint32_t containing the line number
         */
        std::uint32_t line_number() const
        {
            assert(this->is_valid());

            return data_->line_number();
        }
    };
} // namespace definition
} // namespace otf2

#endif // INCLUDE_OTF2XX_DEFINITIONS_SOURCE_CODE_LOCATION_HPP
