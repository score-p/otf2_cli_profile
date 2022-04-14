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

#ifndef INCLUDE_OTF2XX_EVENT_DETAIL_VALUE_PROXY_HPP
#define INCLUDE_OTF2XX_EVENT_DETAIL_VALUE_PROXY_HPP

#include <otf2xx/common.hpp>
#include <otf2xx/definition/detail/weak_ref.hpp>
#include <otf2xx/definition/metric_member.hpp>
#include <otf2xx/exception.hpp>

#include <otf2/OTF2_Events.h>

#include <cmath> // for std::pow

namespace otf2
{
namespace event
{
    namespace detail
    {
        /**
         * \brief A proxy class that allows type safe access to a metric value
         *
         * For performance reasons, the types and values of a metric event are
         * stored in separate locations. This proxy class allows to set and
         * retrieve a value with the correct type.
         */
        template <bool IsMutable>
        class base_typed_value_proxy
        {
        public:
            using type_type =
                typename std::conditional<IsMutable, OTF2_Type, const OTF2_Type>::type;

            using metric_value_type = typename std::conditional<IsMutable, OTF2_MetricValue,
                                                                const OTF2_MetricValue>::type;

        public:
            base_typed_value_proxy(type_type& type, metric_value_type& value)
            : type_(type), value_(value)
            {
            }

            double as_double() const
            {
                switch (type())
                {
                case otf2::common::type::Double:
                    return static_cast<double>(value_.floating_point);
                case otf2::common::type::int64:
                    return static_cast<double>(value_.signed_int);
                case otf2::common::type::uint64:
                    return static_cast<double>(value_.unsigned_int);
                default:
                    make_exception("Unexpected type given in metric member");
                }

                return 0;
            }

            std::int64_t as_int64() const
            {
                switch (type())
                {
                case otf2::common::type::Double:
                    return static_cast<std::int64_t>(value_.floating_point);
                case otf2::common::type::int64:
                    return static_cast<std::int64_t>(value_.signed_int);
                case otf2::common::type::uint64:
                    return static_cast<std::int64_t>(value_.unsigned_int);
                default:
                    make_exception("Unexpected type given in metric member");
                }

                return 0;
            }

            std::uint64_t as_uint64() const
            {
                switch (type())
                {
                case otf2::common::type::Double:
                    return static_cast<std::uint64_t>(value_.floating_point);
                case otf2::common::type::int64:
                    return static_cast<std::uint64_t>(value_.signed_int);
                case otf2::common::type::uint64:
                    return static_cast<std::uint64_t>(value_.unsigned_int);
                default:
                    make_exception("Unexpected type given in metric member");
                }

                return 0;
            }

            template <typename T, bool Mutable = IsMutable>
            std::enable_if_t<Mutable && std::is_arithmetic<T>::value> value(T x)
            {
                switch (type())
                {
                case otf2::common::type::Double:
                    value_.floating_point = static_cast<double>(x);
                    break;
                case otf2::common::type::int64:
                    value_.signed_int = static_cast<std::int64_t>(x);
                    break;
                case otf2::common::type::uint64:
                    value_.unsigned_int = static_cast<std::uint64_t>(x);
                    break;
                default:
                    make_exception("Unexpected type given in metric member");
                }
            }

            template <typename T>
            base_typed_value_proxy operator=(T x)
            {
                value(x);
                return { *this };
            }

            otf2::common::type type() const
            {
                return static_cast<otf2::common::type>(type_);
            }

            template <bool Mutable = IsMutable>
            std::enable_if_t<Mutable> type(otf2::common::type type_id)
            {
                type_ = static_cast<OTF2_Type>(type_id);
            }

            template <bool Mutable = IsMutable>
            std::enable_if_t<Mutable> type(OTF2_Type type_id)
            {
                type_ = type_id;
            }

        private:
            type_type& type_;
            metric_value_type& value_;
        };

        using typed_value_proxy = base_typed_value_proxy<true>;
        using const_typed_value_proxy = base_typed_value_proxy<false>;

        /**
         * \brief A proxy class allowing type safe, correctly scaled access to a
         *        metric value
         */
        template <bool IsMutable>
        class base_value_proxy
        {
            template <typename T>
            T scale(T x) const
            {
                using value_base_type = otf2::definition::metric_member::value_base_type;

                int base;
                switch (metric_->value_base())
                {
                case value_base_type::binary:
                    base = 2;
                    break;
                case value_base_type::decimal:
                    base = 10;
                    break;
                default:
                    make_exception("Unexpected base given in metric member");
                }
                return x * std::pow(base, metric_->value_exponent());
            }

        public:
            using typed_value_proxy = detail::base_typed_value_proxy<IsMutable>;

            // This is OTF2_Type, or const OTF2_Type if IsMutable == true
            using type_type = typename typed_value_proxy::type_type;

            // ... and the same thing for OTF2_MetricValue
            using metric_value_type = typename typed_value_proxy::metric_value_type;

            base_value_proxy(
                type_type& type, metric_value_type& value,
                otf2::definition::detail::weak_ref<otf2::definition::metric_member> metric)
            : value_(type, value), metric_(metric)
            {
            }

            base_value_proxy(
                const typed_value_proxy& value,
                otf2::definition::detail::weak_ref<otf2::definition::metric_member> metric)
            : value_(value), metric_(metric)
            {
            }

            base_value_proxy(
                typed_value_proxy&& value,
                otf2::definition::detail::weak_ref<otf2::definition::metric_member> metric)
            : value_(std::move(value)), metric_(metric)
            {
            }

            double as_double() const
            {
                return scale<double>(value_.as_double());
            }

            std::int64_t as_int64() const
            {
                return scale<std::int64_t>(value_.as_int64());
            }

            std::uint64_t as_uint64() const
            {
                return scale<std::uint64_t>(value_.as_uint64());
            }

            template <typename T, bool Mutable = IsMutable>
            std::enable_if_t<Mutable> set(T x)
            {
                value_.value(x);
            }

            template <typename T, bool Mutable = IsMutable>
            std::enable_if_t<Mutable, base_value_proxy&> operator=(T x)
            {
                value_ = x;
                return *this;
            }

            otf2::common::type type() const
            {
                assert(value_.type() == metric_->value_type());

                return value_.type();
            }

            otf2::definition::metric_member metric() const
            {
                return metric_;
            }

        private:
            typed_value_proxy value_;
            otf2::definition::detail::weak_ref<otf2::definition::metric_member> metric_;
        };

        using value_proxy = base_value_proxy<true>;
        using const_value_proxy = base_value_proxy<false>;
    } // namespace detail
} // namespace event
} // namespace otf2

#endif // INCLUDE_OTF2XX_EVENT_DETAIL_VALUE_PROXY_HPP
