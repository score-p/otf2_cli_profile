//
//  OTF2Reader.cpp
//  
//
//  Created by Jens Doleschal on 23/06/14.
//
//

#include <iostream>
#include <sstream>

#include "OTF2Reader.h"

#if MPI_VERSION < 3
#define OTF2_MPI_UINT64_T MPI_UNSIGNED_LONG
#define OTF2_MPI_INT64_T  MPI_LONG
#endif

#ifdef OTFPROFILE_MPI
#include <otf2/OTF2_MPI_Collectives.h>
#endif /* OTFPROFILE_MPI */

/** OTF2 reader handle */
static OTF2_Reader* reader;

static std::map< OTF2_StringRef, string > stringIdToString;

static uint64_t systemTreeNodeId;

/** */
static uint64_t location_pos, location_it, rma_cpt_nid;

static string OTF2ParadigmToString(OTF2_Paradigm paradigm);

//tmp data for metric(counter) values -> firstValue
static map< uint64_t, StackType::CounterData > tmp_data;

//only metrics wich are strictly synchronous
static set< uint64_t > allowed_metrics;

/* TMP for OTF2 metrics */

static string
OTF2ParadigmToString( OTF2_Paradigm paradigm )
{
    switch ( paradigm )
    {
        case OTF2_PARADIGM_UNKNOWN:
            return "UNKNOWN";
        case OTF2_PARADIGM_USER:
            return "USER";
        case OTF2_PARADIGM_COMPILER:
            return "COMPILER";
        case OTF2_PARADIGM_OPENMP:
            return "OPENMP";
        case OTF2_PARADIGM_MPI:
            return "MPI";
        case OTF2_PARADIGM_CUDA:
            return "CUDA";
        case OTF2_PARADIGM_MEASUREMENT_SYSTEM:
            return "MEASUREMENT_SYSTEM";
        case OTF2_PARADIGM_PTHREAD:
            return "PTHREAD";
        case OTF2_PARADIGM_HMPP:
            return "HMPP";
        case OTF2_PARADIGM_OMPSS:
            return "OMPSS";
        case OTF2_PARADIGM_HARDWARE:
            return "HARDWARE";
        case OTF2_PARADIGM_GASPI:
            return "GASPI";
        case OTF2_PARADIGM_UPC:
            return "UPC";
        case OTF2_PARADIGM_SHMEM:
            return "SHMEM";

        default:
            return "UNKNOWN";
    }
}

OTF2Reader::OTF2Reader()
{

    reader = NULL;
    location_pos= 0;
    location_it = 0;
    systemTreeNodeId = 0;

}

OTF2Reader::~OTF2Reader()
{

    close();
    
}

bool
OTF2Reader::open( AllData& alldata )
{
    bool error= false;
    
    reader = OTF2_Reader_Open(alldata.params.input_file_name.c_str() );

    if ( NULL == reader ) {

        error= true;
        return !error;

    }
    
#ifdef OTFPROFILE_MPI
    OTF2_MPI_Reader_SetCollectiveCallbacks( reader, MPI_COMM_WORLD );
#endif /* OTFPROFILE_MPI */
    
    return !error;

}

void
OTF2Reader::close()
{
    if ( NULL != reader ) {

        OTF2_Reader_Close(reader);
        reader = NULL;

    }
}

bool
OTF2Reader::assignProcs( AllData& alldata )
{

    bool error= false;
    
    uint64_t number_of_locations;

    OTF2_Reader_GetNumberOfLocations( reader, &number_of_locations );
    
    uint64_t r_ranks= number_of_locations;
    uint32_t r_workers= alldata.numRanks;
    
    uint64_t pos= 0;
    bool warn_for_empty= true;
    for ( int w= 0; w < (int)alldata.numRanks; w++ ) {
        
        uint32_t n= ( ( r_ranks / r_workers ) * r_workers < r_ranks) ?
        ( r_ranks / r_workers +1 ) : ( r_ranks / r_workers );
        
        if ( ( 0 == n ) && warn_for_empty ) {
            
            std::cerr << "Warning: more analysis ranks than trace processes, "
            << "ranks " << w << " to " << alldata.numRanks -1
            << " are unemployed" << std::endl;
            
            warn_for_empty= false;

        }
        
        if ( alldata.myRank == w ) {
            
            alldata.myProcessesNum= n;
            location_pos = pos;
            alldata.myProcessesList = (uint64_t*) malloc(alldata.myProcessesNum
                                                         * sizeof(uint64_t));

        }

        pos += n;
        r_ranks -= n;
        r_workers -= 1;

    }

    return !error;

}

/* ****************************************************************** */
/*                                                                    */
/*                            DEFINITIONS                             */
/*                                                                    */
/* ****************************************************************** */


OTF2_CallbackCode
OTF2Reader::handle_def_clock_properties( void*    userData,
                                         uint64_t timerResolution,
                                         uint64_t globalOffset,
                                         uint64_t traceLength )
{

    AllData* alldata= (AllData*) userData;
    
    /*
     * At the moment Score-P writes one global clock resolution definition.
     * If there will be multiple clock resolution definitions in the future,
     * OTF2 Reader and Token Manager need to be adapted.
     */
    
    alldata->timerResolution= timerResolution;
    alldata->clockOffset= globalOffset;
    
    return OTF2_CALLBACK_SUCCESS;

}

//narrowing the metrics to strict synchronous -> to align them with function calls
OTF2_CallbackCode
OTF2Reader::handle_def_metric_class(    void*                       userData,
                                        OTF2_MetricRef              self,
                                        uint8_t                     numberOfMetrics,
                                        const OTF2_MetricMemberRef* metricMembers,
                                        OTF2_MetricOccurrence       metricOccurence,
                                        OTF2_RecorderKind           recorderKind )
{

    if( metricOccurence == OTF2_METRIC_SYNCHRONOUS_STRICT ) {

        allowed_metrics.insert( self );

    }

    return OTF2_CALLBACK_SUCCESS;

}

OTF2_CallbackCode
OTF2Reader::handle_def_metrics( void*                   userData,
                                OTF2_MetricMemberRef    self,
                                OTF2_StringRef          name,
                                OTF2_StringRef          description,
                                OTF2_MetricType         metricType,
                                OTF2_MetricMode         metricMode,
                                OTF2_Type               valueType,
                                OTF2_Base               base,
                                int64_t                 exponent,
                                OTF2_StringRef          unit)
{

    AllData* alldata= (AllData*) userData;

    alldata->countersOfInterest.insert( self );
        
    alldata->counterIdNameMap[self] = stringIdToString.find( name )->second;

    return OTF2_CALLBACK_SUCCESS;

}

