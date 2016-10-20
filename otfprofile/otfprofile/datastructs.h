/*
 This is part of the OTF library. Copyright by ZIH, TU Dresden 2005-2013.
 Authors: Andreas Knuepfer, Robert Dietrich, Matthias Jurenz
*/

#ifndef DATASTRUCTS_H
#define DATASTRUCTS_H




#include <stdlib.h>
#include <cassert>
#include <sys/time.h>

#include <string>
#include <map>
#include <list>
#include <set>
#include <vector>

#ifdef OTFPROFILE_MPI
#   include "mpi.h"
#endif /* OTFPROFILE_MPI */

#ifdef HAVE_OPEN_TRACE_FORMAT

#include <OTF_inttypes.h>

#endif

#ifndef OTF_UINT64_MAX
    #ifndef UINT64_MAX
        #define OTF_UINT64_MAX (uint64_t)-1
    #else
        #define OTF_UINT64_MAX UINT64_MAX
    #endif

    #define OTF_INTTYPES_H //ugly hacks to avoid redefinition warning

#endif

    /* Types of collective operations */
#ifndef OTF_COLLECTIVE_TYPE_UNKNOWN
    #define OTF_COLLECTIVE_TYPE_UNKNOWN 	0
#endif

#ifndef OTF_COLLECTIVE_TYPE_BARRIER
    #define OTF_COLLECTIVE_TYPE_BARRIER 	1
#endif

#ifndef OTF_COLLECTIVE_TYPE_ONE2ALL
    #define OTF_COLLECTIVE_TYPE_ONE2ALL 	2
#endif

#ifndef OTF_COLLECTIVE_TYPE_ALL2ONE
    #define OTF_COLLECTIVE_TYPE_ALL2ONE 	3
#endif

#ifndef OTF_COLLECTIVE_TYPE_ALL2ALL
    #define OTF_COLLECTIVE_TYPE_ALL2ALL 	4
#endif

    //block - END!

#include "callpathtree.h"


using namespace std;


/* *** process clustering algorithms *** */

typedef enum {

    CLUSTER_ALG_CLINKAGE,
    CLUSTER_ALG_KMEANS

} ClusterAlgorithm;

/* *** dispersion modes *** */

typedef enum {

    DISPERSION_MODE_PERFUNCTION = 0,
    DISPERSION_MODE_PERCALLPATH = 1

} DispersionMode;

/* *** dispersion options *** */

typedef enum {

    DISPERSION_OPT_INFO   = 0x1,
    DISPERSION_OPT_MARKER = 0x2,
    DISPERSION_OPT_FILTER = 0x4

} DispersionOptions;

typedef enum {

    SYSTEMTREE_OTHER,
    SYSTEMTREE_LOCATION,
    SYSTEMTREE_LOCATION_GROUP,
    SYSTEMTREE_NODE,
    SYSTEMTREE_BLADE,
    SYSTEMTREE_CAGE,
    SYSTEMTREE_CABINET,
    SYSTEMTREE_CABINET_ROW,
    SYSTEMTREE_MACHINE,
    SYSTEMTREE_UNKNOWN

} SystemTreeClass;

/* *** program parameters *** */

struct Params {

    /* general parameters */

    static const uint32_t DEFAULT_MAX_FILE_HANDLES= 50;
    static const uint32_t DEFAULT_BUFFER_SIZE= 1024 * 1024;
    static const uint32_t DEFAULT_MAX_GROUPS= 16;
    static const bool     DEFAULT_LOG_AXIS= true;
    static const uint8_t  DEFAULT_VERBOSE_LEVEL= 0;
    static const bool     DEFAULT_CREATE_CSV= false;
    static const bool     DEFAULT_CREATE_CUBE= false;
    static const bool     DEFAULT_NO_REDUCE = false;
    static const bool     DEFAULT_CREATE_CIRCOS= false;
    static const bool     DEFAULT_CIRCOS_AGGREGATE= false;
    static const bool     DEFAULT_CREATE_HTML= false;
    static const bool     DEFAULT_CREATE_TEX= false;
    static const bool     DEFAULT_CREATE_PDF= false;
    static const string   DEFAULT_OUTPUT_FILE_PREFIX() { return "result"; }
    static const bool     DEFAULT_QUARTILE_CIRCOS = false;
    static const bool     DEFAULT_DISPERSION = false;


    uint32_t max_file_handles;
    uint32_t buffer_size;
    uint32_t max_groups;
    bool     logaxis;
    uint8_t  verbose_level;
    bool     progress;
    bool     read_from_stats;

    bool     disp;

    bool     create_csv;
    bool     create_tex;
    bool     create_pdf;
    bool     create_cube;
    bool     no_reduce;
    bool     create_html;
    bool     create_circos;

    bool     circos_aggregate;          
    bool     circos_quartile;           
    uint32_t circos_quartile_percent;   
    string   circos_layout;

    bool     write_csv_msg_matrices;
    bool     write_csv_msg_sizes;

    string   input_file_name;
    string   input_file_prefix;
    string   output_file_prefix;

    bool     is_otf2;



    /* process clustering related parameters */

    struct Clustering {

        static const ClusterAlgorithm DEFAULT_ALGORITHM= CLUSTER_ALG_KMEANS;
        static double DEFAULT_QUALITY_THRESHOLD() { return 0.1; }

