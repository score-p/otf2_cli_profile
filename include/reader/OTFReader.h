/*
 This is part of the OTF-Profiler. Copyright by ZIH, TU Dresden 2016-2018.
 Authors: Maximillian Neumann, Denis Hünich, Jens Doleschal
*/

#ifndef OTFREADER_H
#define OTFREADER_H

#include <otf.h>

#include "tracereader.h"

class OTFReader : public TraceReader {
   public:
    OTFReader() = default;

    ~OTFReader() { close(); };

    void close();
    bool initialize(AllData& alldata);
    bool readDefinitions(AllData& alldata);
    bool readEvents(AllData& alldata);
    bool readStatistics(AllData& alldata);

   private:
    OTF_FileManager* _manager = nullptr;
    OTF_Reader*      _reader  = nullptr;

   private:
    /* *** handlers *** */
    /**
     * Callback function for file creator information.
     *
     *  @param userData     Pointer to user data.
     *  @param stream       Identifies the stream to which this definition belongs to.
     *  @param creator      String which identifies the creator of the
     *                      file e.g. "TAU Version x.y.z".
     *
     *  @param list         Pointer to an OTF_KeyValueList() that contains individual data.
     *
     *  @return             OTF_RETURN_ABORT  for aborting the reading process immediately
     *                      OTF_RETURN_OK     for continue reading
     */
    /* TODO nicht verwendet
    static int handle_def_creator(void* userData, uint32_t stream, const char* creator,
                                  OTF_KeyValueList* list);
    */

    /**
     * Callback function on the trace´s otf-version.
     *
     *  @param userData     Pointer to user data.
     *  @param stream       Identifies the stream to which this definition belongs to.
     *  @param major        major version number
     *  @param minor        minor version number
     *  @param sub          sub version number
     *  @param string       string identifing the version
     *
     *  @param list         Pointer to an OTF_KeyValueList() that contains individual data.
     *
     *  @return             OTF_RETURN_ABORT  for aborting the reading process immediately
     *                      OTF_RETURN_OK     for continue reading
     */
    /* TODO nicht verwendet
    static int handle_def_version(void* userData, uint32_t stream, uint8_t major, uint8_t minor,
                                  uint8_t sub, const char* string, OTF_KeyValueList* list);
    */

    /** @brief Callback function for a comment record.
     *
     *  @param userData     Pointer to user data.
     *  @param stream       Identifies the stream to which this definition belongs to.
     *  @param comment      Arbitrary comment string.
     *
     *  @param list         Pointer to an OTF_KeyValueList() that contains individual data.
     *
     *  @return             OTF_RETURN_ABORT  for aborting the reading process immediately
     *                      OTF_RETURN_OK     for continue reading
     */
    /* TODO nicht verwendet
    static int handle_def_comment(void* userData, uint32_t stream, const char* comment,
                                  OTF_KeyValueList* list);
    */
    /** @brief Callback function for a timer resolution record.
     *
     *  @param userData         Pointer to user data.
     *  @param stream           Identifies the stream to which this definition belongs to.
     *  @param ticksPerSecond   Clock ticks per second of the timer.
     *
     *  @param list             Pointer to an OTF_KeyValueList() that contains individual data.
     *
     *  @return                 OTF_RETURN_ABORT  for aborting the reading process immediately
     *                          OTF_RETURN_OK     for continue reading
     */
    static int handle_def_timerres(void* userData, uint32_t stream, uint64_t ticksPerSecond, OTF_KeyValueList* list);

    /** @brief Callback function for a process definition record.
     *
     *  @param userData     Pointer to user data.
     *  @param stream       Identifies the stream to which this definition belongs to.
     *  @param process      Arbitrary but unique process identifier > 0.
     *  @param name         Name of the process e.g. "Process X".
     *  @param parent       Previously declared parent process identifier or 0 if process has no
     * parent.
     *
     *  @param list         Pointer to an OTF_KeyValueList() that contains individual data.
     *
     *  @return             OTF_RETURN_ABORT  for aborting the reading process immediately
     *                      OTF_RETURN_OK     for continue reading
     */
    static int handle_def_process(void* userData, uint32_t stream, uint32_t process, const char* name, uint32_t parent,
                                  OTF_KeyValueList* list);

