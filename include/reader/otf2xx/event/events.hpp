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

#ifndef INCLUDE_OTF2XX_EVENT_EVENTS_HPP
#define INCLUDE_OTF2XX_EVENT_EVENTS_HPP

#include <otf2xx/event/buffer_flush.hpp>
#include <otf2xx/event/enter.hpp>
#include <otf2xx/event/leave.hpp>
#include <otf2xx/event/measurement.hpp>

#include <otf2xx/event/metric.hpp>

#include <otf2xx/event/mpi_collective_begin.hpp>
#include <otf2xx/event/mpi_collective_end.hpp>
#include <otf2xx/event/mpi_ireceive.hpp>
#include <otf2xx/event/mpi_ireceive_request.hpp>
#include <otf2xx/event/mpi_isend.hpp>
#include <otf2xx/event/mpi_isend_complete.hpp>
#include <otf2xx/event/mpi_receive.hpp>
#include <otf2xx/event/mpi_request_cancelled.hpp>
#include <otf2xx/event/mpi_request_test.hpp>
#include <otf2xx/event/mpi_send.hpp>
#include <otf2xx/event/non_blocking_collective_complete.hpp>
#include <otf2xx/event/non_blocking_collective_request.hpp>

#include <otf2xx/event/comm_create.hpp>
#include <otf2xx/event/comm_destroy.hpp>

#include <otf2xx/event/parameter_int.hpp>
#include <otf2xx/event/parameter_string.hpp>
#include <otf2xx/event/parameter_unsigned_int.hpp>

#include <otf2xx/event/calling_context_enter.hpp>
#include <otf2xx/event/calling_context_leave.hpp>
#include <otf2xx/event/calling_context_sample.hpp>

#include <otf2xx/event/thread_acquire_lock.hpp>
#include <otf2xx/event/thread_begin.hpp>
#include <otf2xx/event/thread_create.hpp>
#include <otf2xx/event/thread_end.hpp>
#include <otf2xx/event/thread_fork.hpp>
#include <otf2xx/event/thread_join.hpp>
#include <otf2xx/event/thread_release_lock.hpp>
#include <otf2xx/event/thread_task_complete.hpp>
#include <otf2xx/event/thread_task_create.hpp>
#include <otf2xx/event/thread_task_switch.hpp>
#include <otf2xx/event/thread_team_begin.hpp>
#include <otf2xx/event/thread_team_end.hpp>
#include <otf2xx/event/thread_wait.hpp>

#include <otf2xx/event/marker.hpp>

#include <otf2xx/event/rma_acquire_lock.hpp>
#include <otf2xx/event/rma_atomic.hpp>
#include <otf2xx/event/rma_collective_begin.hpp>
#include <otf2xx/event/rma_collective_end.hpp>
#include <otf2xx/event/rma_get.hpp>
#include <otf2xx/event/rma_group_sync.hpp>
#include <otf2xx/event/rma_op_complete_blocking.hpp>
#include <otf2xx/event/rma_op_complete_non_blocking.hpp>
#include <otf2xx/event/rma_op_complete_remote.hpp>
#include <otf2xx/event/rma_op_test.hpp>
#include <otf2xx/event/rma_put.hpp>
#include <otf2xx/event/rma_release_lock.hpp>
#include <otf2xx/event/rma_request_lock.hpp>
#include <otf2xx/event/rma_sync.hpp>
#include <otf2xx/event/rma_try_lock.hpp>
#include <otf2xx/event/rma_wait_change.hpp>
#include <otf2xx/event/rma_win_create.hpp>
#include <otf2xx/event/rma_win_destroy.hpp>

#include <otf2xx/event/io_acquire_lock.hpp>
#include <otf2xx/event/io_change_status_flag.hpp>
#include <otf2xx/event/io_create_handle.hpp>
#include <otf2xx/event/io_delete_file.hpp>
#include <otf2xx/event/io_destroy_handle.hpp>
#include <otf2xx/event/io_duplicate_handle.hpp>
#include <otf2xx/event/io_operation_begin.hpp>
#include <otf2xx/event/io_operation_cancelled.hpp>
#include <otf2xx/event/io_operation_complete.hpp>
#include <otf2xx/event/io_operation_issued.hpp>
#include <otf2xx/event/io_operation_test.hpp>
#include <otf2xx/event/io_release_lock.hpp>
#include <otf2xx/event/io_seek.hpp>
#include <otf2xx/event/io_try_lock.hpp>

#include <otf2xx/event/program_begin.hpp>
#include <otf2xx/event/program_end.hpp>

#include <otf2xx/event/unknown.hpp>
#endif // INCLUDE_OTF2XX_EVENT_EVENTS_HPP
