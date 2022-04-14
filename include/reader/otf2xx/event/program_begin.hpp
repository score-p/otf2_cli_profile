/*
 * This file is part of otf2xx (https://github.com/tud-zih-energy/otf2xx)
 * otf2xx - A wrapper for the Open Trace Format 2 library
 *
 * Copyright (c) 2013-2018, Technische Universität Dresden, Germany
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

#ifndef INCLUDE_OTF2XX_EVENT_PROGRAM_BEGIN_HPP
#define INCLUDE_OTF2XX_EVENT_PROGRAM_BEGIN_HPP

#include <otf2xx/definition/string.hpp>
#include <otf2xx/event/base.hpp>

#include <otf2xx/definition/detail/weak_ref.hpp>
#include <otf2xx/writer/fwd.hpp>

#include <otf2xx/chrono/chrono.hpp>

#include <vector>

namespace otf2
{
namespace event
{

    class program_begin : public base<program_begin>
    {
    public:
        // construct with values
        program_begin(
            otf2::chrono::time_point timestamp, const otf2::definition::string& name,
            const std::vector<otf2::definition::detail::weak_ref<otf2::definition::string>>&
                arguments)
        : base<program_begin>(timestamp), name_(name), arguments_(arguments)
        {
        }

        program_begin(
            OTF2_AttributeList* al, otf2::chrono::time_point timestamp,
            const otf2::definition::string& name,
            const std::vector<otf2::definition::detail::weak_ref<otf2::definition::string>>&
                arguments)
        : base<program_begin>(al, timestamp), name_(name), arguments_(arguments)
        {
        }

        // copy constructor with new timestamp
        program_begin(const otf2::event::program_begin& other, otf2::chrono::time_point timestamp)
        : base<program_begin>(other, timestamp), name_(other.name()), arguments_(other.arguments())
        {
        }

        otf2::definition::string name() const
        {
            return name_;
        }

        const std::vector<otf2::definition::detail::weak_ref<otf2::definition::string>>&
        arguments() const
        {
            return arguments_;
        }

        friend class otf2::writer::local;

    private:
        otf2::definition::detail::weak_ref<otf2::definition::string> name_;
        std::vector<otf2::definition::detail::weak_ref<otf2::definition::string>> arguments_;
    };
} // namespace event
} // namespace otf2

#endif // INCLUDE_OTF2XX_EVENT_PROGRAM_BEGIN_HPP
