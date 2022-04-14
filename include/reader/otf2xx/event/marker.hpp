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

#ifndef INCLUDE_OTF2XX_EVENT_MARKER_HPP
#define INCLUDE_OTF2XX_EVENT_MARKER_HPP

#include <otf2xx/definition/fwd.hpp>
#include <otf2xx/definition/marker.hpp>
#include <otf2xx/event/base.hpp>

#include <otf2xx/chrono/chrono.hpp>

namespace otf2
{
namespace event
{

    class marker : public base<event::marker>
    {
    public:
        using scope_type = otf2::common::marker_scope_type;

        // construct with values
        marker(otf2::chrono::time_point timestamp, otf2::chrono::duration duration,
               const otf2::definition::marker& def_marker, scope_type scope,
               std::uint64_t scope_ref, std::string text)
        : base<event::marker>(timestamp), duration_(duration), def_marker_(def_marker),
          scope_(scope), scope_ref_(scope_ref), text_(text)
        {
        }

        marker(OTF2_AttributeList* al, otf2::chrono::time_point timestamp,
               otf2::chrono::duration duration, const otf2::definition::marker& def_marker,
               scope_type scope, std::uint64_t scope_ref, std::string text)
        : base<event::marker>(al, timestamp), duration_(duration), def_marker_(def_marker),
          scope_(scope), scope_ref_(scope_ref), text_(text)
        {
        }

        // copy constructor with new timestamp
        marker(const otf2::event::marker& other, otf2::chrono::time_point timestamp)
        : base<event::marker>(other, timestamp), duration_(other.duration()),
          def_marker_(other.def_marker()), scope_(other.scope()), scope_ref_(other.scope_ref()),
          text_(other.text())
        {
        }

    public:
        otf2::chrono::duration duration() const
        {
            return duration_;
        }

        otf2::definition::marker def_marker() const
        {
            return def_marker_;
        }

        scope_type scope() const
        {
            return scope_;
        }

        std::uint64_t scope_ref() const
        {
            return scope_ref_;
        }

        std::string text() const
        {
            return text_;
        }

    private:
        otf2::chrono::duration duration_;
        otf2::definition::detail::weak_ref<otf2::definition::marker> def_marker_;
        scope_type scope_;
        std::uint64_t scope_ref_;
        std::string text_;
    };
} // namespace event
} // namespace otf2

#endif // INCLUDE_OTF2XX_EVENT_MARKER_HPP