OTF2_CallbackCode
OTF2Reader::handle_def_location_group( void*                  userData,
                                        OTF2_LocationGroupRef  groupIdentifier,
                                        OTF2_StringRef         name,
                                        OTF2_LocationGroupType locationGroupType,
                                        OTF2_SystemTreeNodeRef systemTreeParent )
{

    AllData* alldata = (AllData*) userData;
    map<OTF2_StringRef,string>::iterator iter_string;

    if( (iter_string=stringIdToString.find(name)) == stringIdToString.end() ) {

        /* error unknown string definition */        
        return OTF2_CALLBACK_INTERRUPT;

    }

    switch (locationGroupType)
    {
        case OTF2_LOCATION_GROUP_TYPE_PROCESS:
        {


            //workaround for csv output
            alldata->processIdNameMap[groupIdentifier] = iter_string->second;


            uint64_t locationgroupid= systemTreeNodeId++;
            
            map< Pair, uint64_t>::iterator iter_idmapping;
            
            if ( (iter_idmapping=alldata->traceIdToSystemTreeId.find(Pair(SYSTEMTREE_NODE,systemTreeParent))) != alldata->traceIdToSystemTreeId.end() ) {

                alldata->systemTreeIdToSystemTreeNodeMap[locationgroupid] = SystemTreeNode(iter_string->second,SYSTEMTREE_LOCATION_GROUP,0,iter_idmapping->second);
                alldata->systemTreeClassToNodeIdsMap[SYSTEMTREE_LOCATION_GROUP].insert(locationgroupid);
                
                alldata->systemTreeIdToSystemTreeNodeMap[iter_idmapping->second].children.insert(locationgroupid);
            
                alldata->traceIdToSystemTreeId[ Pair(SYSTEMTREE_LOCATION_GROUP, groupIdentifier) ] = locationgroupid;

            }else {
                /* unknown system tree parent */
            }
            
            break;

        }

        case OTF2_LOCATION_GROUP_TYPE_UNKNOWN:
        {

            break;

        }

        default:
        {

            /* unknown location group type */
            break;

        }

    }
    
    return OTF2_CALLBACK_SUCCESS;

}

OTF2_CallbackCode
OTF2Reader::handle_def_location( void*                 userData,
                                 OTF2_LocationRef      locationIdentifier,
                                 OTF2_StringRef        name,
                                 OTF2_LocationType     locationType,
                                 uint64_t              numberOfEvents,
                                 OTF2_LocationGroupRef locationGroup )
{

    AllData* alldata = ( AllData* )userData;
    map<OTF2_StringRef,string>::iterator iter_string;

    if (  location_pos <= location_it && location_it < location_pos + alldata->myProcessesNum ) {
        
        alldata->myProcessesList[location_it-location_pos]= locationIdentifier; 

    }

    if( (iter_string=stringIdToString.find(name)) == stringIdToString.end() ) {

        /* error unknown string definition */      
        return OTF2_CALLBACK_INTERRUPT;

    }

    location_it++;

    switch (locationType) {
        case OTF2_LOCATION_TYPE_CPU_THREAD:
        {

            OTF2_Reader_SelectLocation( reader, locationIdentifier );
            
           
            alldata->allProcesses.insert( Process(locationIdentifier,0) );
            
            uint64_t locationid = systemTreeNodeId++;
            
            map< Pair, uint64_t>::iterator iter_idmapping;
            
            if ( (iter_idmapping=alldata->traceIdToSystemTreeId.find(Pair(SYSTEMTREE_LOCATION_GROUP,locationGroup))) != alldata->traceIdToSystemTreeId.end() ) {

                alldata->systemTreeIdToSystemTreeNodeMap[iter_idmapping->second].children.insert(locationid);


                set< uint64_t >& children_ref = alldata->systemTreeIdToSystemTreeNodeMap[iter_idmapping->second].children;

                uint64_t dist = distance( children_ref.begin(), children_ref.find( locationid ) );
        
                //string stream to add ":<number>" to location name, to differentiate between OMP_Thread 1 on Rank 0 and on Rank 1 etc.
                    //-> OMP_Thread 1:0, OMP_Thread 1:1 etc....
                ostringstream o;


                 if ( iter_string->second.length() == 0 ) { 

                    //fix for semi broken traces with no location name
                    o << "Thread " << dist << ":" << locationGroup;
                    alldata->processIdNameMap[locationIdentifier] = o.str();

                    alldata->systemTreeIdToSystemTreeNodeMap[locationid] = SystemTreeNode( o.str(), SYSTEMTREE_LOCATION, locationIdentifier, iter_idmapping->second );

                } else {

                    o << iter_string->second << ":" << locationGroup;
                    alldata->processIdNameMap[locationIdentifier] = o.str();

                    alldata->systemTreeIdToSystemTreeNodeMap[locationid] = SystemTreeNode( o.str(), SYSTEMTREE_LOCATION, locationIdentifier, iter_idmapping->second );

                }



                alldata->systemTreeClassToNodeIdsMap[SYSTEMTREE_LOCATION].insert(locationid);
                              
                alldata->traceIdToSystemTreeId[ Pair(SYSTEMTREE_LOCATION, locationIdentifier) ] = locationid;


            }else {
                /* unknown system tree parent */
            }
            
            break;

        }

        case OTF2_LOCATION_TYPE_GPU:
        {

            alldata->processIdNameMap[locationIdentifier] = iter_string->second;

            alldata->allProcesses.insert( Process(locationIdentifier,0) );
            
            uint64_t locationid = systemTreeNodeId++;
            
            map< Pair, uint64_t>::iterator iter_idmapping;
            
            if ( (iter_idmapping=alldata->traceIdToSystemTreeId.find(Pair(SYSTEMTREE_LOCATION_GROUP,locationGroup))) != alldata->traceIdToSystemTreeId.end() ) {
                
                alldata->systemTreeIdToSystemTreeNodeMap[iter_idmapping->second].children.insert(locationid);


                set< uint64_t >& children_ref = alldata->systemTreeIdToSystemTreeNodeMap[iter_idmapping->second].children;

                uint64_t dist = distance( children_ref.begin(), children_ref.find( locationid ) );
        
                ostringstream o;


                 if ( iter_string->second.length() == 0 ) { 

                    o << "GPU_Thread " << dist << ":" << locationGroup;
                    alldata->processIdNameMap[locationIdentifier] = o.str();

                    alldata->systemTreeIdToSystemTreeNodeMap[locationid] = SystemTreeNode( o.str(), SYSTEMTREE_LOCATION, locationIdentifier, iter_idmapping->second );

                } else {

                    o << iter_string->second << ":" << locationGroup;
                    alldata->processIdNameMap[locationIdentifier] = o.str();

                    alldata->systemTreeIdToSystemTreeNodeMap[locationid] = SystemTreeNode( o.str(), SYSTEMTREE_LOCATION, locationIdentifier, iter_idmapping->second );

                }

                alldata->systemTreeClassToNodeIdsMap[SYSTEMTREE_LOCATION].insert(locationid);
                
                alldata->traceIdToSystemTreeId[ Pair(SYSTEMTREE_LOCATION, locationIdentifier) ] = locationid;

            }else {
                /* unknown system tree parent */
            }
            


            break;

        }
        case OTF2_LOCATION_TYPE_METRIC:
        {
            break;
        }

        case OTF2_LOCATION_TYPE_UNKNOWN:
        {
            break;
        }

        default:
        {    
            /* unknown location type */
            break;
        }

    }
    
    return OTF2_CALLBACK_SUCCESS;

}