        ClusterAlgorithm alg;
        bool             enabled;
        bool             shrink;
        bool             hard_grouping;
        double           quality_threshold;

        string           shrink_output_prefix;

        bool             synth_data;
        uint32_t         synth_ranks_num;
        uint32_t         synth_funcs_num;

        Clustering()
           : alg(DEFAULT_ALGORITHM), enabled(false), shrink(false),
             hard_grouping(false),
             quality_threshold(DEFAULT_QUALITY_THRESHOLD()),
             synth_data(false), synth_ranks_num(0), synth_funcs_num(0) {}

    } clustering;

    /* irregularity analysis related parameters */

    struct Dispersion {

        static const DispersionMode    DEFAULT_MODE= DISPERSION_MODE_PERFUNCTION;
        static const DispersionOptions DEFAULT_OPTIONS= DISPERSION_OPT_INFO;
        static const uint32_t          DEFAULT_REDUCTION= 15;

        DispersionMode     mode;
        bool               enabled;
        uint32_t           options;
        uint32_t           reduction;
        std::string        filter_file_name;

        Dispersion()
           : mode(DEFAULT_MODE), enabled(false), options(DEFAULT_OPTIONS),
             reduction(DEFAULT_REDUCTION) {}

    } dispersion;

    Params()
        : max_file_handles(DEFAULT_MAX_FILE_HANDLES),
          buffer_size(DEFAULT_BUFFER_SIZE), max_groups(DEFAULT_MAX_GROUPS),
          logaxis(DEFAULT_LOG_AXIS),
          verbose_level(DEFAULT_VERBOSE_LEVEL), progress(false),
          read_from_stats(false), create_csv(DEFAULT_CREATE_CSV),
          create_tex(DEFAULT_CREATE_TEX), create_pdf(DEFAULT_CREATE_PDF),
          create_cube(DEFAULT_CREATE_CUBE), no_reduce(DEFAULT_NO_REDUCE),
          create_circos(DEFAULT_CREATE_CIRCOS),
          circos_quartile(DEFAULT_QUARTILE_CIRCOS),
          create_html(DEFAULT_CREATE_HTML),
          circos_aggregate(DEFAULT_CIRCOS_AGGREGATE),
          write_csv_msg_matrices(false), write_csv_msg_sizes(false),
          disp(DEFAULT_DISPERSION),
          output_file_prefix(DEFAULT_OUTPUT_FILE_PREFIX()) {}

};


/* *** progress information *** */

struct Progress {

    /* maximum number of records to read between progress updates */
    static const uint64_t EVENTS_RECORD_LIMIT= 1000000;
    static const uint64_t STATS_RECORD_LIMIT= 100;

    uint64_t     cur_bytes;      /* current bytes read */
    uint64_t     max_bytes;      /* max. bytes readable */

#ifdef OTFPROFILE_MPI
    /* message tag to use for communication */
    static const int MSG_TAG= 500;

    MPI_Request  send_request;   /* sender request handle */

    uint64_t*    recv_buffers;   /* receive buffers */
    MPI_Request* recv_requests;  /* persistent receive request handles */
    MPI_Status*  recv_statuses;  /* receive statuses */
    int*         recv_indices;   /* indices of completed recv. operations */

    uint64_t*    rank_cur_bytes; /* current bytes read per rank (except rank 0) */
    uint32_t     ranks_left;     /* root keeps track of ranks left to query */
#endif /* OTFPROFILE_MPI */

    Progress()
        : cur_bytes(0), max_bytes(0) {
#ifdef OTFPROFILE_MPI
        send_request   = MPI_REQUEST_NULL;
        recv_buffers   = NULL;
        recv_requests  = NULL;
        recv_statuses  = NULL;
        recv_indices   = NULL;
        rank_cur_bytes = NULL;
        ranks_left     = 0;
#endif /* OTFPROFILE_MPI */
    }
};


/* *** runtime measurement *** */

struct Measurement {

    struct Scope {

        double        start_time;    /* start timestamp of measurement scope */
        double        stop_time;     /* stop timestamp of measurement scope */
        const uint8_t verbose_level; /* verbose level required to
                                     perform measurement */

        Scope( const uint8_t _verbose_level )
            : start_time(-1.0), stop_time(-1.0),
              verbose_level(_verbose_level) {}
    };

    /* are there any completed runtime measurement result? */
    bool                 have_data;

    /* store per-measurement scope runtimes */
    map< string, Scope > scope_map;

    Measurement() : have_data(false) {}

    /* get global timestamp in seconds */
    static double gettime() {

#ifdef OTFPROFILE_MPI
        return MPI_Wtime();
#else /* OTFPROFILE_MPI */
        struct timeval tv;
        gettimeofday( &tv, NULL );
        return (double)( tv.tv_sec * 1e6 + tv.tv_usec ) / 1.0e6;
#endif /* OTFPROFILE_MPI */
    }
};


/* *** pair of values as map key *** */

struct Pair {

    uint64_t a;
    uint64_t b;

    Pair() : a(0), b(0) {}
    Pair( uint64_t aa, uint64_t bb ) : a(aa), b(bb) {}

    ~Pair() {}

};


struct ltPair {

