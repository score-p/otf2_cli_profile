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

#ifndef INCLUDE_OTF2XX_EVENT_BUFFER_HPP
#define INCLUDE_OTF2XX_EVENT_BUFFER_HPP

#include <otf2xx/definition/location.hpp>
#include <otf2xx/event/events.hpp>
#include <otf2xx/reader/callback.hpp>

#include <otf2xx/common.hpp>

#include <otf2xx/exception.hpp>

#include <cassert>
#include <deque>

namespace otf2
{
namespace event
{

    namespace detail
    {

        struct buffer_node
        {
            buffer_node(const buffer_node&) = delete;
            buffer_node& operator=(const buffer_node&) = delete;

            buffer_node(const otf2::definition::location& loc, otf2::common::event_type type,
                        void* event, bool completed = true)
            : location(loc), type(type), event(event), completed(completed)
            {
            }

        public:
            buffer_node(const otf2::definition::location& loc,
                        const otf2::event::buffer_flush& event)
            : buffer_node(loc, otf2::common::event_type::buffer_flush,
                          new otf2::event::buffer_flush(event))
            {
            }

            buffer_node(const otf2::definition::location& loc, const otf2::event::enter& event)
            : buffer_node(loc, otf2::common::event_type::enter, new otf2::event::enter(event))
            {
            }

            buffer_node(const otf2::definition::location& loc,
                        const otf2::event::io_acquire_lock& event)
            : buffer_node(loc, otf2::common::event_type::io_acquire_lock,
                          new otf2::event::io_acquire_lock(event))
            {
            }

            buffer_node(const otf2::definition::location& loc,
                        const otf2::event::io_change_status_flag& event)
            : buffer_node(loc, otf2::common::event_type::io_change_status_flag,
                          new otf2::event::io_change_status_flag(event))
            {
            }

            buffer_node(const otf2::definition::location& loc,
                        const otf2::event::io_create_handle& event)
            : buffer_node(loc, otf2::common::event_type::io_create_handle,
                          new otf2::event::io_create_handle(event))
            {
            }

            buffer_node(const otf2::definition::location& loc,
                        const otf2::event::io_delete_file& event)
            : buffer_node(loc, otf2::common::event_type::io_delete_file,
                          new otf2::event::io_delete_file(event))
            {
            }

            buffer_node(const otf2::definition::location& loc,
                        const otf2::event::io_destroy_handle& event)
            : buffer_node(loc, otf2::common::event_type::io_destroy_handle,
                          new otf2::event::io_destroy_handle(event))
            {
            }

            buffer_node(const otf2::definition::location& loc,
                        const otf2::event::io_duplicate_handle& event)
            : buffer_node(loc, otf2::common::event_type::io_duplicate_handle,
                          new otf2::event::io_duplicate_handle(event))
            {
            }

            buffer_node(const otf2::definition::location& loc,
                        const otf2::event::io_operation_begin& event)
            : buffer_node(loc, otf2::common::event_type::io_operation_begin,
                          new otf2::event::io_operation_begin(event))
            {
            }

            buffer_node(const otf2::definition::location& loc,
                        const otf2::event::io_operation_cancelled& event)
            : buffer_node(loc, otf2::common::event_type::io_operation_cancelled,
                          new otf2::event::io_operation_cancelled(event))
            {
            }

            buffer_node(const otf2::definition::location& loc,
                        const otf2::event::io_operation_complete& event)
            : buffer_node(loc, otf2::common::event_type::io_operation_complete,
                          new otf2::event::io_operation_complete(event))
            {
            }

            buffer_node(const otf2::definition::location& loc,
                        const otf2::event::io_operation_issued& event)
            : buffer_node(loc, otf2::common::event_type::io_operation_issued,
                          new otf2::event::io_operation_issued(event))
            {
            }

            buffer_node(const otf2::definition::location& loc,
                        const otf2::event::io_operation_test& event)
            : buffer_node(loc, otf2::common::event_type::io_operation_test,
                          new otf2::event::io_operation_test(event))
            {
            }

