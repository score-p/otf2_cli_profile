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

#ifndef INCLUDE_OTF2XX_EVENT_MPI_IRECEIVE_REQUEST_HPP
#define INCLUDE_OTF2XX_EVENT_MPI_IRECEIVE_REQUEST_HPP

#include <otf2xx/definition/fwd.hpp>

#include <otf2xx/chrono/chrono.hpp>

#include <otf2xx/event/base.hpp>

#include <otf2xx/definition/detail/weak_ref.hpp>
#include <otf2xx/writer/fwd.hpp>

namespace otf2
{
namespace event
{

    class buffer;

    class mpi_ireceive_request : public base<mpi_ireceive_request>
    {
    public:
        mpi_ireceive_request(otf2::chrono::time_point timestamp, uint64_t request_id)
        : base<mpi_ireceive_request>(timestamp), request_id_(request_id)
        {
        }

        mpi_ireceive_request(OTF2_AttributeList* al, otf2::chrono::time_point timestamp,
                             uint64_t request_id)
        : base<mpi_ireceive_request>(al, timestamp), request_id_(request_id)
        {
        }

        mpi_ireceive_request(const otf2::event::mpi_ireceive_request& other,
                             otf2::chrono::time_point timestamp)
        : base<mpi_ireceive_request>(other, timestamp), request_id_(other.request_id())
        {
            if (other.has_attached_data())
            {
                sender_ = other.sender();
                comm_ = other.comm_;
                msg_tag_ = other.msg_tag();
                msg_length_ = other.msg_length();
                attached_data_ = true;
            }
        }

        uint64_t request_id() const
        {
            return request_id_;
        }

    public:
        uint32_t sender() const
        {
            assert(has_attached_data());
            return sender_;
        }

        auto comm() const
        {
            assert(has_attached_data());
            return otf2::definition::variants_from_weak(comm_);
        }

        uint32_t msg_tag() const
        {
            assert(has_attached_data());
            return msg_tag_;
        }

        uint64_t msg_length() const
        {
            assert(has_attached_data());
            return msg_length_;
        }

        bool has_attached_data() const
        {
            return attached_data_;
        }

    private:
        void attach_data(
            uint32_t sender,
            const std::variant<otf2::definition::detail::weak_ref<otf2::definition::comm>,
                               otf2::definition::detail::weak_ref<otf2::definition::inter_comm>>&
                comm,
            uint32_t msg_tag, uint64_t msg_length)
        {
            sender_ = sender;
            comm_ = comm;
            msg_tag_ = msg_tag;
            msg_length_ = msg_length;

            attached_data_ = true;
        }

        friend class buffer;
        friend class otf2::writer::local;

        uint64_t request_id_;
        bool attached_data_ = false;

        uint32_t sender_;
        std::variant<otf2::definition::detail::weak_ref<otf2::definition::comm>,
                     otf2::definition::detail::weak_ref<otf2::definition::inter_comm>>
            comm_;
        uint32_t msg_tag_;
        uint64_t msg_length_;
    };
} // namespace event
} // namespace otf2

#endif // INCLUDE_OTF2XX_EVENT_MPI_IRECEIVE_REQUEST_HPP
