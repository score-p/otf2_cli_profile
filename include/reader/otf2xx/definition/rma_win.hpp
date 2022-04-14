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

#ifndef INCLUDE_OTF2XX_DEFINITIONS_RMA_WIN_HPP
#define INCLUDE_OTF2XX_DEFINITIONS_RMA_WIN_HPP

#include <otf2xx/common.hpp>
#include <otf2xx/fwd.hpp>
#include <otf2xx/reference.hpp>

#include <otf2xx/definition/string.hpp>

#include <otf2xx/definition/detail/referable_base.hpp>
#include <otf2xx/definition/detail/rma_win_impl.hpp>

namespace otf2
{
namespace definition
{

    /**
     * \brief class for represening a rma_win definition
     */
    class rma_win : public detail::referable_base<rma_win, detail::rma_win_impl>
    {
        using base = detail::referable_base<rma_win, detail::rma_win_impl>;
        using base::base;

    public:
        using rma_win_flag_type = impl_type::rma_win_flag_type;

        rma_win(reference_type ref, const otf2::definition::string& name,
                const otf2::definition::comm& comm, rma_win_flag_type flags)
        : base(ref, new impl_type(name, comm, flags))
        {
        }

        rma_win() = default;

        /**
         * \brief returns the name of the rma_win definion as a string definition
         *
         * \returns a \ref string definiton containing the name
         *
         */
        const otf2::definition::string& name() const
        {
            assert(this->is_valid());
            return data_->name();
        }

        /**
         * Communicator object used to create the window.
         * \brief returns the comm of the rma_win definition
         * \returns a \ref to the comm definition
         */
        const otf2::definition::comm& comm() const
        {
            assert(this->is_valid());
            return data_->comm();
        }

        /**
         * Special characteristics of this RMA window.
         * \brief Special characteristics of this RMA window
         * \returns the rma window flags
         */
        rma_win_flag_type flags() const
        {
            assert(this->is_valid());
            return data_->flags();
        }
    };
} // namespace definition
} // namespace otf2

#endif // INCLUDE_OTF2XX_DEFINITIONS_RMA_WIN_HPP
