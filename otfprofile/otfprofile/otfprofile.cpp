/*
 This is part of the OTF library. Copyright by ZIH, TU Dresden 2005-2013.
 Authors: Andreas Knuepfer, Robert Dietrich, Matthias Jurenz
 */

#include <cassert>
#include <iostream>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#ifdef HAVE_OPEN_TRACE_FORMAT
#include <otf.h>

#endif

#include "collect_data.h"
#include "otfprofile.h"
#include "summarize_data.h"
#include "clustering.h"
#include "create_csv.h"
#include "create_latex.h"
#include "create_circos.h"
#include "circos_fix.h"

#ifdef HAVE_CUBE

    #include <cube/Cube.h>
    #include "create_cube.h"

#endif /* HAVE_CUBE*/

#ifdef OTFPROFILE_MPI
#   include "reduce_data.h"

/* define the following macro to synchronize the error indicator with all
 worker ranks (only significant for otfprofile-mpi)

 This enforces that all ranks will be terminated by calling MPI_Abort if
 anyone fails. This is necessary to work around a bug that appears at least
 with Open MPI where calling MPI_Abort on one task doesn't terminate all
 other ranks. */
#   define SYNC_ERROR
#endif /* OTFPROFILE_MPI */

/* define the following macro to print result data to stdout */
/*#define SHOW_RESULTS*/

/* define the following macro to enable support for synthetic input data for
 CLINKAGE clustering (introduces options -R <nranks> and -F <nfuncs>) */
/*#define CLINKAGE_SYNTHDATA*/

using namespace std;

/* name of program executable */
#ifdef OTFPROFILE_MPI
const string ExeName= "otfprofile-mpi";
#else /* OTFPROFILE_MPI */
const string ExeName = "otfprofile";
#endif /* OTFPROFILE_MPI */

/* parse command line options
 return 0 if succeeded, 1 if help text or version showed, -1 if failed */
static int parse_command_line(int argc, char** argv, AllData& alldata);

#ifdef SHOW_RESULTS
/* show result data on stdout */
static void show_results( const AllData& alldata );
#endif /* SHOW_RESULTS */

/* show helptext */
static void show_helptext(void);

int main(int argc, char** argv) {

    int ret = 0;
    
#ifdef OTFPROFILE_MPI
    /* start MPI */

    int my_rank;
    int num_ranks;

    MPI_Init( &argc, &argv );

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank );
    MPI_Comm_size(MPI_COMM_WORLD, &num_ranks );

    AllData alldata( my_rank, num_ranks );
#else /* OTFPROFILE_MPI */
    AllData alldata;
#endif /* OTFPROFILE_MPI */

    do {

        
        /* step 0: parse command line options */
        if (0 != (ret = parse_command_line(argc, argv, alldata))) {

            if (1 == ret) {

                ret = 0;

            } else { /* -1 == ret */

                ret = 1;

            }

            break;

        }

        VerbosePrint(alldata, 1, true, "initializing\n");

        /* step 1: collect data by reading input trace file */
        if (!CollectData(alldata)) {

            ret = 1;
            break;

        }

     
#ifndef SHOW_RESULTS
        if (alldata.params.create_tex || alldata.params.clustering.enabled || alldata.params.create_circos )

#endif /* SHOW_RESULTS */
        {
            /* step 3: summarize data; every analysis rank summarizes its local
             data independently; only necessary if producing LaTeX output or
             showing result data on stdout is enabled */

            if( alldata.params.create_circos ) {

                setup_data(alldata);

            }

            if (!SummarizeData(alldata)) {

                ret = 1;
                break;

            }

        }
        
#ifdef OTFPROFILE_MPI
        if ( 1 < alldata.numRanks &&
                ( alldata.params.create_tex ||
                  alldata.params.dispersion.enabled ||
                  alldata.params.clustering.enabled ||
                  alldata.params.create_csv ||
                  ( alldata.params.create_cube && !alldata.params.no_reduce ) ||
                  alldata.params.create_circos ) ) {
            /* step 4: reduce data to master; summarized data for producing
             LaTeX output; per-process/function statistics for additional
             clustering */

            if ( !ReduceData( alldata ) ) {

                ret= 1;
                break;

            }

        }
#endif /* OTFPROFILE_MPI */

        /* step 6: produce outputs */
        if (alldata.params.create_csv) {

            /* step 6.1: create CSV output */
            if (!CreateCSV(alldata)) {

                ret = 1;
                break;

            }

        }

        if (alldata.params.create_circos && 0 == alldata.myRank) {
            
            /* step 6.4: create Circos output */
            CreateCircos(alldata);

        }
        
        if (alldata.params.create_tex && 0 == alldata.myRank) {

            /* step 6.2: create LaTeX output */
            if (!CreateTex(alldata)) {

                ret = 1;
                break;

            }
        }
#ifdef HAVE_CUBE

        if (alldata.params.create_cube) {

            /* step 6.3: create CUBE output */
            
            CreateCube(alldata);

        }

#endif /* HAVE_CUBE */
        
#ifdef SHOW_RESULTS
        /* step 6.3: show result data on stdout */

        if ( 0 == alldata.myRank ) {

            show_results( alldata );

        }
#endif /* SHOW_RESULTS */

        if (alldata.params.clustering.enabled) {
            /* step 7: do additional process clustering */
            if (!ProcessClustering(alldata)) {

                ret = 1;
                break;

            }

        }

    } while (false);

    if (0 == ret) {

        if (0 == alldata.myRank) {

            /* print runtime measurement results to stdout */
            PrintMeasurement(alldata);

        }

        VerbosePrint(alldata, 1, true, "done\n");

    }

