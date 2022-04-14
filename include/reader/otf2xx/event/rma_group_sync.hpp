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

#ifndef INCLUDE_OTF2XX_EVENT_RMA_GROUP_SYNC_HPP
#define INCLUDE_OTF2XX_EVENT_RMA_GROUP_SYNC_HPP

#include <otf2xx/common.hpp>
#include <otf2xx/event/base.hpp>

#include <otf2xx/chrono/chrono.hpp>

#include <otf2xx/definition/detail/weak_ref.hpp>

namespace otf2
{
namespace event
{
    class rma_group_sync : public base<rma_group_sync>
    {
    public:
        using sync_level_type = otf2::common::rma_sync_level_type;

        rma_group_sync(otf2::chrono::time_point timestamp, sync_level_type level,
                       const otf2::definition::rma_win& win, otf2::definition::comm_group& group)
        : base<rma_group_sync>(timestamp), level_(level), win_(win), group_(group)
        {
        }

        rma_group_sync(OTF2_AttributeList* al, otf2::chrono::time_point timestamp,
                       sync_level_type level, const otf2::definition::rma_win& win,
                       const otf2::definition::comm_group& group)
        : base<rma_group_sync>(al, timestamp), level_(level), win_(win), group_(group)
        {
        }

        // copy constructor with new timestamp
        rma_group_sync(const otf2::event::rma_group_sync& other, otf2::chrono::time_point timestamp)
        : base<rma_group_sync>(timestamp), level_(other.level()), win_(other.win()),
          group_(other.group())
        {
        }

        sync_level_type level() const
        {
            return level_;
        }

        otf2::definition::rma_win win() const
        {
            return win_;
        }

        otf2::definition::comm_group group() const
        {
            return group_;
        }

    private:
        sync_level_type level_;
        otf2::definition::detail::weak_ref<otf2::definition::rma_win> win_;
        otf2::definition::detail::weak_ref<otf2::definition::comm_group> group_;
    };
} // namespace event
} // namespace otf2

#endif // INCLUDE_OTF2XX_EVENT_RMA_GROUP_SYNC_HPP
