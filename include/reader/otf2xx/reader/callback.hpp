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

#ifndef INCLUDE_OTF2XX_READER_CALLBACK_HPP
#define INCLUDE_OTF2XX_READER_CALLBACK_HPP

#include <otf2xx/reader/fwd.hpp>

#include <otf2xx/definition/fwd.hpp>
#include <otf2xx/event/fwd.hpp>

#include <otf2xx/definition/calling_context.hpp>
#include <otf2xx/event/calling_context_enter.hpp>
#include <otf2xx/event/calling_context_leave.hpp>
#include <otf2xx/event/enter.hpp>
#include <otf2xx/event/leave.hpp>

#include <otf2xx/chrono/time_point.hpp>
#include <otf2xx/common.hpp>

#define NOT_IMPLEMENTED_YET

namespace otf2
{
namespace reader
{

    /**
     * \brief base class for otf2 reader callbacks.
     *
     * Inherit from this class, but be careful!
     * If you override one function, all other member functions names will be
     * hidden by your override.
     *
     * Fix this be inserting the following two lines in your class:
     *
     * using otf2::reader::callback::event;
     * using otf2::reader::callback::definition;
     *
     * Make sure to adopt to your base class if your class inherit indirectly
     * from this class.
     *
     * Tip: Add the keyword override to your methodes you intent to override.
     *      This will prevent you from accidentally defining a methode with a
     *      different signature.
     *
     **/
    class callback
    {
    public:
        // clang-format off

        // events
        virtual void event(const otf2::definition::location&, const otf2::event::buffer_flush&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::enter&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::leave&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::measurement&) {}

        virtual void event(const otf2::definition::location&, const otf2::event::metric&) {}

        virtual void event(const otf2::definition::location&, const otf2::event::mpi_send&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::mpi_receive&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::mpi_isend_request&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::mpi_isend_complete&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::mpi_ireceive_complete&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::mpi_ireceive_request&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::mpi_request_test&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::mpi_request_cancelled&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::mpi_collective_begin&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::mpi_collective_end&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::non_blocking_collective_request&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::non_blocking_collective_complete&) {}
        
        virtual void event(const otf2::definition::location&, const otf2::event::comm_create&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::comm_destroy&) {}

        virtual void event(const otf2::definition::location&, const otf2::event::parameter_string&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::parameter_int&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::parameter_unsigned_int&) {}

        virtual void event(const otf2::definition::location& loc, const otf2::event::calling_context_enter& evt)
        {
            event(loc, otf2::event::enter(evt.attribute_list().clone().get(), evt.timestamp(), evt.calling_context().region()));
        }

        virtual void event(const otf2::definition::location& loc, const otf2::event::calling_context_leave& evt)
        {
            event(loc, otf2::event::leave(evt.attribute_list().clone().get(), evt.timestamp(), evt.calling_context().region()));
        }

        virtual void event(const otf2::definition::location&, const otf2::event::calling_context_sample&) {}

        virtual void event(const otf2::definition::location&, const otf2::event::thread_fork&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::thread_join&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::thread_team_begin&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::thread_team_end&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::thread_acquire_lock&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::thread_release_lock&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::thread_task_create&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::thread_task_switch&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::thread_task_complete&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::thread_create&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::thread_begin&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::thread_wait&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::thread_end&) {}

        virtual void event(const otf2::definition::location&, const otf2::event::rma_acquire_lock&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::rma_atomic&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::rma_collective_begin&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::rma_collective_end&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::rma_get&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::rma_group_sync&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::rma_op_complete_blocking&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::rma_op_complete_non_blocking&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::rma_op_complete_remote&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::rma_op_test&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::rma_put&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::rma_release_lock&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::rma_request_lock&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::rma_sync&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::rma_try_lock&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::rma_wait_change&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::rma_win_create&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::rma_win_destroy&) {}

