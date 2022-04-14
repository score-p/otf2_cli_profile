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

#ifndef INCLUDE_OTF2XX_WRITER_LOCAL_HPP
#define INCLUDE_OTF2XX_WRITER_LOCAL_HPP

#include <otf2/OTF2_DefWriter.h>
#include <otf2/OTF2_EvtWriter.h>

#include <otf2xx/definition/definitions.hpp>
#include <otf2xx/event/events.hpp>
#include <otf2xx/exception.hpp>

#include <otf2xx/chrono/chrono.hpp>

namespace otf2
{
namespace writer
{

    class local
    {
    public:
        local(OTF2_Archive* ar, const otf2::definition::location& location)
        : location_(location), ar_(ar), def_wrt_(OTF2_Archive_GetDefWriter(ar, location.ref())),
          evt_wrt_(OTF2_Archive_GetEvtWriter(ar, location.ref()))

        {
            if (evt_wrt_ == nullptr)
            {
                make_exception("Couldn't open local event writer for '", location, "'");
            }
            if (def_wrt_ == nullptr)
            {
                make_exception("Couldn't open local definition writer for '", location, "'");
            }
        }

        local(const local&) = delete;
        local& operator=(const local&) = delete;

        local(local&& other)
        : location_(std::move(other.location_)), ar_(nullptr), def_wrt_(nullptr), evt_wrt_(nullptr)
        {
            using std::swap;

            swap(ar_, other.ar_);
            swap(def_wrt_, other.def_wrt_);
            swap(evt_wrt_, other.evt_wrt_);
        }

        local& operator=(local&& other)
        {
            using std::swap;

            location_ = std::move(other.location_);

            swap(ar_, other.ar_);
            swap(def_wrt_, other.def_wrt_);
            swap(evt_wrt_, other.evt_wrt_);

            return *this;
        }

        ~local()
        {
            if (ar_ != nullptr)
            {
                if (def_wrt_ != nullptr)
                {
                    check(OTF2_Archive_CloseDefWriter(ar_, def_wrt_),
                          "Couldn't close definition writer");
                }
                if (evt_wrt_ != nullptr)
                {
                    check(OTF2_Archive_CloseEvtWriter(ar_, evt_wrt_),
                          "Couldn't close event writer");
                }
            }
        }

    public:
        const otf2::definition::location& location()
        {
            return location_;
        }

        std::uint64_t num_events() const
        {
            std::uint64_t tmp;

            check(OTF2_EvtWriter_GetNumberOfEvents(evt_wrt_, &tmp),
                  "Couldn't get number of events");

            return tmp;
        }

        void close_definition_writer()
        {
            if (def_wrt_ != nullptr)
            {
                check(OTF2_Archive_CloseDefWriter(ar_, def_wrt_),
                      "Couldn't close definition writer");

                def_wrt_ = nullptr;
            }
        }

        void close_event_writer()
        {
            if (evt_wrt_ != nullptr)
            {
                check(OTF2_Archive_CloseEvtWriter(ar_, evt_wrt_), "Couldn't close event writer");

                evt_wrt_ = nullptr;
            }
        }

