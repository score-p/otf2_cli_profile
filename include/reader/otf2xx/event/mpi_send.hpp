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

#ifndef INCLUDE_OTF2XX_EVENT_MPI_SEND_HPP
#define INCLUDE_OTF2XX_EVENT_MPI_SEND_HPP

#include <otf2xx/definition/fwd.hpp>

#include <otf2xx/chrono/chrono.hpp>

#include <otf2xx/event/base.hpp>

#include <otf2xx/definition/detail/weak_ref.hpp>
#include <otf2xx/writer/fwd.hpp>

namespace otf2
{
namespace event
{

    class mpi_send : public base<mpi_send>
    {
    public:
        mpi_send(
            otf2::chrono::time_point timestamp, uint32_t receiver,
            const std::variant<otf2::definition::detail::weak_ref<otf2::definition::comm>,
                               otf2::definition::detail::weak_ref<otf2::definition::inter_comm>>&
                comm,
            uint32_t msg_tag, uint64_t msg_length)
        : base<mpi_send>(timestamp), receiver_(receiver), comm_(comm), msg_tag_(msg_tag),
          msg_length_(msg_length)
        {
        }

        mpi_send(
            OTF2_AttributeList* al, otf2::chrono::time_point timestamp, uint32_t receiver,
            const std::variant<otf2::definition::detail::weak_ref<otf2::definition::comm>,
                               otf2::definition::detail::weak_ref<otf2::definition::inter_comm>>&
                comm,
            uint32_t msg_tag, uint64_t msg_length)
        : base<mpi_send>(al, timestamp), receiver_(receiver), comm_(comm), msg_tag_(msg_tag),
          msg_length_(msg_length)
        {
        }

        mpi_send(const otf2::event::mpi_send& other, otf2::chrono::time_point timestamp)
        : base<mpi_send>(other, timestamp), receiver_(other.receiver()), comm_(other.comm_),
          msg_tag_(other.msg_tag()), msg_length_(other.msg_length())
        {
        }

        uint32_t receiver() const
        {
            return receiver_;
        }

        auto comm() const
        {
            return otf2::definition::variants_from_weak(comm_);
        }

        uint32_t msg_tag() const
        {
            return msg_tag_;
        }

        uint64_t msg_length() const
        {
            return msg_length_;
        }

        friend class otf2::writer::local;

    private:
        uint32_t receiver_;
        std::variant<otf2::definition::detail::weak_ref<otf2::definition::comm>,
                     otf2::definition::detail::weak_ref<otf2::definition::inter_comm>>
            comm_;
        uint32_t msg_tag_;
        uint64_t msg_length_;
    };
} // namespace event
} // namespace otf2

#endif // INCLUDE_OTF2XX_EVENT_MPI_SEND_HPP