    /** @brief Callback function for a process group definition record.
     *
     *  @param userData         Pointer to user data.
     *  @param stream           Identifies the stream to which this definition belongs to.
     *  @param procGroup        Arbitrary but unique process group identifier > 0.
     *  @param name             Name of the process group e.g. "Well Balanced".
     *  @param numberOfProcs    The number of processes in the process group.
     *  @param procs            Vector of process identifiers as provided by
     * OTF_Handler_DefProcess().
     *
     *  @param list             Pointer to an OTF_KeyValueList() that contains individual data.
     *
     *  @return                 OTF_RETURN_ABORT  for aborting the reading process immediately
     *                          OTF_RETURN_OK     for continue reading
     */
    /* TODO nicht verwendet
    static int handle_def_processgroup(void* userData, uint32_t stream, uint32_t procGroup,
                                       const char* name, uint32_t numberOfProcs,
                                       const uint32_t* procs, OTF_KeyValueList* list);
    */

    /** @brief Callback function for a function group definition record.
     *
     *  @param userData     Pointer to user data.
     *  @param stream       Identifies the stream to which this definition belongs to.
     *  @param funcGroup    An arbitrary but unique function group identifier > 0.
     *  @param name         Name of the function group e.g. "Computation".
     *
     *  @param list         Pointer to an OTF_KeyValueList() that contains individual data.
     *
     *  @return             OTF_RETURN_ABORT  for aborting the reading process immediately
     *                      OTF_RETURN_OK     for continue reading
     */
    static int handle_def_functiongroup(void* userData, uint32_t stream, uint32_t funcGroup, const char* name,
                                        OTF_KeyValueList* list);

    /** @brief Callback function for a function definition record.
     *
     *  @param userData             Pointer to user data.
     *  @param stream               Identifies the stream to which this definition belongs to.
     *  @param func                 Arbitrary but unique function identifier > 0.
     *  @param name                 Name of the function e.g. "DoSomething".
     *  @param funcGroup            A function group identifier preliminary provided by
     * OTF_Handler_DefFunctionGroup()
     *                              or 0 for no function group assignment.
     *  @param source               Reference to the function's source code location preliminary
     * provided by
     *                              OTF_Handler_DefScl() or 0 for no source code location
     * assignment.
     *
     *  @param list                 Pointer to an OTF_KeyValueList() that contains individual data.
     *
     *  @return                     OTF_RETURN_ABORT  for aborting the reading process immediately
     *                              OTF_RETURN_OK     for continue reading
     */
    static int handle_def_function(void* userData, uint32_t stream, uint32_t func, const char* name, uint32_t funcGroup,
                                   uint32_t source, OTF_KeyValueList* list);

    /** @brief Callback function for a collective operation definition record.
     *
     *  @param userData             Pointer to user data.
     *  @param stream               Identifies the stream to which this definition belongs to.
     *  @param collOp               An arbitrary but unique collective op. identifier > 0.
     *  @param name                 Name of the collective operation e.g. "MPI_Bcast".
     *  @param type                 One of the five supported collective classes:
     * OTF_COLLECTIVE_TYPE_UNKNOWN (default),
     *                              OTF_COLLECTIVE_TYPE_BARRIER, OTF_COLLECTIVE_TYPE_ONE2ALL,
     * OTF_COLLECTIVE_TYPE_ALL2ONE,
     *                              OTF_COLLECTIVE_TYPE_ALL2ALL.
     *
     *  @param list                 Pointer to an OTF_KeyValueList() that contains individual data.
     *
     *  @return                     OTF_RETURN_ABORT  for aborting the reading process immediately
     *                              OTF_RETURN_OK     for continue reading
     */
    static int handle_def_collop(void* userData, uint32_t stream, uint32_t collOp, const char* name, uint32_t type,
                                 OTF_KeyValueList* list);

