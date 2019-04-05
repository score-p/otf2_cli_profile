/*
 This is part of the OTF-Profiler. Copyright by ZIH, TU Dresden 2016-2018.
 Authors: Maximillian Neumann, Denis Hünich, Jens Doleschal
*/

#ifndef OTF2READER_H
#define OTF2READER_H

#include <otf2/otf2.h>
#include "tracereader.h"

template <typename RefT>
class StringIdentifier {
   public:
    template <typename... Refs>
    using Result_t = std::array<std::string*, sizeof...(Refs)>;

   public:
    StringIdentifier() { string_definitions[OTF2_UNDEFINED_STRING] = ""; }

    void add(RefT ref, const char* string_def) { string_definitions[ref] = string_def; }

    template <typename... Refs>
    const std::pair<Result_t<Refs...>, OTF2_CallbackCode> get(Refs... refs) {
        auto              pos = 0;
        Result_t<Refs...> result{};

        for (auto ref : {refs...}) {
            auto it = string_definitions.find(ref);
            if (it != string_definitions.end())
                result[pos] = &it->second;
            else
                return std::make_pair(result, OTF2_CALLBACK_INTERRUPT);

            ++pos;
        }

        return std::make_pair(result, OTF2_CALLBACK_SUCCESS);
    }

   private:
    std::map<RefT, std::string> string_definitions;
};

class OTF2Reader : public TraceReader {
   public:
    OTF2Reader() = default;

    ~OTF2Reader() { close(); }

    void close();
    bool initialize(AllData& alldata);
    bool readDefinitions(AllData& alldata);
    bool readEvents(AllData& alldata);
    bool readStatistics(AllData& alldata);

   private:
    OTF2_Reader* _reader;

   private:
    /* ************************************************************** */
    /*                                                                */
    /*                            EVENTS                              */
    /*                                                                */
    /* ************************************************************** */

    static inline OTF2_CallbackCode handle_io_begin(OTF2_LocationRef locationID, OTF2_TimeStamp time,
                                                    uint64_t eventPosition, void* userData,
                                                    OTF2_AttributeList* attributeList, OTF2_IoHandleRef handle,
                                                    OTF2_IoOperationMode mode, OTF2_IoOperationFlag flag,
                                                    uint64_t bytesRequest, uint64_t matchingId);
    static inline OTF2_CallbackCode handle_io_end(OTF2_LocationRef locationID, OTF2_TimeStamp time,
                                                  uint64_t eventPosition, void* userData,
                                                  OTF2_AttributeList* attributeList, OTF2_IoHandleRef handle,
                                                  uint64_t bytesResult, uint64_t matchingId);
    static inline OTF2_CallbackCode handle_def_io_handle(void* userData, OTF2_IoHandleRef self, OTF2_StringRef name,
                                                         OTF2_IoFileRef file, OTF2_IoParadigmRef ioParadigm,
                                                         OTF2_IoHandleFlag ioHandleFlags, OTF2_CommRef comm,
                                                         OTF2_IoHandleRef parent);
    static inline OTF2_CallbackCode handle_def_io_fs_entry(void* userData, OTF2_IoFileRef self, OTF2_StringRef name,
                                                           OTF2_SystemTreeNodeRef scope);
    static inline OTF2_CallbackCode handle_def_io_paradigm(void* userData, OTF2_IoParadigmRef paradigm,
                                                           OTF2_StringRef id, OTF2_StringRef name,
                                                           OTF2_IoParadigmClass paradigmClass,
                                                           OTF2_IoParadigmFlag flags, uint8_t numProperties,
                                                           const OTF2_IoParadigmProperty* properties,
                                                           const OTF2_Type* types, const OTF2_AttributeValue* values);

    /** @brief Callback for the Enter event record.
     *
     *  An enter record indicates that the program enters a code region.
     *
     *  @param locationID    The location where this event happened.
     *  @param time          The time when this event happened.
     *  @param userData      User data.
     *  @param attributeList Additional attributes for this event.
     *  @param region        Needs to be defined in a definition record.
     *                       References a Region definition and will be
     *                       mapped to the global definition if a
     *                       mapping table of type OTF2_MAPPING_REGION
     *                       is available.
     *
     *  @return @eref{OTF2_CALLBACK_SUCCESS} or @eref{OTF2_CALLBACK_INTERRUPT}.
     */
    static inline OTF2_CallbackCode handle_enter(OTF2_LocationRef locationID, OTF2_TimeStamp time,
                                                 uint64_t eventPosition, void* userData,
                                                 OTF2_AttributeList* attributeList, OTF2_RegionRef region);

