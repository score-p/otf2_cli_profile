# otf2_cli_profile
## a command-line tool to read OTF and OTF2 trace files and convert them to profiles

## Output formats
* CUBE profile files for Scalasca and other compatible tools
* High-level JSON summaries of the contents of the trace

## Usage
```
[mpirun] otf-profiler -i input-filename -o output-basename [ARGS]
```
### Arguments
`--cube`: produce a CUBE profile

`--json`: produce a JSON summary

`-v n`: increase output verbosity

`--version`: print version

`-nm`, `--no-metrics`: skip metrics

`-b`: set buffer size for reader (default 1MB)

`-f`: set maximal file handles per MPI rank

`-h`, `--help`: get usage message

## Details

There are four main components to the JSON output produced by `otf-profiler`: metadata about the job being traced, a breakdown of the job's CPU time into computation/communication/IO categories, a summary of function call information, and a summary of I/O handles accessed by the job.

The metadata contained in the JSON output includes the job ID and the number of nodes, processes, and threads present in the trace. It also includes a pointer to the input file and the timestamp resolution of that file.

The JSON output produced by `otf-profiler` provides a single-level overview of the computation, communication, and I/O behavior represented by the input trace. Communication presently includes MPI and OpenMP functions. I/O includes any I/O operations represented in the input OTF2 file; currently this includes ISO C, POSIX, MPI I/O, netCDF, and HDF5 I/O operations. All other CPU time is assumed to be computation.

The JSON profile also approximates the time spent during the lifetime of the job in serial regions (only one thread of execution) and parallel regions (more than one thread or process active). It does not presently distinguish between single-node and multi-node parallelism. It also provides the total number of function invocations and the number of unique functions invoked.

Finally, the I/O handle summary provides a list of files accessed by the process, their associated I/O paradigms, their access modes, and the name of the parent file if it differs (e.g. if an HDF5 file is associated with multiple POSIX files, the entries for the POSIX files will point to the parent HDF5 file). When a user combines this information from multiple JSON summaries, they can determine what jobs in their workflow contain actual data dependencies and which jobs could be run independently.