            buffer_node(const otf2::definition::location& loc,
                        const otf2::event::io_release_lock& event)
            : buffer_node(loc, otf2::common::event_type::io_release_lock,
                          new otf2::event::io_release_lock(event))
            {
            }

            buffer_node(const otf2::definition::location& loc, const otf2::event::io_seek& event)
            : buffer_node(loc, otf2::common::event_type::io_seek, new otf2::event::io_seek(event))
            {
            }

            buffer_node(const otf2::definition::location& loc,
                        const otf2::event::io_try_lock& event)
            : buffer_node(loc, otf2::common::event_type::io_try_lock,
                          new otf2::event::io_try_lock(event))
            {
            }

            buffer_node(const otf2::definition::location& loc, const otf2::event::leave& event)
            : buffer_node(loc, otf2::common::event_type::leave, new otf2::event::leave(event))
            {
            }

            buffer_node(const otf2::definition::location& loc,
                        const otf2::event::measurement& event)
            : buffer_node(loc, otf2::common::event_type::measurement,
                          new otf2::event::measurement(event))
            {
            }

            buffer_node(const otf2::definition::location& loc, const otf2::event::metric& event)
            : buffer_node(loc, otf2::common::event_type::metric, new otf2::event::metric(event))
            {
            }

            buffer_node(const otf2::definition::location& loc,
                        const otf2::event::mpi_collective_begin& event)
            : buffer_node(loc, otf2::common::event_type::mpi_collective_begin,
                          new otf2::event::mpi_collective_begin(event))
            {
            }

            buffer_node(const otf2::definition::location& loc,
                        const otf2::event::mpi_collective_end& event)
            : buffer_node(loc, otf2::common::event_type::mpi_collective_end,
                          new otf2::event::mpi_collective_end(event))
            {
            }

            buffer_node(const otf2::definition::location& loc,
                        const otf2::event::mpi_ireceive& event)
            : buffer_node(loc, otf2::common::event_type::mpi_ireceive,
                          new otf2::event::mpi_ireceive(event))
            {
            }

            buffer_node(const otf2::definition::location& loc,
                        const otf2::event::mpi_ireceive_request& event)
            : buffer_node(loc, otf2::common::event_type::mpi_ireceive_request,
                          new otf2::event::mpi_ireceive_request(event), false)
            {
            }

            buffer_node(const otf2::definition::location& loc, const otf2::event::mpi_isend& event)
            : buffer_node(loc, otf2::common::event_type::mpi_isend,
                          new otf2::event::mpi_isend(event))
            {
            }

            buffer_node(const otf2::definition::location& loc,
                        const otf2::event::mpi_isend_complete& event)
            : buffer_node(loc, otf2::common::event_type::mpi_isend_complete,
                          new otf2::event::mpi_isend_complete(event))
            {
            }

            buffer_node(const otf2::definition::location& loc,
                        const otf2::event::mpi_receive& event)
            : buffer_node(loc, otf2::common::event_type::mpi_receive,
                          new otf2::event::mpi_receive(event))
            {
            }

            buffer_node(const otf2::definition::location& loc,
                        const otf2::event::mpi_request_cancelled& event)
            : buffer_node(loc, otf2::common::event_type::mpi_request_cancelled,
                          new otf2::event::mpi_request_cancelled(event))
            {
            }

            buffer_node(const otf2::definition::location& loc,
                        const otf2::event::mpi_request_test& event)
            : buffer_node(loc, otf2::common::event_type::mpi_request_test,
                          new otf2::event::mpi_request_test(event))
            {
            }

            buffer_node(const otf2::definition::location& loc, const otf2::event::mpi_send& event)
            : buffer_node(loc, otf2::common::event_type::mpi_send, new otf2::event::mpi_send(event))
            {
            }

            buffer_node(const otf2::definition::location& loc,
                        const otf2::event::parameter_int& event)
            : buffer_node(loc, otf2::common::event_type::parameter_int,
                          new otf2::event::parameter_int(event))
            {
            }

            buffer_node(const otf2::definition::location& loc,
                        const otf2::event::parameter_string& event)
            : buffer_node(loc, otf2::common::event_type::parameter_string,
                          new otf2::event::parameter_string(event))
            {
            }

