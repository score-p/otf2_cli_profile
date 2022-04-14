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

#ifndef INCLUDE_OTF2XX_DEFINITIONS_METRIC_INSTANCE_HPP
#define INCLUDE_OTF2XX_DEFINITIONS_METRIC_INSTANCE_HPP

#include <otf2xx/fwd.hpp>
#include <otf2xx/reference.hpp>

#include <otf2xx/definition/detail/metric_instance_impl.hpp>
#include <otf2xx/definition/detail/referable_base.hpp>

namespace otf2
{
namespace definition
{

    /**
     * \brief class for representing metric instance definitions
     */
    class metric_instance
    : public detail::referable_base<metric_instance, detail::metric_instance_impl>
    {
        using base = detail::referable_base<metric_instance, detail::metric_instance_impl>;
        using base::base;

    public:
        typedef impl_type::metric_occurence metric_occurence;
        typedef impl_type::metric_scope metric_scope;

        metric_instance(reference_type ref, const otf2::definition::metric_class& metric_class,
                        const otf2::definition::location& recorder,
                        const otf2::definition::location& scope)
        : base(ref, new impl_type(metric_class, recorder, scope))
        {
        }

        metric_instance(reference_type ref, const otf2::definition::metric_class& metric_class,
                        const otf2::definition::location& recorder,
                        const otf2::definition::location_group& scope)
        : base(ref, new impl_type(metric_class, recorder, scope))
        {
        }

        metric_instance(reference_type ref, const otf2::definition::metric_class& metric_class,
                        const otf2::definition::location& recorder,
                        const otf2::definition::system_tree_node& scope)
        : base(ref, new impl_type(metric_class, recorder, scope))
        {
        }

        metric_instance(reference_type ref, const otf2::definition::metric_class& metric_class,
                        const otf2::definition::location& recorder,
                        const otf2::definition::locations_group& scope)
        : base(ref, new impl_type(metric_class, recorder, scope))
        {
        }

        metric_instance() = default;

        /**
         * \brief the occurence of this metric class
         *
         * \see \ref otf2::common::metric_occurence
         */
        metric_occurence occurence() const
        {
            assert(this->is_valid());
            return this->data_->occurence();
        }

        /**
         * \brief returns the referenced metric class
         */
        const otf2::definition::metric_class& metric_class() const
        {
            assert(this->is_valid());
            return this->data_->metric_class();
        }

        /**
         * \brief returns the recorder
         *
         * The recorder is the location, where this metric was recorded
         */
        const otf2::definition::location& recorder() const
        {
            assert(this->is_valid());
            return this->data_->recorder();
        }

        /**
         * \brief returns the scope type of this metric
         *
         * This tells if the values are valid for a location, a location group,
         * a system tree node or a custom group of locations.
         *
         * \see \ref otf2::common::metric_scope
         */
        metric_scope scope() const
        {
            assert(this->is_valid());
            return this->data_->scope();
        }

        /**
         * \brief returns the scope
         *
         * This returns the scope as location definition.
         *
         * \note It might not be a valid definition
         * \see scope()
         */
        const otf2::definition::location& location_scope() const
        {
            assert(this->is_valid());
            return this->data_->location_scope();
        }

        /**
         * \brief returns the scope
         *
         * This returns the scope as location group definition.
         *
         * \note It might not be a valid definition
         * \see scope()
         */
        const otf2::definition::location_group& location_group_scope() const
        {
            assert(this->is_valid());
            return this->data_->location_group_scope();
        }

        /**
         * \brief returns the scope
         *
         * This returns the scope as system tree node definition.
         *
         * \note It might not be a valid definition
         * \see scope()
         */
        const otf2::definition::system_tree_node& system_tree_node_scope() const
        {
            assert(this->is_valid());
            return this->data_->system_tree_node_scope();
        }

        /**
         * \brief returns the scope
         *
         * This returns the scope as group of locations definition.
         *
         * \note It might not be a valid definition
         * \see scope()
         */
        const otf2::definition::locations_group& group_scope() const
        {
            assert(this->is_valid());
            return this->data_->group_scope();
        }
    };
} // namespace definition
} // namespace otf2

#endif // INCLUDE_OTF2XX_DEFINITIONS_METRIC_INSTANCE_HPP