    bool operator()( const Pair& p1, const Pair& p2 ) const {

        /* a is the major number for comparison, this gives a better
        order when reducing the entries over the first argument */

        if ( p1.a == p2.a ) {

            return p1.b < p2.b;

        } else {

            return p1.a < p2.a;

        }

    }

};

struct gtPair {

    bool operator()( const Pair& p1, const Pair& p2 ) const {

        /* a is the major number for comparison, this gives a better
         order when reducing the entries over the first argument */

        if ( p1.a == p2.a ) {

            return p1.b < p2.b;

        } else {

            return p1.a > p2.a;

        }

    }

};

/* *** pair of values as map key *** */

struct PairCallpath {

    uint64_t a;
    string b;

    PairCallpath() : a(0), b("") {}
    PairCallpath( uint64_t aa, string bb ) : a(aa), b(bb) {}

    ~PairCallpath() {}

};

struct ltPairCallpath {

    bool operator()( const PairCallpath& p1, const PairCallpath& p2 ) const {

        /* a is the major number for comparison, this gives a better
        order when reducing the entries over the first argument */
        if ( p1.a == p2.a ) {

        	if(p1.b.compare(p2.b) < 0) {

        		return true;

        	} else {

        		return false;

            }

        } else {


            return p1.a < p2.a;

        }

    }

};

/* *** triplett of values as map key *** */
struct Triple {

    uint64_t a;
    uint64_t b;
    uint64_t c;

    Triple() : a(0), b(0), c(0) {}
    Triple( uint64_t aa, uint64_t bb, uint64_t cc ) : a(aa), b(bb), c(cc) {}

    ~Triple() {}

};


struct ltTriple {

    bool operator()( const Triple& p1, const Triple& p2 ) const {

        /* a is the major number for comparison, this gives a better
        order when reducing the entries over the first argument */

        if ( p1.a == p2.a ) {

            if ( p1.b == p2.b ) {

                return p1.c < p2.c;


            } else {

                return p1.b < p2.b;

            }

        } else {

            return p1.a < p2.a;

        }

    }

};

struct TripleCallpath {

    uint64_t a;
    string b;
    uint64_t c;

    TripleCallpath() : a(0), b(""), c(0) {}
    TripleCallpath( uint64_t aa, string bb, uint64_t cc ) : a(aa), b(bb), c(cc) {}

    ~TripleCallpath() {}

};

struct gtTripleCallpathSortByCallpath {

    bool operator()( const TripleCallpath& p1, const TripleCallpath& p2 ) const {

        /* a is the major number for comparison, this gives a better
        order when reducing the entries over the first argument */

        if ( p1.c == p2.c ) {

            if ( p1.a == p2.a ) {

                if(p1.b.compare(p2.b) < 0) {

            	    return true;

                }else{

                    return false;

                }

            } else {

                return p1.a > p2.a;

            }

        } else {

			return p1.c > p2.c;

        }

    }

};

struct gtTripleCallpathSortByDispersion {

    bool operator()( const TripleCallpath& p1, const TripleCallpath& p2 ) const {

        /* a is the major number for comparison, this gives a better
        order when reducing the entries over the first argument */

        if ( p1.a == p2.a ) {

            if ( p1.b.compare(p2.b) == 0 ) {

                return p1.c > p2.c;

            } else {

            	if(p1.b.compare(p2.b) < 0) {

            		return true;

            	} else {

            		return false;

                }

            }

        } else {

            return p1.a > p2.a;

        }

    }

};

struct ltTripleCallpath {

    bool operator()( const TripleCallpath& p1, const TripleCallpath& p2 ) const {

        /* a is the major number for comparison, this gives a better
        order when reducing the entries over the first argument */

        if ( p1.a == p2.a ) {

            if ( p1.b.compare(p2.b) == 0 ) {

                return p1.c < p2.c;

            } else {

            	if(p1.b.compare(p2.b) > 0) {

            		return true;

            	} else {

            		return false;

                }

            }

        } else {

            return p1.a < p2.a;

        }

    }

};

/* *** quartet of values as map key *** */
struct Quadruple {

    uint64_t a;
    uint64_t b;
    string c;
    uint64_t d;

    Quadruple() : a(0), b(0), c(""), d(0) {}

    Quadruple( uint64_t aa, uint64_t bb, string cc, uint64_t dd ) : a(aa),
              b(bb), c(cc), d(dd) {}

    ~Quadruple() {}

};

struct ltQuadruple {

    bool operator()( const Quadruple& p1, const Quadruple& p2 ) const {

        /* a is the major number for comparison, this gives a better
        order when reducing the entries over the first argument */

        if ( p1.a == p2.a ) {

            if ( p1.b == p2.b ) {

            	if ( p1.d == p2.d ) {

                	if(p1.c.compare(p2.c) > 0) {

                		return true;

                	} else {

                		return false;
                    }

                } else {

                return p1.d < p2.d;

                }

            } else {

                return p1.b < p2.b;

            }

        } else {

            return p1.a < p2.a;

        }

    }

};

struct Process {

    uint64_t process;
    uint64_t parent;

    Process() : process(0), parent(0) {}
    Process( uint64_t _process, uint64_t _parent )
        : process(_process), parent(_parent) {}

    ~Process() {}

};

struct ltProcess {