            buffer_node(const otf2::definition::location& loc,
                        const otf2::event::parameter_unsigned_int& event)
            : buffer_node(loc, otf2::common::event_type::parameter_unsigned_int,
                          new otf2::event::parameter_unsigned_int(event))
            {
            }

            buffer_node(const otf2::definition::location& loc,
                        const otf2::event::thread_acquire_lock& event)
            : buffer_node(loc, otf2::common::event_type::thread_acquire_lock,
                          new otf2::event::thread_acquire_lock(event))
            {
            }

            buffer_node(const otf2::definition::location& loc,
                        const otf2::event::thread_fork& event)
            : buffer_node(loc, otf2::common::event_type::thread_fork,
                          new otf2::event::thread_fork(event))
            {
            }

            buffer_node(const otf2::definition::location& loc,
                        const otf2::event::thread_join& event)
            : buffer_node(loc, otf2::common::event_type::thread_join,
                          new otf2::event::thread_join(event))
            {
            }

            buffer_node(const otf2::definition::location& loc,
                        const otf2::event::thread_release_lock& event)
            : buffer_node(loc, otf2::common::event_type::thread_release_lock,
                          new otf2::event::thread_release_lock(event))
            {
            }

            buffer_node(const otf2::definition::location& loc,
                        const otf2::event::thread_task_complete& event)
            : buffer_node(loc, otf2::common::event_type::thread_task_complete,
                          new otf2::event::thread_task_complete(event))
            {
            }

            buffer_node(const otf2::definition::location& loc,
                        const otf2::event::thread_task_create& event)
            : buffer_node(loc, otf2::common::event_type::thread_task_create,
                          new otf2::event::thread_task_create(event))
            {
            }

            buffer_node(const otf2::definition::location& loc,
                        const otf2::event::thread_task_switch& event)
            : buffer_node(loc, otf2::common::event_type::thread_task_switch,
                          new otf2::event::thread_task_switch(event))
            {
            }

            buffer_node(const otf2::definition::location& loc,
                        const otf2::event::thread_team_begin& event)
            : buffer_node(loc, otf2::common::event_type::thread_team_begin,
                          new otf2::event::thread_team_begin(event))
            {
            }

            buffer_node(const otf2::definition::location& loc,
                        const otf2::event::thread_team_end& event)
            : buffer_node(loc, otf2::common::event_type::thread_team_end,
                          new otf2::event::thread_team_end(event))
            {
            }

        public:
            otf2::definition::location location;
            otf2::common::event_type type;
            void* event;
            bool completed;

