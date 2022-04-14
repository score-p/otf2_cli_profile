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

#ifndef INCLUDE_OTF2XX_CHRONO_TIMEPOINT_HPP
#define INCLUDE_OTF2XX_CHRONO_TIMEPOINT_HPP

#include <otf2xx/chrono/clock.hpp>

#include <iostream>
#include <limits>

namespace otf2
{
namespace chrono
{

    /**
     * \class time_point
     * \brief typedef of the time point
     *
     * @see \ref otf2::chrono::clock
     */
    typedef clock::time_point time_point;

    inline std::ostream& operator<<(std::ostream& s, time_point tp)
    {
        return s << tp.time_since_epoch().count();
    }

    /**
     * \brief returns latest representable time_point
     * \note I'm not Nostradamus nor I'm predicting an armageddon to be at this time_point.
     * Additionally, as the epoch isn't defined for otf2::chrono::clock, there isn't a reliable way
     * to determine this timepoint anyways.
     */
    inline time_point armageddon()
    {
        return time_point(duration(std::numeric_limits<typename duration::rep>::max()));
    }

    /**
     * \brief returns the first representable time_point
     */
    inline time_point genesis()
    {
        return time_point(duration(std::numeric_limits<typename duration::rep>::min()));
    }
} // namespace chrono
} // namespace otf2

// make it posible to serialize otf2::chrono::time_point
namespace boost
{
namespace serialization
{

    template <class Archive>
    inline void save(Archive& ar, const otf2::chrono::time_point& tp,
                     const unsigned int /* file_version */
    )
    {
        auto duration = tp.time_since_epoch();
        ar& duration;
    }

    template <class Archive>
    inline void load(Archive& ar, otf2::chrono::time_point& tp, const unsigned int /*file_version*/
    )
    {
        otf2::chrono::duration dur;

        ar& dur;

        tp = otf2::chrono::time_point(dur);
    }

    // split non-intrusive serialization function member into separate
    // non intrusive save/load member functions
    template <class Archive>
    inline void serialize(Archive& ar, otf2::chrono::time_point& tp,
                          const unsigned int file_version)
    {
        // god bless ADL rules (ღˇ◡ˇ)~♥
        // This can't and won't be checked until a later instantiation phase.
        // So even though split_free is neither declared nor defined, it should be fine until
        // a user instantiated this template. In order to instantiate this template,
        // a user first need to include boost::serialization. So I will be fine. Probably. Maybe?
        split_free(ar, tp, file_version);
    }

} // namespace serialization
} // namespace boost

#endif // INCLUDE_OTF2XX_CHRONO_TIMEPOINT_HPP