OTF2_CallbackCode
OTF2Reader::handle_def_group( void*           userData,
                              OTF2_GroupRef   groupIdentifier,
                              OTF2_StringRef  name,
                              OTF2_GroupType  groupType,
                              OTF2_Paradigm   paradigm,
                              OTF2_GroupFlag  groupFlags,
                              uint32_t        numberOfMembers,
                              const uint64_t* members )
{

    AllData* alldata = ( AllData* )userData;
    map<OTF2_StringRef,string>::iterator iter_string;

    if( (iter_string=stringIdToString.find(name)) == stringIdToString.end() ) {

        /* error unknown string definition */        
        return OTF2_CALLBACK_INTERRUPT;

    }
    
    switch (groupType) {

        case OTF2_GROUP_TYPE_METRIC:
        {
            break;
        }

        case OTF2_GROUP_TYPE_COMM_GROUP:

            if( groupType == OTF2_GROUP_TYPE_COMM_GROUP ) {

                vector< uint64_t> tmp_vec;

                for (uint32_t i = 0; i < numberOfMembers; ++i) {

                    tmp_vec.push_back( members[i] ) ;                

                }

                alldata->comRankToWorldRank.insert( make_pair( groupIdentifier, tmp_vec ) );

            }

            //FALLTHRU

        case OTF2_GROUP_TYPE_LOCATIONS:

            //FALLTHRU

        case OTF2_GROUP_TYPE_COMM_LOCATIONS:

            //FALLTHRU

        case OTF2_GROUP_TYPE_COMM_SELF:
        {

            assert(alldata->groups.find(groupIdentifier) == alldata->groups.end());
            alldata->groups[groupIdentifier].clear();
            
            for (uint32_t i = 0; i < numberOfMembers; ++i) {

                alldata->groups[groupIdentifier].insert(members[i]);

            }
            
            break;

        }

        case OTF2_GROUP_TYPE_REGIONS:
        {    

            alldata->functionGroupIdNameMap[groupIdentifier]= iter_string->second;
            break;

        }
   
        default:
        {

            break;
        
        }

    }

    return OTF2_CALLBACK_SUCCESS;

}

OTF2_CallbackCode
OTF2Reader::handle_def_region( void*           userData,
                               OTF2_RegionRef  regionIdentifier,
                               OTF2_StringRef  name,
                               OTF2_StringRef  canonicalName,
                               OTF2_StringRef  description,
                               OTF2_RegionRole regionRole,
                               OTF2_Paradigm   paradigm,
                               OTF2_RegionFlag regionFlags,
                               OTF2_StringRef  sourceFile,
                               uint32_t        beginLineNumber,
                               uint32_t        endLineNumber )
{

    AllData* alldata = ( AllData* )userData;
    map<OTF2_StringRef,string>::iterator iter_string;

	string filename;    

    if( (iter_string=stringIdToString.find(name)) == stringIdToString.end() ) {

        /* error unknown string definition */        
        return OTF2_CALLBACK_INTERRUPT;

    }

    alldata->functionIdNameMap[regionIdentifier] = iter_string->second;

    string workAround;

    map<OTF2_StringRef,string>::iterator it;

	if( (it = stringIdToString.find(sourceFile)) == stringIdToString.end()) {

        workAround = "UNKNOWN";

	} else {

        workAround = it->second;

    }

	alldata->functionDetails.insert(pair<uint64_t, functionDetail >(regionIdentifier, functionDetail(beginLineNumber, workAround)));

    alldata->functionIdToGroupIdMap[regionIdentifier] = paradigm;

    alldata->functionGroupIdNameMap[paradigm] = OTF2ParadigmToString(paradigm);

    return OTF2_CALLBACK_SUCCESS;

}


OTF2_CallbackCode
OTF2Reader::handle_def_system_tree_node( void*                  userData,
                                         OTF2_SystemTreeNodeRef systemTreeIdentifier,
                                         OTF2_StringRef         name,
                                         OTF2_StringRef         className,
                                         OTF2_SystemTreeNodeRef parent )
{

    AllData* alldata = ( AllData* )userData;
    map<OTF2_StringRef,string>::iterator iter_string;

    if( (iter_string=stringIdToString.find(name)) == stringIdToString.end() ) {

        /* error unknown string definition */        
        return OTF2_CALLBACK_INTERRUPT;

    }

    string namenode= iter_string->second;
    
    if( (iter_string=stringIdToString.find(className)) == stringIdToString.end() ) {

        /* error unknown string definition */        
        return OTF2_CALLBACK_INTERRUPT;

    }
    
    string nameclass = iter_string->second;
    uint64_t nodeid = systemTreeNodeId++;


    SystemTreeClass classtype;
    
    if ( 0 == nameclass.compare("machine") ) {

        classtype = SYSTEMTREE_MACHINE;

    } else if ( 0 == nameclass.compare("cabinet row") ) {

        classtype = SYSTEMTREE_CABINET_ROW;

    } else if ( 0 == nameclass.compare("cabinet") || 0 == nameclass.compare("rack") ) {

        classtype = SYSTEMTREE_CABINET;

    } else if ( 0 == nameclass.compare("cage") || 0 == nameclass.compare("midplane") ) {

        classtype = SYSTEMTREE_CAGE;

    } else if ( 0 == nameclass.compare("blade") || 0 == nameclass.compare("nodeboard") ) {

        classtype = SYSTEMTREE_BLADE;

    } else if ( 0 == nameclass.compare("node") || 0 == nameclass.compare("nodecard") ) {

        classtype = SYSTEMTREE_NODE;

    } else {

        classtype = SYSTEMTREE_OTHER; 

    }

    nameclass.append(" ");
    nameclass.append(namenode);
    
    map< Pair, uint64_t>::iterator iter_idmapping;
    
    if ( alldata->traceIdToSystemTreeId.empty() ) {

        alldata->systemTreeIdToSystemTreeNodeMap[nodeid] = SystemTreeNode(nameclass,classtype,0,0);
        alldata->systemTreeClassToNodeIdsMap[classtype].insert(nodeid);
        
        alldata->traceIdToSystemTreeId[ Pair(SYSTEMTREE_NODE, systemTreeIdentifier) ] = nodeid;


    } else {

        if ( (iter_idmapping = alldata->traceIdToSystemTreeId.find(Pair(SYSTEMTREE_NODE,parent))) != alldata->traceIdToSystemTreeId.end() ) {
        
            alldata->systemTreeIdToSystemTreeNodeMap[nodeid] = SystemTreeNode(nameclass,classtype,0,iter_idmapping->second);
            alldata->systemTreeClassToNodeIdsMap[classtype].insert(nodeid);
            
            alldata->systemTreeIdToSystemTreeNodeMap[iter_idmapping->second].children.insert(nodeid);
        
            alldata->traceIdToSystemTreeId[ Pair(SYSTEMTREE_NODE, systemTreeIdentifier) ] = nodeid;


        }else {
            /* unknown system tree parent */
        }
    }
    
    return OTF2_CALLBACK_SUCCESS;

}