    /** @brief Callback for the Leave event record.
     *
     *  An leave record indicates that the program leaves a code region.
     *
     *  @param locationID    The location where this event happened.
     *  @param time          The time when this event happened.
     *  @param userData      User data.
     *  @param attributeList Additional attributes for this event.
     *  @param region        Needs to be defined in a definition record.
     *                       References a Region definition and will be
     *                       mapped to the global definition if a
     *                       mapping table of type OTF2_MAPPING_REGION
     *                       is available.
     *
     *  @return @eref{OTF2_CALLBACK_SUCCESS} or @eref{OTF2_CALLBACK_INTERRUPT}.
     */
    static inline OTF2_CallbackCode handle_leave(OTF2_LocationRef locationID, OTF2_TimeStamp time,
                                                 uint64_t eventPosition, void* userData,
                                                 OTF2_AttributeList* attributeList, OTF2_RegionRef region);

    /** @brief Callback for the MpiSend event record.
     *
     *  A MpiSend record indicates that a MPI message send process was
     *  initiated (MPI_SEND). It keeps the necessary information for this
     *  event: time, sender and receiver of the message and the
     *  communicator. You can optionally add further informations like
     *  message tag and message length (size of the send buffer).
     *
     *  @param locationID    The location where this event happened.
     *  @param time          The time when this event happened.
     *  @param userData      User data.
     *  @param attributeList Additional attributes for this event.
     *  @param receiver      MPI rank of receiver in @a communicator.
     *  @param communicator  Communicator ID. References a MpiComm definition and will
     *                       be mapped to the global definition if a mapping table
     *                       of type OTF2_MAPPING_MPI_COMMUNICATOR is available.
     *  @param msgTag        Message tag.
     *  @param msgLength     Message length.
     *
     *  @return @eref{OTF2_CALLBACK_SUCCESS} or @eref{OTF2_CALLBACK_INTERRUPT}.
     */
    static inline OTF2_CallbackCode handle_mpi_send(OTF2_LocationRef locationID, OTF2_TimeStamp time,
                                                    uint64_t eventPosition, void* userData,
                                                    OTF2_AttributeList* attributeList, uint32_t receiver,
                                                    OTF2_CommRef communicator, uint32_t msgTag, uint64_t msgLength);

    /** @brief Callback for the MpiIsend event record.
     *
     *  A MpiIsend record indicates that a MPI message send process was
     *  initiated (MPI_ISEND). It keeps the necessary information for this
     *  event: time, sender and receiver of the message and the
     *  communicator. You can optionally add further informations like
     *  message tag and message length (size of the send buffer).
     *
     *  @param locationID    The location where this event happened.
     *  @param time          The time when this event happened.
     *  @param userData      User data.
     *  @param attributeList Additional attributes for this event.
     *  @param receiver      MPI rank of receiver in @a communicator.
     *  @param communicator  Communicator ID. References a MpiComm definition and will
     *                       be mapped to the global definition if a mapping table
     *                       of type OTF2_MAPPING_MPI_COMMUNICATOR is available.
     *  @param msgtag        Message tag.
     *  @param msgLength     Message length.
     *  @param requestID     ID of the related request.
     *
     *  @return @eref{OTF2_CALLBACK_SUCCESS} or @eref{OTF2_CALLBACK_INTERRUPT}.
     */
    static inline OTF2_CallbackCode handle_mpi_isend(OTF2_LocationRef locationID, OTF2_TimeStamp time,
                                                     uint64_t eventPosition, void* userData,
                                                     OTF2_AttributeList* attributeList, uint32_t receiver,
                                                     OTF2_CommRef communicator, uint32_t msgTag, uint64_t msgLength,
                                                     uint64_t requestID);

    /** @brief Callback for the MpiIsendComplete event record.
     *
     *  Signals the completion of non-blocking send request. It keeps the
     *  necessary information for this event: time, sender and receiver of
     *  the message and the communicator. You can optionally add further
     *  informations like message tag and message length (size of the send
     *  buffer).
     *
     *  @param locationID    The location where this event happened.
     *  @param time          The time when this event happened.
     *  @param userData      User data.
     *  @param attributeList Additional attributes for this event.
     *  @param requestID     ID of the related request.
     *
     *  @return @eref{OTF2_CALLBACK_SUCCESS} or @eref{OTF2_CALLBACK_INTERRUPT}.
     */
    // TODO nicht verwendet
    /*
    static inline OTF2_CallbackCode handle_mpi_isend_complete(
        OTF2_LocationRef locationID, OTF2_TimeStamp time, uint64_t eventPosition, void* userData,
        OTF2_AttributeList* attributeList, uint64_t requestID);
*/