    /** @brief Callback function for a counter definition record.
     *
     *  @param userData             Pointer to user data.
     *  @param stream               Identifies the stream to which this definition belongs to.
     *  @param counter              An arbitrary but unique counter identifier.
     *  @param name                 Name of the counter e.g. "Cache Misses".
     *  @param properties           A combination of a type, scope and vartype counter property.
     *  @param counterGroup         A previously defined counter group identifier or 0 for no group.
     *  @param unit                 Unit of the counter e.g. "#" for "number of..." or 0 for no
     * unit.
     *
     *  @param list                 Pointer to an OTF_KeyValueList() that contains individual data.
     *
     *  @return                     OTF_RETURN_ABORT  for aborting the reading process immediately
     *                              OTF_RETURN_OK     for continue reading
     */
    static int handle_def_counter(void* userData, uint32_t stream, uint32_t counter, const char* name,
                                  uint32_t properties, uint32_t counterGroup, const char* unit, OTF_KeyValueList* list);

    /** @brief Callback function for a KeyValue definition.
     *
     *  @param userData             Pointer to user data.
     *  @param stream               Identifies the stream to which this definition belongs to.
     *  @param key                  Arbitrary, unique identifier of the KeyValue.
     *  @param type                 Type of the KeyValue. See OTF_Type().
     *  @param name                 Name of the KeyValue.
     *  @param description          Description of the KeyValue.
     *
     *  @param list                 Pointer to an OTF_KeyValueList() that contains individual data.
     *
     *  @return                     OTF_RETURN_ABORT  for aborting the reading process immediately
     *                              OTF_RETURN_OK     for continue reading
     */
    /* TODO nicht verwendet
    static int handle_def_keyvalue(void* userData, uint32_t stream, uint32_t key, OTF_Type type,
                                   const char* name, const char* description,
                                   OTF_KeyValueList* list);
    */
    /** @brief Callback function for a function entry event.
     *
     *  @param userData             Pointer to user data.
     *  @param time                 The time when the function entry took place.
     *  @param function             Function which has been entered as defined
     *                              with OTF_Writer_defFunction.
     *  @param process              Process where action took place.
     *  @param source               Explicit source code location identifier > 0
     *                              or 0 if no source information available.
     *
     *  @param list                 Pointer to an OTF_KeyValueList() that contains individual data.
     *
     *  @return                     OTF_RETURN_ABORT  for aborting the reading process immediately
     *                              OTF_RETURN_OK     for continue reading
     */
    static int handle_enter(void* userData, uint64_t time, uint32_t function, uint32_t process, uint32_t source,
                            OTF_KeyValueList* list);

    /** @brief Callback function for a function leave event.
     *
     *  @param userData             Pointer to user data.
     *  @param time                 The time when the function leave took place.
     *  @param function             Function which was left or 0 if stack integrety
     *                              checking is not available.
     *  @param process              Process where action took place.
     *  @param source               Explicit source code location identifier > 0
     *                              or 0 if no source information available.
     *
     *  @param list                 Pointer to an OTF_KeyValueList() that contains individual data.
     *
     *  @return                     OTF_RETURN_ABORT  for aborting the reading process immediately
     *                              OTF_RETURN_OK     for continue reading
     */
    static int handle_leave(void* userData, uint64_t time, uint32_t function, uint32_t process, uint32_t source,
                            OTF_KeyValueList* list);

    /** @brief Callback function for a counter measurement event.
     *
     *  @param userData             Pointer to user data.
     *  @param time                 The time when the message was send.
     *  @param process              Process where counter measurment took place.
     *  @param counter              Counter which was measured.
     *  @param value                Counter value.
     *
     *  @param list                 Pointer to an OTF_KeyValueList() that contains individual data.
     *
     *  @return                     OTF_RETURN_ABORT  for aborting the reading process immediately
     *                              OTF_RETURN_OK     for continue reading
     */
    static int handle_counter(void* userData, uint64_t time, uint32_t process, uint32_t counter, uint64_t value,
                              OTF_KeyValueList* list);

    /** @brief Callback function for a message send event.
     *
     *  @param userData             Pointer to user data.
     *  @param time                 The time when the message was send.
     *  @param sender               Sender of the message.
     *  @param receiver             Receiver of the message.
     *  @param group                Process-group to which sender and receiver belong to or 0 for no
     * group assignment.
     *  @param type                 Message type information > 0 or 0 for no information.
     *  @param length               Optional message length information.
     *  @param source               Explicit source code location identifier > 0 or 0 if no source
     * information available.
     *
     *  @param list                 Pointer to an OTF_KeyValueList() that contains individual data.
     *
     *  @return                     OTF_RETURN_ABORT  for aborting the reading process immediately
     *                              OTF_RETURN_OK     for continue reading
     */
    static int handle_send(void* userData, uint64_t time, uint32_t sender, uint32_t receiver, uint32_t group,
                           uint32_t type, uint32_t length, uint32_t source, OTF_KeyValueList* list);

