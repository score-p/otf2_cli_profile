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

#ifndef INCLUDE_OTF2XX_CHRONO_CONVERT_HPP
#define INCLUDE_OTF2XX_CHRONO_CONVERT_HPP

#include <otf2xx/chrono/clock.hpp>
#include <otf2xx/chrono/ticks.hpp>
#include <otf2xx/chrono/time_point.hpp>

#include <otf2xx/definition/clock_properties.hpp>
#include <otf2xx/exception.hpp>

#include <cassert>
#include <cmath>
#include <limits>

namespace otf2
{
namespace chrono
{

    /**
     * \brief class to convert between ticks and time points
     *
     * This class can convert between ticks and time points.
     * For this, it needs the number of ticks per second.
     *
     * \note The time epoch is assumed to be equal between the time point and
     *       time point represented with the number ticks given.
     */
    class convert
    {
        static_assert(clock::period::num == 1, "Don't mess around with chrono!");

    public:
        explicit convert(otf2::chrono::ticks ticks_per_second =
                             otf2::chrono::ticks(otf2::chrono::clock::period::den),
                         otf2::chrono::ticks offset = otf2::chrono::ticks(0))
        : offset_(offset.count()),
          factor_(static_cast<double>(clock::period::den) / ticks_per_second.count()),
          inverse_factor_(ticks_per_second.count() / static_cast<double>(clock::period::den))
        {
        }

        explicit convert(const otf2::definition::clock_properties& cp)
        : convert(cp.ticks_per_second(), cp.start_time())
        {
            // WARNING: Be careful, when changing clock::period::den.
            // You will have to think about every calculations twice, as there
            // might be narrowing and rounding anywhere.
            // We also assumed here, that we have nanoseconds or picoseconds resolution and the
            // input resolution is about nanoseconds or a few hundred
            // picoseconds.
            // These assumptions have to be double checked!

            // The real question here is, whether we can represent the largest timestamp of the
            // trace with a time_point in our clock
            if (cp.length().count() >
                static_cast<uint64_t>(static_cast<double>(std::numeric_limits<int64_t>::max())) /
                    factor_)
            {
                otf2::make_exception("This traces' timepoints cannot be represented in the "
                                     "selected otf2::chrono::time_point. Recompile with "
                                     "nanoseconds for OTF2XX_CHRONO_DURATION_TYPE.");
            }

            // Note: Due to rounding errors and depending on the OTF2XX_CHRONO_DURATION_TYPE, some
            // OTF2_TimeStamp values might be mapped to the same otf2::chrono::time_point.
        }

        convert(const convert&) = default;
        convert& operator=(const convert&) = default;

        convert(convert&&) = default;
        convert& operator=(convert&&) = default;

        /**
         * \brief converts from ticks to time point
         *
         * \param[in] ticks since epoch
         * \return time_point with a duration equal to the passed time
         *         since the epoch.
         */
        otf2::chrono::time_point operator()(otf2::chrono::ticks ticks) const
        {
            // VTTI please remember that those two inputs are uint64_t and then look at the next
            // line
            assert(ticks.count() >= offset_);

            auto tp = ticks.count() - offset_;

            assert(tp <=
                   static_cast<uint64_t>(static_cast<double>(std::numeric_limits<int64_t>::max())) /
                       factor_);

            return time_point(otf2::chrono::duration(static_cast<int64_t>(tp * factor_)));
        }

        /**
         * \brief converts from time points to ticks
         *
         * \param[in] t a time point
         * \return number ticks equal to passed time of the duration of the time
         *         point
         */
        otf2::chrono::ticks operator()(time_point t) const
        {
            auto tp = static_cast<uint64_t>(t.time_since_epoch().count());

            assert(tp <
                   static_cast<uint64_t>(static_cast<double>(std::numeric_limits<int64_t>::max())) /
                       inverse_factor_);

            // Note 1: Using ceil here has its origins in the observation that casting from double
            // to int in the other conversion above leads to an implicit round down. Thus, we
            // counter that here with an explicit round up.
            // Note 2: Using an multiplication with the inverse yields a better performance. Though,
            // there might be cases, where a different sequence of multiplication or division
            // operations would result in lower rounding errors.
            auto tpi = static_cast<uint64_t>(std::ceil(tp * inverse_factor_));

            assert(tpi <= std::numeric_limits<std::uint64_t>::max() - offset_);

            return ticks(tpi + offset_);
        }

    private:
        uint64_t offset_;

        double factor_;
        double inverse_factor_;
    };

    /**
     * \brief converts from std::chrono::timepoint to otf2::chrono::time_point
     *
     * \param[in] tp the std::chrono time point
     * \return the same time point as otf2::chrono::time_point
     */
    template <typename Clock, typename Duration>
    otf2::chrono::time_point convert_time_point(std::chrono::time_point<Clock, Duration> tp)
    {
        return otf2::chrono::time_point(
            std::chrono::duration_cast<otf2::chrono::clock::duration>(tp.time_since_epoch()));
    }
} // namespace chrono
} // namespace otf2

#endif // INCLUDE_OTF2XX_CHRONO_CONVERT_HPP