#ifdef OTFPROFILE_MPI
    /* either finalize or abort on error */

    if ( 0 == ret ) {

        MPI_Finalize();

    } else {

        MPI_Abort( MPI_COMM_WORLD, ret );

    }
#endif /* OTFPROFILE_MPI */

    return ret;

}

static int parse_command_line(int argc, char** argv, AllData& alldata) {

    int ret = 0;
    Params& params = alldata.params;

    /* parse command line options */

    enum {
        ERR_OK, ERR_OPT_UNKNOWN, ERR_ARG_MISSING, ERR_ARG_INVALID
    };
    int parse_error = ERR_OK;

    int i;

    for (i = 1; i < argc; i++) {

        /* -h, --help */
        if (0 == strcmp("-h", argv[i]) || 0 == strcmp("--help", argv[i])) {

            if (0 == alldata.myRank) {

                show_helptext();

            }

            return 1;

            /* -v */
        } else if (0 == strcmp("-v", argv[i])) {

            params.verbose_level++;

            /* -p */
        } else if (0 == strcmp("-p", argv[i])) {

            params.progress = true;

            /* -i */
        } else if (0 == strcmp("-i", argv[i])) {

            if (argc - 1 == i) {

                parse_error = ERR_ARG_MISSING;
                break;

            }
            
            params.input_file_name = argv[++i];


            /* -f */
        } else if (0 == strcmp("-f", argv[i])) {

            if (argc - 1 == i) {

                parse_error = ERR_ARG_MISSING;
                break;

            }

            int tmp = atoi(argv[i + 1]);
            if (0 >= tmp) {

                parse_error = ERR_ARG_INVALID;
                break;
            }

            params.max_file_handles = tmp;
            i++;

            /* -b */
        } else if (0 == strcmp("-b", argv[i])) {

            if (argc - 1 == i) {

                parse_error = ERR_ARG_MISSING;
                break;

            }

            int tmp = atoi(argv[i + 1]);
            if (0 >= tmp) {

                parse_error = ERR_ARG_INVALID;
                break;
            }

            params.buffer_size = tmp;
            i++;

            /* -o */
        } else if (0 == strcmp("-o", argv[i])) {

            if (argc - 1 == i) {

                parse_error = ERR_ARG_MISSING;
                break;

            }
            params.output_file_prefix = argv[++i];

            /* -g */
        } else if (0 == strcmp("-g", argv[i])) {

            if (argc - 1 == i) {

                parse_error = ERR_ARG_MISSING;
                break;

            }

            int tmp = atoi(argv[i + 1]);
            if (1 > tmp || (int) Grouping::MAX_GROUPS < tmp) {

                parse_error = ERR_ARG_INVALID;
                break;
            }

            params.max_groups = tmp;
            i++;

            /* -c, --cluster */
        } else if (0 == strcmp("-c", argv[i]) ||
                   0 == strcmp("--cluster", argv[i])) {

            params.clustering.enabled = true;

            /* --cluster-alg */
        } else if (0 == strcmp("--cluster-alg", argv[i])) {

            if (argc - 1 == i) {

                parse_error = ERR_ARG_MISSING;
                break;

            }

            if (0 == strcmp("CLINKAGE", argv[i + 1])) {

                params.clustering.alg = CLUSTER_ALG_CLINKAGE;

            } else if (0 == strcmp("KMEANS", argv[i + 1])) {

                params.clustering.alg = CLUSTER_ALG_KMEANS;

            } else {

                parse_error = ERR_ARG_INVALID;
                break;

            }

            params.clustering.enabled = true;
            i++;

            /* -H */
        } else if (0 == strcmp("-H", argv[i])) {

            params.clustering.enabled = true;
            params.clustering.alg = CLUSTER_ALG_CLINKAGE;
            params.clustering.hard_grouping = true;

            /* -q */
        } else if (0 == strcmp("-q", argv[i])) {

            if (argc - 1 == i) {

                parse_error = ERR_ARG_MISSING;
                break;

            }

            double tmp = atof(argv[i + 1]);
            if (0.0 > tmp || 1.0 < tmp) {

                parse_error = ERR_ARG_INVALID;
                break;
            }

            params.clustering.enabled = true;
            params.clustering.alg = CLUSTER_ALG_CLINKAGE;
            params.clustering.quality_threshold = tmp;
            i++;

#ifdef CLINKAGE_SYNTHDATA 
            /* -R */
        } else if ( 0 == strcmp( "-R", argv[i] ) ) {

            if ( argc - 1 == i ) {

                parse_error= ERR_ARG_MISSING;
                break;

            }

            int tmp= atoi( argv[i+1] );
            if( 0 >= tmp ) {

                parse_error= ERR_ARG_INVALID;
                break;
            }

            params.clustering.enabled= true;
            params.clustering.alg= CLUSTER_ALG_CLINKAGE;
            params.clustering.synth_data= true;
            params.clustering.synth_ranks_num= tmp;
            i++;

            /* -F */
        } else if ( 0 == strcmp( "-F", argv[i] ) ) { 

            if ( argc - 1 == i ) {

                parse_error= ERR_ARG_MISSING;
                break;

            }

            int tmp= atoi( argv[i+1] );
            if( 0 >= tmp ) {

                parse_error= ERR_ARG_INVALID;
                break;
            }

            params.clustering.enabled= true;
            params.clustering.alg= CLUSTER_ALG_CLINKAGE;
            params.clustering.synth_data= true;
            params.clustering.synth_funcs_num= tmp;
            i++;
#endif /* CLINKAGE_SYNTHDATA */


            /* -M, --csv-msg-matrix */

#ifdef HAVE_OPEN_TRACE_FORMAT

        } else if (0 == strcmp("-M", argv[i]) ||
                   0 == strcmp("--csv-msg-matrix", argv[i])) {

            params.create_csv = params.write_csv_msg_matrices = true;


            /* -S, --csv-msg-size */
        } else if (0 == strcmp("-S", argv[i]) ||
                   0 == strcmp("--csv-msg-size", argv[i])) {

            params.create_csv = params.write_csv_msg_sizes = true;

            /* --csv */

#endif

        } else if (0 == strcmp("--csv", argv[i])) {

            params.create_csv = true;

            /* --nocsv */
        } else if (0 == strcmp("--nocsv", argv[i])) {

            params.create_csv = false;

            /* --tex */
        } else if (0 == strcmp("--tex", argv[i])) {

            params.create_tex = true;

            /* --notex */
        } else if (0 == strcmp("--notex", argv[i])) {

            params.create_tex = false;

#if defined(PDFTEX) && defined(HAVE_PGFPLOTS_1_4) && HAVE_PGFPLOTS_1_4
            /* --pdf */
        } else if ( 0 == strcmp( "--pdf", argv[i] ) ) {

            params.create_tex= true;
            params.create_pdf= true;

            /* --nopdf */
        } else if ( 0 == strcmp( "--nopdf", argv[i] ) ) {

            params.create_pdf= false;
#endif /* PDFTEX && HAVE_PGFPLOTS_1_4 */
            /* --cube */
        } else if( 0 == strcmp( "--cube", argv[i] ) ) {
            
            params.create_cube = true;
            /* --noreduce */
        } else if( 0 == strcmp( "--noreduce", argv[i] ) ) {
            
            params.no_reduce = true;
            params.create_cube = true;
            /* --nocube */
        } else if ( 0 == strcmp( "--nocube", argv[i] ) ) {
    
            params.create_cube = false;

            /* --circos */
        } else if( 0 == strcmp( "--circos", argv[i] ) ) {
    
            params.create_circos = true;
            /* --nocircos */
        } else if ( 0 == strcmp( "--nocircos", argv[i] )) {
    
            params.create_circos = false; 
            /* circos Layout */            

        } else {
            parse_error = ERR_OPT_UNKNOWN;
            break;
        }
        
        
    }
            
    /* show specific message on error */
    if (ERR_OK != parse_error) {

        if (0 == alldata.myRank) {

            switch (parse_error) {

            case ERR_OPT_UNKNOWN:

                cerr << "ERROR: Unknown option '" << argv[i] << "'." << endl;
                break;

            case ERR_ARG_MISSING:

                cerr << "ERROR: Expected argument for option '" << argv[i]
                        << "'." << endl;
                break;

            case ERR_ARG_INVALID:

                cerr << "ERROR: Invalid argument for option '" << argv[i]
                        << "'." << endl;
                break;

            }

        }

        ret = -1;

        /* show help text if no input trace file is given */
    } else if (0 == params.input_file_name.length()) {

        if (0 == alldata.myRank) {

            show_helptext();

        }

        ret = 1;

    }


    return ret;
}


