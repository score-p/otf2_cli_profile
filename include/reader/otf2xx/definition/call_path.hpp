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

#ifndef INCLUDE_OTF2XX_DEFINITIONS_CALL_PATH_HPP
#define INCLUDE_OTF2XX_DEFINITIONS_CALL_PATH_HPP

#include <otf2xx/exception.hpp>

#include <otf2xx/fwd.hpp>
#include <otf2xx/reference.hpp>

#include <otf2xx/definition/region.hpp>

#include <otf2xx/definition/detail/call_path_impl.hpp>
#include <otf2xx/definition/detail/referable_base.hpp>

namespace otf2
{
namespace definition
{

    /**
     * \brief class for representing call path definitions
     */
    class call_path : public detail::referable_base<call_path, detail::call_path_impl>
    {
        using base = detail::referable_base<call_path, detail::call_path_impl>;
        using base::base;

    public:
        call_path(reference<call_path> ref, const otf2::definition::region& region,
                  const otf2::definition::call_path& parent)
        : base(ref, new impl_type(region, parent.get(), parent.ref()))
        {
        }

        call_path(reference<call_path> ref, const otf2::definition::region& region)
        : base(ref, new impl_type(region))
        {
        }

        call_path() = default;

        /**
         * \brief returns the region of the call path
         *
         * \returns a region definiton
         *
         */
        const otf2::definition::region& region() const
        {
            assert(this->is_valid());
            return data_->region();
        }

        /**
         * \brief returns the parent
         * \returns an optional containing the parent call_path, or notihng
         */
        otf2::definition::call_path parent() const
        {
            assert(this->is_valid());
            auto p = data_->parent();

            if (p.first != nullptr)
            {
                return otf2::definition::call_path{ p.second, p.first };
            }
            else
            {
                return {};
            }
        }
    };

} // namespace definition

} // namespace otf2

#endif // INCLUDE_OTF2XX_DEFINITIONS_CALL_PATH_HPP
