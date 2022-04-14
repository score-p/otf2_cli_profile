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

#ifndef INCLUDE_OTF2XX_DEFINITIONS_DETAIL_REFERABLE_BASE_HPP
#define INCLUDE_OTF2XX_DEFINITIONS_DETAIL_REFERABLE_BASE_HPP

#include <otf2xx/definition/detail/base.hpp>
#include <otf2xx/definition/fwd.hpp>
#include <otf2xx/traits/definition.hpp>

#include <cassert>

namespace otf2
{
namespace definition
{
    namespace detail
    {
        /**
         * \brief CRTP base class for definition with a reference
         *
         * This class is implemented using CRTP.
         */
        template <typename Definition, typename Impl>
        class referable_base : public otf2::definition::detail::base<Impl>
        {
            using base = otf2::definition::detail::base<Impl>;

        public:
            using tag_type = typename Impl::tag_type;
            using reference_type = otf2::reference_impl<Definition, tag_type>;

            referable_base() : ref_(reference_type::undefined())
            {
            }

            explicit referable_base(reference_type ref, Impl* data = nullptr)
            : base(data), ref_(ref)
            {
            }

            referable_base(const referable_base& other) = default;
            referable_base(referable_base&& other) = default;
            referable_base& operator=(const referable_base& other) = default;
            referable_base& operator=(referable_base&& other) = default;

            /**
             * \brief Returns the reference number of the definition
             *
             * This number is used by libotf2 to identify a definition record.
             *
             * \returns a reference number
             */
            reference_type ref() const
            {
                return ref_;
            }

        protected:
            reference_type ref_;
        };

        template <typename Def, typename Def2, typename Impl, typename Impl2>
        inline std::enable_if_t<
            std::is_same<typename Impl::tag_type, typename Impl2::tag_type>::value, bool>
        operator==(const referable_base<Def, Impl>& a, const referable_base<Def2, Impl2>& b)
        {
            return a.ref() == b.ref();
        }

        template <typename Def, typename Def2, typename Impl, typename Impl2>
        inline bool operator!=(const referable_base<Def, Impl>& a,
                               const referable_base<Def2, Impl2>& b)
        {
            return !(a == b);
        }
    } // namespace detail
} // namespace definition
} // namespace otf2

#endif // INCLUDE_OTF2XX_DEFINITIONS_DETAIL_REFERABLE_BASE_HPP