    /** @brief Callback function for a message send event.
     *
     *  @param userData             Pointer to user data.
     *  @param time                 The time when the message was send.
     *  @param recvProc             Identifier of receiving process.
     *  @param sendProc             Identifier of sending process.
     *  @param group                Process-group to which sender and receiver belong to or 0 for no
     * group assignment.
     *  @param type                 Message type information > 0 or 0 for no information.
     *  @param length               Optional message length information.
     *  @param source               Explicit source code location identifier > 0 or 0 if no source
     * information available.
     *
     *  @param list                 Pointer to an OTF_KeyValueList() that contains individual data.
     *
     *  @return                     OTF_RETURN_ABORT  for aborting the reading process immediately
     *                              OTF_RETURN_OK     for continue reading
     */
    static int handle_recv(void* userData, uint64_t time, uint32_t recvProc, uint32_t sendProc, uint32_t group,
                           uint32_t type, uint32_t length, uint32_t source, OTF_KeyValueList* list);

    /** @brief Callback function for a collective operation member event.
     *
     *  @param userData             Pointer to user data.
     *  @param time                 The time when the message was send.
     *  @param process              Process where counter measurment took place.
     *  @param collOp               Collective identifier as defined with
     * OTF_Handler_DefCollectiveOperation().
     *  @param procGroup            Group of processes participating in this collective.
     *  @param rootProc             Root process if != 0.
     *  @param sent                 Data volume sent by member or 0.
     *  @param received             Data volume received by member or 0.
     *  @param duration             Time spent in collective operation.
     *  @param source               Explicit source code location or 0.
     *
     *  @param list                 Pointer to an OTF_KeyValueList() that contains individual data.
     *
     *  @return                     OTF_RETURN_ABORT  for aborting the reading process immediately
     *                              OTF_RETURN_OK     for continue reading
     */
    static int handle_collop(void* userData, uint64_t time, uint32_t process, uint32_t collOp, uint32_t procGroup,
                             uint32_t rootProc, uint32_t sent, uint32_t received, uint64_t duration, uint32_t source,
                             OTF_KeyValueList* list);

    /** @brief Provides a begin collective operation member event.
     *
     *  @param userData             Pointer to user data.
     *  @param time                 Time when collective operation was entered by member.
     *  @param process              Process identifier i.e. collective member.
     *  @param collOp               Collective identifier.
     *  @param matchingId           Identifier for finding the associated end collective event
     * record.
     *                              It must be unique within this process.
     *  @param procGroup            Group of processes participating in this collective.
     *  @param rootProc             Root process if != 0.
     *  @param sent                 Data volume sent by member or 0.
     *  @param received             Data volume received by member or 0.
     *  @param scltoken             Explicit source code location or 0.
     *
     *  @param list                 Pointer to an OTF_KeyValueList() that contains individual data.
     *
     *  @return                     OTF_RETURN_ABORT  for aborting the reading process immediately
     *                              OTF_RETURN_OK     for continue reading
     */
    /* TODO nicht verwendet
    static int handle_begin_collop(void* userData, uint64_t time, uint32_t process, uint32_t collOp,
                                   uint64_t matchingId, uint32_t procGroup, uint32_t rootProc,
                                   uint64_t sent, uint64_t received, uint32_t scltoken,
                                   OTF_KeyValueList* list);
*/
    /** @brief Provides an end collective operation member event.
     *
     *  @param userData             Pointer to user data.
     *  @param time                 Time when collective operation was entered by member.
     *  @param process              Process identifier i.e. collective member.
     *  @param matchingId           Matching identifier, must match a previous start collective
     * operation.
     *
     *  @param list                 Pointer to an OTF_KeyValueList() that contains individual data.
     *
     *  @return                     OTF_RETURN_ABORT  for aborting the reading process immediately
     *                              OTF_RETURN_OK     for continue reading
     */
    /* TODO nicht verwendet
    static int handle_end_collop(void* userData, uint64_t time, uint32_t process,
                                 uint64_t matchingId, OTF_KeyValueList* list);
*/