    /** @brief Callback for the MpiRecv event record.
     *
     *  A MpiRecv record indicates that a MPI message was recieved (MPI_RECV).
     *  It keeps the necessary information for this event: time, sender
     *  and reciever of the message and the communicator. You can
     *  optionally add further informations like message tag and message
     *  lenght (size of the recieve buffer).
     *
     *  @param locationID    The location where this event happened.
     *  @param time          The time when this event happened.
     *  @param userData      User data.
     *  @param attributeList Additional attributes for this event.
     *  @param sender        MPI rank of sender in @a communicator.
     *  @param communicator  Communicator ID. References a MpiComm definition and will
     *                       be mapped to the global definition if a mapping table
     *                       of type OTF2_MAPPING_MPI_COMMUNICATOR is available.
     *  @param msgtTg        Message tag.
     *  @param msgLength     Message length.
     *
     *  @return @eref{OTF2_CALLBACK_SUCCESS} or @eref{OTF2_CALLBACK_INTERRUPT}.
     */
    static inline OTF2_CallbackCode handle_mpi_recv(OTF2_LocationRef locationID, OTF2_TimeStamp time,
                                                    uint64_t eventPosition, void* userData,
                                                    OTF2_AttributeList* attributeList, uint32_t sender,
                                                    OTF2_CommRef communicator, uint32_t msgTag, uint64_t msgLength);

    /** @brief Callback for the MpiIrecvRequest event record.
     *
     *  Signals the request of an receive, which can be completed later.
     *
     *  @param locationID    The location where this event happened.
     *  @param time          The time when this event happened.
     *  @param userData      User data.
     *  @param attributeList Additional attributes for this event.
     *  @param requestID     ID of the requested receive.
     *
     *  @return @eref{OTF2_CALLBACK_SUCCESS} or @eref{OTF2_CALLBACK_INTERRUPT}.
     */
    /*TODO nie verwendet
    static inline OTF2_CallbackCode handle_mpi_irecv_request(OTF2_LocationRef locationID,
                                                             OTF2_TimeStamp   time,
                                                             uint64_t eventPosition, void* userData,
                                                             OTF2_AttributeList* attributeList,
                                                             uint64_t            requestID);
*/
    /** @brief Callback for the MpiIrecv event record.
     *
     *  An MpiIrecv record indicates that a non-blocking MPI message was
     *  recieved (MPI_IRECV). It keeps the necessary information for this
     *  event: time, sender the message, the communicator and the request
     *  ID. You can optionally add further information like message tag
     *  and message lenght (size of the recieve buffer).
     *
     *  @param locationID    The location where this event happened.
     *  @param time          The time when this event happened.
     *  @param userData      User data.
     *  @param attributeList Additional attributes for this event.
     *  @param sender        MPI rank of sender in @a communicator.
     *  @param communicator  Communicator ID. References a MpiComm definition and will
     *                       be mapped to the global definition if a mapping table
     *                       of type OTF2_MAPPING_MPI_COMMUNICATOR is available.
     *  @param msgTag        Message tag.
     *  @param msgLength     Message length.
     *  @param requestID     ID of the related request.
     *
     *  @return @eref{OTF2_CALLBACK_SUCCESS} or @eref{OTF2_CALLBACK_INTERRUPT}.
     */
    static inline OTF2_CallbackCode handle_mpi_irecv(OTF2_LocationRef locationID, OTF2_TimeStamp time,
                                                     uint64_t eventPosition, void* userData,
                                                     OTF2_AttributeList* attributeList, uint32_t sender,
                                                     OTF2_CommRef communicator, uint32_t msgTag, uint64_t msgLength,
                                                     uint64_t requestID);