            ~buffer_node()
            {
                switch (type)
                {
                case otf2::common::event_type::buffer_flush:
                    delete static_cast<otf2::event::buffer_flush*>(event);
                    break;

                case otf2::common::event_type::enter:
                    delete static_cast<otf2::event::enter*>(event);
                    break;

                case otf2::common::event_type::io_acquire_lock:
                    delete static_cast<otf2::event::io_acquire_lock*>(event);
                    break;

                case otf2::common::event_type::io_change_status_flag:
                    delete static_cast<otf2::event::io_change_status_flag*>(event);
                    break;

                case otf2::common::event_type::io_create_handle:
                    delete static_cast<otf2::event::io_create_handle*>(event);
                    break;

                case otf2::common::event_type::io_delete_file:
                    delete static_cast<otf2::event::io_delete_file*>(event);
                    break;

                case otf2::common::event_type::io_destroy_handle:
                    delete static_cast<otf2::event::io_destroy_handle*>(event);
                    break;

                case otf2::common::event_type::io_duplicate_handle:
                    delete static_cast<otf2::event::io_duplicate_handle*>(event);
                    break;

                case otf2::common::event_type::io_operation_begin:
                    delete static_cast<otf2::event::io_operation_begin*>(event);
                    break;

                case otf2::common::event_type::io_operation_cancelled:
                    delete static_cast<otf2::event::io_operation_cancelled*>(event);
                    break;

                case otf2::common::event_type::io_operation_complete:
                    delete static_cast<otf2::event::io_operation_complete*>(event);
                    break;

                case otf2::common::event_type::io_operation_issued:
                    delete static_cast<otf2::event::io_operation_issued*>(event);
                    break;

                case otf2::common::event_type::io_operation_test:
                    delete static_cast<otf2::event::io_operation_test*>(event);
                    break;

                case otf2::common::event_type::io_release_lock:
                    delete static_cast<otf2::event::io_release_lock*>(event);
                    break;

                case otf2::common::event_type::io_seek:
                    delete static_cast<otf2::event::io_seek*>(event);
                    break;

                case otf2::common::event_type::io_try_lock:
                    delete static_cast<otf2::event::io_try_lock*>(event);
                    break;

                case otf2::common::event_type::leave:
                    delete static_cast<otf2::event::leave*>(event);
                    break;

                case otf2::common::event_type::measurement:
                    delete static_cast<otf2::event::measurement*>(event);
                    break;

                case otf2::common::event_type::metric:
                    delete static_cast<otf2::event::metric*>(event);
                    break;

                case otf2::common::event_type::mpi_collective_begin:
                    delete static_cast<otf2::event::mpi_collective_begin*>(event);
                    break;

                case otf2::common::event_type::mpi_collective_end:
                    delete static_cast<otf2::event::mpi_collective_end*>(event);
                    break;

                case otf2::common::event_type::mpi_ireceive:
                    delete static_cast<otf2::event::mpi_ireceive*>(event);
                    break;

                case otf2::common::event_type::mpi_ireceive_request:
                    delete static_cast<otf2::event::mpi_ireceive_request*>(event);
                    break;

                case otf2::common::event_type::mpi_isend:
                    delete static_cast<otf2::event::mpi_isend*>(event);
                    break;

                case otf2::common::event_type::mpi_isend_complete:
                    delete static_cast<otf2::event::mpi_isend_complete*>(event);
                    break;

                case otf2::common::event_type::mpi_receive:
                    delete static_cast<otf2::event::mpi_receive*>(event);
                    break;

                case otf2::common::event_type::mpi_request_cancelled:
                    delete static_cast<otf2::event::mpi_request_cancelled*>(event);
                    break;

                case otf2::common::event_type::mpi_request_test:
                    delete static_cast<otf2::event::mpi_request_test*>(event);
                    break;

                case otf2::common::event_type::mpi_send:
                    delete static_cast<otf2::event::mpi_send*>(event);
                    break;

                case otf2::common::event_type::parameter_int:
                    delete static_cast<otf2::event::parameter_int*>(event);
                    break;

                case otf2::common::event_type::parameter_string:
                    delete static_cast<otf2::event::parameter_string*>(event);
                    break;

                case otf2::common::event_type::parameter_unsigned_int:
                    delete static_cast<otf2::event::parameter_unsigned_int*>(event);
                    break;

                case otf2::common::event_type::thread_acquire_lock:
                    delete static_cast<otf2::event::thread_acquire_lock*>(event);
                    break;

                case otf2::common::event_type::thread_fork:
                    delete static_cast<otf2::event::thread_fork*>(event);
                    break;

                case otf2::common::event_type::thread_join:
                    delete static_cast<otf2::event::thread_join*>(event);
                    break;

                case otf2::common::event_type::thread_release_lock:
                    delete static_cast<otf2::event::thread_release_lock*>(event);
                    break;

                case otf2::common::event_type::thread_task_complete:
                    delete static_cast<otf2::event::thread_task_complete*>(event);
                    break;

                case otf2::common::event_type::thread_task_create:
                    delete static_cast<otf2::event::thread_task_create*>(event);
                    break;

                case otf2::common::event_type::thread_task_switch:
                    delete static_cast<otf2::event::thread_task_switch*>(event);
                    break;

                case otf2::common::event_type::thread_team_begin:
                    delete static_cast<otf2::event::thread_team_begin*>(event);
                    break;

                case otf2::common::event_type::thread_team_end:
                    delete static_cast<otf2::event::thread_team_end*>(event);
                    break;

                default:
                    make_exception("Added unknown event type to event buffer");
                    break;
                }
            }
        };
    } // namespace detail