        virtual void event(const otf2::definition::location&, const otf2::event::io_create_handle&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::io_destroy_handle&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::io_duplicate_handle&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::io_seek&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::io_change_status_flag&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::io_delete_file&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::io_operation_begin&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::io_operation_test&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::io_operation_issued&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::io_operation_cancelled&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::io_operation_complete&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::io_acquire_lock&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::io_release_lock&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::io_try_lock&) {}

        virtual void event(const otf2::definition::location&, const otf2::event::program_begin&) {}
        virtual void event(const otf2::definition::location&, const otf2::event::program_end&) {}

        virtual void event(const otf2::definition::location&, const otf2::event::unknown&) {}


        // Global definitions
        virtual void definition(const otf2::definition::attribute&) {}
        virtual void definition(const otf2::definition::comm&) {}
        virtual void definition(const otf2::definition::inter_comm&) {}

        virtual void definition(const otf2::definition::locations_group&) {}
        virtual void definition(const otf2::definition::regions_group&) {}
        //virtual void definition(const otf2::definition::metric_group&) {}
        virtual void definition(const otf2::definition::comm_locations_group&) {}
        virtual void definition(const otf2::definition::comm_group&) {}
        virtual void definition(const otf2::definition::comm_self_group&) {}

        virtual void definition(const otf2::definition::location&) {}
        virtual void definition(const otf2::definition::location_group&) {}
        virtual void definition(const otf2::definition::parameter&) {}
        virtual void definition(const otf2::definition::region&) {}
        virtual void definition(const otf2::definition::string&) {}
        virtual void definition(const otf2::definition::system_tree_node&) {}
        virtual void definition(const otf2::definition::system_tree_node_domain&) {}
        virtual void definition(const otf2::definition::clock_properties&) {}

        virtual void definition(const otf2::definition::call_path&) {}
        virtual void definition(const otf2::definition::call_path_parameter&) {}
        virtual void definition(const otf2::definition::source_code_location&) {}
        virtual void definition(const otf2::definition::calling_context&) {}
        virtual void definition(const otf2::definition::interrupt_generator&) {}

        virtual void definition(const otf2::definition::rma_win&) {}

        virtual void definition(const otf2::definition::io_regular_file&) {}
        virtual void definition(const otf2::definition::io_directory&) {}
        virtual void definition(const otf2::definition::io_handle&) {}
        virtual void definition(const otf2::definition::io_paradigm&) {}
        virtual void definition(const otf2::definition::io_pre_created_handle_state&) {}

        virtual void definition(const otf2::definition::metric_class&) {}
        virtual void definition(const otf2::definition::metric_class_recorder&) {}
        virtual void definition(const otf2::definition::metric_member&) {}
        virtual void definition(const otf2::definition::metric_instance&) {}

        virtual void definition(const otf2::definition::cart_dimension&) {}
        virtual void definition(const otf2::definition::cart_topology&) {}
        virtual void definition(const otf2::definition::cart_coordinate&) {}

        virtual void definition(const otf2::definition::location_property&) {}
        virtual void definition(const otf2::definition::location_group_property&) {}
        virtual void definition(const otf2::definition::system_tree_node_property&) {}
        virtual void definition(const otf2::definition::calling_context_property&) {}
        virtual void definition(const otf2::definition::io_file_property&) {}

        virtual void definition(const otf2::definition::unknown&) {}

        // clang-format on

    public:
        /**
         * \brief definitions done callback
         *
         * This callback gets called after the otf::reader::reader has finished
         * reading all definition records
         */
        virtual void definitions_done(const otf2::reader::reader&)
        {
        }

        /**
         * \brief events done callback
         *
         * This callback gets called after the otf::reader::reader finished
         * reading all events and definitions in the trace file.
         */
        virtual void events_done(const otf2::reader::reader&)
        {
        }

    public:
        // pure virtual
        virtual ~callback() = 0;
    };

    inline callback::~callback()
    {
    }
} // namespace reader
} // namespace otf2

#endif // INCLUDE_OTF2XX_READER_CALLBACK_HPP
