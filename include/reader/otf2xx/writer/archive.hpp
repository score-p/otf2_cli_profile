/*
 * This file is part of otf2xx (https://github.com/tud-zih-energy/otf2xx)
 * otf2xx - A wrapper for the Open Trace Format 2 library
 *
 * Copyright (c) 2013-2020, Technische Universität Dresden, Germany
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

#ifndef INCLUDE_OTF2XX_ARCHIVE_HPP
#define INCLUDE_OTF2XX_ARCHIVE_HPP

#include <otf2/OTF2_Archive.h>
#include <otf2/OTF2_Callbacks.h>

#include <otf2xx/exception.hpp>
#include <otf2xx/fwd.hpp>

#include <otf2xx/writer/global.hpp>
#include <otf2xx/writer/local.hpp>

#ifdef OTF2XX_HAS_MPI
#include <mpi.h>
#include <otf2/OTF2_MPI_Collectives.h>
#endif

#include <functional>
#include <string>
#include <vector>

namespace otf2
{
namespace writer
{
    template <typename Registry>
    class Archive
    {
    public:
#ifdef OTF2XX_HAS_MPI
        Archive(const std::string& path, const std::string& name, MPI_Comm comm,
                OTF2_FileMode_enum mode = OTF2_FILEMODE_WRITE,
                std::size_t event_chunk_size = 1024 * 1024,
                std::size_t definition_chunk_size = 4 * 1024 * 1024,
                OTF2_FileSubstrate_enum file_substrate = OTF2_SUBSTRATE_POSIX,
                OTF2_Compression_enum compression = OTF2_COMPRESSION_NONE)
        : ar(OTF2_Archive_Open(path.c_str(), name.c_str(), mode, event_chunk_size,
                               definition_chunk_size, file_substrate, compression)),
          serial(false)
        {
            if (ar == nullptr)
                make_exception("Couldn't open the archive '", name, "'");

            set_flush_callbacks();

            check(OTF2_MPI_Archive_SetCollectiveCallbacks(ar, comm, MPI_COMM_NULL),
                  "Couldn't set collective callbacks");

            int rank;
            MPI_Comm_rank(comm, &rank);
            is_master_ = rank == 0;

            check(OTF2_Archive_OpenDefFiles(ar), "Couldn't open definition files");
            check(OTF2_Archive_OpenEvtFiles(ar), "Couldn't open event files");
        }
#endif
        Archive(const std::string& path, const std::string& name,
                OTF2_FileMode_enum mode = OTF2_FILEMODE_WRITE,
                std::size_t event_chunk_size = 1024 * 1024,
                std::size_t definition_chunk_size = 4 * 1024 * 1024,
                OTF2_FileSubstrate_enum file_substrate = OTF2_SUBSTRATE_POSIX,
                OTF2_Compression_enum compression = OTF2_COMPRESSION_NONE)
        : ar(OTF2_Archive_Open(path.c_str(), name.c_str(), mode, event_chunk_size,
                               definition_chunk_size, file_substrate, compression)),
          serial(true)
        {
            if (ar == nullptr)
                make_exception("Couldn't open the archive '", name, "'");

            set_flush_callbacks();
            OTF2_Archive_SetSerialCollectiveCallbacks(ar);

            check(OTF2_Archive_OpenDefFiles(ar), "Couldn't open definition files");
            check(OTF2_Archive_OpenEvtFiles(ar), "Couldn't open event files");
        }

        ~Archive()
        {
            // close all local writer
            local_writers_.clear();
            OTF2_Archive_CloseEvtFiles(ar);
            OTF2_Archive_CloseDefFiles(ar);

            // close global writer
            global_writer_.reset(0);

            // close archive
            OTF2_Archive_Close(ar);
        }

        /* Takes ownership !*/
        explicit Archive(OTF2_Archive* ar) : ar(ar), serial(true)
        {
        }

        OTF2_Archive* get()
        {
            return ar;
        }

    private:
        void set_flush_callbacks()
        {
            // setting post_flush to nullptr omits writting of the buffer flush event
            flush_callbacks_.otf2_post_flush = nullptr;
            flush_callbacks_.otf2_pre_flush = detail::pre_flush<Registry>;
            OTF2_Archive_SetFlushCallbacks(ar, &flush_callbacks_, static_cast<void*>(this));

            pre_flush_callback_ =
                [](__attribute__((unused)) otf2::reference<otf2::definition::location> ref,
                   __attribute__((unused)) bool final) { return OTF2_FLUSH; };
        }

    public:
        bool is_slave() const
        {
            return !is_master();
        }

        bool is_master() const
        {
#ifdef OTF2XX_HAS_MPI
            return serial || is_master_;
#else
            assert(serial);
            return true;
#endif
        }

    public:
        void set_creator(const std::string& creator)
        {
            check(OTF2_Archive_SetCreator(ar, creator.c_str()), "Couldn't set creator");
        }

        std::string get_creator() const
        {
            char* creator;

            check(OTF2_Archive_GetCreator(ar, &creator), "Couldn't get creator");

            std::string result(creator);

            free(creator);

            return result;
        }

        std::uint64_t get_events_chunk_size() const
        {
            std::uint64_t result, tmp;

            check(OTF2_Archive_GetChunkSize(ar, &result, &tmp), "Couldn't get chunk size");

            return result;
        }

        std::uint64_t get_definitions_chunk_size() const
        {
            std::uint64_t result, tmp;

            check(OTF2_Archive_GetChunkSize(ar, &tmp, &result), "Couldn't get chunk size");

            return result;
        }

        OTF2_Compression_enum get_compression() const
        {
            OTF2_Compression result;

            check(OTF2_Archive_GetCompression(ar, &result), "Couldn't get compression");

            return static_cast<OTF2_Compression_enum>(result);
        }

        std::string get_description() const
        {
            char* tmp;

            check(OTF2_Archive_GetDescription(ar, &tmp), "Couldn't get description");

            std::string result(tmp);

            free(tmp);

            return result;
        }

        void set_description(const std::string& desc)
        {
            check(OTF2_Archive_SetDescription(ar, desc.c_str()), "Couldn't set description");
        }

        OTF2_FileSubstrate_enum get_file_substrate() const
        {
            OTF2_FileSubstrate result;

            check(OTF2_Archive_GetFileSubstrate(ar, &result), "Couldn't get file substrate");

            return static_cast<OTF2_FileSubstrate_enum>(result);
        }

        std::string get_machine_name() const
        {
            char* tmp;

            check(OTF2_Archive_GetMachineName(ar, &tmp), "Couldn't get machine name");

            std::string result(tmp);

            free(tmp);

            return result;
        }

        void set_machine_name(const std::string& name)
        {
            check(OTF2_Archive_SetMachineName(ar, name.c_str()), "Couldn't set machine name");
        }

        std::uint64_t num_global_definitions() const
        {
            uint64_t num;

            check(OTF2_Archive_GetNumberOfGlobalDefinitions(ar, &num),
                  "Couldn't get number of global definitions");

            return num;
        }

        std::uint64_t num_locations() const
        {
            uint64_t num;

            check(OTF2_Archive_GetNumberOfLocations(ar, &num), "Couldn't get number of locations");

            return num;
        }

        std::uint32_t num_snapshots() const
        {
            uint32_t num;

            check(OTF2_Archive_GetNumberOfSnapshots(ar, &num), "Couldn't get number of snapshots");

            return num;
        }

        void set_num_snapshots(std::uint32_t num)
        {
            check(OTF2_Archive_SetNumberOfSnapshots(ar, num), "Couldn't set number of snapshots");
        }

        std::uint32_t num_thumbnails() const
        {
            uint32_t num;

            check(OTF2_Archive_GetNumberOfThumbnails(ar, &num),
                  "Couldn't get number of thumbnails");

            return num;
        }

        std::uint64_t get_trace_id() const
        {
            uint64_t id;

            check(OTF2_Archive_GetTraceId(ar, &id), "Couldn't get trace id");

            return id;
        }

    public:
        std::string get_property(const std::string& name) const
        {
            char* tmp;

            check(OTF2_Archive_GetProperty(ar, name.c_str(), &tmp), "Couldn't get property");

            std::string result(tmp);

            free(tmp);

            return result;
        }

        std::vector<std::string> get_property_names() const
        {
            char** tmp;

            std::uint32_t size;

            check(OTF2_Archive_GetPropertyNames(ar, &size, &tmp), "Couldn't get property names");

            std::vector<std::string> result(size);

            for (uint32_t i = 0; i < size; ++i)
                result.push_back(tmp[i]);

            free(tmp);

            return result;
        }

        void set_property(const std::string& name, bool value, bool overwrite = false)
        {
            check(OTF2_Archive_SetBoolProperty(ar, name.c_str(), value, overwrite),
                  "Couldn't set bool property");
        }

        void set_property(const std::string& name, const std::string& value, bool overwrite = false)
        {
            check(OTF2_Archive_SetProperty(ar, name.c_str(), value.c_str(), overwrite),
                  "Couldn't set property");
        }

        using post_flush_func =
            std::function<otf2::chrono::time_point(otf2::reference<otf2::definition::location>)>;
        using pre_flush_func =
            std::function<OTF2_FlushType(otf2::reference<otf2::definition::location>, bool)>;

        void set_pre_flush_callback(pre_flush_func f)
        {
            pre_flush_callback_ = f;
        }

        void set_post_flush_callback(post_flush_func f)
        {
            flush_callbacks_.otf2_post_flush = detail::post_flush<Registry>;
            post_flush_callback_ = f;
        }

        Registry& registry()
        {
            return get_global_writer().registry();
        }

    public:
        writer::local& operator()(const otf2::definition::location& loc)
        {
            return get_local_writer(loc);
        }

        writer::global<Registry>& operator()()
        {
            return get_global_writer();
        }

        friend OTF2_FlushType detail::pre_flush<Registry>(void*, OTF2_FileType, OTF2_LocationRef,
                                                          void*, bool);
        friend OTF2_TimeStamp detail::post_flush<Registry>(void*, OTF2_FileType, OTF2_LocationRef);

        global<Registry>& get_global_writer()
        {
            if (is_master())
            {
                if (!global_writer_)
                    global_writer_.reset(new global<Registry>(OTF2_Archive_GetGlobalDefWriter(ar),
                                                              OTF2_Archive_GetMarkerWriter(ar)));
            }
            else
            {
                make_exception(
                    "Archive is in slave mode, so there can't be any global definition writer");
            }

            assert(global_writer_);

            return *global_writer_;
        }

        local& get_local_writer(const otf2::definition::location& loc)
        {
            auto it = local_writers_.find(loc.ref());
            if (it == local_writers_.end())
            {
                auto res = local_writers_.emplace(
                    std::piecewise_construct, std::make_tuple(loc.ref()), std::make_tuple(ar, loc));
                it = res.first;
            }

            return it->second;
        }

        /** Closes the local writer for the given location
         *
         * Attention:
         *  - All references to the writer are invalid after this operation
         *  - Calling get_local_writer() for the same location after this is undefined behavior
         */
        void close_local_writer(const otf2::definition::location& loc)
        {
            auto it = local_writers_.find(loc.ref());

            if (it == local_writers_.end())
            {
                make_exception("Cannot close not existing writer for location #", loc.ref());
            }

            local_writers_.erase(it);
        }

    private:
        OTF2_Archive* ar;
        bool serial;
