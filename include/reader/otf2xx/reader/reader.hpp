/*
 * This file is part of otf2xx (https://github.com/tud-zih-energy/otf2xx)
 * otf2xx - A wrapper for the Open Trace Format 2 library
 *
 * Copyright (c) 2013-2021, Technische Universität Dresden, Germany
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

#ifndef INCLUDE_OTF2XX_READER_READER_HPP
#define INCLUDE_OTF2XX_READER_READER_HPP

#include <otf2xx/chrono/convert.hpp>
#include <otf2xx/definition/definitions.hpp>
#include <otf2xx/event/buffer.hpp>
#include <otf2xx/exception.hpp>
#include <otf2xx/reader/callback.hpp>
#include <otf2xx/reader/fwd.hpp>
#include <otf2xx/reader/util.hpp>
#include <otf2xx/registry.hpp>

#include "tracereader.h"

#include <otf2/OTF2_GlobalDefReader.h>
#include <otf2/OTF2_GlobalEvtReader.h>
#include <otf2/OTF2_Reader.h>

#ifdef OTF2XX_HAS_MPI
#include <mpi.h>
#include <otf2/OTF2_MPI_Collectives.h>
#endif

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace otf2
{
namespace reader
{

    /**
     * \brief the class for reading in trace files
     *
     */
    class reader : public TraceReader
    {
        template <typename Definition>
        using map_type = otf2::definition::container<Definition>;

    public:
        /**
         * \brief constructor
         * Triggers initialization done callback
         * \param name the path to the otf2 anchor file. Usually traces.otf2 in a score-p trace
         * \throws if it can't open the trace file
         */
        reader(const std::string& name)
        : rdr(OTF2_Reader_Open(name.c_str())), definition_files_(rdr), event_files_(rdr),
          callback_(nullptr)
        {
            if (rdr == nullptr)
                make_exception("Couldn't open the trace file: ", name);

            check(OTF2_Reader_SetSerialCollectiveCallbacks(rdr), "Couldn't set serial callbacks");
        }

#ifdef OTF2XX_HAS_MPI
        reader(const std::string& name, MPI_Comm comm)
        : rdr(OTF2_Reader_Open(name.c_str())), definition_files_(rdr), event_files_(rdr),
          callback_(nullptr)
        {
            if (rdr == nullptr)
                make_exception("Couldn't open the trace file: ", name);

            check(OTF2_MPI_Reader_SetCollectiveCallbacks(rdr, comm),
                  "Couldn't set collective callbacks");
        }
#endif

        /**
         * \brief triggers the read of all definition records
         *
         * For each definition the callback is called.
         *
         * After all definitions are read, the method \ref
         *otf2::reader::callback::definitions_done() is called.
         */
        bool readDefinitions(AllData& alldata)
        {
            def_rdr = OTF2_Reader_GetGlobalDefReader(rdr);

            register_definition_callbacks();

            uint64_t definitions_read = 0;
            OTF2_Reader_ReadAllGlobalDefinitions(rdr, def_rdr, &definitions_read);

            callback().definitions_done(*this);

            return true;
        }

        /**
         * \brief tells the reader, that it should read the events of the given location
         *
         * Call this method for every location you want to have the events read.
         *
         * Defaults to no locations.
         *
         * \param [in] location the location, for which the events should be read
         */
        void register_location(const otf2::definition::location& location)
        {
            registered_locations_.emplace_back(location);
        }

        /**
         * \brief triggers the read of all event records
         *
         * For each event the callback is called.
         *
         * After all events are read, the method \ref otf2::reader::callback::events_done() is
         *called.
         */
        bool read_events(AllData& alldata)
        {
            for (auto& location : registered_locations_)
            {
                check(OTF2_Reader_SelectLocation(rdr, location.ref()), "Couldn't select location ",
                      location, " for reading events.");
            }

            definition_files_.open();
            event_files_.open();

            for (auto& location : registered_locations_)
            {
                // read definition files, if they are present
                if (definition_files_.are_open())
                {
                    OTF2_DefReader* def_reader = OTF2_Reader_GetDefReader(rdr, location.ref());

                    uint64_t definitions_read = 0;
                    check(OTF2_Reader_ReadAllLocalDefinitions(rdr, def_reader, &definitions_read),
                          "Couldn't read local definitions for location ", location,
                          " from trace file");

                    OTF2_Reader_CloseDefReader(rdr, def_reader);
                }

                OTF2_Reader_GetEvtReader(rdr, location.ref());
            }

            definition_files_.close();

            if (!registered_locations_.empty())
            {
                evt_rdr = OTF2_Reader_GetGlobalEvtReader(rdr);
                register_event_callbacks();

                uint64_t events_read = 0;
                check(OTF2_Reader_ReadAllGlobalEvents(rdr, evt_rdr, &events_read),
                      "Couldn't read events from trace file");
            }

            check(OTF2_Reader_CloseGlobalEvtReader(rdr, evt_rdr),
                  "Couldn't close global event reader");

            event_files_.close();

            callback().events_done(*this);

            return true;
        }

        bool readStatistics(AllData& alldata) { return true; }
        bool initialize(Alldata& alldata) { return true; }
        bool close() { return true; }

    private:
        /**
         * \internal
         *
         * \brief prepares the otf2 callback struct for definition callbacks
         */
        void register_definition_callbacks()
        {
            OTF2_GlobalDefReaderCallbacks* global_def_callbacks =
                OTF2_GlobalDefReaderCallbacks_New();

            // clang-format off

            check(OTF2_GlobalDefReaderCallbacks_SetAttributeCallback(global_def_callbacks, detail::definition::global::attribute), "Couldn't set attribute callback handler");
            check(OTF2_GlobalDefReaderCallbacks_SetCallpathCallback(global_def_callbacks, detail::definition::global::call_path), "Couldn't set call_path callback handler");
            check(OTF2_GlobalDefReaderCallbacks_SetCallpathParameterCallback(global_def_callbacks, detail::definition::global::call_path_parameter), "Couldn't set call_path_parameter callback handler");
            check(OTF2_GlobalDefReaderCallbacks_SetClockPropertiesCallback(global_def_callbacks, detail::definition::global::clock_properties), "Couldn't set clock_properties callback handler");
            check(OTF2_GlobalDefReaderCallbacks_SetCommCallback(global_def_callbacks, detail::definition::global::comm), "Couldn't set comm callback handler");
            check(OTF2_GlobalDefReaderCallbacks_SetInterCommCallback(global_def_callbacks, detail::definition::global::inter_comm), "Couldn't set inter_comm callback handler");
            check(OTF2_GlobalDefReaderCallbacks_SetGroupCallback(global_def_callbacks, detail::definition::global::group), "Couldn't set group callback handler");
            check(OTF2_GlobalDefReaderCallbacks_SetLocationCallback(global_def_callbacks, otf2::reader::detail::definition::global::location), "Couldn't set location callback handler");
            check(OTF2_GlobalDefReaderCallbacks_SetLocationGroupCallback(global_def_callbacks, detail::definition::global::location_group), "Couldn't set location_group callback handler");
            check(OTF2_GlobalDefReaderCallbacks_SetMetricClassCallback (global_def_callbacks, detail::definition::global::metric_class), "Couldn't set metric class callback handler");
            check(OTF2_GlobalDefReaderCallbacks_SetMetricClassRecorderCallback (global_def_callbacks, detail::definition::global::metric_class_recorder), "Couldn't set metric class recorder callback handler");
            check(OTF2_GlobalDefReaderCallbacks_SetMetricInstanceCallback (global_def_callbacks, detail::definition::global::metric_instance), "Couldn't set metric instance callback handler");
            check(OTF2_GlobalDefReaderCallbacks_SetMetricMemberCallback (global_def_callbacks, detail::definition::global::metric_member), "Couldn't set metric member callback handler");
            check(OTF2_GlobalDefReaderCallbacks_SetParameterCallback(global_def_callbacks, detail::definition::global::parameter), "Couldn't set parameter callback handler");
            check(OTF2_GlobalDefReaderCallbacks_SetRegionCallback(global_def_callbacks, detail::definition::global::region), "Couldn't set region callback handler");
            check(OTF2_GlobalDefReaderCallbacks_SetRmaWinCallback (global_def_callbacks, detail::definition::global::rma_win), "Couldn't set rma_win callback handler");
            check(OTF2_GlobalDefReaderCallbacks_SetStringCallback(global_def_callbacks, detail::definition::global::string), "Couldn't set string callback handler");
            check(OTF2_GlobalDefReaderCallbacks_SetSystemTreeNodeCallback(global_def_callbacks, detail::definition::global::system_tree_node), "Couldn't set system_tree_node callback handler");
            check(OTF2_GlobalDefReaderCallbacks_SetSystemTreeNodeDomainCallback (global_def_callbacks, detail::definition::global::system_tree_node_domain), "Couldn't set system_tree_node_domain callback handler");

            check(OTF2_GlobalDefReaderCallbacks_SetSystemTreeNodePropertyCallback (global_def_callbacks, detail::definition::global::system_tree_node_property), "Couldn't set attribute callback handler");
            check(OTF2_GlobalDefReaderCallbacks_SetLocationPropertyCallback (global_def_callbacks, detail::definition::global::location_property), "Couldn't set attribute callback handler");
            check(OTF2_GlobalDefReaderCallbacks_SetLocationGroupPropertyCallback (global_def_callbacks, detail::definition::global::location_group_property), "Couldn't set attribute callback handler");

            check(OTF2_GlobalDefReaderCallbacks_SetSourceCodeLocationCallback (global_def_callbacks, detail::definition::global::source_code_location), "Couldn't set source code location callback handler");
            check(OTF2_GlobalDefReaderCallbacks_SetCallingContextCallback (global_def_callbacks, detail::definition::global::calling_context), "Couldn't set calling context callback handler");
            check(OTF2_GlobalDefReaderCallbacks_SetCallingContextPropertyCallback (global_def_callbacks, detail::definition::global::calling_context_property), "Couldn't set calling context property callback handler");
            check(OTF2_GlobalDefReaderCallbacks_SetInterruptGeneratorCallback (global_def_callbacks, detail::definition::global::interrupt_generator), "Couldn't set interrupt generator callback handler");

            check(OTF2_GlobalDefReaderCallbacks_SetIoRegularFileCallback (global_def_callbacks, detail::definition::global::io_regular_file), "Couldn't set io file callback handler");
            check(OTF2_GlobalDefReaderCallbacks_SetIoDirectoryCallback (global_def_callbacks, detail::definition::global::io_directory), "Couldn't set io directory callback handler");
            check(OTF2_GlobalDefReaderCallbacks_SetIoHandleCallback (global_def_callbacks, detail::definition::global::io_handle), "Couldn't set io handle callback handler");
            check(OTF2_GlobalDefReaderCallbacks_SetIoParadigmCallback (global_def_callbacks, detail::definition::global::io_paradigm), "Couldn't set io paradigm callback handler");
            check(OTF2_GlobalDefReaderCallbacks_SetIoFilePropertyCallback (global_def_callbacks, detail::definition::global::io_file_property), "Couldn't set io file properties callback handler");
            check(OTF2_GlobalDefReaderCallbacks_SetIoPreCreatedHandleStateCallback (global_def_callbacks, detail::definition::global::io_pre_created_handle_state), "Couldn't set io pre created handle state callback handler");

            check(OTF2_GlobalDefReaderCallbacks_SetCartTopologyCallback (global_def_callbacks, detail::definition::global::cart_topology), "Couldn't set cart topology callback handler");
            check(OTF2_GlobalDefReaderCallbacks_SetCartDimensionCallback (global_def_callbacks, detail::definition::global::cart_dimension), "Couldn't set cart dimension callback handler");

            check(OTF2_GlobalDefReaderCallbacks_SetUnknownCallback(global_def_callbacks, detail::definition::global::unknown), "Couldn't set unknown callback handler");

            // clang-format on

            check(OTF2_Reader_RegisterGlobalDefCallbacks(rdr, def_rdr, global_def_callbacks,
                                                         static_cast<void*>(this)),
                  "Couldn't register callbacks in reader");
            OTF2_GlobalDefReaderCallbacks_Delete(global_def_callbacks);
        }

        /**
         * \internal
         *
         * \brief prepares the otf2 callback struct for event callbacks
         */
        void register_event_callbacks()
        {
            OTF2_GlobalEvtReaderCallbacks* event_callbacks = OTF2_GlobalEvtReaderCallbacks_New();

            // clang-format off
            check(OTF2_GlobalEvtReaderCallbacks_SetBufferFlushCallback(event_callbacks, detail::event::buffer_flush), "Couldn't set buffer_flush event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetEnterCallback(event_callbacks, detail::event::enter), "Couldn't set enter event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetLeaveCallback(event_callbacks, detail::event::leave), "Couldn't set leave event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetMeasurementOnOffCallback(event_callbacks, detail::event::measurement), "Couldn't set measurement event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetMetricCallback (event_callbacks, detail::event::metric), "Couldn't set metric event callback");

            check(OTF2_GlobalEvtReaderCallbacks_SetMpiCollectiveBeginCallback(event_callbacks, detail::event::mpi_collective_begin), "Couldn't set mpi collective begin event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetMpiCollectiveEndCallback(event_callbacks, detail::event::mpi_collective_end), "Couldn't set mpi collective end event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetMpiIrecvCallback(event_callbacks, detail::event::mpi_irecv), "Couldn't set mpi_irecv event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetMpiIrecvRequestCallback(event_callbacks, detail::event::mpi_irecv_request), "Couldn't set mpi_irecv_request event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetMpiIsendCallback(event_callbacks, detail::event::mpi_isend), "Couldn't set mpi_isend event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetMpiIsendCompleteCallback(event_callbacks, detail::event::mpi_isend_complete), "Couldn't set mpi_isend_complete event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetMpiRecvCallback(event_callbacks, detail::event::mpi_recv), "Couldn't set mpi_recv event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetMpiRequestCancelledCallback(event_callbacks, detail::event::mpi_request_cancelled), "Couldn't set mpi_request_cancelled event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetMpiRequestTestCallback(event_callbacks, detail::event::mpi_request_test), "Couldn't set mpi_request_test event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetMpiSendCallback(event_callbacks, detail::event::mpi_send), "Couldn't set mpi_send event callback");

            check(OTF2_GlobalEvtReaderCallbacks_SetParameterIntCallback(event_callbacks, detail::event::parameter_int), "Couldn't set parameter_int event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetParameterStringCallback(event_callbacks, detail::event::parameter_string), "Couldn't set parameter_string event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetParameterUnsignedIntCallback(event_callbacks, detail::event::parameter_unsigned_int), "Couldn't set parameter_unsigned_int event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetCallingContextEnterCallback(event_callbacks, detail::event::calling_context_enter), "Couldn't set calling_context_enter event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetCallingContextLeaveCallback(event_callbacks, detail::event::calling_context_leave), "Couldn't set calling_context_leave event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetCallingContextSampleCallback(event_callbacks, detail::event::calling_context_sample), "Couldn't set calling_context_sample event callback");

            check(OTF2_GlobalEvtReaderCallbacks_SetRmaAcquireLockCallback (event_callbacks, detail::event::rma_acquire_lock), "Couldn't set rma_acquire_lock event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetRmaAtomicCallback (event_callbacks, detail::event::rma_atomic), "Couldn't set rma_atomic event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetRmaCollectiveBeginCallback (event_callbacks, detail::event::rma_collective_begin), "Couldn't set rma_collective_begin event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetRmaCollectiveEndCallback (event_callbacks, detail::event::rma_collective_end), "Couldn't set rma_collective_end event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetRmaGetCallback (event_callbacks, detail::event::rma_get), "Couldn't set rma_get event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetRmaGroupSyncCallback (event_callbacks, detail::event::rma_group_sync), "Couldn't set rma_group_sync event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetRmaOpCompleteBlockingCallback (event_callbacks, detail::event::rma_op_complete_blocking), "Couldn't set rma_op_complete_blocking event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetRmaOpCompleteNonBlockingCallback (event_callbacks, detail::event::rma_op_complete_non_blocking), "Couldn't set rma_op_complete_non_blocking event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetRmaOpCompleteRemoteCallback (event_callbacks, detail::event::rma_op_complete_remote), "Couldn't set rma_op_complete_remote event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetRmaOpTestCallback (event_callbacks, detail::event::rma_op_test), "Couldn't set rma_op_test event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetRmaPutCallback (event_callbacks, detail::event::rma_put), "Couldn't set rma_put event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetRmaReleaseLockCallback (event_callbacks, detail::event::rma_release_lock), "Couldn't set rma_release_lock event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetRmaRequestLockCallback (event_callbacks, detail::event::rma_request_lock), "Couldn't set rma_request_lock event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetRmaSyncCallback (event_callbacks, detail::event::rma_sync), "Couldn't set rma_sync event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetRmaTryLockCallback (event_callbacks, detail::event::rma_try_lock), "Couldn't set rma_try_lock event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetRmaWaitChangeCallback (event_callbacks, detail::event::rma_wait_change), "Couldn't set rma_wait_change event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetRmaWinCreateCallback (event_callbacks, detail::event::rma_win_create), "Couldn't set rma_win_create event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetRmaWinDestroyCallback (event_callbacks, detail::event::rma_win_destroy), "Couldn't set rma_win_destroy event callback");

            check(OTF2_GlobalEvtReaderCallbacks_SetThreadAcquireLockCallback(event_callbacks, detail::event::thread_acquire_lock), "Couldn't set thread_acquire_lock event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetThreadForkCallback(event_callbacks, detail::event::thread_fork), "Couldn't set thread_fork event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetThreadJoinCallback(event_callbacks, detail::event::thread_join), "Couldn't set thread_join event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetThreadReleaseLockCallback(event_callbacks, detail::event::thread_release_lock), "Couldn't set thread_release_lock event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetThreadTaskCompleteCallback(event_callbacks, detail::event::thread_task_complete), "Couldn't set thread_task_complete event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetThreadTaskCreateCallback(event_callbacks, detail::event::thread_task_create), "Couldn't set thread_task_create event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetThreadTaskSwitchCallback(event_callbacks, detail::event::thread_task_switch), "Couldn't set thread_task_switch event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetThreadTeamBeginCallback(event_callbacks, detail::event::thread_team_begin), "Couldn't set thread_team_begin event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetThreadTeamEndCallback(event_callbacks, detail::event::thread_team_end), "Couldn't set thread_team_end event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetThreadCreateCallback(event_callbacks, detail::event::thread_create), "Couldn't set thread_create event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetThreadBeginCallback(event_callbacks, detail::event::thread_begin), "Couldn't set thread_begin event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetThreadWaitCallback(event_callbacks, detail::event::thread_wait), "Couldn't set thread_wait event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetThreadEndCallback(event_callbacks, detail::event::thread_end), "Couldn't set thread_end event callback");

            check(OTF2_GlobalEvtReaderCallbacks_SetIoCreateHandleCallback(event_callbacks, detail::event::io_create_handle), "Couldn't set io_create_handle event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetIoDestroyHandleCallback(event_callbacks, detail::event::io_destroy_handle), "Couldn't set io_destroy_handle event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetIoDuplicateHandleCallback(event_callbacks, detail::event::io_duplicate_handle), "Couldn't set io_duplicate_handle event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetIoSeekCallback(event_callbacks, detail::event::io_seek), "Couldn't set io_seek event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetIoChangeStatusFlagsCallback(event_callbacks, detail::event::io_change_status_flag), "Couldn't set io_change_status_flag event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetIoDeleteFileCallback(event_callbacks, detail::event::io_delete_file), "Couldn't set io_delete_file event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetIoOperationBeginCallback(event_callbacks, detail::event::io_operation_begin), "Couldn't set io_operation_begin event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetIoOperationTestCallback(event_callbacks, detail::event::io_operation_test), "Couldn't set io_operation_test event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetIoOperationIssuedCallback(event_callbacks, detail::event::io_operation_issued), "Couldn't set io_operation_issued event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetIoOperationCancelledCallback(event_callbacks, detail::event::io_operation_cancelled), "Couldn't set io_operation_cancelled event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetIoOperationCompleteCallback(event_callbacks, detail::event::io_operation_complete), "Couldn't set io_operation_complete event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetIoAcquireLockCallback(event_callbacks, detail::event::io_acquire_lock), "Couldn't set io_acquire_lock event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetIoReleaseLockCallback(event_callbacks, detail::event::io_release_lock), "Couldn't set io_release_lock event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetIoTryLockCallback(event_callbacks, detail::event::io_try_lock), "Couldn't set io_try_lock event callback");

            check(OTF2_GlobalEvtReaderCallbacks_SetProgramBeginCallback(event_callbacks, detail::event::program_begin), "Couldn't set program_begin event callback");
            check(OTF2_GlobalEvtReaderCallbacks_SetProgramEndCallback(event_callbacks, detail::event::program_end), "Couldn't set program_end event callback");

            check(OTF2_GlobalEvtReaderCallbacks_SetUnknownCallback(event_callbacks, detail::event::unknown), "Couldn't set unknown event callback");

            // clang-format on

            check(OTF2_Reader_RegisterGlobalEvtCallbacks(rdr, evt_rdr, event_callbacks,
                                                         static_cast<void*>(this)),
                  "Couldn't register event callbacks");
            OTF2_GlobalEvtReaderCallbacks_Delete(event_callbacks);
        }

    public:
        /**
         * \brief returns the number of locations
         */
        std::uint64_t num_locations() const
        {
            std::uint64_t result;

            check(OTF2_Reader_GetNumberOfLocations(rdr, &result),
                  "Couldn't get the number of locations from archive");

            return result;
        }

    public:
        /**
         * \brief returns the callback instance
         *
         * \return the instance of the callback
         */
        otf2::reader::callback& callback()
        {
            assert(has_callback());

            return *callback_;
        }

        /**
         * \brief returns true, if a callback was set
         *
         * \return whether a callback was set
         */
        bool has_callback() const
        {
            return callback_ != nullptr;
        }

        /**
         * \brief set the given callback as callback for the reader
         * \param callback an otf2::reader::callback instance
         * \param buffered if it's set to true, when internally use otf2::event::buffer
         */
        void set_callback(otf2::reader::callback& callback, bool buffered = false)
        {
            if (buffered)
            {
                buffer_.reset(new otf2::event::buffer(callback));
                callback_ = buffer_.get();
            }
            else
            {
                callback_ = &callback;
            }
        }

        reader(reader&) = delete;
        reader& operator=(reader&) = delete;

        /**
         * \brief destructor
         *
         * \internal
         *
         * Closes the OTF2_Reader
         */
        ~reader()
        {
            OTF2_Reader_Close(rdr);
        }

    public:
        const otf2::registry& registry() const
        {
            return reg_;
        }

        otf2::registry& registry()
        {
            return reg_;
        }

    public:
        /**
         * \brief returns the ticks per second
         *
         * You should check with \ref has_clock_properties() if there was a clock properties
         * definition before you rely on this. Otherwise you will get the default ticks per second.
         */
        otf2::chrono::ticks ticks_per_second() const
        {
            if (has_clock_properties())
                return clock_properties_->ticks_per_second();
            else
                return otf2::chrono::ticks(otf2::chrono::clock::period::den);
        }

        /**
         * \brief returns if clock properties were read from the trace already
         */
        bool has_clock_properties() const
        {
            return static_cast<bool>(clock_properties_);
        }

        /**
         * \brief set the clock properties definition
         * You shouldn't call this function
         * \internal
         * \param cp a unique_ptr to a clock properties definition
         */
        void set_clock_properties(std::unique_ptr<otf2::definition::clock_properties>&& cp)
        {
            clock_properties_ = std::move(cp);
            clock_convert_ = otf2::chrono::convert{ clock_properties() };
        }

        /**
         * \brief returns clock properties definition
         * You should check with \ref has_clock_properties() if there was a clock properties
         * definition before you rely on this.
         */
        const otf2::definition::clock_properties& clock_properties() const
        {
            assert(has_clock_properties());
            return *clock_properties_;
        }

        /**
         * \brief returns the chrono convert for this trace
         */
        const otf2::chrono::convert& clock_convert() const
        {
            return clock_convert_;
        }

    private:
        OTF2_Reader* rdr;

        definition_files definition_files_;
        event_files event_files_;

        std::vector<otf2::definition::location> registered_locations_;

        OTF2_GlobalDefReader* def_rdr;
        OTF2_GlobalEvtReader* evt_rdr;

        otf2::registry reg_;

        std::unique_ptr<otf2::definition::clock_properties> clock_properties_;
        otf2::chrono::convert clock_convert_;

        std::unique_ptr<otf2::reader::callback> buffer_;
        otf2::reader::callback* callback_;
    };

} // namespace reader
} // namespace otf2

#endif // INCLUDE_OTF2XX_READER_READER_HPP