OTF2_CallbackCode
OTF2Reader::handle_def_comm(void*          userData,
                            OTF2_CommRef   self,
                            OTF2_StringRef name,
                            OTF2_GroupRef  group,
                            OTF2_CommRef   parent)
{

    AllData* alldata = ( AllData* )userData;
    
    alldata->communicators[self] = group;
    
    return OTF2_CALLBACK_SUCCESS;

}

OTF2_CallbackCode
OTF2Reader::handle_def_string( void*          userData,
                               OTF2_StringRef stringIdentifier,
                               const char*    string )
{
    AllData* alldata = ( AllData* )userData;

    stringIdToString[stringIdentifier]= string;

    if(alldata->progname.compare("") == 0) {
    
        std::string this_string = string;
        int this_string_lenght = this_string.length();
        int this_string_start_pos = 0;
        std::string com_string = "";
        std::string sub_string = "";
        
        /* .cpp */
        this_string_start_pos = this_string_lenght - 4;
        com_string = ".cpp";
        
        if( this_string_start_pos >= 0 )
        {

            sub_string = this_string.substr(this_string_start_pos, 4);
            if(sub_string.compare(com_string) == 0)
                alldata->progname = string;

        }
        
        /* .c */
        this_string_start_pos = this_string_lenght - 2;
        com_string = ".c";
        
        if( this_string_start_pos >= 0 )
        {

            sub_string = this_string.substr(this_string_start_pos, 2);
            if(sub_string.compare(com_string) == 0)
                alldata->progname = string;

        }
            
        /* .f */
        this_string_start_pos = this_string_lenght - 2;
        com_string = ".f";
        
        if( this_string_start_pos >= 0 )
        {

            sub_string = this_string.substr(this_string_start_pos, 2);
            if(sub_string.compare(com_string) == 0)
                alldata->progname = string;

        }  
  
     }
     
    return OTF2_CALLBACK_SUCCESS;

}

/* ****************************************************************** */
/*                                                                    */
/*                               EVENTS                               */
/*                                                                    */
/* ****************************************************************** */

OTF2_CallbackCode
OTF2Reader::handle_metric(  OTF2_LocationRef        locationID, 
                            OTF2_TimeStamp          time,
                            void*                   userData,
                            OTF2_AttributeList*     attributeList,
                            OTF2_MetricRef          metric,
                            uint8_t                 numberOfMetrics,
                            const OTF2_Type*        typeIDs,
                            const OTF2_MetricValue* metricValues)
{
    AllData* alldata = ( AllData* ) userData;
 

    for( int i = 0; i < numberOfMetrics; i++) {

        if( typeIDs[i] == OTF2_TYPE_UINT64 && allowed_metrics.find( metric ) != allowed_metrics.end() ) {

            StackType::CounterData tmp_metric = StackType::CounterData();

            tmp_metric.lastTime = time; //abused for actual timestamp
            tmp_metric.firstValue = metricValues[i].unsigned_int;

            tmp_data.insert( make_pair( i, tmp_metric ) ); 


        }
    }

    return OTF2_CALLBACK_SUCCESS;

}

OTF2_CallbackCode
OTF2Reader::handle_enter( OTF2_LocationRef    locationID,
                          OTF2_TimeStamp      time,
                          void*               userData,
                          OTF2_AttributeList* attributeList,
                          OTF2_RegionRef      region )
{
    AllData* alldata = ( AllData* )userData;
    
    list<StackType>& stack = alldata->stackPerProcess[ locationID ];

	uint64_t res;


    if( stack.empty() ){

	    res = alldata->callPathTree.insertNode( CallPathTreeNode( region, 0 ), locationID );

    } else {

	    res = alldata->callPathTree.insertNode( CallPathTreeNode( region, stack.back().cpt_nid ), locationID );

    }

	stack.push_back( StackType( region, time, res ) );

    StackType& top= stack.back();

    map< uint64_t, StackType::CounterData>::iterator it = tmp_data.begin();

    for(; it != tmp_data.end(); it++) {

        if( it->second.lastTime == time ){

            top.counterIdDataMap[ it->first ].firstValue = it->second.firstValue;
        
        } else {

            map< uint64_t, StackType::CounterData >:: iterator it_top = top.counterIdDataMap.find( it->first );

            if( it_top != top.counterIdDataMap.end() ) {

                it_top->second.lastValue    = it->second.firstValue;
                it_top->second.lastTime     = it->second.lastTime;

            }

        }

    }

    tmp_data.clear();

    return OTF2_CALLBACK_SUCCESS;

}


