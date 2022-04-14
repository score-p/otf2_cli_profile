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

#pragma once

#include <otf2xx/traits/definition.hpp>

#include <memory>
#include <variant>

namespace otf2
{
namespace definition
{
    namespace detail
    {
        template <typename Definition, typename Enable>
        class weak_ref
        {
            static_assert(otf2::traits::is_definition<Definition>::value,
                          "Definition must be an otf2xx definition.");

            using Impl = typename Definition::impl_type;

        public:
            weak_ref() : ptr_(nullptr)
            {
            }

            weak_ref(const Definition& def) : ptr_(def.get())
            {
            }

            weak_ref& operator=(const Definition& def)
            {
                ptr_ = def.get();

                return *this;
            }

            weak_ref(const weak_ref&) = default;
            weak_ref& operator=(const weak_ref&) = default;

            weak_ref(weak_ref&&) = default;
            weak_ref& operator=(weak_ref&&) = default;

            operator Definition() const
            {
                return lock();
            }

            Definition lock() const
            {
                return ptr_;
            }

            Impl& get()
            {
                return *ptr_;
            }

            Impl* operator->()
            {
                return ptr_;
            }

            Impl& operator*()
            {
                return *ptr_;
            }

            const Impl& get() const
            {
                return *ptr_;
            }

            const Impl* operator->() const
            {
                return ptr_;
            }

            const Impl& operator*() const
            {
                return *ptr_;
            }

            operator bool() const
            {
                return ptr_ != nullptr;
            }

        private:
            Impl* ptr_;
        };

        template <typename Definition>
        class weak_ref<Definition, typename std::enable_if_t<
                                       otf2::traits::is_referable_definition<Definition>::value>>
        {
            static_assert(otf2::traits::is_definition<Definition>::value,
                          "Definition must be an otf2xx definition.");

            using Impl = typename Definition::impl_type;
            using ref_type = typename Definition::reference_type;

        public:
            weak_ref() : ptr_(nullptr), ref_(ref_type::undefined())
            {
            }

            weak_ref(const Definition& def) : ptr_(def.get()), ref_(def.ref())
            {
            }

            weak_ref& operator=(const Definition& def)
            {
                ptr_ = def.get();
                ref_ = def.ref();

                return *this;
            }

            weak_ref(const weak_ref&) = default;
            weak_ref& operator=(const weak_ref&) = default;

            weak_ref(weak_ref&&) = default;
            weak_ref& operator=(weak_ref&&) = default;

            operator Definition() const
            {
                return lock();
            }

            Definition lock() const
            {
                return Definition{ ref_, ptr_ };
            }

            Impl& get()
            {
                return *ptr_;
            }

            Impl* operator->()
            {
                return ptr_;
            }

            Impl& operator*()
            {
                return *ptr_;
            }

            const Impl& get() const
            {
                return *ptr_;
            }

            const Impl* operator->() const
            {
                return ptr_;
            }

            const Impl& operator*() const
            {
                return *ptr_;
            }

            operator bool() const
            {
                return ptr_ != nullptr;
            }

            ref_type ref() const
            {
                return ref_;
            }

        private:
            Impl* ptr_;
            ref_type ref_;
        };
    } // namespace detail

    template <typename Definition>
    detail::weak_ref<Definition> make_weak_ref(const Definition& def)
    {
        return { def };
    }

    template <typename... Variants>
    std::variant<Variants...>
    variants_from_weak(const std::variant<detail::weak_ref<Variants>...>& refs)
    {
        return std::visit([](auto&& ref) { return std::variant<Variants...>(ref); }, refs);
    }
} // namespace definition
} // namespace otf2
