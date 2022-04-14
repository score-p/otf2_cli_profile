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

#ifndef INCLUDE_OTF2XX_EVENT_CALLING_CONTEXT_SAMPLE_HPP
#define INCLUDE_OTF2XX_EVENT_CALLING_CONTEXT_SAMPLE_HPP

#include <otf2xx/definition/calling_context.hpp>
#include <otf2xx/definition/interrupt_generator.hpp>
#include <otf2xx/event/base.hpp>

#include <otf2xx/definition/detail/weak_ref.hpp>
#include <otf2xx/writer/fwd.hpp>

#include <otf2xx/chrono/chrono.hpp>

namespace otf2
{
namespace event
{

    class calling_context_sample : public base<calling_context_sample>
    {
    public:
        // construct with values
        calling_context_sample(otf2::chrono::time_point timestamp,
                               const otf2::definition::calling_context& calling_context,
                               std::uint32_t unwind_distance,
                               const otf2::definition::interrupt_generator& interrupt_generator)
        : base<calling_context_sample>(timestamp), calling_context_(calling_context),
          unwind_distance_(unwind_distance), interrupt_generator_(interrupt_generator)
        {
        }

        calling_context_sample(OTF2_AttributeList* al, otf2::chrono::time_point timestamp,
                               const otf2::definition::calling_context& calling_context,
                               std::uint32_t unwind_distance,
                               const otf2::definition::interrupt_generator& interrupt_generator)
        : base<calling_context_sample>(al, timestamp), calling_context_(calling_context),
          unwind_distance_(unwind_distance), interrupt_generator_(interrupt_generator)
        {
        }

        // copy constructor with new timestamp
        calling_context_sample(const otf2::event::calling_context_sample& other,
                               otf2::chrono::time_point timestamp)
        : base<calling_context_sample>(other, timestamp), calling_context_(other.calling_context()),
          unwind_distance_(other.unwind_distance()),
          interrupt_generator_(other.interrupt_generator())
        {
        }

        otf2::definition::calling_context calling_context() const
        {
            return calling_context_;
        }

        std::uint32_t unwind_distance() const
        {
            return unwind_distance_;
        }

        otf2::definition::interrupt_generator interrupt_generator() const
        {
            return interrupt_generator_;
        }

        friend class otf2::writer::local;

    private:
        otf2::definition::detail::weak_ref<otf2::definition::calling_context> calling_context_;
        std::uint32_t unwind_distance_;
        otf2::definition::detail::weak_ref<otf2::definition::interrupt_generator>
            interrupt_generator_;
    };
} // namespace event
} // namespace otf2

#endif // INCLUDE_OTF2XX_EVENT_CALLING_CONTEXT_SAMPLE_HPP