OTF2_CallbackCode
OTF2Reader::handle_leave( OTF2_LocationRef    locationID,
                          OTF2_TimeStamp      time,
                          void*               userData,
                          OTF2_AttributeList* attributeList,
                          OTF2_RegionRef      region )
{

    AllData* alldata = ( AllData* )userData;
    
    list<StackType>& stack= alldata->stackPerProcess[ locationID ];
    assert( !stack.empty() );
    
    StackType& top= stack.back();
    list<StackType>::reverse_iterator parent_it= ++stack.rbegin();
    
    uint64_t func= top.fid;
    uint64_t incl= (time - top.timestamp);
    uint64_t excl= incl - top.childDuration;

    uint64_t cpt_nid = top.cpt_nid;    

    if ( parent_it != stack.rend() ) {
        
        parent_it->childDuration += incl;
        
    }

    //metric -- mostly copied from otfreader

    map< uint64_t, StackType::CounterData>::iterator it_tmp = tmp_data.begin();

    for(; it_tmp != tmp_data.end(); it_tmp++) {

        map< uint64_t, StackType::CounterData >:: iterator it_top = top.counterIdDataMap.find( it_tmp->first );

        if( it_top != top.counterIdDataMap.end() ) {

            it_top->second.lastValue    = it_tmp->second.firstValue;
            it_top->second.lastTime     = it_tmp->second.lastTime;

        }

    }

    tmp_data.clear();

    map< uint64_t, StackType::CounterData >::const_iterator it = top.counterIdDataMap.begin( );
    for ( ; it != top.counterIdDataMap.end( ); it++ ) {
        
        const uint64_t& counter= it->first;
        const uint64_t& firstvalue= it->second.firstValue;
        const uint64_t& lastvalue= it->second.lastValue;
        const uint64_t& lasttime= it->second.lastTime;
        
        if ( lasttime == time && firstvalue != (uint64_t)-1 &&
            lastvalue != (uint64_t)-1 ) {
            
            uint64_t counter_incl= lastvalue - firstvalue;
            uint64_t counter_excl= counter_incl - it->second.childDelta;
            
            alldata->counterMapPerFunctionRank[ Triple( locationID, func, counter ) ]
            .add( 1, counter_excl, counter_incl );
            
            if ( parent_it != stack.rend() ) {
                
                parent_it->counterIdDataMap[ counter ].childDelta+= counter_incl;
                
            }
            
        }
        
    }
    
    alldata->functionMapPerRank[ Pair( locationID, func ) ].add( 1, excl, incl );    
    stack.pop_back();

    map< pair< uint64_t, uint64_t >, FunctionData>::iterator it_func;
    it_func = alldata->functionDataPerCPTNode.find( pair< uint64_t, uint64_t >( cpt_nid, locationID ) ) ;

    if( it_func == alldata->functionDataPerCPTNode.end() ) {

        pair< uint64_t, uint64_t > insertMe = pair< uint64_t, uint64_t >(cpt_nid,  locationID );
        FunctionData insertMeToo = FunctionData(1, ( (double) excl / (double) alldata->timerResolution ) , ( (double) incl / (double) alldata->timerResolution ), cpt_nid );

        alldata->functionDataPerCPTNode.insert( pair< pair< uint64_t, uint64_t >, FunctionData >(insertMe, insertMeToo) );

    } else {

        it_func->second.add(1, ( (double) excl / (double) alldata->timerResolution ), ( (double) incl / (double) alldata->timerResolution ) );

    }

    return OTF2_CALLBACK_SUCCESS;

}


OTF2_CallbackCode
OTF2Reader::handle_mpi_send( OTF2_LocationRef    locationID,
                             OTF2_TimeStamp      time,
                             void*               userData,
                             OTF2_AttributeList* attributeList,
                             uint32_t            receiver,
                             OTF2_CommRef        communicator,
                             uint32_t            msgTag,
                             uint64_t            msgLength )
{

    AllData* alldata = ( AllData* )userData;
    
    double duration= 0.0;
    uint64_t sender= locationID;
    
    if( communicator != 0 ) {

        uint64_t com = alldata->communicators.find( communicator)->second;

        receiver = alldata->comRankToWorldRank.find( com )->second[ receiver ];

    }


    alldata->messageMapPerRankPair[ Pair(sender, receiver) ]
    .add_send( 1, msgLength, duration );
    alldata->messageMapPerRank[ sender ].add_send( 1, msgLength, duration );
    
    if (alldata->params.write_csv_msg_sizes)
    {

        alldata->messageMapPerSize[ msgLength ].count++;

        if ( duration > 0.0 )
        {

            alldata->messageMapPerSize[ msgLength ].time.append((uint64_t)duration);

        }

    }
    
    list<StackType>& stack = alldata->stackPerProcess[ sender ];

    if(alldata->P2PFuncIds.end() == alldata->P2PFuncIds.find(stack.back().fid))
    {

        alldata->P2PFuncIds.insert(stack.back().fid);

    }
    


    map< pair< uint64_t, uint64_t >, MessageData >::iterator it;  
    it = alldata->MessageDataPerNode.find( pair< uint64_t , uint64_t >( stack.back().cpt_nid, sender ) );
        

    if( it != alldata->MessageDataPerNode.end()) {

        it->second.add_send(1, msgLength, 0); //0 == time -> if needed enter value
        //actual time between send and recv would need active message matching

    } else {

        alldata->MessageDataPerNode[ pair<uint64_t, uint64_t >( stack.back().cpt_nid, sender ) ].add_send( 1, msgLength, 0 );

    }

    
    return OTF2_CALLBACK_SUCCESS;

}


OTF2_CallbackCode
OTF2Reader::handle_mpi_recv(    OTF2_LocationRef    locationID,
                                OTF2_TimeStamp      time,
                                void*               userData,
                                OTF2_AttributeList* attributeList,
                                uint32_t            sender,
                                OTF2_CommRef        communicator,
                                uint32_t            msgTag,
                                uint64_t            msgLength )
{

    AllData* alldata = ( AllData* )userData;
    
    /* duration will never be available at receive event */
    double duration= 0.0;

    uint64_t receiver= locationID;

    if( communicator != 0 ) {

        uint64_t com = alldata->communicators.find( communicator)->second;

        sender = alldata->comRankToWorldRank.find( com )->second[ sender ];

    }

    alldata->messageMapPerRankPair[ Pair(receiver, sender) ]
    .add_recv( 1, msgLength, duration );
    alldata->messageMapPerRank[ receiver ].add_recv( 1, msgLength, duration );

    
    list<StackType>& stack = alldata->stackPerProcess[ receiver ];

    if(alldata->P2PFuncIds.end() == alldata->P2PFuncIds.find(stack.back().fid))
    {

        alldata->P2PFuncIds.insert(stack.back().fid);

    }
    
    map< pair< uint64_t, uint64_t >, MessageData >::iterator it;  
    it = alldata->MessageDataPerNode.find( pair< uint64_t , uint64_t >( stack.back().cpt_nid, receiver ) );
        

    if( it != alldata->MessageDataPerNode.end()) {

        it->second.add_recv(1, msgLength, 0); //0 == time -> if needed enter value
        //actual time between send and recv would need active message matching

    } else {

        alldata->MessageDataPerNode[ pair<uint64_t, uint64_t >( stack.back().cpt_nid, receiver ) ].add_recv( 1, msgLength, 0 );

    }

    return OTF2_CALLBACK_SUCCESS;

}


OTF2_CallbackCode
OTF2Reader::handle_mpi_isend( OTF2_LocationRef    locationID,
                            OTF2_TimeStamp      time,
                            void*               userData,
                            OTF2_AttributeList* attributeList,
                            uint32_t            receiver,
                            OTF2_CommRef        communicator,
                            uint32_t            msgTag,
                            uint64_t            msgLength,
                            uint64_t            requestID )
{

    AllData* alldata = ( AllData* )userData;
    
    double duration= 0.0;
    uint64_t sender= locationID;
    
    if( communicator != 0 ) {

        uint64_t com = alldata->communicators.find( communicator)->second;

        receiver = alldata->comRankToWorldRank.find( com )->second[ receiver ];

    }

    alldata->messageMapPerRankPair[ Pair(sender, receiver) ]
    .add_send( 1, msgLength, duration );
    alldata->messageMapPerRank[ sender ].add_send( 1, msgLength, duration );
    
    if (alldata->params.write_csv_msg_sizes)
    {

        alldata->messageMapPerSize[ msgLength ].count++;

        if ( duration > 0.0 )
        {

            alldata->messageMapPerSize[ msgLength ].time.append((uint64_t)duration);

        }

    }
    
    list<StackType>& stack = alldata->stackPerProcess[ sender ];

    if(alldata->P2PFuncIds.end() == alldata->P2PFuncIds.find(stack.back().fid))
    {

        alldata->P2PFuncIds.insert(stack.back().fid);

    }

    
    map< pair< uint64_t, uint64_t >, MessageData >::iterator it;  
    it = alldata->MessageDataPerNode.find( pair< uint64_t , uint64_t >( stack.back().cpt_nid, sender ) );
        

    if( it != alldata->MessageDataPerNode.end()) {

        it->second.add_send(1, msgLength, 0); //0 == time -> if needed enter value 
        //actual time between send and recv would need active message matching

    } else {

            alldata->MessageDataPerNode[ pair<uint64_t, uint64_t >( stack.back().cpt_nid, sender ) ].add_send( 1, msgLength, 0 );

    }

    return OTF2_CALLBACK_SUCCESS;
    
}

