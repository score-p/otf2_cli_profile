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

#ifndef INCLUDE_OTF2XX_DEFINITIONS_DETAIL_REGION_HPP
#define INCLUDE_OTF2XX_DEFINITIONS_DETAIL_REGION_HPP

#include <otf2xx/common.hpp>
#include <otf2xx/fwd.hpp>

#include <otf2xx/definition/detail/ref_counted.hpp>

#include <otf2xx/definition/string.hpp>

namespace otf2
{
namespace definition
{
    namespace detail
    {

        class region_impl : public ref_counted
        {
        public:
            using tag_type = region;

            typedef otf2::common::role_type role_type;
            typedef otf2::common::paradigm_type paradigm_type;
            typedef otf2::common::flags_type flags_type;

            region_impl(const otf2::definition::string& name,
                        const otf2::definition::string& canonical_name,
                        const otf2::definition::string& description, role_type role,
                        paradigm_type paradigm, flags_type flags,
                        const otf2::definition::string& source_file, uint32_t begin_line,
                        uint32_t end_line, std::int64_t retain_count = 0)
            : ref_counted(retain_count), name_(name), canonical_name_(canonical_name),
              description_(description), role_(role), paradigm_(paradigm), flags_(flags),
              source_file_(source_file), begin_line_(begin_line), end_line_(end_line)
            {
            }

            const otf2::definition::string& name() const
            {
                return name_;
            }

            void name(const otf2::definition::string& str)
            {
                name_ = str;
            }

            const otf2::definition::string& canonical_name() const
            {
                return canonical_name_;
            }

            void canonical_name(const otf2::definition::string& str)
            {
                canonical_name_ = str;
            }

            const otf2::definition::string& description() const
            {
                return description_;
            }

            void description(const otf2::definition::string& str)
            {
                description_ = str;
            }

            role_type role() const
            {
                return role_;
            }

            paradigm_type paradigm() const
            {
                return paradigm_;
            }

            flags_type flags() const
            {
                return flags_;
            }

            const otf2::definition::string& source_file() const
            {
                return source_file_;
            }

            void source_file(const otf2::definition::string& str)
            {
                source_file_ = str;
            }

            uint32_t begin_line() const
            {
                return begin_line_;
            }

            uint32_t end_line() const
            {
                return end_line_;
            }

        private:
            otf2::definition::string name_;
            otf2::definition::string canonical_name_;
            otf2::definition::string description_;
            role_type role_;
            paradigm_type paradigm_;
            flags_type flags_;
            otf2::definition::string source_file_;
            uint32_t begin_line_;
            uint32_t end_line_;
        };
    } // namespace detail
} // namespace definition
} // namespace otf2

#endif // INCLUDE_OTF2XX_DEFINITIONS_DETAIL_REGION_HPP
