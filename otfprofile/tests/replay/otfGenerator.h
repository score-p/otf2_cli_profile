#include <otf.h>
#include <string>
#include <vector>

using namespace std;

typedef enum enum_BufEntryType{
	BufEntryType_DefFunction,
	BufEntryType_DefTimerResolution,
	BufEntryType_DefProcessGroup,
	BufEntryType_DefKeyValue,
	BufEntryType_Enter,
	BufEntryType_Leave,
	BufEntryType_SendMsg,
	BufEntryType_RecvMsg,
	BufEntryType_BeginCollOp,
	BufEntryType_EndCollOp
} BufEntryType;

typedef struct struct_BufEntry_Base{
	BufEntryType type;
	struct_BufEntry_Base(BufEntryType type) : type(type) { }
} BufEntry_Base;


typedef struct struct_BufEntry_BaseEvent : BufEntry_Base{
	const uint64_t time;
	OTF_KeyValueList* list;
	struct_BufEntry_BaseEvent(BufEntryType type, const uint64_t time, OTF_KeyValueList* list) : 
			BufEntry_Base(type), time(time), list(list) { }
} BufEntry_BaseEvent;

typedef struct struct_BufEntry_DefFunction : public BufEntry_Base{
	uint32_t func;
	char* name;
	uint32_t funcGroup;
	uint32_t source;
	struct_BufEntry_DefFunction(uint32_t func, char* name, uint32_t funcGroup, uint32_t source) : 
			BufEntry_Base(BufEntryType_DefFunction), name(name), funcGroup(funcGroup), source(source) { }
			
} BufEntry_DefFunction;



typedef struct struct_BufEntry_DefTimerResolution : public BufEntry_Base{
	uint64_t ticksPerSecond;
	struct_BufEntry_DefTimerResolution(uint64_t ticksPerSecond) : 
			BufEntry_Base(BufEntryType_DefTimerResolution), ticksPerSecond(ticksPerSecond) { }
			
} BufEntry_DefTimerResolution;

typedef struct struct_BufEntry_DefProcessGroup : public BufEntry_Base{
	const uint32_t procGroup;
	const char* name;
	const uint32_t numProcs;
	const uint32_t* procs;
	struct_BufEntry_DefProcessGroup(const uint32_t procGroup, const char* name, const uint32_t numProcs, const uint32_t* procs) : 
			BufEntry_Base(BufEntryType_DefProcessGroup), procGroup(procGroup), name(name), numProcs(numProcs), procs(procs) { }
			
} BufEntry_DefProcessGroup;


typedef struct struct_BufEntry_DefKeyValue : public BufEntry_Base{
	const uint32_t token;
	const char* name;
	OTF_KeyValueList* list;
	struct_BufEntry_DefKeyValue(const uint32_t token, const char* name, OTF_KeyValueList* list) : 
			BufEntry_Base(BufEntryType_DefKeyValue), token(token), name(name), list(list) { }
			
} BufEntry_DefKeyValue;

typedef struct struct_BufEntry_Enter : public BufEntry_BaseEvent{
	const uint32_t function;
	const uint32_t process;
	const uint32_t source;
	struct_BufEntry_Enter(const uint64_t time, const uint32_t function, const uint32_t process, const uint32_t source, OTF_KeyValueList* list):
			BufEntry_BaseEvent(BufEntryType_Enter, time, list), function(function), process(process), source(source) { }
} BufEntry_Enter;

typedef struct struct_BufEntry_Leave : public BufEntry_BaseEvent{
	const uint32_t function;
	const uint32_t process;
	const uint32_t source;
	struct_BufEntry_Leave(const uint64_t time, const uint32_t function, const uint32_t process, const uint32_t source, OTF_KeyValueList* list):
			BufEntry_BaseEvent(BufEntryType_Leave, time, list), function(function), process(process), source(source) { } 
} BufEntry_Leave;

typedef struct struct_BufEntry_BeginCollOp : public BufEntry_BaseEvent{
	const uint32_t process;
 	const uint32_t collOp;
	const uint64_t matchingId; 
	const uint32_t procGroup;
 	const uint32_t rootProc;
	const uint64_t sent;
	const uint64_t received;
 	const uint32_t scltoken;
	struct_BufEntry_BeginCollOp(const uint64_t time, const uint32_t process,
				    const uint32_t collOp, const uint64_t matchingId, const uint32_t procGroup,
				    const uint32_t rootProc, const uint64_t sent, const uint64_t received,
 				    const uint32_t scltoken, OTF_KeyValueList *list) :
			BufEntry_BaseEvent(BufEntryType_BeginCollOp, time, list), process(process), collOp(collOp), matchingId(matchingId),
			procGroup(procGroup), rootProc(rootProc), sent(sent), received(received), scltoken(scltoken) { }
} BufEntry_BeginCollOp;

typedef struct struct_BufEntry_EndCollOp : public BufEntry_BaseEvent{
	const uint32_t process;
	const uint64_t matchingId;
	struct_BufEntry_EndCollOp(const uint64_t time, const uint32_t process, const uint64_t matchingId, OTF_KeyValueList *list) :
			BufEntry_BaseEvent(BufEntryType_EndCollOp, time, list), process(process), matchingId(matchingId) { }
} BufEntry_EndCollOp;


class OtfGenerator{
	private:
		string namestub;
		OTF_FileManager* fileman;
		OTF_WStream* wstream;
	
	public:	
		OtfGenerator(const string& namestub, int id);
		~OtfGenerator();
		
		int writeDefFunction(const uint32_t func, const char *name,
				       const uint32_t funcGroup, const uint32_t source);
		int writeDefTimerResolution(const uint64_t ticksPerSecond);
		int writeDefProcessGroup(const uint32_t procGroup, const char *name, 
					 const uint32_t numberOfProcs, const uint32_t *procs);
		int writeDefKeyValue(const uint32_t token, const char *name, const OTF_KeyValueList *list);

		int writeEnter(const uint64_t time, const uint32_t function, const uint32_t process,
				 const uint32_t source, OTF_KeyValueList *list);

		int writeLeave (const uint64_t time, const uint32_t function, const uint32_t process,
				 const uint32_t source, OTF_KeyValueList *list);

		int writeSendMsg (const uint64_t time, const uint32_t sender, const uint32_t receiver,
				   const uint32_t group, const uint32_t type, const uint32_t length, 
       				   const uint32_t source, OTF_KeyValueList *list);

		int writeRecvMsg (const uint64_t time, const uint32_t recvProc, const uint32_t sendProc,
				   const uint32_t group, const uint32_t type, const uint32_t length, const uint32_t source, OTF_KeyValueList *list);
		
		int writeBeginCollectiveOperation(const uint64_t time, const uint32_t process,
				const uint32_t collOp, const uint64_t matchingId, const uint32_t procGroup,
    				const uint32_t rootProc, const uint64_t sent, const uint64_t received,
    				const uint32_t scltoken, OTF_KeyValueList *list);

		int writeEndCollectiveOperation(const uint64_t time, const uint32_t process, const uint64_t matchingId, OTF_KeyValueList *list);
	
	
};