    /** @brief Callback for the BufferFlush event record.
     *
     *  This event signals that the internal buffer was flushed at the given
     *  time.
     *
     *  @param locationID    The location where this event happened.
     *  @param time          The time when this event happened.
     *  @param userData      User data.
     *  @param attributeList Additional attributes for this event.
     *  @param stopTime      The time the buffer flush finished.
     *
     *  @return @eref{OTF2_CALLBACK_SUCCESS} or @eref{OTF2_CALLBACK_INTERRUPT}.
     */
    // TODO nicht verwendet
    /*
    static inline OTF2_CallbackCode handle_buffer_flush(OTF2_LocationRef locationID,
                                                        OTF2_TimeStamp time, void* userData,
                                                        OTF2_AttributeList* attributeList,
                                                        OTF2_TimeStamp      stopTime);
*/

    /** @brief Callback for the MpiRequestTest event record.
     *
     *  This events appears if the program test a request.
     *
     *  @param locationID    The location where this event happened.
     *  @param time          The time when this event happened.
     *  @param userData      User data.
     *  @param attributeList Additional attributes for this event.
     *  @param requestID     ID of the related request.
     *
     *  @return @eref{OTF2_CALLBACK_SUCCESS} or @eref{OTF2_CALLBACK_INTERRUPT}.
     */
    /*TODO nicht verwendet
    static inline OTF2_CallbackCode handle_mpi_request_test(OTF2_LocationRef locationID,
                                                            OTF2_TimeStamp   time,
                                                            uint64_t eventPosition, void* userData,
                                                            OTF2_AttributeList* attributeList,
                                                            uint64_t            requestID);
    */

    /** @brief Callback for the MpiCollectiveBegin event record.
     *
     *  A MpiCollectiveBegin record marks the begin of an MPI collective
     *  operation (MPI_GATHER, MPI_SCATTER etc.). This event is always
     *  surrounded by appropriate enter and leave records.
     *
     *  @param locationID    The location where this event happened.
     *  @param time          The time when this event happened.
     *  @param userData      User data.
     *  @param attributeList Additional attributes for this event.
     *
     *  @return @eref{OTF2_CALLBACK_SUCCESS} or @eref{OTF2_CALLBACK_INTERRUPT}.
     */
    /* TODO nicht verwendet
    static inline OTF2_CallbackCode handle_mpi_collective_begin(OTF2_LocationRef    locationID,
                                                                OTF2_TimeStamp      time,
                                                                uint64_t            eventPosition,
                                                                void*               userData,
                                                                OTF2_AttributeList* attributeList);
    */

    /** @brief Callback for the MpiCollectiveEnd event record.
     *
     *  A MpiCollectiveEnd record marks the end of an MPI collective operation
     *  (MPI_GATHER, MPI_SCATTER etc.). It keeps the necessary information
     *  for this event: type of collective operation, communicator, the
     *  root of this collective operation. You can optionally add further
     *  informations like sent bytes and received bytes. This event is
     *  always surrounded by appropriate enter and leave records.
     *
     *  @param locationID    The location where this event happened.
     *  @param time          The time when this event happened.
     *  @param userData      User data.
     *  @param attributeList Additional attributes for this event.
     *  @param type          Determines which collective operation it is.
     *  @param communicator  Communicator References a MpiComm definition and will be
     *                       mapped to the global definition if a mapping table
     *                       of type OTF2_MAPPING_MPI_COMMUNICATOR is available.
     *  @param root          MPI rank of root in @a communicator.
     *  @param sizeSent      Size of the sent message.
     *  @param sizeReceived  Size of the received message.
     *
     *  @return @eref{OTF2_CALLBACK_SUCCESS} or @eref{OTF2_CALLBACK_INTERRUPT}.
     */
    static inline OTF2_CallbackCode handle_mpi_collective_end(OTF2_LocationRef locationID, OTF2_TimeStamp time,
                                                              uint64_t eventPosition, void* userData,
                                                              OTF2_AttributeList* attributeList, OTF2_CollectiveOp type,
                                                              OTF2_CommRef communicator, uint32_t root,
                                                              uint64_t sizeSent, uint64_t sizeReceived);

    /** @brief Callback for the OmpFork event record.
     *
     *  An OmpFork record marks that an OpenMP Thread forks a thread team.
     *
     *  @param locationID               The location where this event happened.
     *  @param time                     The time when this event happened.
     *  @param userData                 User data.
     *  @param attributeList            Additional attributes for this event.
     *  @param numberOfRequestedThreads Requested size of the team.
     *
     *  @return @eref{OTF2_CALLBACK_SUCCESS} or @eref{OTF2_CALLBACK_INTERRUPT}.
     */
    // TODO nicht verwendet
    /*
    static inline OTF2_CallbackCode handle_omp_fork(OTF2_LocationRef locationID,
                                                    OTF2_TimeStamp time, void* userData,
                                                    OTF2_AttributeList* attributeList,
                                                    uint32_t            numberOfRequestedThreads);
    */