    bool operator()( const Process& p1, const Process& p2 ) const {

        /* involve parent for sorting? */
        return p1.process < p2.process;

    }

};


/* class that collects the minimum, the maximum, and the sum for some values.
the minimum will ignore the value '0' though because it should only record
actual occurences */
template <class type>
class min_max_avg {

public:

    type min;
    type max;
    type sum;
    uint64_t cnt;

    min_max_avg( type a= (type) OTF_UINT64_MAX,
                 type b= (type) 0, type s= (type) 0, uint64_t c= 0 ) :
            min( a ), max( b ), sum( s ), cnt( c ) {}

    ~min_max_avg() {}

    /* append a single value */
    void append( const type& value ) {

        if ( ((type) 0) != value ) {

            min= ( value < min ) ? value : min;
            max= ( value > max ) ? value : max;
            sum += value;
            cnt += 1;
        }

    }

    /* add another min_max_avg object as if all their values were appended to
       on object */
    void add( const min_max_avg<type>& other ) {

        min= ( other.min < min ) ? other.min : min;
        max= ( other.max > max ) ? other.max : max;
        sum += other.sum;
        cnt += other.cnt;

    }

};

static const uint64_t msgSizesBytes[] = {64, 512, 1024, 4096, 64 * 1024, 1024 * 1024,
        4 * 1024 * 1024, 64 * 1024 * 1024, 512 * 1024 * 1024, 1024 * 1024 * 1024};

class msgSizeHistogram {

public:

    static const size_t numBins = 10;

    vector<uint64_t> hist;

    msgSizeHistogram(): hist(numBins)
    {}

    ~msgSizeHistogram() {}

    msgSizeHistogram( const msgSizeHistogram& other ) {

      for (size_t i = 0; i < numBins; i++) {

          hist[i] = other.hist[i];

      }

    }

    void operator=( const msgSizeHistogram& other ) {


      for (size_t i = 0; i < numBins; i++) {

          hist[i] = other.hist[i];

      }

    }

    /* insert a single value */
    void addValue( const uint64_t& bytes ) {

        for (size_t i = 0; i < numBins; i++) {

            if (bytes <= msgSizesBytes[i] || i == numBins - 1) {

                hist[i]++;
                break;

            }

        }

    }

    /* add another histogram object as if all their values were appended to
       on object */
    void addHistogram( const msgSizeHistogram& other ) {

       for (size_t i = 0; i < numBins; i++) {

           hist[i] += other.hist[i];

        }

    }
};

/* class that collects the location of the minimum and maximum and their values
for some values */
template <class type>
class min_max_Location {

public:

    type min;
    type max;
    uint64_t loc_min;
    uint64_t loc_max;
    uint64_t time_min;
    uint64_t time_max;

    min_max_Location( type a= (type) OTF_UINT64_MAX, type b= (type) 0,
                     uint64_t p= 0, uint64_t q= 0,
                     uint64_t s= 0, uint64_t t= 0 ) :
    min( a ), max( b ), loc_min( p ), loc_max( q ), time_min( s ),
    time_max( t ) {}

    ~min_max_Location() {}

    /* append a single value with its location*/
    void append( const type& value, uint64_t location, uint64_t time ) {

        if ( ((type) 0) != value ) {

            if ( value < min ) {

                min= value;
                loc_min= location;
                time_min= time;

            }

            if ( value > max ) {

                max= value;
                loc_max= location;
                time_max= time;

            }

        }

    }

    /* add another min_max_Location object as if all their values were appended
       to on object */
    void add( const min_max_Location<type>& other ) {

        if ( other.min < min ) {

            min= other.min;
            loc_min= other.loc_min;
            time_min= other.time_min;

        }

        if ( other.max > max ) {

            max= other.max;
            loc_max= other.loc_max;
            time_max= other.time_max;

        }

    }

};


/* manage grouping of processes (or ranks/threads/whatever) -- grouping
reduces the potentially unlimited numbers of processes to a fixed number of
groups (or bins, buckets, ... ). Instead of a per-process basis global
statisitics are collected per group. The processes can be spread over the groups
either consecutively, or round-robin, or randomly, or in any special scheme,
e.g. separate groups for GPU theads and host processes. Therefore, the Grouping
structure manages explicit mappings from group IDs to process IDs.
Every process belongs to one group exclusively. */

struct Grouping {

    /* maximum number of groups
    (limited by LaTeX output; defined in create_latex.cpp) */
    static const uint32_t MAX_GROUPS;

    /* store process/group mappings */
    map< uint64_t, uint64_t > processesToGroups;
    map< uint64_t, set<uint64_t> > groupsToProcesses;

    /* indicates whether grouping is enabled
    (more processes than maximum number of groups) */
    bool enabled;

    Grouping() : enabled( false ) {}
    ~Grouping() {}

    /* insert process into a group, return true if succeeded */
    bool insert( uint64_t group, uint64_t process ) {

        /* insert the new entry if and only if there was no process with this ID
        before, because every process can only be in one group */

        pair< map< uint64_t, uint64_t >::const_iterator, bool> ret=
            processesToGroups.insert( pair< uint64_t, uint64_t >( process, group ) );

        if ( ret.second ) {

            groupsToProcesses[ group ].insert( process );

            /* set indicator that grouping is enabled, if there are more than
            one process within a group */
            if ( !enabled && 1 < groupsToProcesses[ group ].size() ) {

                enabled= true;

            }

            return true;

        }

        return false;

    }