#ifdef SHOW_RESULTS
static void show_results( const AllData& alldata ) {

#   define PRINT_MIN_MAX_AVG(v,u) (v.cnt) << " x avg " << ((double)(v.sum))/(v.cnt) << "(" << (v.min) << "-" << (v.max) << ") " << u 
    /*
     cout << endl << " global data per function: " << endl;
     {
     map< uint64_t, FunctionData >::const_iterator it= alldata.functionMapGlobal.begin();
     map< uint64_t, FunctionData >::const_iterator itend= alldata.functionMapGlobal.end();
     while ( itend != it ) {

     cout << "     global function " << it->first << " -> " ;
     if ( it->second.count.cnt ) {
     cout << "\t"<<
     " cnt: " << PRINT_MIN_MAX_AVG(it->second.count,"[#]") <<
     " exc: " << PRINT_MIN_MAX_AVG(it->second.excl_time,"[t]") <<
     " inc: " << PRINT_MIN_MAX_AVG(it->second.incl_time,"[t]") << endl;
     }

     it++;
     }
     }

     cout << endl << " global counter data per function: " << endl;
     {
     map< Pair, FunctionData, ltPair >::const_iterator it= alldata.counterMapGlobal.begin();
     map< Pair, FunctionData, ltPair >::const_iterator itend= alldata.counterMapGlobal.end();
     while ( itend != it ) {

     cout << "     global counter " << it->first.a << " per function " << it->first.b << " -> " << endl;
     if ( it->second.count.cnt ) {
     cout << "\t"<<
     " cnt: " << PRINT_MIN_MAX_AVG(it->second.count,"[#]");
     cout << " exc: ";
     if ( it->second.excl_time.cnt ) {
     cout << PRINT_MIN_MAX_AVG(it->second.excl_time,"[#]");
     } else {
     cout << "0 [#]";
     }
     cout << " inc: ";
     if ( it->second.incl_time.cnt ) {
     cout << PRINT_MIN_MAX_AVG(it->second.incl_time,"[#]");
     } else {
     cout << "0 [#]";
     }
     cout << endl;
     }

     it++;
     }
     }
     */
    cout << endl << " global function duration section data per bin: " << endl;
    {

        map< Pair, FunctionData, ltPair >::const_iterator it= alldata.functionDurationSectionMapGlobal.begin();
        map< Pair, FunctionData, ltPair >::const_iterator itend= alldata.functionDurationSectionMapGlobal.end();

        while ( itend != it ) {

            cout << "     global function  " << it->first.a << " bin " << it->first.b << " -> ";
            if ( it->second.count.cnt ) {
                cout << "\t"<<
                " cnt: " << PRINT_MIN_MAX_AVG(it->second.count,"[#]") <<
                " exc: " << PRINT_MIN_MAX_AVG(it->second.excl_time,"[t]") <<
                " inc: " << PRINT_MIN_MAX_AVG(it->second.incl_time,"[t]") << endl;
            }

            it++;
        }
    }
    /*
     
     cout << endl << " global function callpath duration section data per bin: " << endl;
     {

     map< Triplec, FunctionData, ltTriplec >::const_iterator it= alldata.functionDurationSectionCallpathMapGlobal.begin();
     map< Triplec, FunctionData, ltTriplec >::const_iterator itend= alldata.functionDurationSectionCallpathMapGlobal.end();

     while ( itend != it ) {

     cout << "     global function  " << it->first.a << " callpath " << it->first.b << " bin " << it->first.c << " -> ";
     if ( it->second.count.cnt ) {
     cout << "\t"<<
     " cnt: " << PRINT_MIN_MAX_AVG(it->second.count,"[#]") <<
     " exc: " << PRINT_MIN_MAX_AVG(it->second.excl_time,"[t]") <<
     " inc: " << PRINT_MIN_MAX_AVG(it->second.incl_time,"[t]") << endl;
     }

     it++;
     }
     }
     */
    /*
     cout << endl << " global function dispersion: " << endl;
     {

     map< Pair, FunctionDispersionData, gtPair >::const_iterator it= alldata.functionDispersionMap.begin();
     map< Pair, FunctionDispersionData, gtPair >::const_iterator itend= alldata.functionDispersionMap.end();


     while ( itend != it ) {

     cout << "     dispersion  " << it->first.a << " global function " << it->first.b << " -> ";

     if ( it->second.count ) {
     cout << "\t" <<
     " tmin: " << it->second.excl_time_minimum <<
     "\t t_25: " << it->second.excl_time_low_quartile <<
     "\t tmed: " << it->second.excl_time_median <<
     "\t t_75: " << it->second.excl_time_top_quartile <<
     "\t tmax: " << it->second.excl_time_maximum <<
     "\t tavg: " << it->second.excl_time_sum / it->second.count << endl;
     }

     it++;
     }

     }

     cout << endl << " global function dispersion per call-path: " << endl;
     {

     map< TripleCallpath, FunctionDispersionData, gtTripleCallpathSortByCallpath >::const_iterator it= alldata.functionDispersionCallpathMap.begin();
     map< TripleCallpath, FunctionDispersionData, gtTripleCallpathSortByCallpath >::const_iterator itend= alldata.functionDispersionCallpathMap.end();


     while ( itend != it ) {

     cout << "     dispersion  " << it->first.a << " callpath " << it->first.b << " global function " << it->first.c << " -> ";

     if ( it->second.count ) {
     cout << "\t" <<
     " tmin: " << it->second.excl_time_minimum <<
     "\t t_25: " << it->second.excl_time_low_quartile <<
     "\t tmed: " << it->second.excl_time_median <<
     "\t t_75: " << it->second.excl_time_top_quartile <<
     "\t tmax: " << it->second.excl_time_maximum <<
     "\t tavg: " << it->second.excl_time_sum / it->second.count << endl;
     }

     it++;
     }

     }

     cout << endl << " global message data per group pair: " << endl;
     {
     map< Pair, MessageData >::const_iterator it=    alldata.messageMapPerGroupPair.begin();
     map< Pair, MessageData >::const_iterator itend= alldata.messageMapPerGroupPair.end();
     while ( itend != it ) {

     if ( it->second.count_send.cnt ) {
     cout << "\tsent " << it->first.a << " --> " << it->first.b <<
     " cnt: " << PRINT_MIN_MAX_AVG(it->second.count_send,"[#]");
     cout << " byt: ";
     if ( it->second.bytes_send.cnt ) {
     cout << PRINT_MIN_MAX_AVG(it->second.bytes_send,"[b]");
     } else {
     cout << "0 [b]";
     }
     cout << " dur: ";
     if ( it->second.duration_send.cnt ) {
     cout << PRINT_MIN_MAX_AVG(it->second.duration_send,"[t]");
     } else {
     cout << "0 [s]";
     }
     cout << endl;
     }
     if ( it->second.count_recv.cnt ) {
     cout << "\trecv " << it->first.a << " <-- " << it->first.b <<
     " cnt: " << PRINT_MIN_MAX_AVG(it->second.count_recv,"[#]");
     cout << " byt: ";
     if ( it->second.bytes_recv.cnt ) {
     cout << PRINT_MIN_MAX_AVG(it->second.bytes_recv,"[b]");
     } else {
     cout << "0 [b]";
     }
     cout << " dur: ";
     if ( it->second.duration_recv.cnt ) {
     cout << PRINT_MIN_MAX_AVG(it->second.duration_recv,"[t]");
     } else {
     cout << "0 [s]";
     }
     cout << endl;
     }

     it++;
     }
     }

     cout << endl << " global message data per group: " << endl;
     {
     map< uint64_t, MessageData >::const_iterator it=    alldata.messageMapPerGroup.begin();
     map< uint64_t, MessageData >::const_iterator itend= alldata.messageMapPerGroup.end();
     while ( itend != it ) {

     cout << "     msg of group " << it->first << " -> " << endl;
     if ( it->second.count_send.cnt ) {
     cout << "\tsent" <<
     " cnt: " << PRINT_MIN_MAX_AVG(it->second.count_send,"[#]");
     cout << " byt: ";
     if ( it->second.bytes_send.cnt ) {
     cout << PRINT_MIN_MAX_AVG(it->second.bytes_send,"[b]");
     } else {
     cout << "0 [b]";
     }
     cout << " dur: ";
     if ( it->second.duration_send.cnt ) {
     cout << PRINT_MIN_MAX_AVG(it->second.duration_send,"[t]");
     } else {
     cout << "0 [s]";
     }
     cout << endl;
     }
     if ( it->second.count_recv.cnt ) {
     cout << "\trecv" <<
     " cnt: " << PRINT_MIN_MAX_AVG(it->second.count_recv,"[#]");
     cout << " byt: ";
     if ( it->second.bytes_recv.cnt ) {
     cout << PRINT_MIN_MAX_AVG(it->second.bytes_recv,"[b]");
     } else {
     cout << "0 [b]";
     }
     cout << " dur: ";
     if ( it->second.duration_recv.cnt ) {
     cout << PRINT_MIN_MAX_AVG(it->second.duration_recv,"[t]");
     } else {
     cout << "0 [s]";
     }
     cout << endl;
     }

     it++;
     }
     }

     cout << endl << " global message speed per length: " << endl;
     {
     map< Pair, MessageSpeedData, ltPair >::const_iterator it= alldata.messageSpeedMapPerLength.begin();
     map< Pair, MessageSpeedData, ltPair >::const_iterator itend= alldata.messageSpeedMapPerLength.end();
     while ( itend != it ) {

     cout << "     msg of speed-bin " << it->first.a << " length-bin " << it->first.b << " -> ";
     if ( it->second.count.cnt ) {
     cout << "\t" <<
     " cnt: " << PRINT_MIN_MAX_AVG(it->second.count,"[#]") << endl;
     }

     it++;
     }
     }

     cout << endl << " global collective data per group: " << endl;
     {
     map< Pair, CollectiveData, ltPair >::const_iterator it=    alldata.collectiveMapPerGroup.begin();
     map< Pair, CollectiveData, ltPair >::const_iterator itend= alldata.collectiveMapPerGroup.end();
     while ( itend != it ) {

     cout << "     collop of class " << it->first.a << " group " << it->first.b << " -> " << endl;
     if ( it->second.count_send.cnt ) {
     cout << "\tsent" <<
     " cnt: " << PRINT_MIN_MAX_AVG(it->second.count_send,"[#]");
     cout << " byt: ";
     if ( it->second.bytes_send.cnt ) {
     cout << PRINT_MIN_MAX_AVG(it->second.bytes_send,"[b]");
     } else {
     cout << "0 [b]";
     }
     cout << " dur: ";
     if ( it->second.duration_send.cnt ) {
     cout << PRINT_MIN_MAX_AVG(it->second.duration_send,"[t]");
     } else {
     cout << "0 [s]";
     }
     cout << endl;
     }
     if ( it->second.count_recv.cnt ) {
     cout << "\trecv" <<
     " cnt: " << PRINT_MIN_MAX_AVG(it->second.count_recv,"[#]");
     cout << " byt: ";
     if ( it->second.bytes_recv.cnt ) {
     cout << PRINT_MIN_MAX_AVG(it->second.bytes_recv,"[b]");
     } else {
     cout << "0 [b]";
     }
     cout << " dur: ";
     if ( it->second.duration_recv.cnt ) {
     cout << PRINT_MIN_MAX_AVG(it->second.duration_recv,"[t]");
     } else {
     cout << "0 [s]";
     }
     cout << endl;
     }

     it++;
     }
     }
     */
}
#endif /* SHOW_RESULTS */