    /** @brief Callback for the OmpJoin event record.
     *
     *  An OmpJoin record marks that a Team of threads is joint and only the
     *  master thread continues execution.
     *
     *  @param locationID    The location where this event happened.
     *  @param time          The time when this event happened.
     *  @param userData      User data.
     *  @param attributeList Additional attributes for this event.
     *
     *  @return @eref{OTF2_CALLBACK_SUCCESS} or @eref{OTF2_CALLBACK_INTERRUPT}.
     */
    // TODO nicht verwendet
    /*
    static inline OTF2_CallbackCode handle_omp_join(OTF2_LocationRef locationID,
                                                    OTF2_TimeStamp time, void* userData,
                                                    OTF2_AttributeList* attributeList);
    */

    /** @brief Callback for the OmpAcquireLock event record.
     *
     *  An OmpAcquireLock record marks that a thread acquires an OpenMP lock.
     *
     *  @param locationID       The location where this event happened.
     *  @param time             The time when this event happened.
     *  @param eventPosition    The event position of this event in the trace.
     *                          Starting with 1.
     *  @param userData         User data.
     *  @param attributeList    Additional attributes for this event.
     *  @param lockID           ID of the lock.
     *  @param acquisitionOrder A monotonically increasing id to determine the order
     *                          of lock acquisitions (with unsynchronized clocks
     *                          this is otherwise not possible). Corresponding
     *                          acquire-release events have same values.
     *
     *  @return @eref{OTF2_CALLBACK_SUCCESS} or @eref{OTF2_CALLBACK_INTERRUPT}.
     */
    static inline OTF2_CallbackCode handle_omp_acquire_lock(OTF2_LocationRef locationID, OTF2_TimeStamp time,
                                                            void* userData, OTF2_AttributeList* attributeList,
                                                            uint32_t lockID, uint32_t acquisitionOrder);

    /** @brief Callback for the OmpReleaseLock event record.
     *
     *  An OmpReleaseLock record marks that a thread releases an OpenMP lock.
     *
     *  @param locationID        The location where this event happened.
     *  @param time              The time when this event happened.
     *  @param userData          User data.
     *  @param attributeList     Additional attributes for this event.
     *  @param lockID            ID of the lock.
     *  @param acquisitionOrder  A monotonically increasing id to determine the order
     *                           of lock acquisitions (with unsynchronized clocks
     *                           this is otherwise not possible). Corresponding
     *                           acquire-release events have same values.
     *
     *  @return @eref{OTF2_CALLBACK_SUCCESS} or @eref{OTF2_CALLBACK_INTERRUPT}.
     */
    static inline OTF2_CallbackCode handle_omp_release_lock(OTF2_LocationRef locationID, OTF2_TimeStamp time,
                                                            void* userData, OTF2_AttributeList* attributeList,
                                                            uint32_t lockID, uint32_t acquisitionOrder);

    /** @brief Callback for the OmpTaskCreate event record.
     *
     *  An OmpTaskCreate record marks that an OpenMP Task was/will be created
     *  in the current region.
     *
     *  @param locationID    The location where this event happened.
     *  @param time          The time when this event happened.
     *  @param userData      User data.
     *  @param attributeList Additional attributes for this event.
     *  @param taskID        Identifier of the newly created task instance.
     *
     *  @return @eref{OTF2_CALLBACK_SUCCESS} or @eref{OTF2_CALLBACK_INTERRUPT}.
     */
    static inline OTF2_CallbackCode handle_omp_task_create(OTF2_LocationRef locationID, OTF2_TimeStamp time,
                                                           void* userData, OTF2_AttributeList* attributeList,
                                                           uint64_t taskID);

