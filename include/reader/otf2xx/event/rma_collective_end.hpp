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

#ifndef INCLUDE_OTF2XX_EVENT_RMA_COLLECTIVE_END_HPP
#define INCLUDE_OTF2XX_EVENT_RMA_COLLECTIVE_END_HPP

#include <otf2xx/common.hpp>
#include <otf2xx/event/base.hpp>

#include <otf2xx/chrono/chrono.hpp>

#include <otf2xx/definition/detail/weak_ref.hpp>

namespace otf2
{
namespace event
{
    class rma_collective_end : public base<rma_collective_end>
    {
    public:
        using collective_type = otf2::common::collective_type;
        using sync_level_type = otf2::common::rma_sync_level_type;

        rma_collective_end(otf2::chrono::time_point timestamp, collective_type type,
                           sync_level_type level, const otf2::definition::rma_win& win,
                           std::uint32_t root, std::uint64_t sent, std::uint64_t received)
        : base<rma_collective_end>(timestamp), type_(type), level_(level), win_(win), root_(root),
          sent_(sent), received_(received)
        {
        }

        rma_collective_end(OTF2_AttributeList* al, otf2::chrono::time_point timestamp,
                           collective_type type, sync_level_type level,
                           const otf2::definition::rma_win& win, std::uint32_t root,
                           std::uint64_t sent, std::uint64_t received)
        : base<rma_collective_end>(al, timestamp), type_(type), level_(level), win_(win),
          root_(root), sent_(sent), received_(received)
        {
        }

        // copy constructor with new timestamp
        rma_collective_end(const otf2::event::rma_collective_end& other,
                           otf2::chrono::time_point timestamp)
        : base<rma_collective_end>(timestamp), type_(other.type()), level_(other.level()),
          win_(other.win()), root_(other.root()), sent_(other.sent()), received_(other.received())
        {
        }

        collective_type type() const
        {
            return type_;
        }

        sync_level_type level() const
        {
            return level_;
        }

        otf2::definition::rma_win win() const
        {
            return win_;
        }

        std::uint32_t root() const
        {
            return root_;
        }

        std::uint64_t sent() const
        {
            return sent_;
        }

        std::uint64_t received() const
        {
            return received_;
        }

    private:
        collective_type type_;
        sync_level_type level_;
        otf2::definition::detail::weak_ref<otf2::definition::rma_win> win_;
        std::uint32_t root_;
        std::uint64_t sent_;
        std::uint64_t received_;
    };
} // namespace event
} // namespace otf2

#endif // INCLUDE_OTF2XX_EVENT_RMA_COLLECTIVE_END_HPP