    /**
     * \brief This class isn't an event, but a buffer for events
     *
     * It's used to add information to mpi_isend and mpi_ireceive_request
     *
     */
    class buffer : public otf2::reader::callback
    {
    public:
        buffer(otf2::reader::callback& callback) : callback_(callback)
        {
        }

        ~buffer()
        {
            assert(nodes_.size() == 0);
        }

        void process_data()
        {
            while (nodes_.size() > 0 && nodes_.front().completed)
            {
                switch (nodes_.front().type)
                {
                case otf2::common::event_type::buffer_flush:
                    callback_.event(nodes_.front().location,
                                    *static_cast<otf2::event::buffer_flush*>(nodes_.front().event));
                    break;

                case otf2::common::event_type::enter:
                    callback_.event(nodes_.front().location,
                                    *static_cast<otf2::event::enter*>(nodes_.front().event));
                    break;

                case otf2::common::event_type::io_acquire_lock:
                    callback_.event(
                        nodes_.front().location,
                        *static_cast<otf2::event::io_acquire_lock*>(nodes_.front().event));
                    break;

                case otf2::common::event_type::leave:
                    callback_.event(nodes_.front().location,
                                    *static_cast<otf2::event::leave*>(nodes_.front().event));
                    break;

                case otf2::common::event_type::measurement:
                    callback_.event(nodes_.front().location,
                                    *static_cast<otf2::event::measurement*>(nodes_.front().event));
                    break;

                case otf2::common::event_type::metric:
                    callback_.event(nodes_.front().location,
                                    *static_cast<otf2::event::metric*>(nodes_.front().event));
                    break;

                case otf2::common::event_type::mpi_collective_begin:
                    callback_.event(
                        nodes_.front().location,
                        *static_cast<otf2::event::mpi_collective_begin*>(nodes_.front().event));
                    break;

                case otf2::common::event_type::mpi_collective_end:
                    callback_.event(
                        nodes_.front().location,
                        *static_cast<otf2::event::mpi_collective_end*>(nodes_.front().event));
                    break;

                case otf2::common::event_type::mpi_ireceive:
                    callback_.event(nodes_.front().location,
                                    *static_cast<otf2::event::mpi_ireceive*>(nodes_.front().event));
                    break;

                case otf2::common::event_type::mpi_ireceive_request:
                    callback_.event(
                        nodes_.front().location,
                        *static_cast<otf2::event::mpi_ireceive_request*>(nodes_.front().event));
                    break;

                case otf2::common::event_type::mpi_isend:
                    callback_.event(nodes_.front().location,
                                    *static_cast<otf2::event::mpi_isend*>(nodes_.front().event));
                    break;

                case otf2::common::event_type::mpi_isend_complete:
                    callback_.event(
                        nodes_.front().location,
                        *static_cast<otf2::event::mpi_isend_complete*>(nodes_.front().event));
                    break;

                case otf2::common::event_type::mpi_receive:
                    callback_.event(nodes_.front().location,
                                    *static_cast<otf2::event::mpi_receive*>(nodes_.front().event));
                    break;

                case otf2::common::event_type::mpi_request_cancelled:
                    callback_.event(
                        nodes_.front().location,
                        *static_cast<otf2::event::mpi_request_cancelled*>(nodes_.front().event));
                    break;

                case otf2::common::event_type::mpi_request_test:
                    callback_.event(
                        nodes_.front().location,
                        *static_cast<otf2::event::mpi_request_test*>(nodes_.front().event));
                    break;

                case otf2::common::event_type::mpi_send:
                    callback_.event(nodes_.front().location,
                                    *static_cast<otf2::event::mpi_send*>(nodes_.front().event));
                    break;

                case otf2::common::event_type::parameter_int:
                    callback_.event(
                        nodes_.front().location,
                        *static_cast<otf2::event::parameter_int*>(nodes_.front().event));
                    break;

                case otf2::common::event_type::parameter_string:
                    callback_.event(
                        nodes_.front().location,
                        *static_cast<otf2::event::parameter_string*>(nodes_.front().event));
                    break;

                case otf2::common::event_type::parameter_unsigned_int:
                    callback_.event(
                        nodes_.front().location,
                        *static_cast<otf2::event::parameter_unsigned_int*>(nodes_.front().event));
                    break;

                case otf2::common::event_type::thread_acquire_lock:
                    callback_.event(
                        nodes_.front().location,
                        *static_cast<otf2::event::thread_acquire_lock*>(nodes_.front().event));
                    break;

                case otf2::common::event_type::thread_fork:
                    callback_.event(nodes_.front().location,
                                    *static_cast<otf2::event::thread_fork*>(nodes_.front().event));
                    break;

                case otf2::common::event_type::thread_join:
                    callback_.event(nodes_.front().location,
                                    *static_cast<otf2::event::thread_join*>(nodes_.front().event));
                    break;

                case otf2::common::event_type::thread_release_lock:
                    callback_.event(
                        nodes_.front().location,
                        *static_cast<otf2::event::thread_release_lock*>(nodes_.front().event));
                    break;

                case otf2::common::event_type::thread_task_complete:
                    callback_.event(
                        nodes_.front().location,
                        *static_cast<otf2::event::thread_task_complete*>(nodes_.front().event));
                    break;

                case otf2::common::event_type::thread_task_create:
                    callback_.event(
                        nodes_.front().location,
                        *static_cast<otf2::event::thread_task_create*>(nodes_.front().event));
                    break;

                case otf2::common::event_type::thread_task_switch:
                    callback_.event(
                        nodes_.front().location,
                        *static_cast<otf2::event::thread_task_switch*>(nodes_.front().event));
                    break;

                case otf2::common::event_type::thread_team_begin:
                    callback_.event(
                        nodes_.front().location,
                        *static_cast<otf2::event::thread_team_begin*>(nodes_.front().event));
                    break;

                case otf2::common::event_type::thread_team_end:
                    callback_.event(
                        nodes_.front().location,
                        *static_cast<otf2::event::thread_team_end*>(nodes_.front().event));
                    break;

                default:
                    make_exception("Added unknown event type to event buffer");
                    break;
                }

                nodes_.pop_front();
            }
        }