    /* return the group ID for the given process ID, return 0 if not found */
    uint64_t process2group( uint64_t process ) const {

        map< uint64_t, uint64_t >::const_iterator it= processesToGroups.find( process );

        return ( processesToGroups.end() != it ) ? it->second : ( uint64_t) 0 ;

    }

    /* return a const pointer to the set or NULL if there is no such group,
    this is better than the [] operator which would create an empty set if
    a search goes negative */
    const set<uint64_t>* group2processes( uint64_t group ) const {

        map< uint64_t, set<uint64_t> >::const_iterator it= groupsToProcesses.find( group );

        return ( groupsToProcesses.end() != it ) ? ( & it->second ) : NULL ;

    }

    /* return number of groups */
    uint32_t numGroups( ) const {

        return groupsToProcesses.size();

    }
};


struct StackType {

	/* CallPathTree NodeId */
	uint64_t cpt_nid;

    /* the function ID */
    uint64_t fid;

    /* the enter timestamp when this was pushed to the top-of-stack */
    uint64_t timestamp;

    /* the duration consumed by child calls, it is the sum of all child call's
    inclusive durations, with this one can compute the exclusive durations of
    the currrent call based on the inclusive time which comes from the end
    timestamps minus the start timestamp. */
    uint64_t childDuration;

    struct CounterData {

        /* the first counter value relevant to the enter timestamp */
        uint64_t firstValue;

        /* the last counter value */
        uint64_t lastValue;

        /* the timestamp on which the last counter value occurred */
        uint64_t lastTime;

        /* similar to childDuration but for the counter values */
        uint64_t childDelta;

        CounterData()
            : firstValue( (uint64_t)-1 ), lastValue( (uint64_t)-1 ),
              lastTime( (uint64_t)-1 ), childDelta( 0 ) {}

    };

    /* the counter data on this stack level */
    map< uint64_t, CounterData > counterIdDataMap;

    StackType( uint64_t f, uint64_t t)
        : fid( f ), timestamp( t ), childDuration( 0 ) {}

	StackType( uint64_t f, uint64_t t, uint64_t n)
		: fid( f ), timestamp( t ), childDuration( 0 ), cpt_nid( n ) {}

    ~StackType() {}

};


struct FunctionData {

    min_max_avg<uint64_t> count;
    min_max_avg<double> excl_time;
    min_max_avg<double> incl_time;
    string callpath;

    uint64_t start_nid;

    FunctionData( ) {}

    FunctionData(uint64_t n, double ex, double in, uint64_t st_nid) {

        count.append(n);
        excl_time.append(ex);
        incl_time.append(in);
        start_nid = st_nid;

    }

    ~FunctionData( ) {}


    void add( uint64_t n= 0, double ex= 0.0, double in= 0.0 ) {

        count.append( n );
        excl_time.append( ex );
        incl_time.append( in );

    }

    void add( uint64_t n= 0, double ex= 0.0,string  call=0, double in= 0.0 ) {

        count.append( n );
        excl_time.append( ex );
        callpath = call;
        incl_time.append( in );

    }

    void add( const FunctionData& other ) {

        count.add( other.count );
        excl_time.add( other.excl_time );
        callpath = other.callpath;
        incl_time.add( other.incl_time );

    }
};


struct FunctionDispersionData {

    uint64_t count;
    double excl_time_sum;

    double excl_time_minimum;
    double excl_time_low_quartile;
    double excl_time_median;
    double excl_time_top_quartile;
    double excl_time_95_percent;
    double excl_time_maximum;
    int filterRule;

    FunctionDispersionData( uint64_t a= 0, double b=0.0, double c=0.0,
                           double d=0.0, double e=0.0, double f=0.0,
                           double g=0.0, double h=0.0 ) :
        count( a ), excl_time_sum( b ), excl_time_minimum( c ),
        excl_time_low_quartile ( d ), excl_time_median( e ),
        excl_time_top_quartile( f ),excl_time_95_percent(g), excl_time_maximum( h ), filterRule(1){}

    void addFilterRule(int rule) {

    	filterRule = rule;

    }

    ~FunctionDispersionData( ) {}

};


struct FunctionMinMaxLocationData {

    min_max_Location<uint64_t> location;

    FunctionMinMaxLocationData( ) {}
    ~FunctionMinMaxLocationData( ) {}

    void add( uint64_t duration= 0, uint64_t loc= 0, uint64_t time= 0 ) {

        location.append( duration, loc, time);

    }

    void add( const FunctionMinMaxLocationData& other ) {

        location.add( other.location );

    }
};


/* counter data are similar to function data */
typedef FunctionData CounterData;

struct MessageData {

    min_max_avg<uint64_t> count_send;
    min_max_avg<uint64_t> count_recv;
    min_max_avg<uint64_t> bytes_send;
    min_max_avg<uint64_t> bytes_recv;
    min_max_avg<double>   duration_send;
    min_max_avg<double>   duration_recv;
    msgSizeHistogram      msgSizes;

    MessageData( ) { 

        msgSizes = msgSizeHistogram();

    }

