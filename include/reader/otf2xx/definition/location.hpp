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

#ifndef INCLUDE_OTF2XX_DEFINITIONS_LOCATION_HPP
#define INCLUDE_OTF2XX_DEFINITIONS_LOCATION_HPP

#include <otf2xx/common.hpp>
#include <otf2xx/fwd.hpp>
#include <otf2xx/reference.hpp>

#include <otf2xx/definition/location_group.hpp>
#include <otf2xx/definition/string.hpp>

#include <otf2xx/definition/detail/location_impl.hpp>
#include <otf2xx/definition/detail/referable_base.hpp>

namespace otf2
{
namespace definition
{

    /**
     * \brief class for representing location definitions
     */
    class location : public detail::referable_base<location, detail::location_impl>
    {
        using base = detail::referable_base<location, detail::location_impl>;
        using base::base;

    public:
        typedef impl_type::location_type location_type;

        location(reference_type ref, const otf2::definition::string& name,
                 const otf2::definition::location_group& lg, location_type type,
                 std::uint64_t events = 0)
        : base(ref, new impl_type(name, lg, type, events))
        {
        }

        location(const otf2::definition::location& other, std::uint64_t events)
        : base(other.ref(),
               new impl_type(other.name(), other.location_group(), other.type(), events))
        {
        }

        location() = default;

        /**
         * \brief returns the name of the location definion as a string definition
         *
         * \returns a \ref string definiton containing the name
         *
         */
        const otf2::definition::string& name() const
        {
            assert(this->is_valid());
            return data_->name();
        }

        /**
         * \brief set the name of the location definion as a string definition
         *
         */
        void name(const otf2::definition::string& new_name)
        {
            assert(this->is_valid());
            return data_->name(new_name);
        }

        /**
         * \brief returns the location group of the location definion
         *
         * \returns a \ref location_group definiton
         *
         */
        const otf2::definition::location_group& location_group() const
        {
            assert(this->is_valid());
            return data_->location_group();
        }

        /**
         * \brief returns the type of the location defintion
         * \see otf2::common::location_type
         */
        location_type type() const
        {
            assert(this->is_valid());
            return data_->type();
        }

        /**
         * \brief returns the number of events for this location
         * Use with care. If you read in this location and write it to another trace, you have to
         * make sure,
         * that you have also written all the events. Otherwise you should make a copy, like
         * haec_sim::module::sink.
         */
        std::uint64_t num_events() const
        {
            assert(this->is_valid());
            return data_->num_events();
        }

    private:
        /**
         * \internal
         * \brief notifies the location, that there was an event written to it.
         * Increases the number of events
         *
         * Should not called directly
         */
        void event_written()
        {
            assert(this->is_valid());
            data_->event_written();
        }

        friend class writer::local;
    };

    inline std::ostream& operator<<(std::ostream& s, const location& loc)
    {
        return s << "location #" << loc.ref() << " (\"" << loc.name() << "\")";
    }
} // namespace definition
} // namespace otf2

#endif // INCLUDE_OTF2XX_DEFINITIONS_LOCATION_HPP
