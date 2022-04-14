/*
 * This file is part of otf2xx (https://github.com/tud-zih-energy/otf2xx)
 * otf2xx - A wrapper for the Open Trace Format 2 library
 *
 * Copyright (c) 2013-2022, Technische Universität Dresden, Germany
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

#pragma once

#include <otf2xx/definition/comm.hpp>
#include <otf2xx/definition/fwd.hpp>
#include <otf2xx/definition/inter_comm.hpp>

#include <otf2xx/chrono/chrono.hpp>

#include <otf2xx/event/base.hpp>

#include <otf2xx/definition/detail/weak_ref.hpp>
#include <otf2xx/writer/fwd.hpp>

#include <otf2xx/common.hpp>

namespace otf2
{
namespace event
{

    class non_blocking_collective_complete : public base<non_blocking_collective_complete>
    {
    public:
        typedef otf2::common::collective_type collective_type;

        non_blocking_collective_complete(
            otf2::chrono::time_point timestamp, collective_type type,
            const std::variant<otf2::definition::detail::weak_ref<otf2::definition::comm>,
                               otf2::definition::detail::weak_ref<otf2::definition::inter_comm>>&
                comm,
            std::uint32_t root, std::uint64_t sent, std::uint64_t received,
            std::uint64_t request_id)
        : base<non_blocking_collective_complete>(timestamp), type_(type), comm_(comm), root_(root),
          sent_(sent), received_(received), request_id_(request_id)
        {
        }

        non_blocking_collective_complete(
            OTF2_AttributeList* al, otf2::chrono::time_point timestamp, collective_type type,
            const std::variant<otf2::definition::detail::weak_ref<otf2::definition::comm>,
                               otf2::definition::detail::weak_ref<otf2::definition::inter_comm>>&
                comm,
            std::uint32_t root, std::uint64_t sent, std::uint64_t received,
            std::uint64_t request_id)
        : base<non_blocking_collective_complete>(al, timestamp), type_(type), comm_(comm),
          root_(root), sent_(sent), received_(received), request_id_(request_id)
        {
        }

        non_blocking_collective_complete(const non_blocking_collective_complete& other,
                                         otf2::chrono::time_point timestamp)
        : base<non_blocking_collective_complete>(other, timestamp), type_(other.type()),
          comm_(other.comm_), root_(other.root()), sent_(other.sent()), received_(other.received()),
          request_id_(other.request_id())
        {
        }

        collective_type type() const
        {
            return type_;
        }

        auto comm() const
        {
            return otf2::definition::variants_from_weak(comm_);
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

        uint64_t request_id() const
        {
            return request_id_;
        }

        friend class otf2::writer::local;

    private:
        collective_type type_;
        std::variant<otf2::definition::detail::weak_ref<otf2::definition::comm>,
                     otf2::definition::detail::weak_ref<otf2::definition::inter_comm>>
            comm_;
        std::uint32_t root_;
        std::uint64_t sent_;
        std::uint64_t received_;
        uint64_t request_id_;
    };
} // namespace event
} // namespace otf2
