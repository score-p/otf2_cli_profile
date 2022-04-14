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

#ifndef INCLUDE_OTF2XX_EVENT_MPI_REQUEST_TEST_HPP
#define INCLUDE_OTF2XX_EVENT_MPI_REQUEST_TEST_HPP

#include <otf2xx/definition/fwd.hpp>

#include <otf2xx/chrono/chrono.hpp>

#include <otf2xx/event/base.hpp>

namespace otf2
{
namespace event
{

    class mpi_request_test : public base<mpi_request_test>
    {
    public:
        mpi_request_test(otf2::chrono::time_point timestamp, uint64_t request_id)
        : base<mpi_request_test>(timestamp), request_id_(request_id)
        {
        }

        mpi_request_test(OTF2_AttributeList* al, otf2::chrono::time_point timestamp,
                         uint64_t request_id)
        : base<mpi_request_test>(al, timestamp), request_id_(request_id)
        {
        }

        mpi_request_test(const otf2::event::mpi_request_test& other,
                         otf2::chrono::time_point timestamp)
        : base<mpi_request_test>(other, timestamp), request_id_(other.request_id())
        {
        }

        uint64_t request_id() const
        {
            return request_id_;
        }

    private:
        uint64_t request_id_;
    };
} // namespace event
} // namespace otf2

#endif // INCLUDE_OTF2XX_EVENT_MPI_REQUEST_TEST_HPP
