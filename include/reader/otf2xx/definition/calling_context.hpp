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

#ifndef INCLUDE_OTF2XX_DEFINITIONS_CALLING_CONTEXT_HPP
#define INCLUDE_OTF2XX_DEFINITIONS_CALLING_CONTEXT_HPP

#include <otf2xx/common.hpp>
#include <otf2xx/fwd.hpp>
#include <otf2xx/reference.hpp>

#include <otf2xx/definition/region.hpp>
#include <otf2xx/definition/source_code_location.hpp>

#include <otf2xx/definition/detail/calling_context_impl.hpp>
#include <otf2xx/definition/detail/referable_base.hpp>

namespace otf2
{
namespace definition
{

    /**
     * \brief class for representing a calling context definition
     */
    class calling_context
    : public detail::referable_base<calling_context, detail::calling_context_impl>
    {
        using base = detail::referable_base<calling_context, detail::calling_context_impl>;
        using base::base;

    public:
        calling_context(reference_type ref, const otf2::definition::region& region,
                        const otf2::definition::source_code_location& source_code_location,
                        const otf2::definition::calling_context& parent)
        : base(ref, new impl_type(region, source_code_location, parent.get(), parent.ref()))
        {
        }

        calling_context(reference_type ref, const otf2::definition::region& region,
                        const otf2::definition::source_code_location& source_code_location)
        : base(ref, new impl_type(region, source_code_location))
        {
        }

        calling_context() = default;

        /**
         * \brief returns the region
         * \returns otf2::definition::region
         */
        const otf2::definition::region& region() const
        {
            assert(this->is_valid());
            return data_->region();
        }

        /**
         * \brief returns the source_code_location
         * \returns otf2::definition::source_code_location
         */
        const otf2::definition::source_code_location& source_code_location() const
        {
            assert(this->is_valid());
            return data_->source_code_location();
        }

        /**
         * \brief returns the parent
         * \returns an optinal containing the parent calling_context or nothing
         */
        otf2::definition::calling_context parent() const
        {
            assert(this->is_valid());
            auto p = data_->parent();
            if (p.first != nullptr)
            {
                return otf2::definition::calling_context{ p.second, p.first };
            }
            else
            {
                return {};
            }
        }
    };
} // namespace definition
} // namespace otf2

#endif // INCLUDE_OTF2XX_DEFINITIONS_CALLING_CONTEXT_HPP