    /** @brief Callback function for summarized information for a given function.
     *
     *  @param userData     Pointer to user data.
     *  @param time         Time when summary was computed.
     *  @param function     Function as defined with OTF_Handler_DefFunction.
     *  @param process      Process of the given function.
     *  @param invocations  Number of invocations.
     *  @param exclTime     Time spent exclusively in the given function.
     *  @param inclTime     Time spent in the given function including all
     *                      sub-routine calls.
     *
     *  @param list         Pointer to an OTF_KeyValueList() that contains individual data.
     *
     *  @return             OTF_RETURN_ABORT  for aborting the reading process immediately
     *                      OTF_RETURN_OK     for continue reading
     */
    /* TODO nicht verwendet
    static int handle_function_summary(void* userData, uint64_t time, uint32_t function,
                                       uint32_t process, uint64_t invocations, uint64_t exclTime,
                                       uint64_t inclTime, OTF_KeyValueList* list);
    */

    /**
     * Provides summarized information for a given message type.
     *
     *  @param userData       Pointer to user data.
     *  @param time           Time when summary was computed.
     *  @param process        Process where messages originated.
     *  @param peer           Process where the message is sent to
     *  @param comm           Communicator of message summary
     *  @param type           Message type/tag.
     *  @param sentNumber     The number of messages sent.
     *  @param receivedNumber The number of messages received.
     *  @param sentBytes      The number of bytes sent via messages of the given
     *                        type.
     *  @param receivedBytes  The number of bytes received through messages of the
     *                        given type.
     *
     *  @param list           Pointer to an OTF_KeyValueList() that contains individual data.
     *
     *  @return               OTF_RETURN_ABORT  for aborting the reading process immediately
     *                        OTF_RETURN_OK     for continue reading
     */
    /* TODO nicht verwendet
    static int handle_message_summary(void* userData, uint64_t time, uint32_t process,
                                      uint32_t peer, uint32_t comm, uint32_t type,
                                      uint64_t sentNumber, uint64_t receivedNumber,
                                      uint64_t sentBytes, uint64_t receivedBytes,
                                      OTF_KeyValueList* list);
    */

    /**
     * Provides summarized information for collective operations.
     *
     *  @param userData       Pointer to user data.
     *  @param time           Time when summary was computed.
     *  @param process        Process identifier i.e. collective member.
     *  @param comm           Communicator of collective operation summary.
     *  @param collective     Collective identifier as defined with
     *                        OTF_Handler_DefCollectiveOperation().
     *  @param sentNumber     The number of messages sent by member or 0.
     *  @param receivedNumber The number of messages received by member or 0.
     *  @param sentBytes      The number of bytes sent by member or 0.
     *  @param receivedBytes  The number of bytes received by member or 0.
     *
     *  @param list           Pointer to an OTF_KeyValueList() that contains individual data.
     *
     *  @return               OTF_RETURN_ABORT  for aborting the reading process immediately
     *                        OTF_RETURN_OK     for continue reading
     */
    /* TODO nicht verwendet
    static int handle_collop_summary(void* userData, uint64_t time, uint32_t process, uint32_t comm,
                                     uint32_t collective, uint64_t sentNumber,
                                     uint64_t receivedNumber, uint64_t sentBytes,
                                     uint64_t receivedBytes, OTF_KeyValueList* list);
    */

    /** @brief Callback function for records which cannot be read.
     *
     *  @param userData             Pointer to user data.
     *  @param time                 Time when summary was computed.
     *  @param process              If 'time' equals (uin64_t) -1, the unknown record is a definiton
     * record
     *                              and 'process' represents the streamid of the record. If 'time'
     * has a
     *                              valid value ( not (uint64)-1 ) the unknown record is an event-,
     * statistics-
     *                              or snapshotrecord and 'process' represents the processid of the
     * record.
     *  @param record               String which contains the record.
     *
     *  @return Returns OTF_RETURN_ABORT for aborting the reading process immediately,
     *          OTF_RETURN_OK for continue reading.
     */
    /* TODO nicht verwendet
static int handleUnknownRecord(void* userData, uint64_t time, uint32_t process,
                               const char* record);
*/

