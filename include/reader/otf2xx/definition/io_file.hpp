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

#ifndef INCLUDE_OTF2XX_DEFINITIONS_IO_FILE_HPP
#define INCLUDE_OTF2XX_DEFINITIONS_IO_FILE_HPP

#include <otf2xx/common.hpp>
#include <otf2xx/fwd.hpp>
#include <otf2xx/reference.hpp>

#include <otf2xx/definition/string.hpp>
#include <otf2xx/definition/system_tree_node.hpp>

#include <otf2xx/definition/detail/io_file_impl.hpp>
#include <otf2xx/definition/detail/referable_base.hpp>

namespace otf2
{
namespace definition
{
    namespace detail
    {
        class io_file_base
        {
        public:
            using tag_type = io_file_base;
        };
    } // namespace detail

    class io_file : public detail::referable_base<io_file, detail::io_file_impl>
    {
        using base = detail::referable_base<io_file, detail::io_file_impl>;
        using base::base;

    public:
        using reference_type = typename base::reference_type;
        using impl_type = typename base::impl_type;

        io_file(reference_type ref, const otf2::definition::string& name,
                const otf2::definition::system_tree_node& stn)
        : base(ref, new impl_type(name, stn))
        {
        }

        io_file() = default;

        /**
         * \brief returns the name of the io_file definition as a string definition
         *
         * \returns a \ref string definition containing the name
         *
         */
        const otf2::definition::string& name() const
        {
            assert(this->is_valid());
            return this->data_->name();
        }

        /**
         * \brief returns the scope of the io_file definition
         *
         * \returns otf2::definition::system_tree_node
         */
        const otf2::definition::system_tree_node& scope() const
        {
            assert(this->is_valid());
            return this->data_->scope();
        }
    };
} // namespace definition
} // namespace otf2

#endif // INCLUDE_OTF2XX_DEFINITIONS_IO_FILE_HPP