static void show_helptext() {

    cout << endl << " " << ExeName
         << " - Generate a profile of an OTF or OTF2 trace in a variety of formats." << endl
         << endl << " Syntax: " << ExeName << " -i <input file name> [options]"
         << endl << endl
         << "   options:" << endl
         << "      -h, --help    show this help message" << endl
         << "      -v            increase output verbosity" << endl
         << "                    (can be used more than once)" << endl
         << "      -i <file>     specify the input trace file name" << endl
         << "      -p            show progress" << endl
         << "      -f <n>        max. number of filehandles available per rank"
         << endl
         << "                    (default: "
         << Params::DEFAULT_MAX_FILE_HANDLES << ")" << endl
         << "      -b <size>     set buffersize of the reader" << endl
         << "                    (default: " << Params::DEFAULT_BUFFER_SIZE
         << ")" << endl
         << "      -o <prefix>   specify the prefix of output file(s)"<< endl
         << "                    (default: "
         << Params::DEFAULT_OUTPUT_FILE_PREFIX() << ")" << endl
         << "      -g <n>        max. number of process groups in LaTeX output"
         << endl
         << "                    (range: 1-" << Grouping::MAX_GROUPS
         << ", default: " << Params::DEFAULT_MAX_GROUPS << ")" << endl
         << "      -c, --cluster do additional clustering of processes/threads"
         << endl
         << "      --cluster-alg <alg>" << endl
         << "                    set comparison algorithm for clustering to"
         << endl
         << "                    \"KMEANS\" or \"CLINKAGE\"" << endl
         << "                    (default: ";
    if (Params::Clustering::DEFAULT_ALGORITHM == CLUSTER_ALG_CLINKAGE)
        cout << "CLINKAGE)" << endl;
    else
        cout << "KMEANS)" << endl;
        cout 
         << "      -H            use hard groups for CLINKAGE clustering"
         << endl
         << "                    (implies --cluster-alg CLINKAGE)"
         << endl
         << "      -q <0-1>      quality threshold for CLINKAGE clustering"
         << endl
         << "                    (implies --cluster-alg CLINKAGE, default: "
         << Params::Clustering::DEFAULT_QUALITY_THRESHOLD() << ")" << endl;

#ifdef HAVE_OPEN_TRACE_FORMAT
    cout << "      -M, --csv-msg-matrix" << endl
         << "                    write communication matrix in CSV file" << endl
         << "                    (only works with .otf traces)" << endl
         << "                    (implies --csv)" << endl
         << "      -S, --csv-msg-size" << endl
         << "                    write message length statistics in CSV file" << endl
         << "                    (only works with .otf traces, implies --csv)"
         << endl;
#endif
    cout << "      --[no]csv     enable/disable producing CSV output"
         << endl << "                    (default: "
         << (Params::DEFAULT_CREATE_CSV ? "enabled" : "disabled") << ")"<< endl
         << "      --[no]tex     enable/disable producing LaTeX output"<< endl
         << "                    (default: "
         << (Params::DEFAULT_CREATE_TEX ? "enabled" : "disabled") << ")"<< endl
         << "      --[no]cube    enable/disable producing CUBE output" << endl
         << "                    (default: "
         << (Params::DEFAULT_CREATE_CUBE ? "enabled" : "disabled") << ")" << endl

#ifdef OTFPROFILE_MPI
         << "        --noreduce  disable the data reduction on parallel" << endl
         << "                    analysis via otfprofile-mpi" << endl
         << "                    (implies --cube)" << endl
#endif

         << "      --[no]circos  enable/disable producing Circos output" << endl
         << "                    (default: "
         << (Params::DEFAULT_CREATE_CIRCOS ? "enabled" : "disabled") << ")" << endl
#if defined(PDFTEX) && defined(HAVE_PGFPLOTS_1_4) && HAVE_PGFPLOTS_1_4
         << "      --[no]pdf     enable/disable producing PDF output" << endl
         << "                    (implies --tex if enabled, default: "
         << ( Params::DEFAULT_CREATE_PDF ? "enabled" : "disabled" )
         << ")" << endl
#else /* PDFTEX && HAVE_PGFPLOTS_1_4 */
            << endl
            << " PDF creation requires the PGFPLOTS package version >1.4"
            << endl << " http://sourceforge.net/projects/pgfplots/ " << endl
#endif /* !PDFTEX || !HAVE_PGFPLOTS_1_4 */
            << endl;
}