    public:
        // events
        virtual void event(const otf2::definition::location& loc,
                           const otf2::event::buffer_flush& event) override
        {
            add(loc, event);
        }
        virtual void event(const otf2::definition::location& loc,
                           const otf2::event::enter& event) override
        {
            add(loc, event);
        }

        virtual void event(const otf2::definition::location& loc,
                           const otf2::event::io_acquire_lock& event) override
        {
            add(loc, event);
        }

        virtual void event(const otf2::definition::location& loc,
                           const otf2::event::io_change_status_flag& event) override
        {
            add(loc, event);
        }

        virtual void event(const otf2::definition::location& loc,
                           const otf2::event::io_create_handle& event) override
        {
            add(loc, event);
        }

        virtual void event(const otf2::definition::location& loc,
                           const otf2::event::io_delete_file& event) override
        {
            add(loc, event);
        }

        virtual void event(const otf2::definition::location& loc,
                           const otf2::event::io_destroy_handle& event) override
        {
            add(loc, event);
        }

        virtual void event(const otf2::definition::location& loc,
                           const otf2::event::io_duplicate_handle& event) override
        {
            add(loc, event);
        }

        virtual void event(const otf2::definition::location& loc,
                           const otf2::event::io_operation_begin& event) override
        {
            add(loc, event);
        }

        virtual void event(const otf2::definition::location& loc,
                           const otf2::event::io_operation_cancelled& event) override
        {
            add(loc, event);
        }

        virtual void event(const otf2::definition::location& loc,
                           const otf2::event::io_operation_complete& event) override
        {
            add(loc, event);
        }

        virtual void event(const otf2::definition::location& loc,
                           const otf2::event::io_operation_issued& event) override
        {
            add(loc, event);
        }

        virtual void event(const otf2::definition::location& loc,
                           const otf2::event::io_operation_test& event) override
        {
            add(loc, event);
        }

        virtual void event(const otf2::definition::location& loc,
                           const otf2::event::io_release_lock& event) override
        {
            add(loc, event);
        }