    MessageData(uint64_t sn, uint64_t sb, double sd, uint64_t rn, uint64_t rb, double rd ) {

        count_send.append( sn );
        bytes_send.append( sb );
        duration_send.append( sd );
        count_recv.append( rn );
        bytes_recv.append( rb );
        duration_send.append( rd);

    }


    ~MessageData( ) {}

    MessageData( const MessageData& other ) {

      count_send = other.count_send;
      count_recv = other.count_recv;
      bytes_send = other.bytes_send;
      bytes_recv = other.bytes_recv;
      duration_send = other.duration_send;
      duration_recv = other.duration_recv;
      msgSizes = other.msgSizes;

    }

    void operator=( const MessageData& other ) {

      count_send = other.count_send;
      count_recv = other.count_recv;
      bytes_send = other.bytes_send;
      bytes_recv = other.bytes_recv;
      duration_send = other.duration_send;
      duration_recv = other.duration_recv;
      msgSizes = other.msgSizes;

    }

    void add_send( uint64_t n= 0, uint64_t b= 0, double d= 0.0 ) {

        count_send.append( n );
        bytes_send.append( b );
        duration_send.append( d );
        msgSizes.addValue( b );

    }

    void add_recv( uint64_t n= 0, uint64_t b= 0, double d= 0.0 ) {

        count_recv.append( n );
        bytes_recv.append( b );
        duration_recv.append( d );
        msgSizes.addValue( b );

    }

    void add( const MessageData& other ) {

        count_send.add( other.count_send );
        count_recv.add( other.count_recv );
        bytes_send.add( other.bytes_send );
        bytes_recv.add( other.bytes_recv );
        duration_send.add( other.duration_send );
        duration_recv.add( other.duration_recv );
        msgSizes.addHistogram( other.msgSizes );

    }

};


struct MessageSpeedData {

    static const uint32_t BIN_LOG_BASE= 2;

    min_max_avg<uint64_t> count;


    MessageSpeedData( ) {}
    ~MessageSpeedData( ) {}

    void add( uint64_t n= 0 ) {

        count.append( n );

    }

    void add( const MessageSpeedData& other ) {

        count.add( other.count );

    }

};


struct FunctionSummaryData {

    min_max_avg<uint64_t> bytes_send;
    min_max_avg<uint64_t> bytes_recv;
    min_max_avg<uint64_t> count;

    min_max_avg<double> excl_time;

    FunctionSummaryData( ) {}
    ~FunctionSummaryData( ) {}

};

/* collective data are similar to message data */
typedef MessageData CollectiveData;


struct MessageSizeData {

    uint64_t              count;
    min_max_avg<uint64_t> time;

    MessageSizeData() : count(0) {}

    ~MessageSizeData() {}

};


struct PendingCollective {

    uint32_t collop;
    uint64_t bytes_send;
    uint64_t bytes_recv;
    uint64_t begin_time;

    PendingCollective()
        : collop(0), bytes_send(0), bytes_recv(0), begin_time(0) {}

    PendingCollective( uint32_t _collop, uint64_t _bytes_send,
        uint64_t _bytes_recv, uint64_t _begin_time )
        : collop(_collop), bytes_send(_bytes_send), bytes_recv(_bytes_recv),
          begin_time(_begin_time) {}

    ~PendingCollective() {}

};

struct SystemTreeNode {

    string name;
    SystemTreeClass class_id;
    uint64_t location;
    uint64_t parent;
    uint64_t level;
    set < uint64_t > children;
    uint64_t child_counter;

    SystemTreeNode()
        : name(""), class_id(SYSTEMTREE_UNKNOWN),location(0),
    parent(0),level(0),child_counter(0) {children.clear();}

    SystemTreeNode (string _name, SystemTreeClass _class_id,
                    uint64_t _location, uint64_t _parent )
        : name(_name), class_id(_class_id), location(_location),
    parent(_parent),level(0),child_counter(0) {children.clear();}

    ~SystemTreeNode () {children.clear();}

};


struct functionDetail {

	uint64_t beginLine;
	string name;

	functionDetail(uint64_t _beginLine, string _name) {

		beginLine = _beginLine;
		name = _name;

	}

};

struct RmaData {

        uint64_t rma_put_cnt;
        uint64_t rma_get_cnt;

        uint64_t rma_put_bytes;
        uint64_t rma_get_bytes;

};

/* *** management and statistics data structures, needed on all ranks *** */

struct AllData {

    /* MPI-rank and number of analysis processes */
    const uint32_t myRank;
    const uint32_t numRanks;

#ifdef OTFPROFILE_MPI
    /* one instance of send/receive buffer to be re-used all the time */
    uint32_t packBufferSize;
    char*    packBuffer;
#endif /* OTFPROFILE_MPI */


    //COLOUR  ...circos...
    vector<int> vector_R, vector_G, vector_B;

    /* number and list of processes to be handled by every worker */
    uint32_t  myProcessesNum;
    uint64_t* myProcessesList;

    /* program parameters */
    Params params;

    /* progress information */
    Progress progress;

    /* runtime measurement */
    Measurement measurement;

    /* grouping information for ranks */
    Grouping grouping;

    /* OTF2 groups */
    map< uint64_t, set< uint64_t> > groups;

