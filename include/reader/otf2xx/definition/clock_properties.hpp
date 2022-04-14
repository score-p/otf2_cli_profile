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

#ifndef INCLUDE_OTF2XX_DEFINITIONS_CLOCK_PROPERTIES_HPP
#define INCLUDE_OTF2XX_DEFINITIONS_CLOCK_PROPERTIES_HPP

#include <otf2xx/chrono/chrono.hpp>
#include <otf2xx/exception.hpp>
#include <otf2xx/fwd.hpp>
#include <otf2xx/reference.hpp>

namespace otf2
{
namespace definition
{

    /**
     * \brief class for representing a clock properties definition
     */
    class clock_properties
    {
    public:
        clock_properties(otf2::chrono::time_point start_time, otf2::chrono::duration length)
        : clock_properties(otf2::chrono::ticks(otf2::chrono::duration::period::den),
                           otf2::chrono::ticks(start_time.time_since_epoch().count()),
                           otf2::chrono::ticks(length.count()))
        {
        }

        clock_properties(otf2::chrono::time_point start_time, otf2::chrono::time_point end_time)
        : clock_properties(start_time, end_time - start_time)
        {
            if (end_time < start_time)
            {
                make_exception("start_time must be before end_time");
            }
        }

        clock_properties(otf2::chrono::ticks ticks_per_second, otf2::chrono::ticks start_time,
                         otf2::chrono::ticks length, otf2::chrono::ticks realtime_timestamp)
        : ticks_per_second_(ticks_per_second), start_time_(start_time), length_(length),
          realtime_timestamp_(realtime_timestamp)
        {
        }

        clock_properties(otf2::chrono::ticks ticks_per_second, otf2::chrono::ticks start_time,
                         otf2::chrono::ticks length)
        : ticks_per_second_(ticks_per_second), start_time_(start_time), length_(length),
          realtime_timestamp_(OTF2_UNDEFINED_TIMESTAMP)
        {
        }

        clock_properties()
        : ticks_per_second_(otf2::chrono::duration::period::den), start_time_(0), length_(-1),
          realtime_timestamp_(0)
        {
        }

        /**
         * \brief returns the number of ticks per second
         *
         * \return ticks per second
         */
        otf2::chrono::ticks ticks_per_second() const
        {
            return ticks_per_second_;
        }

        /**
         * \brief returns the global start offset
         *
         * This is the number of ticks since the epoch for the first timestamp
         * which is found in the trace
         *
         * \return global offset
         */
        otf2::chrono::ticks start_time() const
        {
            return start_time_;
        }

        /**
         * \brief returns the length of the trace file
         *
         * This is the number of ticks between the first and the last timestamp
         * found in the trace.
         *
         * \return length of the trace
         */
        otf2::chrono::ticks length() const
        {
            return length_;
        }

        /**
         * \brief returns the realtime timestamp of the start of the trace file
         *
         * A realtime timestamp of the `globalOffset` timestamp in nanoseconds since
         * 1970-01-01T00:00 UTC.
         *
         * \return start of the trace
         */
        otf2::chrono::ticks realtime_timestamp() const
        {
            return realtime_timestamp_;
        }

    private:
        otf2::chrono::ticks ticks_per_second_;
        otf2::chrono::ticks start_time_;
        otf2::chrono::ticks length_;
        otf2::chrono::ticks realtime_timestamp_;
    };
} // namespace definition
} // namespace otf2

#endif // INCLUDE_OTF2XX_DEFINITIONS_CLOCK_PROPERTIES_HPP
