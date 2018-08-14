/*
 This is part of the OTF-Profiler. Copyright by ZIH, TU Dresden 2016-2018.
 Authors: Maximillian Neumann, Denis Hünich, Jens Doleschal
*/

#ifndef UTILS_H
#define UTILS_H

#include <chrono>
#include <iostream>
#include <string>

#include "otf-profiler-config.h"

/*
Usage of TimeMeasurement:
For a new module you have to extend the enum class ScopeID by the unique handle for the new module.

Then you register the scope in otfprofile.cpp inside the appropriate verbosity level with:
    alldata.tm.registerScope(ScopeID::<scope_id>, <description in a string>);
The description will be printed to stdout.

You then surround the function call of the new module like this:

    alldata.tm.start(ScopeID::<scope_id>);
    <module_call>(alldata);
    alldata.tm.stop(ScopeID::<scope_id>);
*/

enum class ScopeID : uint8_t { TOTAL, COLLECT, REDUCE, CUBE, JSON };

class TimeMeasurement {
   public:
    TimeMeasurement() = default;

    void registerScope(ScopeID scope_id, std::string desc) { scopes.insert(std::make_pair(scope_id, Scope(desc))); }

    bool isRegistered(ScopeID scope_id) {
        if (scopes.find(scope_id) == scopes.end())
            return false;

        return true;
    }

    void start(ScopeID scope_id) {
        auto it = scopes.find(scope_id);
        if (it == scopes.end())
            return;

        it->second.start_time = std::chrono::system_clock::now();
    }

    void stop(ScopeID scope_id) {
        auto it = scopes.find(scope_id);
        if (it == scopes.end())
            return;

        it->second.stop_time = std::chrono::system_clock::now();
    }

    void printAll() {
        for (const auto& scope : scopes)
            std::cout << scope.second << std::endl;
    }

   private:
    struct Scope {
        using TimePoint = std::chrono::system_clock::time_point;

        // Description of scope
        const std::string desc;
        // start timestamp of measurement scope
        TimePoint start_time;
        // end timestamp of measurement scope
        TimePoint stop_time;

        Scope(const std::string _desc) : desc(_desc), start_time(), stop_time() {}

        friend std::ostream& operator<<(std::ostream& os, const Scope& scope) {
            if (scope.start_time == TimePoint() || scope.stop_time == TimePoint())
                return os;

            os << scope.desc << ": " << std::chrono::duration<double>(scope.stop_time - scope.start_time).count()
               << "s";

            return os;
        }
    };

    // stores all registered scopes
    std::map<ScopeID, Scope> scopes;
};

struct Params {
    uint32_t max_file_handles = 50;           // TODO sinn/unsinn?
    uint32_t buffer_size      = 1024 * 1024;  // TODO sinn/unsinn?
    // uint32_t    max_groups         = 16;
    // bool        logaxis            = true;
    uint8_t verbose_level = 0;
    // bool        read_from_stats    = false;
    bool        read_metrics       = true;  // counter
    bool        create_cube        = false;
    bool        create_json        = false;
    bool        summarize_it       = false;  // TODO added for testing
    std::string input_file_name    = "";
    std::string input_file_prefix  = "";
    std::string output_file_prefix = "result";

    bool parseCommandLine(int argc, char** argv) {
        // TODO help text and check for no arguments
        std::vector<std::string> arguments;
        arguments.assign(argv + 1, argv + argc);

        for (auto i = 0; i < arguments.size(); ++i) {
            if (arguments[i] == "--help" || arguments[i] == "-h") {
                std::cout << std::endl
                          << " " << argv[0] << " - Generates a profile of an OTF or OTF2 trace in CUBE and/or other"
                          << "formats." << std::endl
                          << std::endl
                          << " Syntax: " << argv[0] << " -i <input file name> [options]" << std::endl
                          << std::endl
                          << "   options:" << std::endl
                          << "      -h, --help          show this help message" << std::endl
                          << std::endl
                          << "      --cube              generates CUBE xml profile" << std::endl
                          << "      --json              generates json ouptut file" << std::endl
                          << std::endl
                          << "                          (default: 50)" << std::endl
                          << "      -b <size>           set buffersize of the reader in Byte" << std::endl
                          << "                          (default: 1 M)" << std::endl
                          << "      -f <n>              max. number of filehandles available per rank" << std::endl
                          << "      -i <file>           specify the input tracefile name" << std::endl
                          << "      -nm, --no-metrics   neglect metric events" << std::endl
                          << "      -o <prefix>         specify the prefix of output file(s)" << std::endl
                          << "                          (default: result)" << std::endl
                          << "      -v <level>          set verbosity level" << std::endl
                          << "      --version           prints version information" << std::endl;

                return false;
            } else if (arguments[i] == "-v") {
                auto value = checkNextValue(arguments, i);
                if (value < 0)
                    return false;

                verbose_level = value;
                ++i;
            } else if (arguments[i] == "--version") {
                std::cout << "OTF-Profiler version " << OTFPROFILER_VERSION_MAJOR << "." << OTFPROFILER_VERSION_MINOR
                          << "." << OTFPROFILER_VERSION_PATCH << std::endl;
                return false;
            } else if (arguments[i] == "--summarize" || arguments[i] == "-s") {
                summarize_it = true;
            } else if (arguments[i] == "--cube") {
                create_cube = true;
            } else if (arguments[i] == "--json") {
                create_json = true;
            } else if (arguments[i] == "-i") {
                if (!checkNext(arguments, i))
                    return false;

                input_file_name = arguments[++i];
            } else if (arguments[i] == "-f") {
                auto value = checkNextValue(arguments, i);
                if (value < 0)
                    return false;

                max_file_handles = value;
                ++i;
            } else if (arguments[i] == "-b") {
                auto value = checkNextValue(arguments, i);
                if (value < 0)
                    return false;

                buffer_size = value;
                ++i;
            } else if (arguments[i] == "-o") {
                auto value = checkNext(arguments, i);
                if (value < 1)
                    return false;

                output_file_prefix = arguments[++i];
                // TODO testen ob das arg was bringt -- für OTF ist es momentan wichtig da einige counter seg-faults
                // verursachen (nicht strikt synchrone)
            } else if (arguments[i] == "-nm" || arguments[i] == "--no-metrics") {
                read_metrics = false;
            }
        }

        if (input_file_name == "") {
            std::cerr << "ERROR: No input tracefile name given. See --help | -h for further information." << std::endl;
            return false;
        }

        return true;
    }

   private:
    // TODO tuts nicht -> -o mitten drin, dann nimmts das nächste
    bool checkNext(std::vector<std::string> args, int pos) {
        if (pos + 1 >= args.size()) {
            std::cerr << "ERROR: Missing argument for option '" << args[pos] << "'" << std::endl;
            return false;
        }

        return true;
    }

    int32_t checkNextValue(std::vector<std::string> args, int pos) {
        if (pos + 1 >= args.size()) {
            std::cerr << "ERROR: Missing argument for option '" << args[pos] << "'" << std::endl;
            return -1;
        }
        int32_t value = std::stoi(args[pos + 1]);
        if (value < 0) {
            std::cerr << "ERROR: Invalid argument for option '" << args[pos] << "'" << std::endl;
            return -1;
        }

        return value;
    }
};
#endif  // UTILS_H