void VerbosePrint(AllData& alldata, uint8_t level, bool master_only,
        const char* fmt, ...) {

    if (alldata.params.verbose_level >= level) {

        va_list ap;

        va_start( ap, fmt );

#ifdef OTFPROFILE_MPI
        if ( !master_only ) {

            char msg[1024];

            /* prepend current rank to message */
            snprintf( msg, sizeof( msg ) -1, "[%u] ", alldata.myRank );
            vsnprintf( msg + strlen( msg ), sizeof( msg ) -1, fmt, ap );

            /* print message */
            printf( "%s ", msg );

        }
        else
#endif /* OTFPROFILE_MPI */
        {
            if (0 == alldata.myRank) {

                vprintf(fmt, ap);

            }

        }

        va_end( ap );

    }
}

void StartMeasurement(AllData& alldata, uint8_t verbose_level, bool sync,
        const string& scope_name) {

#ifdef OTFPROFILE_MPI
    if ( sync ) MPI_Barrier( MPI_COMM_WORLD );
#endif /* OTFPROFILE_MPI */

    /* search for measurement scope by its name; fail if already exists */
    map<string, Measurement::Scope>::const_iterator it =
            alldata.measurement.scope_map.find(scope_name);
    if (it != alldata.measurement.scope_map.end()) {

        cerr << "WARNING: Could not start runtime measurement of scope '"
             << scope_name << "'. Scope already exists." << std::endl;
        return;
    }

    /* insert new measurement scope to map */
    Measurement::Scope& scope = alldata.measurement.scope_map.insert(make_pair(
            scope_name, Measurement::Scope(verbose_level))).first->second;

    /* start measurement on master if verbose level is high enough */

    if (0 == alldata.myRank && alldata.params.verbose_level >= verbose_level) {

        scope.start_time = Measurement::gettime();

    }
}

