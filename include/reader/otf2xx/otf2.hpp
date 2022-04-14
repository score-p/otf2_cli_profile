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

#ifndef INCLUDE_OTF2XX_OTF2_HPP
#define INCLUDE_OTF2XX_OTF2_HPP

#include <otf2xx/fwd.hpp>

#include <otf2xx/common.hpp>

#include <otf2xx/writer/archive.hpp>
#include <otf2xx/writer/global.hpp>
#include <otf2xx/writer/local.hpp>

#include <otf2xx/reader/reader.hpp>

#include <otf2xx/definition/definitions.hpp>
#include <otf2xx/event/events.hpp>

/*!
 \page otf2xx Open Trace Format 2 C++ binding

 \tableofcontents

 \section defs Definitions

 Where are several definitons in OTF2. Yet not all definitions are accessible in otf2xx.

 Currently available are the following:

 \li \ref otf2::definition::attribute
 \li \ref otf2::definition::clock_properties
 \li \ref otf2::definition::comm
 \li \ref otf2::definition::group
 \li \ref otf2::definition::location
 \li \ref otf2::definition::location_group
 \li \ref otf2::definition::metric_class
 \li \ref otf2::definition::metric_instance
 \li \ref otf2::definition::metric_member
 \li \ref otf2::definition::parameter
 \li \ref otf2::definition::region
 \li \ref otf2::definition::string
 \li \ref otf2::definition::system_tree_node
 \li \ref otf2::definition::io_file
 \li \ref otf2::definition::io_handle

 For convenience there is a container class similar to a std::map:

 \li \ref otf2::definition::container

 \section evts Events

 Where are several events in OTF2. Yet not all events are accessible in otf2xx.

 Currently available are the following:

 \li \ref otf2::event::buffer_flush
 \li \ref otf2::event::enter
 \li \ref otf2::event::leave
 \li \ref otf2::event::measurement
 \li \ref otf2::event::metric
 \li \ref otf2::event::mpi_collective_begin
 \li \ref otf2::event::mpi_collective_end
 \li \ref otf2::event::mpi_send
 \li \ref otf2::event::mpi_receive
 \li \ref otf2::event::mpi_isend
 \li \ref otf2::event::mpi_isend_complete
 \li \ref otf2::event::mpi_ireceive
 \li \ref otf2::event::mpi_ireceive_request
 \li \ref otf2::event::mpi_request_cancelled
 \li \ref otf2::event::mpi_request_test
 \li \ref otf2::event::parameter_int
 \li \ref otf2::event::parameter_unsigned_int
 \li \ref otf2::event::parameter_string
 \li \ref otf2::event::thread_acquire_lock
 \li \ref otf2::event::thread_fork
 \li \ref otf2::event::thread_join
 \li \ref otf2::event::thread_release_lock
 \li \ref otf2::event::thread_task_complete
 \li \ref otf2::event::thread_task_create
 \li \ref otf2::event::thread_task_switch
 \li \ref otf2::event::thread_team_begin
 \li \ref otf2::event::thread_team_end

 A buffer for events used to read ahead some information for mpi_ireceive_request.

 \li \ref otf2::event::buffer

 \section chrono Timestamp handling

 For time we have got some basic ideas. First, we have time points, which are
 specific points on a timeline. We have durations, which are the distance
 between to time points. And we have a clock, which gives us a point of reference
 on the timeline - called epoch - and a precision.

 \li \ref otf2::chrono::clock
 \li \ref otf2::chrono::time_point
 \li \ref otf2::chrono::duration
 \li \ref otf2::chrono::ticks

 */

#endif // INCLUDE_OTF2XX_OTF2_HPP
