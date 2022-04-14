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

#ifndef INCLUDE_OTF2XX_DEFINITIONS_DETAIL_LOCATION_HPP
#define INCLUDE_OTF2XX_DEFINITIONS_DETAIL_LOCATION_HPP

#include <otf2xx/common.hpp>
#include <otf2xx/fwd.hpp>

#include <otf2xx/definition/detail/ref_counted.hpp>

#include <otf2xx/definition/location_group.hpp>
#include <otf2xx/definition/string.hpp>

#include <memory>

namespace otf2
{
namespace definition
{
    namespace detail
    {

        class location_impl : public ref_counted
        {
        public:
            using tag_type = location;

            typedef otf2::common::location_type location_type;

            location_impl(const otf2::definition::string& name,
                          const otf2::definition::location_group& lg, location_type type,
                          std::uint64_t events = 0, std::int64_t retain_count = 0)
            : ref_counted(retain_count), name_(name), type_(type), lg_(lg), events_(events)
            {
            }

            const otf2::definition::string& name() const
            {
                return name_;
            }

            void name(const otf2::definition::string& new_name)
            {
                name_ = new_name;
            }

            const otf2::definition::location_group& location_group() const
            {
                return lg_;
            }

            location_type type() const
            {
                return type_;
            }

            std::uint64_t num_events() const
            {
                return events_;
            }

            void event_written()
            {
                ++events_;
            }

            friend class writer::local;

        private:
            otf2::definition::string name_;
            location_type type_;
            otf2::definition::location_group lg_;
            std::uint64_t events_;
        };
    } // namespace detail
} // namespace definition
} // namespace otf2

#endif // INCLUDE_OTF2XX_DEFINITIONS_DETAIL_LOCATION_HPP