    /** @brief Callback for the OmpTaskSwitch event record.
     *
     *  An OmpTaskSwitch record indicates that the execution of the current
     *  task will be suspended and another task starts/restarts its
     *  execution. Please note that this may change the current call stack
     *  of the executing location.
     *
     *  @param locationID    The location where this event happened.
     *  @param time          The time when this event happened.
     *  @param userData      User data.
     *  @param attributes    Additional attributes for this event.
     *  @param taskID        Identifier of the now active task instance.
     *
     *  @return @eref{OTF2_CALLBACK_SUCCESS} or @eref{OTF2_CALLBACK_INTERRUPT}.
     */
    static inline OTF2_CallbackCode handle_omp_task_switch(OTF2_LocationRef locationID, OTF2_TimeStamp time,
                                                           void* userData, OTF2_AttributeList* attributeList,
                                                           uint64_t taskID);

    /** @brief Callback for the OmpTaskComplete event record.
     *
     *  An OmpTaskComplete record indicates that the execution of an OpenMP
     *  task has finished.
     *
     *  @param locationID    The location where this event happened.
     *  @param time          The time when this event happened.
     *  @param userData      User data.
     *  @param attributeList Additional attributes for this event.
     *  @param taskID        Identifier of the completed task instance.
     *
     *  @return @eref{OTF2_CALLBACK_SUCCESS} or @eref{OTF2_CALLBACK_INTERRUPT}.
     */
    static inline OTF2_CallbackCode handle_omp_task_complete(OTF2_LocationRef locationID, OTF2_TimeStamp time,
                                                             void* userData, OTF2_AttributeList* attributeList,
                                                             uint64_t taskID);

    /** @brief Callback for the Metric event record.
     *
     *  A metric event is always stored at the location that recorded the
     *  metric. A metric event can be reference to a metric class or
     *  metric instance. Therefore, metric classes and instances share
     *  same ID space. Synchronous metrics are always located right before
     *  the according enter and leave. The metric event can contain
     *  multiple metrics.
     *
     *  @param locationID        The location where this event happened.
     *  @param time              The time when this event happened.
     *  @param userData          User data.
     *  @param attributeList     Additional attributes for this event.
     *  @param metric            Could be a metric class or a metric instance.
     *                           References a MetricClass definition and will be
     *                           mapped to the global definition if a mapping
     *                           table of type OTF2_MAPPING_METRIC is available.
     *  @param numberOfMetrics   Number of metrics with in the set.
     *  @param typeIDs           List of metric types.
     *  @param metricValues      List of metric values.
     *
     *  @return @eref{OTF2_CALLBACK_SUCCESS} or @eref{OTF2_CALLBACK_INTERRUPT}.
     */
    static inline OTF2_CallbackCode handle_metric(OTF2_LocationRef locationID, OTF2_TimeStamp time,
                                                  uint64_t eventPosition, void* userData,
                                                  OTF2_AttributeList* attributeList, OTF2_MetricRef metric,
                                                  uint8_t numberOfMetrics, const OTF2_Type* typeIDs,
                                                  const OTF2_MetricValue* metricValues);

    // TODO comments
    /*TODO nicht verwendet
    static inline OTF2_CallbackCode handle_rma_put(OTF2_LocationRef locationID, OTF2_TimeStamp time,
                                                   void*               userData,
                                                   OTF2_AttributeList* attributeList,
                                                   OTF2_RmaWinRef win, uint32_t remote,
                                                   uint64_t bytes, uint64_t matchingId);

    // TODO comments

    static inline OTF2_CallbackCode handle_rma_get(OTF2_LocationRef locationID, OTF2_TimeStamp time,
                                                   void*               userData,
                                                   OTF2_AttributeList* attributeList,
                                                   OTF2_RmaWinRef win, uint32_t remote,
                                                   uint64_t bytes, uint64_t matchingId);

    // TODO comments

    static inline OTF2_CallbackCode handle_rma_n_block_complete(OTF2_LocationRef locationID,
                                                                OTF2_TimeStamp time, void* userData,
                                                                OTF2_AttributeList* attributeList,
                                                                OTF2_RmaWinRef      win,
                                                                uint64_t            matchingId);

    // TODO comments

    static inline OTF2_CallbackCode handle_rma_block_complete(OTF2_LocationRef locationID,
                                                              OTF2_TimeStamp time, void* userData,
                                                              OTF2_AttributeList* attributeList,
                                                              OTF2_RmaWinRef      win,
                                                              uint64_t            matchingId);
    */

