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

#ifndef INCLUDE_OTF2XX_DEFINITIONS_IO_HANDLE_HPP
#define INCLUDE_OTF2XX_DEFINITIONS_IO_HANDLE_HPP

#include <otf2xx/common.hpp>
#include <otf2xx/fwd.hpp>
#include <otf2xx/reference.hpp>

#include <otf2xx/definition/io_paradigm.hpp>
#include <otf2xx/definition/string.hpp>

#include <otf2xx/definition/detail/io_handle_impl.hpp>
#include <otf2xx/definition/detail/referable_base.hpp>

namespace otf2
{
namespace definition
{

    class io_handle : public detail::referable_base<io_handle, detail::io_handle_impl>
    {
        using base = detail::referable_base<io_handle, detail::io_handle_impl>;
        using base::base;

    public:
        using io_handle_flag_type = impl_type::io_handle_flag_type;

        io_handle(reference_type ref, const otf2::definition::string& name,
                  const otf2::definition::io_file& file,
                  const otf2::definition::io_paradigm& paradigm, io_handle_flag_type handle_flag,
                  const otf2::definition::comm& comm, const otf2::definition::io_handle& parent)
        : base(ref,
               new impl_type(name, file, paradigm, handle_flag, comm, parent.get(), parent.ref()))
        {
        }

        io_handle(reference_type ref, const otf2::definition::string& name,
                  const otf2::definition::io_file& file,
                  const otf2::definition::io_paradigm& paradigm, io_handle_flag_type handle_flag,
                  const otf2::definition::comm& comm)
        : base(ref, new impl_type(name, file, paradigm, handle_flag, comm))
        {
        }

        io_handle() = default;

        /**
         * \brief returns the name of the io_handle definition
         *
         * \returns otf2::definition::string
         */
        const otf2::definition::string& name() const
        {
            assert(this->is_valid());
            return data_->name();
        }

        /**
         * \brief returns the io_file of the io_handle definition
         *
         * \returns otf2::definition::io_file
         */
        const otf2::definition::io_file& file() const
        {
            assert(this->is_valid());
            return data_->file();
        }

        /**
         * \brief returns the io_paradigm of the io_handle definition
         *
         * \returns otf2::definition::io_paradigm
         */
        const otf2::definition::io_paradigm& paradigm() const
        {
            assert(this->is_valid());
            return data_->paradigm();
        }

        /**
         * \brief returns the io_handle_flag
         *
         * \returns otf2::common::io_handle_flag
         */
        io_handle_flag_type io_handle_flag() const
        {
            assert(this->is_valid());
            return data_->io_handle_flag();
        }

        /**
         * \brief returns the comm of the io_handle definition
         *
         * \returns otf2::definition::comm
         */
        const otf2::definition::comm& comm() const
        {
            assert(this->is_valid());
            return data_->comm();
        }

        /**
         * \brief returns the parent of the io_handle definition
         * \returns an optional containing the parent io_handle or nothing
         */
        otf2::definition::io_handle parent() const
        {
            assert(this->is_valid());
            auto p = data_->parent();
            if (p.first != nullptr)
            {
                return otf2::definition::io_handle{ p.second, p.first };
            }
            else
            {
                return {};
            }
        }
    };
} // namespace definition
} // namespace otf2

#endif // INCLUDE_OTF2XX_DEFINITIONS_IO_HANDLE_HPP