OTF2_CallbackCode
OTF2Reader::handle_mpi_isend_complete( OTF2_LocationRef    locationID,
                                       OTF2_TimeStamp      time,
                                       void*               userData,
                                       OTF2_AttributeList* attributeList,
                                        uint64_t            requestID )
{

    return OTF2_CALLBACK_SUCCESS;

}

OTF2_CallbackCode
OTF2Reader::handle_mpi_irecv_request( OTF2_LocationRef    locationID,
                                      OTF2_TimeStamp      time,
                                      void*               userData,
                                      OTF2_AttributeList* attributeList,
                                      uint64_t            requestID )
{

    return OTF2_CALLBACK_SUCCESS;

}

OTF2_CallbackCode
OTF2Reader::handle_mpi_irecv( OTF2_LocationRef    locationID,
                              OTF2_TimeStamp      time,
                              void*               userData,
                              OTF2_AttributeList* attributeList,
                              uint32_t            sender,
                              OTF2_CommRef        communicator,
                              uint32_t            msgTag,
                              uint64_t            msgLength,
                              uint64_t            requestID )
{

    AllData* alldata = ( AllData* )userData;
    
    /* duration will never be available at receive event */
    double duration= 0.0;
    
    uint64_t receiver= locationID;

    if( communicator != 0 ) {

        uint64_t com = alldata->communicators.find( communicator)->second;

        sender = alldata->comRankToWorldRank.find( com )->second[ sender ];

    }
    
    alldata->messageMapPerRankPair[ Pair(receiver, sender) ]
    .add_recv( 1, msgLength, duration );
    alldata->messageMapPerRank[ receiver ].add_recv( 1, msgLength, duration );
    
    list<StackType>& stack = alldata->stackPerProcess[ receiver ];
    
    if(alldata->P2PFuncIds.end() == alldata->P2PFuncIds.find(stack.back().fid))
    {

        alldata->P2PFuncIds.insert(stack.back().fid);

    }
    

    map< pair< uint64_t, uint64_t >, MessageData >::iterator it;  
    it = alldata->MessageDataPerNode.find( pair< uint64_t , uint64_t >( stack.back().cpt_nid, receiver ) );

    if( it != alldata->MessageDataPerNode.end()) {

        it->second.add_recv(1, msgLength, 0); //0 == time -> if needed enter value 
        //actual time between send and recv would need active message matching

    } else {

        alldata->MessageDataPerNode[pair<uint64_t, uint64_t >(stack.back().cpt_nid, receiver)].add_recv( 1, msgLength, 0);

    }

    return OTF2_CALLBACK_SUCCESS;

}

OTF2_CallbackCode
OTF2Reader::handle_buffer_flush( OTF2_LocationRef    locationID,
                               OTF2_TimeStamp      time,
                               void*               userData,
                               OTF2_AttributeList* attributeList,
                               OTF2_TimeStamp      stopTime )
{

    return OTF2_CALLBACK_SUCCESS;

}

OTF2_CallbackCode
OTF2Reader::handle_mpi_request_test( OTF2_LocationRef    locationID,
                                     OTF2_TimeStamp      time,
                                     void*               userData,
                                     OTF2_AttributeList* attributeList,
                                     uint64_t            requestID )
{

    return OTF2_CALLBACK_SUCCESS;

}

OTF2_CallbackCode
OTF2Reader::handle_mpi_collective_begin( OTF2_LocationRef    locationID,
                                         OTF2_TimeStamp      time,
                                         void*               userData,
                                         OTF2_AttributeList* attributeList )
{

    AllData* alldata = ( AllData* )userData;

    PendingCollective pColl;
    pColl.begin_time = time;
    alldata->pendingCollectives[ Pair(0, locationID) ] = pColl;
    
    return OTF2_CALLBACK_SUCCESS;

}