    /** @brief Callback for an unknown event record.
     *
     *  @param locationID        The location where this event happened.
     *  @param time              The time when this event happened.
     *  @param userData          User data.
     *  @param attributeList     Additional attributes for this event.
     *
     *  @return @eref{OTF2_CALLBACK_SUCCESS} or @eref{OTF2_CALLBACK_INTERRUPT}.
     */
    /*TODO nicht verwendet
    static inline OTF2_CallbackCode handle_unknown(OTF2_LocationRef locationID, OTF2_TimeStamp time,
                                                   void*               userData,
                                                   OTF2_AttributeList* attributeList);
    */
    /* ************************************************************** */
    /*                                                                */
    /*                          DEFINITIONS                           */
    /*                                                                */
    /* ************************************************************** */

    // TODO nicht verwendet
    static inline OTF2_CallbackCode handle_def_attribute(void* userData, OTF2_AttributeRef self, OTF2_StringRef name,
                                                         OTF2_StringRef description, OTF2_Type type);

    // TODO comment
    static inline OTF2_CallbackCode handle_def_metric_class(void* userData, OTF2_MetricRef self,
                                                            uint8_t                     numberOfMetrics,
                                                            const OTF2_MetricMemberRef* metricMembers,
                                                            OTF2_MetricOccurrence       metricOccurence,
                                                            OTF2_RecorderKind           recorderKind);

    // TODO comment
    // OTF2_GlobalDefReaderCallback_MetricMember
    static inline OTF2_CallbackCode handle_def_metrics(void* userData, OTF2_MetricMemberRef self, OTF2_StringRef name,
                                                       OTF2_StringRef description, OTF2_MetricType metricType,
                                                       OTF2_MetricMode metricMode, OTF2_Type valueType, OTF2_Base base,
                                                       int64_t exponent, OTF2_StringRef unit);

    /** @brief Function pointer definition for the callback which is
     *         triggered by a ClockProperties definition record.
     *
     *  Defines the timer resolution and time range of this trace. There
     *  will be no event with a timestamp less than global_offset, and no
     *  event with timestamp greater than (global_offset + trace_length).
     *
     *  @param userData         User data.
     *  @param timerResolution  Ticks per seconds.
     *  @param globalOffset     A timestamp smaller than all event timestamps.
     *  @param traceLength      A timespan which includes the timespan
     *                          between the smallest and greatest timestamp
     *                          of all event timestamps.
     *
     * @return @eref{OTF2_CALLBACK_SUCCESS} or @eref{OTF2_CALLBACK_INTERRUPT}.
     */
    static inline OTF2_CallbackCode handle_def_clock_properties(void* userData, uint64_t timerResolution,
                                                                uint64_t globalOffset, uint64_t traceLength);

    /** @brief Callback which is triggered by LocationGroup definition record.
     *
     *  @param userData          User data.
     *  @param groupIdentifier   The unique identifier for this LocationGroup
     *                           definition.
     *  @param name              Name of the group. References String definition.
     *  @param locationGroupType Type of this group.
     *  @param systemTreeParent  Parent of this location group in the system tree.
     *                           References a @ref SystemTreeNode definition.
     *
     *  @return @eref{OTF2_CALLBACK_SUCCESS} or @eref{OTF2_CALLBACK_INTERRUPT}.
     */

    static inline OTF2_CallbackCode handle_def_location_group(void* userData, OTF2_LocationGroupRef groupIdentifier,
                                                              OTF2_StringRef         name,
                                                              OTF2_LocationGroupType locationGroupType,
                                                              OTF2_SystemTreeNodeRef systemTreeParent);

    /** @brief Callback which is triggered by a Location definition record.
     *
     *  @param userData            User data.
     *  @param locationIdentifier  The unique identifier for this Location
     *                             definition.
     *  @param name                Name of the location. References a String
     *                             definition.
     *  @param locationType        Location type.
     *  @param numberOfEvents      Number of events this location has recorded.
     *  @param locationGroup       Location group which includes this location.
     *                             References a LocationGroup definition.
     *
     *  @return @eref{OTF2_CALLBACK_SUCCESS} or @eref{OTF2_CALLBACK_INTERRUPT}.
     */
    static inline OTF2_CallbackCode handle_def_location(void* userData, OTF2_LocationRef locationIdentifier,
                                                        OTF2_StringRef name, OTF2_LocationType locationType,
                                                        uint64_t numberOfEvents, OTF2_LocationGroupRef locationGroup);

