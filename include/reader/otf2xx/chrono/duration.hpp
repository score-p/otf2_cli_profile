/*
 * This file is part of otf2xx (https://github.com/tud-zih-energy/otf2xx)
 * otf2xx - A wrapper for the Open Trace Format 2 library
 *
 * Copyright (c) 2013-2016, Technische Universität Dresden, Germany
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

#ifndef INCLUDE_OTF2XX_CHRONO_DURATION_HPP
#define INCLUDE_OTF2XX_CHRONO_DURATION_HPP

#include <chrono>
#include <iostream>

using namespace std::literals::chrono_literals;

namespace otf2
{
namespace chrono
{

    /**
     * \brief typedef for duration of lenght picosecond
     */
    typedef std::chrono::duration<int64_t, std::pico> picoseconds;

    /**
     * \brief typedef for duration of lenght nanosecond
     */
    typedef std::chrono::nanoseconds nanoseconds;

    /**
     * \brief typedef for duration of lenght microseconds
     */
    typedef std::chrono::microseconds microseconds;

    /**
     * \brief typedef for duration of lenght milliseconds
     */
    typedef std::chrono::milliseconds milliseconds;

    /**
     * \brief typedef for duration of lenght seconds
     */
    typedef std::chrono::seconds seconds;

    /**
     * \brief typedef for duration of lenght minutes
     */
    typedef std::chrono::minutes minutes;

    /**
     * \brief typedef for duration of lenght hours
     */
    typedef std::chrono::hours hours;

    /**
     * \brief otf2::chrono::duration defaults to nanoseconds or picoseconds
     * This can be configured using the build system.
     */
    typedef OTF2XX_CHRONO_DURATION_TYPE duration;

    /**
     * \brief convert between durations
     *
     * This ain't a simple cast, it also takes different prefixes into account.
     * e.g. duration_cast<seconds>milliseconds(1000) == seconds(1)
     *
     * But this could also mean, that there is information loss.
     * e.g. duration_cast<seconds>milliseconds(1300) == seconds(1)
     *
     * \tparam ToDuration type of target duration, defaults to @see otf2::chrono::duration
     * \tparam FromDuration type of source duration
     * \param[in] dtn duration which is casted
     *
     * \return duration to which is casted
     */
    template <typename ToDuration = otf2::chrono::duration,
              typename FromDuration = otf2::chrono::duration>
    constexpr ToDuration duration_cast(const FromDuration& dtn)
    {
        return std::chrono::duration_cast<ToDuration>(dtn);
    }
} // namespace chrono
} // namespace otf2

namespace std
{
namespace chrono
{

    // HACK: BE AWARE OF DEAD KITTENS

    inline std::ostream& operator<<(std::ostream& s, otf2::chrono::picoseconds dur)
    {
        return s << dur.count() << "ps";
    }

    inline std::ostream& operator<<(std::ostream& s, nanoseconds dur)
    {
        return s << dur.count() << "ns";
    }

    inline std::ostream& operator<<(std::ostream& s, microseconds dur)
    {
        return s << dur.count() << "us";
    }

    inline std::ostream& operator<<(std::ostream& s, milliseconds dur)
    {
        return s << dur.count() << "ms";
    }

    inline std::ostream& operator<<(std::ostream& s, seconds dur)
    {
        return s << dur.count() << "s";
    }

    inline std::ostream& operator<<(std::ostream& s, minutes dur)
    {
        return s << dur.count() << "min";
    }

    inline std::ostream& operator<<(std::ostream& s, hours dur)
    {
        return s << dur.count() << "h";
    }
} // namespace chrono
} // namespace std

// make it posible to serialize otf2::chrono::time_point
namespace boost
{
namespace serialization
{

    template <class Archive>
    inline void save(Archive& ar, const otf2::chrono::duration& dur,
                     __attribute__((unused)) const unsigned int file_version)
    {
        auto count = dur.count();
        ar& count;
    }

    template <class Archive>
    inline void load(Archive& ar, otf2::chrono::duration& dur,
                     __attribute__((unused)) const unsigned int file_version)
    {
        std::int64_t count;

        ar& count;

        dur = otf2::chrono::duration(count);
    }

    // split non-intrusive serialization function member into separate
    // non intrusive save/load member functions
    template <class Archive>
    inline void serialize(Archive& ar, otf2::chrono::duration& dur,
                          __attribute__((unused)) const unsigned int file_version)
    {
        // god bless ADL rules (ღˇ◡ˇ)~♥
        // This can't and won't be checked until a later instantiation phase.
        // So even though split_free is neither declared nor defined, it should be fine until
        // a user instantiated this template. In order to instantiate this template,
        // a user first need to include boost::serialization. So I will be fine. Probably. Maybe?
        split_free(ar, dur, file_version);
    }

} // namespace serialization
} // namespace boost

#endif // INCLUDE_OTF2XX_CHRONO_DURATION_HPP