    /* map MPI communicators references to groups */
    map< uint64_t, uint64_t > communicators;

    /* trace context information */
    
    /* program name*/
    string progname;
    
    /* trace creator */
    string creator;

    /* OTF version */
    string version;

    /* definition comments */
    string comments;

    /* all defined process IDs with its parents */
    set< Process, ltProcess > allProcesses; 

    /* map function IDs to their corresponding names */
    map< uint64_t, string > functionIdNameMap; 

	/* map function IDs to starting linenumber and filename */
	map< uint64_t, functionDetail > functionDetails; 

    /* P2P MPI function IDs */
    set< uint64_t > P2PFuncIds;
    
    /* Collective MPI function IDs */
    set< uint64_t > CollFuncIds;
    
    /* map counter IDs to their corresponding names */
    map< uint64_t, string > counterIdNameMap;

    /* map process IDs to their corresponding names */
    map< uint64_t, string > processIdNameMap;

    /* map functiongroup IDs to their corresponding names */
    map< uint64_t, string > functionGroupIdNameMap;

    /* map function IDs to their corresponding Group */
    map< uint64_t, uint64_t > functionIdToGroupIdMap;

    /* map system tree node IDs to their corresponding node information */
    map< uint64_t, SystemTreeNode > systemTreeIdToSystemTreeNodeMap;

    /* map system tree classes to their corresponding nodes */
    map< SystemTreeClass, set < uint64_t > > systemTreeClassToNodeIdsMap;

    /* map trace definition IDs to their corresponding IDs in the system tree
     Pair is <SystemTreeClass,ID> */
    map< Pair, uint64_t, ltPair > traceIdToSystemTreeId;


    /* data collection helper datastructures, they hold management and temp
    information while reading the OTF streams */

    /* this is the function stack per trace process which is tracked while
    reading the trace processes of the current worker */
    map< uint64_t, list<StackType> > stackPerProcess;

    /* temporary store per-rank collective begin operations
    Pair is <matching-id,rank> */
    map< Pair, PendingCollective, ltPair > pendingCollectives;

    /* map all defined collective operations to the four
    classes { barrier, one-to-all, all-to-one, all-to-all }
    use the OTF constants for the four classes */
    map< uint64_t, uint64_t > collectiveOperationsToClasses; 

    /* OTF counter IDs to consider in statistics, ignore all other counters */
    set< uint64_t > countersOfInterest;

    /* timer resolution (ticks per second) */
    uint64_t timerResolution; 

    /* clock offset */
    uint64_t clockOffset; 

    /* key for OTF key-value-pairs with message matching information */
    uint64_t recvTimeKey; 

    /* data collection containers:
    the following maps are for collecting individual data per trace rank,
    they will be summarized to the next set of maps */

    /* store per-function statistics over the ranks, Pair is <rank,funcId>

    in case of additional clustering, collect it to the master node such that
    process clustering according to similar function call patterns can
    be done */
    map< Pair, FunctionData, ltPair > functionMapPerRank;

    /* store per-function statistics over the ranks, Triple is <rank,callpath,funcId>

    in case of additional clustering, collect it to the master node such that
    process clustering according to similar function call patterns can
    be done */
    map< TripleCallpath, FunctionData, ltTripleCallpath > functionCallpathMapPerRank;

    /* store per-function duration section information over the ranks, Triple is
    <rank,funcId,bin> */
    map< Triple, FunctionData, ltTriple > functionDurationSectionMapPerRank;

    /* store per-function duration section information over the ranks, Quadruple is
    <rank,funcId,callpath,bin> */
    map< Quadruple, FunctionData, ltQuadruple > functionDurationSectionCallpathMapPerRank;

    /* store per-counter statistics over the functions and ranks,
    Triple is <rank,funcId,counterId> */
    map< Triple, CounterData, ltTriple > counterMapPerFunctionRank;

    /* store send-recv statistics for P2P messages per communication pairs,
    Pair is <rank,peer> */
    map< Pair, MessageData, ltPair > messageMapPerRankPair;

    map< Pair, MessageData, ltPair > messageMapPerSystemTreeNodePair;

    map< Pair, MessageData, ltPair > messageMapPerSystemTreeNodePairReduce;

    /* store send-recv statistics per rank without differenciating the
    communication partners */
    map< uint64_t, MessageData > messageMapPerRank;

    map< uint64_t, MessageData > messageMapPerSystemTreeNode;

    /* store statistics for each message size */
    map< uint64_t, MessageSizeData > messageMapPerSize;

    /* store per-collop.-class statistics over the ranks,
    Pair is <rank,collective-class> */
    map< Pair, CollectiveData, ltPair > collectiveMapPerRank;

    map< Pair, CollectiveData, ltPair > collectiveMapPerSystemTreeNode;

    /* store per-collop.-class statistics over the ranks,
    Triple is <rank, root/partner, collective-class> */
    map< Triple, CollectiveData, ltTriple > collectiveMapPerRankPair;

    map< Triple, CollectiveData, ltTriple > collectiveMapPerSystemTreeNodePair;
    
    map< Triple, CollectiveData, ltTriple > collectiveMapPerSystemTreeNodePairReduce;

    /* data summarization and reduction containers:
    the following maps are filled when summarizing the previous set of maps,
    they will be considered in the later reduce operation over the analysis ranks. */
    
