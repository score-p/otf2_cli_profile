/*
 * This file is part of otf2xx (https://github.com/tud-zih-energy/otf2xx)
 * otf2xx - A wrapper for the Open Trace Format 2 library
 *
 * Copyright (c) 2013-2018, Technische Universität Dresden, Germany
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

#ifndef INCLUDE_OTF2XX_EVENT_METRIC_HPP
#define INCLUDE_OTF2XX_EVENT_METRIC_HPP

#include <otf2xx/event/detail/metric_values.hpp>
#include <otf2xx/event/detail/value_proxy.hpp>

#include <otf2xx/definition/fwd.hpp>
#include <otf2xx/definition/metric_member.hpp>

#include <otf2xx/event/base.hpp>

#include <otf2xx/chrono/chrono.hpp>

#include <otf2xx/exception.hpp>

#include <otf2xx/definition/detail/weak_ref.hpp>
#include <otf2xx/writer/fwd.hpp>

#include <variant>

namespace otf2
{
namespace event
{
    class metric : public base<metric>
    {
    public:
        using values = detail::metric_values;
        using value_proxy = detail::value_proxy;
        using const_value_proxy = detail::const_value_proxy;

        // construct with values
        metric(otf2::chrono::time_point timestamp,
               const std::variant<otf2::definition::weak_ref<otf2::definition::metric_class>,
                                  otf2::definition::weak_ref<otf2::definition::metric_instance>>&
                   metric_ref,
               values&& values)
        : base<metric>(timestamp), values_(std::move(values))
        {
            std::visit([this](auto&& ref) { metric_ = ref; }, metric_ref);
        }

        // construct with values
        metric(OTF2_AttributeList* al, otf2::chrono::time_point timestamp,
               const std::variant<otf2::definition::weak_ref<otf2::definition::metric_class>,
                                  otf2::definition::weak_ref<otf2::definition::metric_instance>>&
                   metric_ref,
               values&& values)
        : base<metric>(al, timestamp), values_(std::move(values))
        {
            std::visit([this](auto&& ref) { metric_ = ref; }, metric_ref);
        }

        // construct without values, but reserve memory for them
        metric(otf2::chrono::time_point timestamp, const otf2::definition::metric_class& metric_c)
        : base<metric>(timestamp), metric_(metric_c), values_(metric_c)
        {
        }

        metric(otf2::chrono::time_point timestamp,
               const otf2::definition::metric_instance& metric_i)
        : base<metric>(timestamp), metric_(metric_i), values_(metric_i.metric_class())
        {
        }

        metric(OTF2_AttributeList* al, otf2::chrono::time_point timestamp,
               const otf2::definition::metric_class& metric_c)
        : base<metric>(al, timestamp), metric_(metric_c), values_(metric_c)
        {
        }

        metric(OTF2_AttributeList* al, otf2::chrono::time_point timestamp,
               const otf2::definition::metric_instance& metric_i)
        : base<metric>(al, timestamp), metric_(metric_i), values_(metric_i.metric_class())
        {
        }

        // copy constructor with new timestamp
        metric(const otf2::event::metric& other, otf2::chrono::time_point timestamp)
        : base<metric>(other, timestamp), metric_(other.metric_), values_(other.raw_values())
        {
        }

        /// construct without referencing a metric class or a metric instance
        metric(OTF2_AttributeList* al, otf2::chrono::time_point timestamp, values&& values)
        : base<metric>(al, timestamp), metric_(), values_(std::move(values))
        {
        }

        explicit metric(const otf2::definition::metric_class& metric_c)
        : base<metric>(otf2::chrono::genesis()), metric_(metric_c), values_(metric_c)
        {
        }

        explicit metric(const otf2::definition::metric_instance& metric_i)
        : base<metric>(otf2::chrono::genesis()), metric_(metric_i), values_(metric_i.metric_class())
        {
        }

        values& raw_values()
        {
            return values_;
        }

        const values& raw_values() const
        {
            return values_;
        }

        value_proxy get_value_at(std::size_t index)
        {
            auto metric_class = resolve_weak_ref_to_metric_class();

            assert(static_cast<bool>(metric_class));

            return value_proxy(values_[index], (*metric_class)[index]);
        }

        const_value_proxy get_value_at(std::size_t index) const
        {
            auto metric_class = resolve_weak_ref_to_metric_class();

            assert(static_cast<bool>(metric_class));

            return const_value_proxy(values_[index], (*metric_class)[index]);
        }

        detail::typed_value_proxy operator[](std::size_t index)
        {
            return raw_values()[index];
        }

        detail::const_typed_value_proxy operator[](std::size_t index) const
        {
            return raw_values()[index];
        }

        value_proxy get_value_for(const otf2::definition::metric_member& member)
        {
            auto metric_class = resolve_weak_ref_to_metric_class();

            // TODO: maybe check if metric_class is undefined? This might happen
            // if the event was constructed without a reference to a metric
            // class or metric instance.

            assert(static_cast<bool>(metric_class));

            // Look up the index of a member inside of metric class and use it to
            // construct a value_proxy from the right OTF2_Type and OTF2_MetricValue.

            auto it = std::find(metric_class->begin(), metric_class->end(), member);
            if (it == metric_class->end())
            {
                throw std::out_of_range("Failed to look up metric_member inside metric_class");
            }

            auto index = std::distance(metric_class->begin(), it);
            return value_proxy{ values_[index], member };
        }

        value_proxy operator[](const otf2::definition::metric_member& member)
        {
            return get_value_for(member);
        }

        auto metric_def() const
        {
            return otf2::definition::variants_from_weak(metric_);
        }

        otf2::definition::metric_class resolve_metric_class() const
        {
            return otf2::definition::metric_class{ resolve_weak_ref_to_metric_class() };
        }

        friend class otf2::writer::local;

        template <bool IsMutable>
        class iterator
        {
        public:
            iterator(otf2::definition::metric_class::iterator class_it,
                     detail::metric_values::base_iterator<IsMutable> value_it)
            : class_it_(class_it), value_it_(value_it)
            {
            }

            detail::base_value_proxy<IsMutable> operator*()
            {
                return { *value_it_, *class_it_ };
            }

            iterator operator++(int)
            {
                iterator tmp(*this);
                ++(*this);
                return tmp;
            }

            iterator& operator++()
            {
                ++class_it_;
                ++value_it_;
                return *this;
            }

            bool operator!=(const iterator& other)
            {
                return value_it_ != other.value_it_;
            }

        private:
            otf2::definition::metric_class::iterator class_it_;
            detail::metric_values::base_iterator<IsMutable> value_it_;
        };

        auto begin()
        {
            return iterator<true>(resolve_metric_class().begin(), values_.begin());
        }

        auto end()
        {
            return iterator<true>(resolve_metric_class().end(), values_.end());
        }

        auto begin() const
        {
            return iterator<false>(resolve_metric_class().begin(), values_.begin());
        }

        auto end() const
        {
            return iterator<false>(resolve_metric_class().end(), values_.end());
        }

    private:
        template <typename Definition>
        using weak_ref = otf2::definition::weak_ref<Definition>;

        weak_ref<otf2::definition::metric_class> resolve_weak_ref_to_metric_class() const
        {
            if (std::holds_alternative<weak_ref<otf2::definition::metric_instance>>(metric_))
            {
                return otf2::definition::make_weak_ref(
                    std::get<weak_ref<otf2::definition::metric_instance>>(metric_)->metric_class());
            }
            else
            {
                return std::get<weak_ref<otf2::definition::metric_class>>(metric_);
            }
        }

    private:
        std::variant<weak_ref<otf2::definition::metric_class>,
                     weak_ref<otf2::definition::metric_instance>>
            metric_;
        values values_;
    };
} // namespace event
} // namespace otf2

#endif // INCLUDE_OTF2XX_EVENT_ENTER_HPP
