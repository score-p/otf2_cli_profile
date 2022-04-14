/*
 * This file is part of otf2xx (https://github.com/tud-zih-energy/otf2xx)
 * otf2xx - A wrapper for the Open Trace Format 2 library
 *
 * Copyright (c) 2013-2018, Technische Universität Dresden, Germany
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

#ifndef INCLUDE_OTF2XX_EVENT_FWD_HPP
#define INCLUDE_OTF2XX_EVENT_FWD_HPP

namespace otf2
{
namespace event
{
    class buffer;

    template <typename Event>
    class base;

    class buffer_flush;
    class enter;
    class leave;
    class measurement;

    class mpi_send;
    class mpi_receive;
    class mpi_isend;
    class mpi_isend_complete;
    class mpi_ireceive;
    class mpi_ireceive_request;
    class mpi_request_test;
    class mpi_request_cancelled;
    class mpi_collective_begin;
    class mpi_collective_end;

    typedef mpi_ireceive mpi_ireceive_complete;
    typedef mpi_isend mpi_isend_request;

    class non_blocking_collective_request;
    class non_blocking_collective_complete;

    class comm_create;
    class comm_destroy;

    class metric;

    class parameter_string;
    class parameter_int;
    class parameter_unsigned_int;

    class calling_context_enter;
    class calling_context_leave;
    class calling_context_sample;

    class rma_acquire_lock;
    class rma_atomic;
    class rma_collective_begin;
    class rma_collective_end;
    class rma_get;
    class rma_group_sync;
    class rma_op_complete_blocking;
    class rma_op_complete_non_blocking;
    class rma_op_test;
    class rma_op_complete_remote;
    class rma_put;
    class rma_release_lock;
    class rma_request_lock;
    class rma_sync;
    class rma_try_lock;
    class rma_wait_change;
    class rma_win_create;
    class rma_win_destroy;

    class thread_fork;
    class thread_join;
    class thread_team_begin;
    class thread_team_end;
    class thread_acquire_lock;
    class thread_release_lock;
    class thread_task_create;
    class thread_task_switch;
    class thread_task_complete;
    class thread_create;
    class thread_begin;
    class thread_wait;
    class thread_end;

    class marker;

    class io_create_handle;
    class io_destroy_handle;
    class io_duplicate_handle;
    class io_seek;
    class io_change_status_flag;
    class io_delete_file;
    class io_operation_begin;
    class io_operation_test;
    class io_operation_issued;
    class io_operation_cancelled;
    class io_operation_complete;
    class io_acquire_lock;
    class io_release_lock;
    class io_try_lock;

    class program_begin;
    class program_end;

    class unknown;
} // namespace event
} // namespace otf2

#endif // INCLUDE_OTF2XX_EVENT_FWD_HPP