    /* store per-function statistics over the ranks, Pair is <rank,funcId> */
    map< Pair, FunctionData, ltPair > functionMapPerSystemTreeNodeReduce; 
    
    /* compact function statistics summed over all ranks */
    map< uint64_t, FunctionData > functionMapGlobal;

    /* compact function statistics summed over all ranks */
    map< PairCallpath, FunctionData, ltPairCallpath > functionCallpathMapGlobal; 

    /* compact function duration section information over the functions and bins,
    Pair is <funcId,bin> */
    map< Pair, FunctionData, ltPair > functionDurationSectionMapGlobal; 

    /* compact function duration section information over the functions and bins,
    TripleCallpath is <funcId,callpath,bin> */
    map< TripleCallpath, FunctionData, ltTripleCallpath > functionDurationSectionCallpathMapGlobal;

    /* store per-counter statistics over the functions and ranks,
    Pair is <counterId,funcId> */
    map< Pair, CounterData, ltPair > counterMapGlobal;

    /* compact send-recv statistics for P2P messages per communicating groups,
    groups are groups of neigbor ranks,
    Pair is <rank,peer> */
    map< Pair, MessageData, ltPair > messageMapPerGroupPair;

    /* compact send-receive statistics per group without differenciating the
    communication partners; group is a group of ranks */
    map< uint64_t, MessageData > messageMapPerGroup;

    /* store per-speed-bin statistics over the length-bins of P2P messages,
    Pair is <speed-bin,length-bin> where bin is log2(<speed|length>) */
    map< Pair, MessageSpeedData, ltPair > messageSpeedMapPerLength;

    /* compact collective operation statistics per group;
    group is a group of ranks, Pair is <collective-class,group> */
    map< Pair, CollectiveData, ltPair > collectiveMapPerGroup;

    /* compact function location information over the functions */
    map< uint64_t, FunctionMinMaxLocationData > functionMinMaxLocationMap;

    /* compact function location information over the callpathes */
    map< string, FunctionMinMaxLocationData > functionMinMaxLocationCallpathMap;

    /* dispersion information over functions, Pair is < dispersion, funcId > */
    map< Pair, FunctionDispersionData, gtPair > functionDispersionMap;

    /* dispersion information over functions, TripleCallpath is < dispersion,  callpath, funcId > */
    map< TripleCallpath, FunctionDispersionData, gtTripleCallpathSortByCallpath > functionDispersionCallpathMap;

    /* Maximum number of chars to save a callpath*/
    uint64_t maxCallpathLength;

	/* CallPathTree saved in map<uint64_t, CallPathTreeNode> and Mapping saved in map<pair<uint64_t, uint64_t>,uint64_t>*/
	CallPathTree callPathTree;

    /* 
        mapping for custom communicators < comID, members> 
        members -> [rank in Com] -> value = rank in ComWorld
    */
    map< uint64_t, vector< uint64_t > > comRankToWorldRank;

    //Cube
    //
    /* Pair should be <rank, func_id> */
    map< Pair, FunctionData, ltPair > functionMapPerSystemTreeNode;

    /* < NodeID, Rank||Thread > , FunctionData */
    map< pair< uint64_t, uint64_t >, FunctionData > functionDataPerCPTNode;
    
    /* <NodeId, Rank||Thread >, MessageData */
    map< pair < uint64_t, uint64_t >, MessageData > MessageDataPerNode;

    /*<NodeId, Rank||Thread >, MessageData */
    map< pair < uint64_t, uint64_t >, MessageData > CollopDataPerNode;

    /*<NodeId, Rank||Thread >, Count> */
    map< pair < uint64_t, uint64_t >, RmaData > RmaMsgData;

    /* Border where dispersion marker were set */
    uint64_t dispersionMarkerBorder;
    AllData( uint32_t my_rank= 0, uint32_t num_ranks= 1 ) :
        myRank(my_rank), numRanks(num_ranks), myProcessesNum(0),
        myProcessesList(NULL), timerResolution(0), clockOffset(0),
        recvTimeKey(0), dispersionMarkerBorder(0) {

#ifdef OTFPROFILE_MPI
        packBufferSize= 0;
        packBuffer= NULL;
#endif /* OTFPROFILE_MPI */

    }


    ~AllData() {

        myProcessesNum= 0;
        free( myProcessesList );
        myProcessesList= NULL;

#ifdef OTFPROFILE_MPI
        packBufferSize= 0;
        if ( packBuffer ) {

            free( packBuffer );
            packBuffer= NULL;
        }
#endif /* OTFPROFILE_MPI */
    }

#ifdef OTFPROFILE_MPI
    char* guaranteePackBuffer( uint32_t size ) {

        if ( packBufferSize < size ) {

            packBufferSize= size;
            packBuffer= (char*) realloc( packBuffer, packBufferSize * sizeof(char) );
            assert( NULL != packBuffer );
        }

        return packBuffer;
    }


    char* freePackBuffer( ) {

        free( packBuffer );
        packBuffer= NULL;
        packBufferSize= 0;

        return NULL;
    }


    char* getPackBuffer( ) {

        return packBuffer;
    }
#endif /* OTFPROFILE_MPI */

};


#endif /* DATASTRUCTS_H */
