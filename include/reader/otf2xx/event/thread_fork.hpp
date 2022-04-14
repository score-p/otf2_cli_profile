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

#ifndef INCLUDE_OTF2XX_EVENT_THREAD_FORK_HPP
#define INCLUDE_OTF2XX_EVENT_THREAD_FORK_HPP

#include <otf2xx/definition/fwd.hpp>

#include <otf2xx/common.hpp>

#include <otf2xx/chrono/chrono.hpp>

#include <otf2xx/event/base.hpp>

namespace otf2
{
namespace event
{

    class thread_fork : public base<thread_fork>
    {
    public:
        thread_fork(otf2::chrono::time_point timestamp, otf2::common::paradigm_type paradigm,
                    uint32_t num_threads)
        : base<thread_fork>(timestamp), paradigm_(paradigm), num_threads_(num_threads)
        {
        }

        thread_fork(OTF2_AttributeList* al, otf2::chrono::time_point timestamp,
                    otf2::common::paradigm_type paradigm, uint32_t num_threads)
        : base<thread_fork>(al, timestamp), paradigm_(paradigm), num_threads_(num_threads)
        {
        }

        thread_fork(const otf2::event::thread_fork& other, otf2::chrono::time_point timestamp)
        : base<thread_fork>(other, timestamp), paradigm_(other.paradigm()),
          num_threads_(other.num_threads())
        {
        }

        otf2::common::paradigm_type paradigm() const
        {
            return paradigm_;
        }

        uint32_t num_threads() const
        {
            return num_threads_;
        }

    private:
        otf2::common::paradigm_type paradigm_;
        uint32_t num_threads_;
    };
} // namespace event
} // namespace otf2

#endif // INCLUDE_OTF2XX_EVENT_THREAD_FORK_HPP
