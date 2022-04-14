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

#ifndef INCLUDE_OTF2XX_EVENT_RMA_ACQUIRE_LOCK_HPP
#define INCLUDE_OTF2XX_EVENT_RMA_ACQUIRE_LOCK_HPP

#include <otf2xx/common.hpp>
#include <otf2xx/event/base.hpp>

#include <otf2xx/chrono/chrono.hpp>

#include <otf2xx/definition/detail/weak_ref.hpp>

namespace otf2
{
namespace event
{
    class rma_acquire_lock : public base<rma_acquire_lock>
    {
    public:
        using lock_type_type = otf2::common::lock_type;

        rma_acquire_lock(otf2::chrono::time_point timestamp, const otf2::definition::rma_win& win,
                         std::uint32_t remote, std::uint64_t lock, lock_type_type lock_type)
        : base<rma_acquire_lock>(timestamp), win_(win), remote_(remote), lock_(lock),
          lock_type_(lock_type)
        {
        }

        rma_acquire_lock(OTF2_AttributeList* al, otf2::chrono::time_point timestamp,
                         const otf2::definition::rma_win& win, std::uint32_t remote,
                         std::uint64_t lock, lock_type_type lock_type)
        : base<rma_acquire_lock>(al, timestamp), win_(win), remote_(remote), lock_(lock),
          lock_type_(lock_type)
        {
        }

        // copy constructor with new timestamp
        rma_acquire_lock(const otf2::event::rma_acquire_lock& other,
                         otf2::chrono::time_point timestamp)
        : base<rma_acquire_lock>(timestamp), win_(other.win()), remote_(other.remote()),
          lock_(other.lock()), lock_type_(other.lock_type())
        {
        }

        otf2::definition::rma_win win() const
        {
            return win_;
        }

        std::uint32_t remote() const
        {
            return remote_;
        }

        std::uint64_t lock() const
        {
            return lock_;
        }

        lock_type_type lock_type() const
        {
            return lock_type_;
        }

    private:
        otf2::definition::detail::weak_ref<otf2::definition::rma_win> win_;
        std::uint32_t remote_;
        std::uint64_t lock_;
        lock_type_type lock_type_;
    };
} // namespace event
} // namespace otf2

#endif // INCLUDE_OTF2XX_EVENT_RMA_ACQUIRE_LOCK_HPP