    /** @brief Callback function for rma put records.
     *
     *  @param userData             Pointer to user data.
     *  @param time                 Time when summary was computed.
     *  @param process              Process identifier i.e. collective member.
     *  @param origin               Initiating process / thread
     *  @param target               Targetedd process / thread
     *  @param communicator         Communicator identifier to match put start with it's end
     * (together with tag)
     *  @tag                        Tag to match put start with it's end (together with
     * communicator).
     *  @bytes                      Amount of bytes put
     *  @source                     Explicit source code location or 0.
     *
     *  @return                     OTF_RETURN_ABORT  for aborting the reading process immediately
     *                              OTF_RETURN_OK     for continue reading
     */
    /* TODO nicht verwendet
    static int handle_rma_put(void* fha, uint64_t time, uint32_t process, uint32_t origin,
                              uint32_t target, uint32_t communicator, uint32_t tag, uint64_t bytes,
                              uint32_t source, OTF_KeyValueList* list);
    */

    /** @brief Callback function for rma get records.
     *
     *  @param userData             Pointer to user data.
     *  @param time                 Time when summary was computed.
     *  @param process              Process identifier i.e. collective member.
     *  @param origin               Initiating process / thread
     *  @param target               Targetedd process / thread
     *  @param communicator         Communicator identifier to match get start with it's end
     * (together with tag)
     *  @tag                        Tag to match get start with it's end (together with
     * communicator).
     *  @bytes                      Amount of bytes get
     *  @source                     Explicit source code location or 0.
     *
     *  @return                     OTF_RETURN_ABORT  for aborting the reading process immediately
     *                              OTF_RETURN_OK     for continue reading
     */
    /* TODO nicht verwendet
    static int handle_rma_get(void* fha, uint64_t time, uint32_t process, uint32_t origin,
                              uint32_t target, uint32_t communicator, uint32_t tag, uint64_t bytes,
                              uint32_t source, OTF_KeyValueList* list);
    */