void StopMeasurement(AllData& alldata, bool sync, const string& scope_name) {

#ifdef OTFPROFILE_MPI
    if ( sync ) MPI_Barrier( MPI_COMM_WORLD );
#endif /* OTFPROFILE_MPI */

    /* search for measurement scope by its name */
    map<string, Measurement::Scope>::iterator it =
            alldata.measurement.scope_map.find(scope_name);
    if (it == alldata.measurement.scope_map.end()) {

        cerr << "WARNING: Could not stop runtime measurement of scope '"
             << scope_name << "'. Scope not found." << std::endl;
        return;

    }

    Measurement::Scope& scope = it->second;

    /* stop measurement */

    if (0 == alldata.myRank && alldata.params.verbose_level
            >= scope.verbose_level) {

        if (-1.0 == scope.start_time) {

            cerr << "WARNING: Could not stop runtime measurement of scope '"
                 << scope_name << "'. Measurement not started." << std::endl;

            alldata.measurement.scope_map.erase( it );
            return;

        }

        scope.stop_time = Measurement::gettime();

        alldata.measurement.have_data = true;

    }
}

void PrintMeasurement(AllData& alldata, const string& scope_name) {

    assert( 0 == alldata.myRank );

    /* either print measurement result of certain scope or print results of all
     measured scopes */

    if (0 != scope_name.length()) {

        /* search for measurement scope by its name */
        map<string, Measurement::Scope>::const_iterator it =
                alldata.measurement.scope_map.find(scope_name);
        if ( it == alldata.measurement.scope_map.end() ) {

            cerr << "WARNING: Could not print runtime measurement of scope '"
                 << scope_name << "'. Scope not found." << std::endl;
            return;

        }

        const Measurement::Scope& scope = it->second;

        /* print measurement result on stdout */

        if (alldata.params.verbose_level >= scope.verbose_level && -1.0
                != scope.start_time && -1.0 != scope.stop_time) {

            cout << " " << scope_name << ": " << scope.stop_time
                    - scope.start_time << "s" << endl;

        }

    } else if (alldata.measurement.have_data) {

        cout << "runtime measurement results:" << endl;

        /* iterate over all measurement scopes */
        for (map<string, Measurement::Scope>::const_iterator it =
                alldata.measurement.scope_map.begin(); it
                != alldata.measurement.scope_map.end(); it++) {

            /* print measurement result */
            PrintMeasurement(alldata, it->first);

        }

    }
}

uint64_t Logi(uint64_t x, uint64_t b) {

    assert( b > 1 );

    uint64_t c = 1;
    uint64_t i = 0;

    while (c <= x) {

        c *= b;
        i++;
    }

    return i;
}

#ifdef OTFPROFILE_MPI
bool SyncError( AllData& alldata, bool& error, uint32_t root ) {

#ifdef SYNC_ERROR
    if ( 1 < alldata.numRanks ) {

        int buf= ( error ) ? 1 : 0;

        /* either broadcast the error indicator from one rank (root)
         or reduce them from all */

        if ( root != (uint32_t)-1 ) {

            MPI_Bcast( &buf, 1, MPI_INT, (int)root, MPI_COMM_WORLD );

            error= ( 1 == buf );

        } else {

            int recv_buf;

            MPI_Allreduce( &buf, &recv_buf, 1, MPI_INT, MPI_MAX,
                    MPI_COMM_WORLD );

            error= ( 1 == recv_buf );

        }

    }
#endif /* SYNC_ERROR */

    return error;
}
#endif /* OTFPROFILE_MPI */
