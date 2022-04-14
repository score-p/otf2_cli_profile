/*
 * This file is part of otf2xx (https://github.com/tud-zih-energy/otf2xx)
 * otf2xx - A wrapper for the Open Trace Format 2 library
 *
 * Copyright (c) 2013-2019, Technische Universität Dresden, Germany
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

#ifndef INCLUDE_OTF2XX_DEFINITIONS_METRIC_CLASS_RECORDER_HPP
#define INCLUDE_OTF2XX_DEFINITIONS_METRIC_CLASS_RECORDER_HPP

#include <otf2xx/common.hpp>
#include <otf2xx/fwd.hpp>
#include <otf2xx/reference.hpp>

#include <otf2xx/definition/location.hpp>
#include <otf2xx/definition/metric_class.hpp>
#include <otf2xx/definition/metric_instance.hpp>

#include <otf2xx/definition/detail/base.hpp>
#include <otf2xx/definition/detail/metric_class_recorder_impl.hpp>

namespace otf2
{
namespace definition
{

    /**
     * \brief class for representing metric class recorder definitions
     */
    class metric_class_recorder : public detail::base<detail::metric_class_recorder_impl>
    {
        using base = detail::base<detail::metric_class_recorder_impl>;
        using base::base;

    public:
        metric_class_recorder(const std::variant<otf2::definition::metric_class,
                                                 otf2::definition::metric_instance>& metric,
                              const otf2::definition::location& recorder)
        : base(new impl_type(metric, recorder))
        {
        }

        metric_class_recorder() = default;

        /**
         * \brief returns the associated metric
         * \returns a variant of metric_class or metric_instance
         */
        const std::variant<otf2::definition::metric_class, otf2::definition::metric_instance>&
        metric() const
        {
            assert(this->is_valid());
            return data_->metric();
        }

        /**
         * \brief returns the domain
         * \returns otf2::common::metric_class_recorder
         *
         */
        const otf2::definition::location& recorder() const
        {
            assert(this->is_valid());
            return data_->recorder();
        }
    };

} // namespace definition

} // namespace otf2

#endif // INCLUDE_OTF2XX_DEFINITIONS_METRIC_CLASS_RECORDER_HPP