    /*
     * MISSING CALLBACKS:
     int  OTF_Handler_DefAttributeList (void *userData, uint32_t stream, uint32_t attr_token,
     uint32_t num, OTF_ATTR_TYPE *array, OTF_KeyValueList *list)
     int  OTF_Handler_DefProcessOrGroupAttributes (void *userData, uint32_t stream, uint32_t
     proc_token, uint32_t attr_token, OTF_KeyValueList *list)
     int  OTF_Handler_DefFile (void *userData, uint32_t stream, uint32_t token, const char *name,
     uint32_t group, OTF_KeyValueList *list)
     int  OTF_Handler_DefFileGroup (void *userData, uint32_t stream, uint32_t token, const char
     *name, OTF_KeyValueList *list)
     int  OTF_Handler_DefKeyValue (void *userData, uint32_t stream, uint32_t key, OTF_Type type,
     const char *name, const char *description, OTF_KeyValueList *list)
     int  OTF_Handler_DefTimeRange (void *userData, uint32_t stream, uint64_t minTime, uint64_t
     maxTime, OTF_KeyValueList *list)
     int  OTF_Handler_DefCounterAssignments (void *userData, uint32_t stream, uint32_t counter,
     uint32_t number_of_members, const uint32_t *procs_or_groups, OTF_KeyValueList *list)
     int  OTF_Handler_NoOp (void *userData, uint64_t time, uint32_t process, OTF_KeyValueList *list)
     int  OTF_Handler_BeginFileOperation (void *userData, uint64_t time, uint32_t process, uint64_t
     matchingId, uint32_t scltoken, OTF_KeyValueList *list)
     int  OTF_Handler_EndFileOperation (void *userData, uint64_t time, uint32_t process, uint32_t
     fileid, uint64_t matchingId, uint64_t handleId, uint32_t operation, uint64_t bytes, uint32_t
     scltoken, OTF_KeyValueList *list)
     int  OTF_Handler_RMAPut (void *userData, uint64_t time, uint32_t process, uint32_t origin,
     uint32_t target, uint32_t communicator, uint32_t tag, uint64_t bytes, uint32_t source,
     OTF_KeyValueList *list)
     int  OTF_Handler_RMAPutRemoteEnd (void *userData, uint64_t time, uint32_t process, uint32_t
     origin, uint32_t target, uint32_t communicator, uint32_t tag, uint64_t bytes, uint32_t source,
     OTF_KeyValueList *list)
     int  OTF_Handler_RMAGet (void *userData, uint64_t time, uint32_t process, uint32_t origin,
     uint32_t target, uint32_t communicator, uint32_t tag, uint64_t bytes, uint32_t source,
     OTF_KeyValueList *list)
     int  OTF_Handler_RMAEnd (void *userData, uint64_t time, uint32_t process, uint32_t remote,
     uint32_t communicator, uint32_t tag, uint32_t source, OTF_KeyValueList *list)
     int  OTF_Handler_SnapshotComment (void *userData, uint64_t time, uint32_t process, const char
     *comment, OTF_KeyValueList *list)
     int  OTF_Handler_EnterSnapshot (void *userData, uint64_t time, uint64_t originaltime, uint32_t
     function, uint32_t process, uint32_t source, OTF_KeyValueList *list)
     int  OTF_Handler_SendSnapshot (void *userData, uint64_t time, uint64_t originaltime, uint32_t
     sender, uint32_t receiver, uint32_t procGroup, uint32_t tag, uint32_t length, uint32_t source,
     OTF_KeyValueList *list)
     int  OTF_Handler_OpenFileSnapshot (void *userData, uint64_t time, uint64_t originaltime,
     uint32_t fileid, uint32_t process, uint64_t handleid, uint32_t source, OTF_KeyValueList *list)
     int  OTF_Handler_BeginCollopSnapshot (void *userData, uint64_t time, uint64_t originaltime,
     uint32_t process, uint32_t collOp, uint64_t matchingId, uint32_t procGroup, uint32_t rootProc,
     uint64_t sent, uint64_t received, uint32_t scltoken, OTF_KeyValueList *list)
     int  OTF_Handler_BeginFileOpSnapshot (void *userData, uint64_t time, uint64_t originaltime,
     uint32_t process, uint64_t matchingId, uint32_t scltoken, OTF_KeyValueList *list)
     int  OTF_Handler_SummaryComment (void *userData, uint64_t time, uint32_t process, const char
     *comment, OTF_KeyValueList *list)
     int  OTF_Handler_FunctionSummary (void *userData, uint64_t time, uint32_t function, uint32_t
     process, uint64_t invocations, uint64_t exclTime, uint64_t inclTime, OTF_KeyValueList *list)
     int  OTF_Handler_FunctionGroupSummary (void *userData, uint64_t time, uint32_t funcGroup,
     uint32_t process, uint64_t invocations, uint64_t exclTime, uint64_t inclTime, OTF_KeyValueList
     *list)
     int  OTF_Handler_MessageSummary (void *userData, uint64_t time, uint32_t process, uint32_t
     peer, uint32_t comm, uint32_t type, uint64_t sentNumber, uint64_t receivedNumber, uint64_t
     sentBytes, uint64_t receivedBytes, OTF_KeyValueList *list)
     int  OTF_Handler_CollopSummary (void *userData, uint64_t time, uint32_t process, uint32_t comm,
     uint32_t collective, uint64_t sentNumber, uint64_t receivedNumber, uint64_t sentBytes, uint64_t
     receivedBytes, OTF_KeyValueList *list)
     int  OTF_Handler_FileOperationSummary (void *userData, uint64_t time, uint32_t fileid, uint32_t
     process, uint64_t nopen, uint64_t nclose, uint64_t nread, uint64_t nwrite, uint64_t nseek,
     uint64_t bytesread, uint64_t byteswrite, OTF_KeyValueList *list)
     int  OTF_Handler_FileGroupOperationSummary (void *userData, uint64_t time, uint32_t groupid,
     uint32_t process, uint64_t nopen, uint64_t nclose, uint64_t nread, uint64_t nwrite, uint64_t
     nseek, uint64_t bytesread, uint64_t byteswrite, OTF_KeyValueList *list)
     int  OTF_Handler_DefMarker (void *userData, uint32_t stream, uint32_t token, const char *name,
     uint32_t type, OTF_KeyValueList *list)
     int  OTF_Handler_Marker (void *userData, uint64_t time, uint32_t process, uint32_t token, const
     char *text, OTF_KeyValueList *list)
     */
};

#endif /* OTFREADER_H */