        virtual void event(const otf2::definition::location& loc,
                           const otf2::event::io_seek& event) override
        {
            add(loc, event);
        }

        virtual void event(const otf2::definition::location& loc,
                           const otf2::event::io_try_lock& event) override
        {
            add(loc, event);
        }

        virtual void event(const otf2::definition::location& loc,
                           const otf2::event::leave& event) override
        {
            add(loc, event);
        }

        virtual void event(const otf2::definition::location& loc,
                           const otf2::event::measurement& event) override
        {
            add(loc, event);
        }

        virtual void event(const otf2::definition::location& loc,
                           const otf2::event::metric& event) override
        {
            add(loc, event);
        }

        virtual void event(const otf2::definition::location& loc,
                           const otf2::event::mpi_send& event) override
        {
            add(loc, event);
        }
        virtual void event(const otf2::definition::location& loc,
                           const otf2::event::mpi_receive& event) override
        {
            add(loc, event);
        }
        virtual void event(const otf2::definition::location& loc,
                           const otf2::event::mpi_isend& event) override
        {
            add(loc, event);
        }
        virtual void event(const otf2::definition::location& loc,
                           const otf2::event::mpi_isend_complete& event) override
        {
            add(loc, event);
        }
        virtual void event(const otf2::definition::location& loc,
                           const otf2::event::mpi_ireceive& event) override
        {
            add(loc, event);
        }
        virtual void event(const otf2::definition::location& loc,
                           const otf2::event::mpi_ireceive_request& event) override
        {
            add(loc, event);
        }
        virtual void event(const otf2::definition::location& loc,
                           const otf2::event::mpi_request_test& event) override
        {
            add(loc, event);
        }
        virtual void event(const otf2::definition::location& loc,
                           const otf2::event::mpi_request_cancelled& event) override
        {
            add(loc, event);
        }
        virtual void event(const otf2::definition::location& loc,
                           const otf2::event::mpi_collective_begin& event) override
        {
            add(loc, event);
        }
        virtual void event(const otf2::definition::location& loc,
                           const otf2::event::mpi_collective_end& event) override
        {
            add(loc, event);
        }

        virtual void event(const otf2::definition::location& loc,
                           const otf2::event::parameter_string& event) override
        {
            add(loc, event);
        }
        virtual void event(const otf2::definition::location& loc,
                           const otf2::event::parameter_int& event) override
        {
            add(loc, event);
        }
        virtual void event(const otf2::definition::location& loc,
                           const otf2::event::parameter_unsigned_int& event) override
        {
            add(loc, event);
        }

        virtual void event(const otf2::definition::location& loc,
                           const otf2::event::thread_fork& event) override
        {
            add(loc, event);
        }
        virtual void event(const otf2::definition::location& loc,
                           const otf2::event::thread_join& event) override
        {
            add(loc, event);
        }
        virtual void event(const otf2::definition::location& loc,
                           const otf2::event::thread_team_begin& event) override
        {
            add(loc, event);
        }
        virtual void event(const otf2::definition::location& loc,
                           const otf2::event::thread_team_end& event) override
        {
            add(loc, event);
        }
        virtual void event(const otf2::definition::location& loc,
                           const otf2::event::thread_acquire_lock& event) override
        {
            add(loc, event);
        }
        virtual void event(const otf2::definition::location& loc,
                           const otf2::event::thread_release_lock& event) override
        {
            add(loc, event);
        }
        virtual void event(const otf2::definition::location& loc,
                           const otf2::event::thread_task_create& event) override
        {
            add(loc, event);
        }
        virtual void event(const otf2::definition::location& loc,
                           const otf2::event::thread_task_switch& event) override
        {
            add(loc, event);
        }
        virtual void event(const otf2::definition::location& loc,
                           const otf2::event::thread_task_complete& event) override
        {
            add(loc, event);
        }

        template <typename Event>
        void add(const otf2::definition::location& loc, const Event& event)
        {
            if (nodes_.size() > 0)
            {
                nodes_.emplace_back(loc, event);
            }
            else
            {
                callback_.event(loc, event);
            }
        }