    /** @brief Callback which is triggered by Group definition record.
     *
     *  @param userData User data.
     *
     *  @param groupIdentifier The unique identifier for this Group definition.
     *  @param name            Name of this group References a String
     *                         definition.
     *  @param groupType       The type of this group.
     *  @param paradigm        The paradigm of this communication group.
     *  @param groupFlags      Flags for this group.
     *  @param numberOfMembers The number of members in this group.
     *  @param members         The identifiers of the group members.
     *
     *  @return @eref{OTF2_CALLBACK_SUCCESS} or @eref{OTF2_CALLBACK_INTERRUPT}.
     */
    static inline OTF2_CallbackCode handle_def_group(void* userData, OTF2_GroupRef groupIdentifier, OTF2_StringRef name,
                                                     OTF2_GroupType groupType, OTF2_Paradigm paradigm,
                                                     OTF2_GroupFlag groupFlags, uint32_t numberOfMembers,
                                                     const uint64_t* members);

    /** @brief Callback which is triggered by a Region definition record.
     *
     *  @param userData	           User data.
     *  @param regionIdentifier    The unique identifier for this Region
     *                             definition.
     *  @param name                Name of the region (demangled name if available).
     *                             References a String definition.
     *  @param canonicalName       Alternative name of the region (e.g. mangled name).
     *                             References a String definition.
     *  @param description         A more detailed description of this region.
     *                             References a String definition.
     *  @param regionRole          Region role.
     *  @param paradigm            Paradigm.
     *  @param regionFlags         Region flags.
     *  @param sourceFile	       The source file where this region was
     *                             declared. References a String definition.
     *  @param beginLineNumber	   Starting line number of this region in
     *                             the source file.
     *  @param endLineNumber	   Ending line number of this region in the
     *                             source file.
     *
     *  @return @eref{OTF2_CALLBACK_SUCCESS} or @eref{OTF2_CALLBACK_INTERRUPT}.
     */
    static inline OTF2_CallbackCode handle_def_region(void* userData, OTF2_RegionRef regionIdentifier,
                                                      OTF2_StringRef name, OTF2_StringRef canonicalName,
                                                      OTF2_StringRef description, OTF2_RegionRole regionRole,
                                                      OTF2_Paradigm paradigm, OTF2_RegionFlag regionFlags,
                                                      OTF2_StringRef sourceFile, uint32_t beginLineNumber,
                                                      uint32_t endLineNumber);

    /** @brief Callback which is triggered by a @ref SystemTreeNode definition record.
     *
     *  @param userData                 User data.
     *
     *  @param systemTreeIdentifier     The unique identifier for this SystemTreeNode definition.
     *  @param name                     Free form instance name of this node. References String
     * definition.
     *  @param className                Free form class name of this node References a @ref String
     *                                  definition.
     *  @param parent                   Parent id of this node. May be
     *                                  @eref{OTF2_UNDEFINED_SYSTEM_TREE_NODE} to indicate that
     *                                  there is no parent. References a @ref SystemTreeNode
     *                                  definition.
     *
     *  @return @eref{OTF2_CALLBACK_SUCCESS} or @eref{OTF2_CALLBACK_INTERRUPT}.
     */
    static inline OTF2_CallbackCode handle_def_system_tree_node(void*                  userData,
                                                                OTF2_SystemTreeNodeRef systemTreeIdentifier,
                                                                OTF2_StringRef name, OTF2_StringRef className,
                                                                OTF2_SystemTreeNodeRef parent);

    static inline OTF2_CallbackCode handle_def_system_tree_node_property(void*                  userData,
                                                                         OTF2_SystemTreeNodeRef systemTreeNode,
                                                                         OTF2_StringRef name, OTF2_StringRef value);

    static inline OTF2_CallbackCode handle_def_comm(void* userData, OTF2_CommRef self, OTF2_StringRef name,
                                                    OTF2_GroupRef group, OTF2_CommRef parent);

    /** @brief Callback which is triggered by a Region definition record.
     *
     *  @param userData	           User data.
     *  @param stringIdentifier    The unique identifier for this String definition.
     *  @param string	           The string, null terminated.
     *
     *  @return @eref{OTF2_CALLBACK_SUCCESS} or @eref{OTF2_CALLBACK_INTERRUPT}.
     */
    static inline OTF2_CallbackCode handle_def_string(void* userData, OTF2_StringRef stringIdentifier,
                                                      const char* string);

    static inline OTF2_CallbackCode handle_def_paradigm(void* userData, OTF2_Paradigm paradigm, OTF2_StringRef name,
                                                        OTF2_ParadigmClass paradigmClass);
};
#endif /* OTF2_READER_H */
