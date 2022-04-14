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

#ifndef INCLUDE_OTF2XX_DEFINITIONS_DETAIL_CALLING_CONTEXT_HPP
#define INCLUDE_OTF2XX_DEFINITIONS_DETAIL_CALLING_CONTEXT_HPP

#include <otf2xx/exception.hpp>

#include <otf2xx/fwd.hpp>
#include <otf2xx/reference.hpp>

#include <otf2xx/definition/detail/ref_counted.hpp>
#include <otf2xx/intrusive_ptr.hpp>

#include <otf2xx/definition/region.hpp>
#include <otf2xx/definition/source_code_location.hpp>

namespace otf2
{
namespace definition
{
    namespace detail
    {

        class calling_context_impl : public ref_counted
        {
        public:
            using tag_type = calling_context;

        private:
            using reference_type = otf2::reference_impl<calling_context, tag_type>;

        public:
            calling_context_impl(const otf2::definition::region& region,
                                 const otf2::definition::source_code_location& source_code_location,
                                 calling_context_impl* parent, reference_type pref,
                                 std::int64_t retain_count = 0)
            : ref_counted(retain_count), region_(region),
              source_code_location_(source_code_location), parent_(parent), pref_(pref)
            {
            }

            calling_context_impl(const otf2::definition::region& region,
                                 const otf2::definition::source_code_location& source_code_location,
                                 std::int64_t retain_count = 0)
            : ref_counted(retain_count), region_(region),
              source_code_location_(source_code_location), parent_(nullptr),
              pref_(reference_type::undefined())
            {
            }

            const otf2::definition::region& region() const
            {
                return region_;
            }

            const otf2::definition::source_code_location& source_code_location() const
            {
                return source_code_location_;
            }

            auto parent() const
            {
                return std::make_pair(parent_.get(), pref_);
            }

        private:
            otf2::definition::region region_;
            otf2::definition::source_code_location source_code_location_;
            otf2::intrusive_ptr<calling_context_impl> parent_;
            reference_type pref_;
        };
    } // namespace detail
} // namespace definition
} // namespace otf2

#endif // INCLUDE_OTF2XX_DEFINITIONS_DETAIL_CALLING_CONTEXT_HPP