OTF2_CallbackCode
OTF2Reader::handle_mpi_collective_end( OTF2_LocationRef         locationID,
                                       OTF2_TimeStamp           time,
                                       void*                    userData,
                                       OTF2_AttributeList*      attributeList,
                                       OTF2_CollectiveOp        type,
                                       OTF2_CommRef             communicator,
                                       uint32_t                 root,
                                       uint64_t                 sizeSent,
                                       uint64_t                 sizeReceived )
{
    AllData* alldata = ( AllData* )userData;

    /* get surrounding function id*/
    list<StackType>& stack = alldata->stackPerProcess[ locationID ];
    uint64_t fid = stack.back().fid;
    
    alldata->collectiveOperationsToClasses[fid] = type;

    /* add function to collective function ids*/
    if(alldata->CollFuncIds.end() == alldata->CollFuncIds.find(fid))
    {

        alldata->CollFuncIds.insert(fid);

    }
    
    if (type == OTF2_COLLECTIVE_OP_BARRIER)
        return OTF2_CALLBACK_SUCCESS;;
    
    map< Pair, PendingCollective >::const_iterator pCollIter =
            alldata->pendingCollectives.find( Pair(0, locationID) );
    assert( pCollIter != alldata->pendingCollectives.end() );
    
    CollectiveData collData;
    if (sizeSent > 0) {

        collData.add_send(1, sizeSent, time - pCollIter->second.begin_time);

    }

    if (sizeReceived > 0) {

        collData.add_recv(1, sizeReceived, time - pCollIter->second.begin_time);

    }
    
    map< Pair, CollectiveData, ltPair >::iterator IterRank = 
                    alldata->collectiveMapPerRank.find( Pair(locationID, 0) );

     /* data per rank */
    switch (type)
    {
        case OTF2_COLLECTIVE_OP_BCAST:
        case OTF2_COLLECTIVE_OP_GATHER:
        case OTF2_COLLECTIVE_OP_GATHERV:
        case OTF2_COLLECTIVE_OP_SCATTER:
        case OTF2_COLLECTIVE_OP_SCATTERV:
        case OTF2_COLLECTIVE_OP_REDUCE:
            {
                /* operations with root */ //is this working with communicators?
                if ( IterRank == alldata->collectiveMapPerRank.end() )
                {
                    alldata->collectiveMapPerRankPair[ Triple(locationID, root, 0) ] = collData;
                } else
                {
                    alldata->collectiveMapPerRankPair[ Triple(locationID, root, 0) ].add( collData );
                }
            }
            break;
            
        default:
            {
                /* operations without root */
                map< uint64_t, uint64_t >::const_iterator comm_iter = alldata->communicators.find(communicator);
                assert(comm_iter != alldata->communicators.end());
                
                map< uint64_t, set< uint64_t> >::const_iterator group_iter = alldata->groups.find(comm_iter->second);
                assert(group_iter != alldata->groups.end());

                for (set<uint64_t>::const_iterator iter = group_iter->second.begin();
                        iter != group_iter->second.end(); ++iter)
                {

                    uint64_t partner_id = *iter;
                    if ( IterRank == alldata->collectiveMapPerRank.end() )
                    {

                        alldata->collectiveMapPerRankPair[ Triple(locationID, partner_id, 0) ] = collData;

                    } else {

                        alldata->collectiveMapPerRankPair[ Triple(locationID, partner_id, 0) ].add( collData );

                    }

                }

            }

    }


    map< pair< uint64_t, uint64_t>, MessageData >::iterator it_msg = alldata->CollopDataPerNode.find( pair< uint64_t, uint64_t >( stack.back().cpt_nid, locationID ) );

    if(  it_msg == alldata->CollopDataPerNode.end() ) {
            
        MessageData insertMe;

        if( sizeSent > 0 ) {

            insertMe.add_send( 1, sizeSent, 0);

        }

        if( sizeReceived > 0 ) {

            insertMe.add_recv( 1, sizeReceived, 0);

        }

        alldata->CollopDataPerNode.insert(pair< pair< uint64_t, uint64_t>, MessageData >( pair< uint64_t, uint64_t >( stack.back().cpt_nid, locationID ), insertMe ) );

    } else {

        if( sizeSent > 0 ) {

            it_msg->second.add_send( 1, sizeSent, 0);

        }

        if( sizeReceived > 0 ) {

            it_msg->second.add_recv( 1, sizeReceived, 0);

        }

    }

    return OTF2_CALLBACK_SUCCESS;

}


OTF2_CallbackCode
OTF2Reader::handle_rma_put( OTF2_LocationRef         locationID,
                            OTF2_TimeStamp           time,
                            void*                    userData,
                            OTF2_AttributeList*      attributeList,
                            OTF2_RmaWinRef           win,
                            uint32_t                 remote,
                            uint64_t                 bytes,
                            uint64_t                 matchingId )
{

    AllData* alldata = ( AllData* )userData;

    /* get surrounding function id if possible */
    list<StackType>& stack = alldata->stackPerProcess[ locationID ];

    uint64_t cid;

    if( stack.empty() ) {

        cid = -1;

    } else {

        cid = stack.front().cpt_nid;

    }

    map< pair< uint64_t, uint64_t>, RmaData >::iterator it_rma = alldata->RmaMsgData.find( pair< uint64_t, uint64_t >( cid, locationID ) );

    if(  it_rma == alldata->RmaMsgData.end() ) {

        RmaData insertMe;
        insertMe.rma_put_cnt = 1;
        insertMe.rma_get_cnt = 0;

        insertMe.rma_get_bytes = 0;
        insertMe.rma_put_bytes = bytes;

        alldata->RmaMsgData.insert( pair< pair< uint64_t, uint64_t>, RmaData >( pair< uint64_t, uint64_t >( cid, locationID), insertMe) );

    } else {

        it_rma->second.rma_put_cnt += 1;
        it_rma->second.rma_put_bytes += bytes;

    }


    return OTF2_CALLBACK_SUCCESS;

}

OTF2_CallbackCode
OTF2Reader::handle_rma_get( OTF2_LocationRef         locationID,
                            OTF2_TimeStamp           time,
                            void*                    userData,
                            OTF2_AttributeList*      attributeList,
                            OTF2_RmaWinRef           win,
                            uint32_t                 remote,
                            uint64_t                 bytes,
                            uint64_t                 matchingId )
{

    AllData* alldata = ( AllData* )userData;

    /* get surrounding function id if possible */
    list<StackType>& stack = alldata->stackPerProcess[ locationID ];

    uint64_t cid;

    if( stack.empty() ) {

        cid = -1;

    } else {

        cid = stack.front().cpt_nid;

    }

    map< pair< uint64_t, uint64_t>, RmaData >::iterator it_rma = alldata->RmaMsgData.find( pair< uint64_t, uint64_t >( cid, locationID ) );

    if(  it_rma == alldata->RmaMsgData.end() ) {

        RmaData insertMe;
        insertMe.rma_get_cnt = 1;
        insertMe.rma_put_cnt = 0;

        insertMe.rma_get_bytes = bytes;
        insertMe.rma_put_bytes = 0;

        alldata->RmaMsgData.insert( pair< pair< uint64_t, uint64_t>, RmaData >( pair< uint64_t, uint64_t >( cid, locationID), insertMe) );

    } else {

        it_rma->second.rma_get_cnt += 1;
        it_rma->second.rma_get_bytes += bytes;

    }

    return OTF2_CALLBACK_SUCCESS;

}


OTF2_CallbackCode
OTF2Reader::handle_unknown( OTF2_LocationRef    locationID,
                            OTF2_TimeStamp      time,
                            void*               userData,
                            OTF2_AttributeList* attributeList )
{

    return OTF2_CALLBACK_SUCCESS;

}