    public:
        void write(const otf2::event::buffer_flush& data)
        {
            check(OTF2_EvtWriter_BufferFlush(evt_wrt_, data.attribute_list().get(),
                                             convert(data.timestamp()), convert(data.finish())),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

        void write(const otf2::event::enter& data)
        {
            check(OTF2_EvtWriter_Enter(evt_wrt_, data.attribute_list().get(),
                                       convert(data.timestamp()), data.region_.ref().get()),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

        void write(const otf2::event::leave& data)
        {
            check(OTF2_EvtWriter_Leave(evt_wrt_, data.attribute_list().get(),
                                       convert(data.timestamp()), data.region_.ref().get()),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

        void write(const otf2::event::measurement& data)
        {
            check(OTF2_EvtWriter_MeasurementOnOff(evt_wrt_, data.attribute_list().get(),
                                                  convert(data.timestamp()),
                                                  static_cast<OTF2_MeasurementMode>(data.mode())),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

        void write(const otf2::event::metric& metric)
        {
            std::size_t num_members = metric.raw_values().size();
            const auto& type_ids = metric.raw_values().type_ids();
            const auto& metric_values = metric.raw_values().values();

            std::visit(
                [&](auto&& metric_ref)
                {
                    check(OTF2_EvtWriter_Metric(evt_wrt_, metric.attribute_list().get(),
                                                convert(metric.timestamp()), metric_ref.ref(),
                                                num_members, type_ids.data(), metric_values.data()),
                          "Couldn't write event to local event writer.");
                },
                metric.metric_);
            location_.event_written();
        }

    public:
        void write(const otf2::event::mpi_ireceive_complete& data)
        {
            std::visit(
                [&](auto&& comm)
                {
                    check(OTF2_EvtWriter_MpiIrecv(evt_wrt_, data.attribute_list().get(),
                                                  convert(data.timestamp()), data.sender(),
                                                  comm.ref(), data.msg_tag(), data.msg_length(),
                                                  data.request_id()),
                          "Couldn't write event to local event writer.");
                },
                data.comm_);
            location_.event_written();
        }

        void write(const otf2::event::mpi_ireceive_request& data)
        {
            check(OTF2_EvtWriter_MpiIrecvRequest(evt_wrt_, data.attribute_list().get(),
                                                 convert(data.timestamp()), data.request_id()),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

        void write(const otf2::event::mpi_isend_request& data)
        {
            std::visit(
                [&](auto&& comm)
                {
                    check(OTF2_EvtWriter_MpiIsend(evt_wrt_, data.attribute_list().get(),
                                                  convert(data.timestamp()), data.receiver(),
                                                  comm.ref(), data.msg_tag(), data.msg_length(),
                                                  data.request_id()),
                          "Couldn't write event to local event writer.");
                },
                data.comm_);
            location_.event_written();
        }

        void write(const otf2::event::mpi_isend_complete& data)
        {
            check(OTF2_EvtWriter_MpiIsendComplete(evt_wrt_, data.attribute_list().get(),
                                                  convert(data.timestamp()), data.request_id()),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

        void write(const otf2::event::mpi_receive& data)
        {
            std::visit(
                [&](auto&& comm)
                {
                    check(OTF2_EvtWriter_MpiRecv(evt_wrt_, data.attribute_list().get(),
                                                 convert(data.timestamp()), data.sender(),
                                                 comm.ref(), data.msg_tag(), data.msg_length()),
                          "Couldn't write event to local event writer.");
                },
                data.comm_);
            location_.event_written();
        }

        void write(const otf2::event::mpi_request_test& data)
        {
            check(OTF2_EvtWriter_MpiRequestTest(evt_wrt_, data.attribute_list().get(),
                                                convert(data.timestamp()), data.request_id()),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

        void write(const otf2::event::mpi_request_cancelled& data)
        {
            check(OTF2_EvtWriter_MpiRequestCancelled(evt_wrt_, data.attribute_list().get(),
                                                     convert(data.timestamp()), data.request_id()),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

        void write(const otf2::event::mpi_send& data)
        {
            std::visit(
                [&](auto&& comm)
                {
                    check(OTF2_EvtWriter_MpiSend(evt_wrt_, data.attribute_list().get(),
                                                 convert(data.timestamp()), data.receiver(),
                                                 comm.ref(), data.msg_tag(), data.msg_length()),
                          "Couldn't write event to local event writer.");
                },
                data.comm_);
            location_.event_written();
        }

        void write(const otf2::event::mpi_collective_begin& data)
        {
            check(OTF2_EvtWriter_MpiCollectiveBegin(evt_wrt_, data.attribute_list().get(),
                                                    convert(data.timestamp())),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

        void write(const otf2::event::mpi_collective_end& data)
        {
            std::visit(
                [&](auto&& comm)
                {
                    check(OTF2_EvtWriter_MpiCollectiveEnd(
                              evt_wrt_, data.attribute_list().get(), convert(data.timestamp()),
                              static_cast<OTF2_CollectiveOp>(data.type()), comm.ref(), data.root(),
                              data.sent(), data.received()),
                          "Couldn't write event to local event writer.");
                },
                data.comm_);
            location_.event_written();
        }

        void write(const otf2::event::non_blocking_collective_request& data)
        {
            check(OTF2_EvtWriter_NonBlockingCollectiveRequest(evt_wrt_, data.attribute_list().get(),
                                                              convert(data.timestamp()),
                                                              data.request_id()),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

        void write(const otf2::event::non_blocking_collective_complete& data)
        {
            std::visit(
                [&](auto&& comm)
                {
                    check(OTF2_EvtWriter_NonBlockingCollectiveComplete(
                              evt_wrt_, data.attribute_list().get(), convert(data.timestamp()),
                              static_cast<OTF2_CollectiveOp>(data.type()), comm.ref(), data.root(),
                              data.sent(), data.received(), data.request_id()),
                          "Couldn't write event to local event writer.");
                },
                data.comm_);
            location_.event_written();
        }

        void write(const otf2::event::comm_create& data)
        {
            std::visit(
                [&](auto&& comm)
                {
                    check(OTF2_EvtWriter_CommCreate(evt_wrt_, data.attribute_list().get(),
                                                    convert(data.timestamp()), comm.ref()),
                          "Couldn't write event to local event writer.");
                },
                data.comm_);
            location_.event_written();
        }

        void write(const otf2::event::comm_destroy& data)
        {
            std::visit(
                [&](auto&& comm)
                {
                    check(OTF2_EvtWriter_CommDestroy(evt_wrt_, data.attribute_list().get(),
                                                     convert(data.timestamp()), comm.ref()),
                          "Couldn't write event to local event writer.");
                },
                data.comm_);
            location_.event_written();
        }

    public:
        void write(const otf2::event::parameter_int& data)
        {
            check(OTF2_EvtWriter_ParameterInt(evt_wrt_, data.attribute_list().get(),
                                              convert(data.timestamp()), data.parameter_.ref(),
                                              data.value()),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

        void write(const otf2::event::parameter_unsigned_int& data)
        {
            check(OTF2_EvtWriter_ParameterUnsignedInt(evt_wrt_, data.attribute_list().get(),
                                                      convert(data.timestamp()),
                                                      data.parameter_.ref(), data.value()),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

        void write(const otf2::event::parameter_string& data)
        {
            check(OTF2_EvtWriter_ParameterString(evt_wrt_, data.attribute_list().get(),
                                                 convert(data.timestamp()), data.parameter_.ref(),
                                                 data.value_.ref()),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

        void write(const otf2::event::calling_context_enter& data)
        {
            check(OTF2_EvtWriter_CallingContextEnter(
                      evt_wrt_, data.attribute_list().get(), convert(data.timestamp()),
                      data.calling_context_.ref(), data.unwind_distance()),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

        void write(const otf2::event::calling_context_leave& data)
        {
            check(OTF2_EvtWriter_CallingContextLeave(evt_wrt_, data.attribute_list().get(),
                                                     convert(data.timestamp()),
                                                     data.calling_context_.ref()),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

        void write(const otf2::event::calling_context_sample& data)
        {
            check(OTF2_EvtWriter_CallingContextSample(
                      evt_wrt_, data.attribute_list().get(), convert(data.timestamp()),
                      data.calling_context_.ref(), data.unwind_distance(),
                      data.interrupt_generator_.ref()),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

    public:
        // TODO find a better solution to allow high-performance writing of this event without
        // using shread ptrs
        void write_calling_context_enter(otf2::chrono::time_point timestamp,
                                         OTF2_CallingContextRef ref, uint32_t unwind_distance)
        {
            check(OTF2_EvtWriter_CallingContextEnter(evt_wrt_, nullptr, convert(timestamp), ref,
                                                     unwind_distance),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

        void write_calling_context_leave(otf2::chrono::time_point timestamp,
                                         OTF2_CallingContextRef ref)
        {
            check(OTF2_EvtWriter_CallingContextLeave(evt_wrt_, nullptr, convert(timestamp), ref),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

        void write_calling_context_sample(otf2::chrono::time_point timestamp,
                                          OTF2_CallingContextRef ref, uint32_t unwind_distance,
                                          OTF2_InterruptGeneratorRef interrupt_generator_ref)
        {
            check(OTF2_EvtWriter_CallingContextSample(evt_wrt_, nullptr, convert(timestamp), ref,
                                                      unwind_distance, interrupt_generator_ref),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

        void write_enter(otf2::chrono::time_point timestamp, OTF2_RegionRef ref)
        {
            check(OTF2_EvtWriter_Enter(evt_wrt_, nullptr, convert(timestamp), ref),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

        void write_leave(otf2::chrono::time_point timestamp, OTF2_RegionRef ref)
        {
            check(OTF2_EvtWriter_Leave(evt_wrt_, nullptr, convert(timestamp), ref),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

    public:
        void write(const otf2::event::thread_acquire_lock& data)
        {
            check(OTF2_EvtWriter_ThreadAcquireLock(
                      evt_wrt_, data.attribute_list().get(), convert(data.timestamp()),
                      static_cast<OTF2_Paradigm>(data.paradigm()), data.lock_id(), data.order()),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

        void write(const otf2::event::thread_fork& data)
        {
            check(OTF2_EvtWriter_ThreadFork(
                      evt_wrt_, data.attribute_list().get(), convert(data.timestamp()),
                      static_cast<OTF2_Paradigm>(data.paradigm()), data.num_threads()),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

        void write(const otf2::event::thread_join& data)
        {
            check(OTF2_EvtWriter_ThreadJoin(evt_wrt_, data.attribute_list().get(),
                                            convert(data.timestamp()),
                                            static_cast<OTF2_Paradigm>(data.paradigm())),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

        void write(const otf2::event::thread_release_lock& data)
        {
            check(OTF2_EvtWriter_ThreadReleaseLock(
                      evt_wrt_, data.attribute_list().get(), convert(data.timestamp()),
                      static_cast<OTF2_Paradigm>(data.paradigm()), data.lock_id(), data.order()),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

        void write(const otf2::event::thread_task_complete& data)
        {
            std::visit(
                [&](auto&& comm)
                {
                    check(OTF2_EvtWriter_ThreadTaskComplete(evt_wrt_, data.attribute_list().get(),
                                                            convert(data.timestamp()), comm.ref(),
                                                            data.thread(), data.generation()),
                          "Couldn't write event to local event writer.");
                },
                data.team_);
            location_.event_written();
        }

        void write(const otf2::event::thread_task_create& data)
        {
            std::visit(
                [&](auto&& comm)
                {
                    check(OTF2_EvtWriter_ThreadTaskCreate(evt_wrt_, data.attribute_list().get(),
                                                          convert(data.timestamp()), comm.ref(),
                                                          data.thread(), data.generation()),
                          "Couldn't write event to local event writer.");
                },
                data.team_);
            location_.event_written();
        }

        void write(const otf2::event::thread_task_switch& data)
        {
            std::visit(
                [&](auto&& comm)
                {
                    check(OTF2_EvtWriter_ThreadTaskSwitch(evt_wrt_, data.attribute_list().get(),
                                                          convert(data.timestamp()), comm.ref(),
                                                          data.thread(), data.generation()),
                          "Couldn't write event to local event writer.");
                },
                data.team_);
            location_.event_written();
        }

        void write(const otf2::event::thread_team_begin& data)
        {
            std::visit(
                [&](auto&& comm)
                {
                    check(OTF2_EvtWriter_ThreadTeamBegin(evt_wrt_, data.attribute_list().get(),
                                                         convert(data.timestamp()), comm.ref()),
                          "Couldn't write event to local event writer.");
                },
                data.comm_);
            location_.event_written();
        }

        void write(const otf2::event::thread_team_end& data)
        {
            std::visit(
                [&](auto&& comm)
                {
                    check(OTF2_EvtWriter_ThreadTeamEnd(evt_wrt_, data.attribute_list().get(),
                                                       convert(data.timestamp()), comm.ref()),
                          "Couldn't write event to local event writer.");
                },
                data.comm_);
            location_.event_written();
        }

        void write(const otf2::event::thread_create& data)
        {
            std::visit(
                [&](auto&& comm)
                {
                    check(OTF2_EvtWriter_ThreadCreate(evt_wrt_, data.attribute_list().get(),
                                                      convert(data.timestamp()), comm.ref(),
                                                      data.sequence_number()),
                          "Couldn't write event to local event writer.");
                },
                data.thread_contingent_);
            location_.event_written();
        }

        void write(const otf2::event::thread_begin& data)
        {
            std::visit(
                [&](auto&& comm)
                {
                    check(OTF2_EvtWriter_ThreadBegin(evt_wrt_, data.attribute_list().get(),
                                                     convert(data.timestamp()), comm.ref(),
                                                     data.sequence_number()),
                          "Couldn't write event to local event writer.");
                },
                data.thread_contingent_);
            location_.event_written();
        }

        void write(const otf2::event::thread_wait& data)
        {
            std::visit(
                [&](auto&& comm)
                {
                    check(OTF2_EvtWriter_ThreadWait(evt_wrt_, data.attribute_list().get(),
                                                    convert(data.timestamp()), comm.ref(),
                                                    data.sequence_number()),
                          "Couldn't write event to local event writer.");
                },
                data.thread_contingent_);
            location_.event_written();
        }

        void write(const otf2::event::thread_end& data)
        {
            std::visit(
                [&](auto&& comm)
                {
                    check(OTF2_EvtWriter_ThreadEnd(evt_wrt_, data.attribute_list().get(),
                                                   convert(data.timestamp()), comm.ref(),
                                                   data.sequence_number()),
                          "Couldn't write event to local event writer.");
                },
                data.thread_contingent_);
            location_.event_written();
        }

        void write(const otf2::event::rma_acquire_lock& data)
        {
            check(OTF2_EvtWriter_RmaAcquireLock(evt_wrt_, data.attribute_list().get(),
                                                convert(data.timestamp()), data.win().ref(),
                                                data.remote(), data.lock(),
                                                static_cast<OTF2_LockType>(data.lock_type())),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

        void write(const otf2::event::rma_atomic& data)
        {
            check(OTF2_EvtWriter_RmaAtomic(
                      evt_wrt_, data.attribute_list().get(), convert(data.timestamp()),
                      data.win().ref(), data.remote(), static_cast<OTF2_RmaAtomicType>(data.type()),
                      data.sent(), data.received(), data.matching_id()),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

        void write(const otf2::event::rma_collective_begin& data)
        {
            check(OTF2_EvtWriter_RmaCollectiveBegin(evt_wrt_, data.attribute_list().get(),
                                                    convert(data.timestamp())),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

        void write(const otf2::event::rma_collective_end& data)
        {
            check(OTF2_EvtWriter_RmaCollectiveEnd(
                      evt_wrt_, data.attribute_list().get(), convert(data.timestamp()),
                      static_cast<OTF2_CollectiveOp>(data.type()),
                      static_cast<OTF2_RmaSyncLevel>(data.level()), data.win().ref(), data.root(),
                      data.sent(), data.received()),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

        void write(const otf2::event::rma_get& data)
        {
            check(OTF2_EvtWriter_RmaGet(evt_wrt_, data.attribute_list().get(),
                                        convert(data.timestamp()), data.win().ref(), data.remote(),
                                        data.bytes(), data.matching_id()),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

        void write(const otf2::event::rma_group_sync& data)
        {
            check(OTF2_EvtWriter_RmaGroupSync(evt_wrt_, data.attribute_list().get(),
                                              convert(data.timestamp()),
                                              static_cast<OTF2_RmaSyncLevel>(data.level()),
                                              data.win().ref(), data.group().ref()),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

        void write(const otf2::event::rma_op_complete_blocking& data)
        {
            check(OTF2_EvtWriter_RmaOpCompleteBlocking(evt_wrt_, data.attribute_list().get(),
                                                       convert(data.timestamp()), data.win().ref(),
                                                       data.matching_id()),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

        void write(const otf2::event::rma_op_complete_non_blocking& data)
        {
            check(OTF2_EvtWriter_RmaOpCompleteNonBlocking(evt_wrt_, data.attribute_list().get(),
                                                          convert(data.timestamp()),
                                                          data.win().ref(), data.matching_id()),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

        void write(const otf2::event::rma_op_test& data)
        {
            check(OTF2_EvtWriter_RmaOpTest(evt_wrt_, data.attribute_list().get(),
                                           convert(data.timestamp()), data.win().ref(),
                                           data.matching_id()),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

        void write(const otf2::event::rma_op_complete_remote& data)
        {
            check(OTF2_EvtWriter_RmaOpCompleteRemote(evt_wrt_, data.attribute_list().get(),
                                                     convert(data.timestamp()), data.win().ref(),
                                                     data.matching_id()),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

        void write(const otf2::event::rma_put& data)
        {
            check(OTF2_EvtWriter_RmaPut(evt_wrt_, data.attribute_list().get(),
                                        convert(data.timestamp()), data.win().ref(), data.remote(),
                                        data.bytes(), data.matching_id()),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

        void write(const otf2::event::rma_release_lock& data)
        {
            check(OTF2_EvtWriter_RmaReleaseLock(evt_wrt_, data.attribute_list().get(),
                                                convert(data.timestamp()), data.win().ref(),
                                                data.remote(), data.lock()),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

        void write(const otf2::event::rma_request_lock& data)
        {
            check(OTF2_EvtWriter_RmaRequestLock(evt_wrt_, data.attribute_list().get(),
                                                convert(data.timestamp()), data.win().ref(),
                                                data.remote(), data.lock(),
                                                static_cast<OTF2_LockType>(data.lock_type())),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

        void write(const otf2::event::rma_sync& data)
        {
            check(OTF2_EvtWriter_RmaSync(evt_wrt_, data.attribute_list().get(),
                                         convert(data.timestamp()), data.win().ref(), data.remote(),
                                         static_cast<OTF2_RmaSyncType>(data.sync_type())),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

        void write(const otf2::event::rma_try_lock& data)
        {
            check(OTF2_EvtWriter_RmaTryLock(evt_wrt_, data.attribute_list().get(),
                                            convert(data.timestamp()), data.win().ref(),
                                            data.remote(), data.lock(),
                                            static_cast<OTF2_LockType>(data.lock_type())),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

        void write(const otf2::event::rma_wait_change& data)
        {
            check(OTF2_EvtWriter_RmaWaitChange(evt_wrt_, data.attribute_list().get(),
                                               convert(data.timestamp()), data.win().ref()),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

        void write(const otf2::event::rma_win_create& data)
        {
            check(OTF2_EvtWriter_RmaWinCreate(evt_wrt_, data.attribute_list().get(),
                                              convert(data.timestamp()), data.win().ref()),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

        void write(const otf2::event::rma_win_destroy& data)
        {
            check(OTF2_EvtWriter_RmaWinDestroy(evt_wrt_, data.attribute_list().get(),
                                               convert(data.timestamp()), data.win().ref()),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

        void write(const otf2::event::io_create_handle& data)
        {
            check(OTF2_EvtWriter_IoCreateHandle(
                      evt_wrt_, data.attribute_list().get(), convert(data.timestamp()),
                      data.handle().ref(), static_cast<OTF2_IoAccessMode>(data.access_mode()),
                      static_cast<OTF2_IoCreationFlag>(data.creation_flags()),
                      static_cast<OTF2_IoStatusFlag>(data.status_flags())),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

        void write(const otf2::event::io_destroy_handle& data)
        {
            check(OTF2_EvtWriter_IoDestroyHandle(evt_wrt_, data.attribute_list().get(),
                                                 convert(data.timestamp()), data.handle().ref()),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

        void write(const otf2::event::io_duplicate_handle& data)
        {
            check(OTF2_EvtWriter_IoDuplicateHandle(
                      evt_wrt_, data.attribute_list().get(), convert(data.timestamp()),
                      data.old_handle().ref(), data.new_handle().ref(),
                      static_cast<OTF2_IoStatusFlag>(data.status_flags())),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

        void write(const otf2::event::io_seek& data)
        {
            check(OTF2_EvtWriter_IoSeek(
                      evt_wrt_, data.attribute_list().get(), convert(data.timestamp()),
                      data.handle().ref(), data.offset_request(),
                      static_cast<OTF2_IoSeekOption>(data.seek_option()), data.offset_result()),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

        void write(const otf2::event::io_change_status_flag& data)
        {
            check(OTF2_EvtWriter_IoChangeStatusFlags(
                      evt_wrt_, data.attribute_list().get(), convert(data.timestamp()),
                      data.handle().ref(), static_cast<OTF2_IoStatusFlag>(data.status_flags())),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

        void write(const otf2::event::io_delete_file& data)
        {
            check(OTF2_EvtWriter_IoDeleteFile(evt_wrt_, data.attribute_list().get(),
                                              convert(data.timestamp()), data.paradigm().ref(),
                                              data.file().ref()),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

        void write(const otf2::event::io_operation_begin& data)
        {
            check(OTF2_EvtWriter_IoOperationBegin(
                      evt_wrt_, data.attribute_list().get(), convert(data.timestamp()),
                      data.handle().ref(), static_cast<OTF2_IoOperationMode>(data.operation_mode()),
                      static_cast<OTF2_IoOperationFlag>(data.operation_flag()),
                      data.bytes_request(), data.matching_id()),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

        void write(const otf2::event::io_operation_test& data)
        {
            check(OTF2_EvtWriter_IoOperationTest(evt_wrt_, data.attribute_list().get(),
                                                 convert(data.timestamp()), data.handle().ref(),
                                                 data.matching_id()),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

        void write(const otf2::event::io_operation_issued& data)
        {
            check(OTF2_EvtWriter_IoOperationIssued(evt_wrt_, data.attribute_list().get(),
                                                   convert(data.timestamp()), data.handle().ref(),
                                                   data.matching_id()),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

        void write(const otf2::event::io_operation_complete& data)
        {
            check(OTF2_EvtWriter_IoOperationComplete(evt_wrt_, data.attribute_list().get(),
                                                     convert(data.timestamp()), data.handle().ref(),
                                                     data.bytes_request(), data.matching_id()),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

        void write(const otf2::event::io_operation_cancelled& data)
        {
            check(OTF2_EvtWriter_IoOperationCancelled(evt_wrt_, data.attribute_list().get(),
                                                      convert(data.timestamp()),
                                                      data.handle().ref(), data.matching_id()),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

        void write(const otf2::event::io_acquire_lock& data)
        {
            check(OTF2_EvtWriter_IoAcquireLock(evt_wrt_, data.attribute_list().get(),
                                               convert(data.timestamp()), data.handle().ref(),
                                               static_cast<OTF2_LockType>(data.lock_type())),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

        void write(const otf2::event::io_release_lock& data)
        {
            check(OTF2_EvtWriter_IoReleaseLock(evt_wrt_, data.attribute_list().get(),
                                               convert(data.timestamp()), data.handle().ref(),
                                               static_cast<OTF2_LockType>(data.lock_type())),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

        void write(const otf2::event::io_try_lock& data)
        {
            check(OTF2_EvtWriter_IoTryLock(evt_wrt_, data.attribute_list().get(),
                                           convert(data.timestamp()), data.handle().ref(),
                                           static_cast<OTF2_LockType>(data.lock_type())),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

        void write(const otf2::event::program_begin& data)
        {
            auto numberOfArguments = data.arguments().size();
            std::vector<otf2::reference<otf2::definition::string>::ref_type> args;
            args.reserve(numberOfArguments);

            for (const auto& arg : data.arguments())
            {
                args.push_back(arg.ref());
            }

            check(OTF2_EvtWriter_ProgramBegin(evt_wrt_, data.attribute_list().get(),
                                              convert(data.timestamp()), data.name().ref(),
                                              numberOfArguments, args.data()),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

        void write(const otf2::event::program_end& data)
        {
            check(OTF2_EvtWriter_ProgramEnd(evt_wrt_, data.attribute_list().get(),
                                            convert(data.timestamp()), data.exit_status()),
                  "Couldn't write event to local event writer.");
            location_.event_written();
        }

    public:
        void write(const otf2::definition::mapping_table& def)
        {
            check(OTF2_DefWriter_WriteMappingTable(
                      def_wrt_, static_cast<OTF2_MappingType>(def.mapping_type()), def.id_map()),
                  "Couldn't write mapping table definition to local writer");
        }

    private:
        static OTF2_TimeStamp convert(otf2::chrono::time_point tp)
        {
            static otf2::chrono::convert cvrt;
            static_assert(otf2::chrono::clock::period::num == 1,
                          "Don't mess around with the chrono stuff!");
            return cvrt(tp).count();
        }

    private:
        otf2::definition::location location_;
        OTF2_Archive* ar_;
        OTF2_DefWriter* def_wrt_;
        OTF2_EvtWriter* evt_wrt_;
    };

    template <typename Record>
    local& operator<<(local& wrt, Record rec)
    {
        wrt.write(rec);
        return wrt;
    }
} // namespace writer
} // namespace otf2

#endif // INCLUDE_OTF2XX_WRITER_LOCAL_HPP
