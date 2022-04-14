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

#ifndef INCLUDE_OTF2XX_DEFINITIONS_METRIC_MEMBER_HPP
#define INCLUDE_OTF2XX_DEFINITIONS_METRIC_MEMBER_HPP

#include <otf2xx/common.hpp>
#include <otf2xx/fwd.hpp>
#include <otf2xx/reference.hpp>

#include <otf2xx/definition/string.hpp>

#include <otf2xx/definition/detail/metric_member_impl.hpp>
#include <otf2xx/definition/detail/referable_base.hpp>

namespace otf2
{
namespace definition
{

    /**
     * \brief class representing a metric member definition
     *
     * A metric member defines one "channel" of an metric. It defines the value
     * type, name etc.
     */
    class metric_member : public detail::referable_base<metric_member, detail::metric_member_impl>
    {
        using base = detail::referable_base<metric_member, detail::metric_member_impl>;
        using base::base;

    public:
        typedef impl_type::metric_type metric_type;
        typedef impl_type::metric_mode metric_mode;
        typedef impl_type::value_type_type value_type_type;
        typedef impl_type::value_base_type value_base_type;
        typedef impl_type::value_exponent_type value_exponent_type;

        metric_member(reference_type ref, const otf2::definition::string& name,
                      const otf2::definition::string& description, metric_type type,
                      metric_mode mode, value_type_type value_type, value_base_type value_base,
                      value_exponent_type value_exponent,
                      const otf2::definition::string& value_unit)
        : base(ref, new impl_type(name, description, type, mode, value_type, value_base,
                                  value_exponent, value_unit))
        {
        }

        metric_member() = default;

        /**
         * \brief returns the name of the metric member
         */
        const otf2::definition::string& name() const
        {
            assert(this->is_valid());
            return data_->name();
        }

        /**
         * \brief returns the description of the metric member
         */
        const otf2::definition::string& description() const
        {
            assert(this->is_valid());
            return data_->name();
        }

        /**
         * \brief returns the type of the metric member
         *
         * \see \ref otf2::common::metric_type
         */
        metric_type type() const
        {
            assert(this->is_valid());
            return data_->type();
        }

        /**
         * \brief returns the mode of the metric member
         *
         * \see \ref otf2::common::metric_mode
         */
        metric_mode mode() const
        {
            assert(this->is_valid());
            return data_->mode();
        }

        otf2::common::metric_value_property property() const
        {
            assert(this->is_valid());
            return data_->property();
        }

        /**
         * \brief returns the type of the value of the metric member
         *
         * \see \ref otf2::common::type
         */
        value_type_type value_type() const
        {
            assert(this->is_valid());
            return data_->value_type();
        }

        /**
         * \brief returns the base to scale the values with
         *
         * Either decimal(10) or binary(2)
         * \see \ref otf2::common::metric_base
         */
        value_base_type value_base() const
        {
            assert(this->is_valid());
            return data_->value_base();
        }

        /**
         * \brief returns the exponent to scale the values with
         */
        value_exponent_type value_exponent() const
        {
            assert(this->is_valid());
            return data_->value_exponent();
        }

        /**
         * \brief returns the unit of the values without prefixes
         */
        const otf2::definition::string& value_unit() const
        {
            assert(this->is_valid());
            return data_->value_unit();
        }
    };
} // namespace definition
} // namespace otf2

#endif // INCLUDE_OTF2XX_DEFINITIONS_METRIC_MEMBER_HPP