#ifdef OTF2XX_HAS_MPI
        bool is_master_;
#endif
        OTF2_FlushCallbacks flush_callbacks_;
        post_flush_func post_flush_callback_;
        pre_flush_func pre_flush_callback_;

        std::unique_ptr<global<Registry>> global_writer_;
        std::map<otf2::reference<otf2::definition::location>::ref_type, local> local_writers_;
    };

    template <typename Anything, typename Registry>
    inline std::enable_if_t<
        otf2::traits::is_definition<std::remove_cv_t<std::remove_reference_t<Anything>>>::value,
        global<Registry>&>
    operator<<(Archive<Registry>& ar, Anything&& any)
    {
        return ar() << std::forward<Anything>(any);
    }

    template <typename Registry>
    inline global<Registry>& operator<<(Archive<Registry>& ar, const otf2::event::marker& evt)
    {
        return ar() << evt;
    }

    template <typename Definition, typename Registry>
    inline global<Registry>& operator<<(Archive<Registry>& ar,
                                        const otf2::definition::container<Definition>& c)
    {

        global<Registry>& wrt = ar();

        for (auto def : c)
            wrt << def;

        return wrt;
    }

    namespace detail
    {

        template <typename Registry>
        inline OTF2_FlushType
        pre_flush(void* userData, __attribute__((unused)) OTF2_FileType fileType,
                  OTF2_LocationRef location, __attribute__((unused)) void* callerData, bool final)
        {
            Archive<Registry>* ar = static_cast<Archive<Registry>*>(userData);

            return ar->pre_flush_callback_(location, final);
        }

        template <typename Registry>
        inline OTF2_TimeStamp post_flush(void* userData,
                                         __attribute__((unused)) OTF2_FileType fileType,
                                         OTF2_LocationRef location)
        {
            Archive<Registry>* ar = static_cast<Archive<Registry>*>(userData);

            static_assert(otf2::chrono::clock::period::num == 1,
                          "Don't mess around with the chrono stuff!");

            otf2::chrono::convert convert;

            return convert(ar->post_flush_callback_(location)).count();
        }
    } // namespace detail
} // namespace writer
} // namespace otf2

#endif // INCLUDE_OTF2XX_ARCHIVE_HPP
