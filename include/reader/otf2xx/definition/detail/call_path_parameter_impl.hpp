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

#ifndef INCLUDE_OTF2XX_DEFINITIONS_DETAIL_CALL_PATH_PARAMETER_HPP
#define INCLUDE_OTF2XX_DEFINITIONS_DETAIL_CALL_PATH_PARAMETER_HPP

#include <otf2xx/attribute_value.hpp>
#include <otf2xx/common.hpp>
#include <otf2xx/fwd.hpp>

#include <otf2xx/definition/detail/ref_counted.hpp>

#include <otf2xx/definition/call_path.hpp>
#include <otf2xx/definition/parameter.hpp>

#include <memory>

namespace otf2
{
namespace definition
{
    namespace detail
    {
        class call_path_parameter_impl : public ref_counted
        {
        public:
            using type_type = otf2::common::type;
            using value_type = OTF2_AttributeValue;

            call_path_parameter_impl(const otf2::definition::call_path& def,
                                     const otf2::definition::parameter& param, type_type type,
                                     value_type value, std::int64_t retain_count = 0)
            : ref_counted(retain_count), path_(def), param_(param), value_(type, value)
            {
            }

            call_path_parameter_impl(const otf2::definition::call_path& def,
                                     const otf2::definition::parameter& param,
                                     const otf2::attribute_value& value,
                                     std::int64_t retain_count = 0)
            : ref_counted(retain_count), path_(def), param_(param), value_(value)
            {
            }

            const otf2::definition::parameter& parameter() const
            {
                return param_;
            }

            type_type type() const
            {
                return value_.type();
            }

            value_type value() const
            {
                return value_.value();
            }

            const otf2::definition::call_path& call_path() const
            {
                return path_;
            }

        private:
            otf2::definition::call_path path_;
            otf2::definition::parameter param_;
            otf2::attribute_value value_;
        };
    } // namespace detail
} // namespace definition
} // namespace otf2

#endif // INCLUDE_OTF2XX_DEFINITIONS_DETAIL_CALL_PATH_PARAMETER_HPP