        void add(const otf2::definition::location& loc,
                 const otf2::event::mpi_ireceive_request& event)
        {
            nodes_.emplace_back(loc, event);
        }

        void add(const otf2::definition::location& loc, const otf2::event::mpi_ireceive& event)
        {
            for (auto& node : nodes_)
            {
                if (node.type == otf2::common::event_type::mpi_ireceive_request &&
                    node.location == loc)
                {
                    otf2::event::mpi_ireceive_request* current =
                        static_cast<otf2::event::mpi_ireceive_request*>(node.event);

                    if (current->request_id() == event.request_id())
                    {
                        current->attach_data(event.sender(), event.comm_, event.msg_tag(),
                                             event.msg_length());
                        node.completed = true;
                        break;
                    }
                }
            }

            nodes_.emplace_back(loc, event);

            process_data();
        }

    public:
        // Global definitions
        virtual void definition(const otf2::definition::attribute& def) override
        {
            callback_.definition(def);
        }

        virtual void definition(const otf2::definition::comm& def) override
        {
            callback_.definition(def);
        }

        virtual void definition(const otf2::definition::locations_group& def) override
        {
            callback_.definition(def);
        }

        virtual void definition(const otf2::definition::regions_group& def) override
        {
            callback_.definition(def);
        }

        // virtual void definition(otf2::definition::metric_group& def) override {}

        virtual void definition(const otf2::definition::comm_locations_group& def) override
        {
            callback_.definition(def);
        }

        virtual void definition(const otf2::definition::comm_group& def) override
        {
            callback_.definition(def);
        }

        virtual void definition(const otf2::definition::comm_self_group& def) override
        {
            callback_.definition(def);
        }

        virtual void definition(const otf2::definition::location& def) override
        {
            callback_.definition(def);
        }

        virtual void definition(const otf2::definition::location_group& def) override
        {
            callback_.definition(def);
        }

        virtual void definition(const otf2::definition::parameter& def) override
        {
            callback_.definition(def);
        }

        virtual void definition(const otf2::definition::region& def) override
        {
            callback_.definition(def);
        }

        virtual void definition(const otf2::definition::string& def) override
        {
            callback_.definition(def);
        }

        virtual void definition(const otf2::definition::system_tree_node& def) override
        {
            callback_.definition(def);
        }

        virtual void definition(const otf2::definition::clock_properties& def) override
        {
            callback_.definition(def);
        }

        virtual void definition(const otf2::definition::metric_member& def) override
        {
            callback_.definition(def);
        }

        virtual void definition(const otf2::definition::metric_class& def) override
        {
            callback_.definition(def);
        }

        virtual void definition(const otf2::definition::metric_instance& def) override
        {
            callback_.definition(def);
        }

        virtual void definition(const otf2::definition::system_tree_node_property& def) override
        {
            callback_.definition(def);
        }

        virtual void definition(const otf2::definition::location_group_property& def) override
        {
            callback_.definition(def);
        }

        virtual void definition(const otf2::definition::location_property& def) override
        {
            callback_.definition(def);
        }

        virtual void definition(const otf2::definition::io_regular_file& def) override
        {
            callback_.definition(def);
        }

        virtual void definition(const otf2::definition::io_directory& def) override
        {
            callback_.definition(def);
        }

        virtual void definition(const otf2::definition::io_paradigm& def) override
        {
            callback_.definition(def);
        }

        virtual void definition(const otf2::definition::io_handle& def) override
        {
            callback_.definition(def);
        }

        virtual void definition(const otf2::definition::io_file_property& def) override
        {
            callback_.definition(def);
        }

        virtual void definition(const otf2::definition::io_pre_created_handle_state& def) override
        {
            callback_.definition(def);
        }

    public:
        virtual void definitions_done(const otf2::reader::reader& rdr) override
        {
            callback_.definitions_done(rdr);
        }

        virtual void events_done(const otf2::reader::reader& rdr) override
        {
            callback_.events_done(rdr);
        }

    private:
        std::deque<detail::buffer_node> nodes_;
        otf2::reader::callback& callback_;
    };
} // namespace event
} // namespace otf2

#endif // INCLUDE_OTF2XX_EVENT_BUFFER_HPP
