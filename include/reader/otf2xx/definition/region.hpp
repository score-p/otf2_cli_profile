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

#ifndef INCLUDE_OTF2XX_DEFINITIONS_REGION_HPP
#define INCLUDE_OTF2XX_DEFINITIONS_REGION_HPP

#include <otf2xx/common.hpp>
#include <otf2xx/fwd.hpp>
#include <otf2xx/reference.hpp>

#include <otf2xx/definition/string.hpp>

#include <otf2xx/definition/detail/referable_base.hpp>
#include <otf2xx/definition/detail/region_impl.hpp>

namespace otf2
{
namespace definition
{

    /**
     * \brief class for represening a region definition
     */
    class region : public detail::referable_base<region, detail::region_impl>
    {
        using base = detail::referable_base<region, detail::region_impl>;
        using base::base;

    public:
        typedef typename impl_type::role_type role_type;
        typedef typename impl_type::paradigm_type paradigm_type;
        typedef typename impl_type::flags_type flags_type;

        region(reference_type ref, const otf2::definition::string& name,
               const otf2::definition::string& canonical_name,
               const otf2::definition::string& description, role_type role, paradigm_type paradigm,
               flags_type flags, const otf2::definition::string& source_file, uint32_t begin_line,
               uint32_t end_line)
        : base(ref, new impl_type(name, canonical_name, description, role, paradigm, flags,
                                  source_file, begin_line, end_line))
        {
        }

        region() = default;

        /**
         * \brief returns the name of the region definion as a string definition
         *
         * \returns a \ref string definiton containing the name
         *
         */
        const otf2::definition::string& name() const
        {
            assert(this->is_valid());
            return data_->name();
        }

        void name(const otf2::definition::string& str)
        {
            assert(this->is_valid());
            return data_->name(str);
        }

        /**
         * \brief returns the canonical name of the region definition as a string definition
         * e.g. demangled function name
         * \returns a \ref string definiton containing the canonical name
         */
        const otf2::definition::string& canonical_name() const
        {
            assert(this->is_valid());
            return data_->canonical_name();
        }

        void canonical_name(const otf2::definition::string& str)
        {
            assert(this->is_valid());
            return data_->canonical_name(str);
        }

        /**
         * \brief returns the description of the region definion as a string definition
         *
         * \returns a \ref string definiton containing the description
         *
         */
        const otf2::definition::string& description() const
        {
            assert(this->is_valid());
            return data_->description();
        }

        void description(const otf2::definition::string& str)
        {
            assert(this->is_valid());
            return data_->description(str);
        }

        /**
         * \brief returns the role of this region
         * \see otf2::common::role_type
         */
        role_type role() const
        {
            assert(this->is_valid());
            return data_->role();
        }

        /**
         * \brief returns the paradigm of this region
         * \see otf2::common::paradigm_type
         */
        paradigm_type paradigm() const
        {
            assert(this->is_valid());
            return data_->paradigm();
        }

        /**
         * \brief returns the flags of this region
         * \see otf2::common::flags_type
         */
        flags_type flags() const
        {
            assert(this->is_valid());
            return data_->flags();
        }

        /**
         * \brief returns the name of file containing the region definion as a string definition
         *
         * \returns a \ref string definiton containing the file name
         *
         */
        const otf2::definition::string& source_file() const
        {
            assert(this->is_valid());
            return data_->source_file();
        }

        void source_file(const otf2::definition::string& str)
        {
            assert(this->is_valid());
            return data_->source_file(str);
        }

        /**
         * \brief returns the line number, where the region starts
         */
        uint32_t begin_line() const
        {
            assert(this->is_valid());
            return data_->begin_line();
        }

        /**
         * \brief returns the line number, where the region ends
         */
        uint32_t end_line() const
        {
            assert(this->is_valid());
            return data_->end_line();
        }
    };
} // namespace definition
} // namespace otf2

#endif // INCLUDE_OTF2XX_DEFINITIONS_REGION_HPP
