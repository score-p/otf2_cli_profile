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

#ifndef INCLUDE_OTF2XX_EVENT_DETAIL_METRIC_VALUES_HPP
#define INCLUDE_OTF2XX_EVENT_DETAIL_METRIC_VALUES_HPP

#include <otf2xx/event/detail/value_proxy.hpp>

#include <otf2xx/definition/metric_class.hpp>

#include <vector>

namespace otf2
{
namespace event
{
    namespace detail
    {
        class metric_values
        {
        public:
            metric_values(std::vector<OTF2_Type>&& type_ids,
                          std::vector<OTF2_MetricValue>&& metric_values)
            : type_ids_(std::move(type_ids)), values_(std::move(metric_values))
            {
                if (type_ids_.size() != values_.size())
                {
                    make_exception(
                        "attempting to construct metric_values from data of different sizes");
                }
            }

            metric_values(const otf2::definition::metric_class& metric_class)
            : type_ids_(metric_class.size()), values_(metric_class.size())
            {
                for (std::size_t i = 0; i < metric_class.size(); ++i)
                {
                    type_ids_[i] = static_cast<OTF2_Type>(metric_class[i].value_type());
                }
            }

            std::size_t size() const
            {
                return type_ids_.size();
            }

            const std::vector<OTF2_Type>& type_ids() const
            {
                return type_ids_;
            }

            const std::vector<OTF2_MetricValue>& values() const
            {
                return values_;
            }

            detail::typed_value_proxy at(std::size_t index)
            {
                if (index >= size())
                {
                    throw std::out_of_range("Out of bounds access in metric_values");
                }

                return { type_ids_[index], values_[index] };
            }

            detail::const_typed_value_proxy at(std::size_t index) const
            {
                if (index >= size())
                {
                    throw std::out_of_range("Out of bounds access in metric_values");
                }

                return { type_ids_[index], values_[index] };
            }

            detail::typed_value_proxy operator[](std::size_t index)
            {
                return { type_ids_[index], values_[index] };
            }

            detail::const_typed_value_proxy operator[](std::size_t index) const
            {
                return { type_ids_[index], values_[index] };
            }

            template <bool IsMutable>
            class base_iterator
            {
            public:
                using value_type = detail::base_typed_value_proxy<IsMutable>;

                using type_type = typename value_type::type_type;
                using metric_value_type = typename value_type::metric_value_type;

            private:
                friend metric_values;

                base_iterator(type_type* type_id, metric_value_type* value)
                : type_ptr_(type_id), value_ptr_(value)
                {
                }

            public:
                bool operator==(const base_iterator& rhs) const
                {
                    return type_ptr_ == rhs.type_ptr_ && value_ptr_ == rhs.value_ptr_;
                }

                bool operator!=(const base_iterator& rhs) const
                {
                    return !(*this == rhs);
                }

                value_type operator*()
                {
                    return { *type_ptr_, *value_ptr_ };
                }

                base_iterator& operator++()
                {
                    ++type_ptr_;
                    ++value_ptr_;
                    return *this;
                }

                base_iterator operator++(int)
                {
                    return { type_ptr_++, value_ptr_++ };
                }

            private:
                type_type* type_ptr_;
                metric_value_type* value_ptr_;
            };

            using iterator = base_iterator<true>;
            using const_iterator = base_iterator<false>;

            const_iterator begin() const
            {
                return const_iterator{ type_ids_.data(), values_.data() };
            }

            iterator begin()
            {
                return iterator{ type_ids_.data(), values_.data() };
            }

            const_iterator end() const
            {
                return const_iterator{ type_ids_.data() + size(), values_.data() + size() };
            }

            iterator end()
            {
                return iterator{ type_ids_.data() + size(), values_.data() + size() };
            }

        private:
            std::vector<OTF2_Type> type_ids_;
            std::vector<OTF2_MetricValue> values_;
        };
    } // namespace detail
} // namespace event
} // namespace otf2

#endif // INCLUDE_OTF2XX_EVENT_DETAIL_METRIC_VALUES_HPP
