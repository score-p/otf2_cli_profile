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

#ifndef INCLUDE_OTF2XX_EVENT_IO_OPERATION_BEGIN_HPP
#define INCLUDE_OTF2XX_EVENT_IO_OPERATION_BEGIN_HPP

#include <otf2xx/common.hpp>
#include <otf2xx/event/base.hpp>

#include <otf2xx/chrono/chrono.hpp>

#include <otf2xx/definition/detail/weak_ref.hpp>

namespace otf2
{
namespace event
{
    class io_operation_begin : public base<io_operation_begin>
    {
    public:
        using io_operation_mode_type = otf2::common::io_operation_mode_type;
        using io_operation_flag_type = otf2::common::io_operation_flag_type;

        io_operation_begin(otf2::chrono::time_point timestamp,
                           const otf2::definition::io_handle& handle,
                           io_operation_mode_type io_operation_mode,
                           io_operation_flag_type io_operation_flag, std::uint64_t bytes_request,
                           std::uint64_t matching_id)
        : base<io_operation_begin>(timestamp), handle_(handle), operation_mode_(io_operation_mode),
          operation_flag_(io_operation_flag), bytes_request_(bytes_request),
          matching_id_(matching_id)
        {
        }

        io_operation_begin(OTF2_AttributeList* al, otf2::chrono::time_point timestamp,
                           const otf2::definition::io_handle& handle,
                           io_operation_mode_type io_operation_mode,
                           io_operation_flag_type io_operation_flag, std::uint64_t bytes_request,
                           std::uint64_t matching_id)
        : base<io_operation_begin>(al, timestamp), handle_(handle),
          operation_mode_(io_operation_mode), operation_flag_(io_operation_flag),
          bytes_request_(bytes_request), matching_id_(matching_id)
        {
        }

        // copy constructor with new timestamp
        io_operation_begin(const otf2::event::io_operation_begin& other,
                           otf2::chrono::time_point timestamp)
        : base<io_operation_begin>(timestamp), handle_(other.handle()),
          operation_mode_(other.operation_mode()), operation_flag_(other.operation_flag()),
          bytes_request_(other.bytes_request()), matching_id_(other.matching_id())
        {
        }

        otf2::definition::io_handle handle() const
        {
            return handle_;
        }

        io_operation_mode_type operation_mode() const
        {
            return operation_mode_;
        }

        io_operation_flag_type operation_flag() const
        {
            return operation_flag_;
        }

        std::uint64_t bytes_request() const
        {
            return bytes_request_;
        }

        std::uint64_t matching_id() const
        {
            return matching_id_;
        }

    private:
        otf2::definition::detail::weak_ref<otf2::definition::io_handle> handle_;
        io_operation_mode_type operation_mode_;
        io_operation_flag_type operation_flag_;
        std::uint64_t bytes_request_;
        std::uint64_t matching_id_;
    };
} // namespace event
} // namespace otf2

#endif // INCLUDE_OTF2XX_EVENT_IO_OPERATION_BEGIN_HPP