bool
OTF2Reader::readDefinitions( AllData& alldata )
{

    bool error= false;
    OTF2_ErrorCode status;

    OTF2_GlobalDefReader* glob_def_reader = OTF2_Reader_GetGlobalDefReader( reader );

    if ( NULL == glob_def_reader )
    {

        return error;

    }
    
    OTF2_GlobalDefReaderCallbacks* glob_def_callbacks = OTF2_GlobalDefReaderCallbacks_New();

    if ( NULL == glob_def_callbacks )
    {

        return error;

    }
    
    status = OTF2_GlobalDefReaderCallbacks_SetClockPropertiesCallback( glob_def_callbacks, handle_def_clock_properties);

    if ( OTF2_SUCCESS != status )
    {

        return error;

    }
    
    status = OTF2_GlobalDefReaderCallbacks_SetLocationGroupCallback( glob_def_callbacks, handle_def_location_group );

    if ( OTF2_SUCCESS != status )
    {

        return error;

    }
    
    status = OTF2_GlobalDefReaderCallbacks_SetLocationCallback( glob_def_callbacks, handle_def_location );

    if ( OTF2_SUCCESS != status )
    {

        return error;

    }

    status = OTF2_GlobalDefReaderCallbacks_SetGroupCallback( glob_def_callbacks, handle_def_group );

    if ( OTF2_SUCCESS != status )
    {

        return error;

    }
    
    status = OTF2_GlobalDefReaderCallbacks_SetRegionCallback( glob_def_callbacks, handle_def_region );

    if ( OTF2_SUCCESS != status )
    {

        return error;

    }

    status = OTF2_GlobalDefReaderCallbacks_SetMetricMemberCallback( glob_def_callbacks, handle_def_metrics );

    if ( OTF2_SUCCESS != status )
    {

        return error;

    }

    status = OTF2_GlobalDefReaderCallbacks_SetMetricClassCallback( glob_def_callbacks, handle_def_metric_class );

    if ( OTF2_SUCCESS != status )
    {

        return error;

    }

    status = OTF2_GlobalDefReaderCallbacks_SetSystemTreeNodeCallback( glob_def_callbacks, handle_def_system_tree_node );

    if ( OTF2_SUCCESS != status )
    {

        return error;

    }

    status = OTF2_GlobalDefReaderCallbacks_SetCommCallback( glob_def_callbacks, handle_def_comm );

    if ( OTF2_SUCCESS != status )
    {

        return error;

    }
    
    status = OTF2_GlobalDefReaderCallbacks_SetStringCallback( glob_def_callbacks, handle_def_string );

    if ( OTF2_SUCCESS != status )
    {

        return error;

    }
    
    status = OTF2_Reader_RegisterGlobalDefCallbacks( reader, glob_def_reader, glob_def_callbacks, &alldata );
    if ( OTF2_SUCCESS != status )
    {

        return error;

    }

    uint64_t definitions_read = 0;
    status = OTF2_Reader_ReadAllGlobalDefinitions( reader, glob_def_reader, &definitions_read );

    if ( OTF2_SUCCESS != status )
    {

        std::cerr << "ERROR: Could not read definitions from OTF2 trace." << std::endl;
        error= true;

    }
    
    OTF2_GlobalDefReaderCallbacks_Delete( glob_def_callbacks );
    
    /* read local definitions if available */
    
    bool successful_open_def_files = OTF2_Reader_OpenDefFiles( reader ) == OTF2_SUCCESS;
    OTF2_Reader_OpenEvtFiles( reader );

    for ( uint64_t i = 0; i < alldata.myProcessesNum; i++ ) {

        /* mandatory but unused */
        OTF2_EvtReader* evt_reader = OTF2_Reader_GetEvtReader( reader, alldata.myProcessesList[i] );

        if ( NULL == evt_reader )
        {

            error= true;
            return !error;

        }
    
        OTF2_DefReader* def_reader;
        def_reader = OTF2_Reader_GetDefReader( reader, alldata.myProcessesList[i] );

        if ( NULL == def_reader )
        {

            return OTF2_CALLBACK_INTERRUPT;

        }
    
        uint64_t       definitions_read;
        OTF2_ErrorCode status;
        status = OTF2_Reader_ReadAllLocalDefinitions( reader, def_reader, &definitions_read );

        if ( OTF2_SUCCESS != status )
        {

            return OTF2_CALLBACK_INTERRUPT;

        }
    
        /* Close def reader, it is no longer useful and occupies memory */
        status = OTF2_Reader_CloseDefReader( reader, def_reader );

        if ( OTF2_SUCCESS != status )
        {

            error= true;
            return !error;

        }

    }
    
    if ( successful_open_def_files ) {

        OTF2_Reader_CloseDefFiles( reader ); 

    }
    
    return !error;

}

bool
OTF2Reader::readEvents( AllData& alldata )
{

    bool error= false;

    uint64_t otf2_STEP = OTF2_UNDEFINED_UINT64;
    uint64_t events_read;

    OTF2_ErrorCode status;
    OTF2_GlobalEvtReader* glob_evt_reader;
    
    OTF2_GlobalEvtReaderCallbacks* evt_callbacks = OTF2_GlobalEvtReaderCallbacks_New();

    if ( NULL == evt_callbacks )
    {

        return error;

    }

    
    OTF2_GlobalEvtReaderCallbacks_SetEnterCallback( evt_callbacks, handle_enter );
    OTF2_GlobalEvtReaderCallbacks_SetLeaveCallback( evt_callbacks, handle_leave );
    OTF2_GlobalEvtReaderCallbacks_SetMpiSendCallback( evt_callbacks, handle_mpi_send );
    OTF2_GlobalEvtReaderCallbacks_SetMpiIsendCallback( evt_callbacks, handle_mpi_isend );
    OTF2_GlobalEvtReaderCallbacks_SetMpiIsendCompleteCallback( evt_callbacks, handle_mpi_isend_complete );
    OTF2_GlobalEvtReaderCallbacks_SetMpiIrecvRequestCallback( evt_callbacks, handle_mpi_irecv_request );
    OTF2_GlobalEvtReaderCallbacks_SetMpiRecvCallback( evt_callbacks, handle_mpi_recv);
    OTF2_GlobalEvtReaderCallbacks_SetMpiIrecvCallback( evt_callbacks, handle_mpi_irecv );
    OTF2_GlobalEvtReaderCallbacks_SetMpiRequestTestCallback( evt_callbacks, handle_mpi_request_test );
    OTF2_GlobalEvtReaderCallbacks_SetMpiCollectiveBeginCallback( evt_callbacks, handle_mpi_collective_begin );
    OTF2_GlobalEvtReaderCallbacks_SetMpiCollectiveEndCallback( evt_callbacks, handle_mpi_collective_end );

    OTF2_GlobalEvtReaderCallbacks_SetRmaPutCallback( evt_callbacks, handle_rma_put );
    OTF2_GlobalEvtReaderCallbacks_SetRmaGetCallback( evt_callbacks, handle_rma_get );

    OTF2_GlobalEvtReaderCallbacks_SetMetricCallback( evt_callbacks, handle_metric );
    

    if( alldata.myProcessesNum > 0 ) {

        glob_evt_reader = OTF2_Reader_GetGlobalEvtReader( reader );

        if ( NULL == glob_evt_reader ) {

            return error;

        }

    } else {

        OTF2_GlobalEvtReaderCallbacks_Delete( evt_callbacks );
        return !error;

    }
            
    status = OTF2_Reader_RegisterGlobalEvtCallbacks( reader, glob_evt_reader, evt_callbacks, &alldata );
    
    status = OTF2_Reader_ReadGlobalEvents( reader, glob_evt_reader, otf2_STEP, &events_read );

    if ( OTF2_SUCCESS != status )
    {

        std::cerr << "Error while reading events from OTF2 trace." << std::endl;

    }
    
    /* Clean up */
    OTF2_GlobalEvtReaderCallbacks_Delete( evt_callbacks );

    return !error;

}

bool
OTF2Reader::readStatistics( AllData& alldata )
{

    bool error= false;
    
    return !error;

}
