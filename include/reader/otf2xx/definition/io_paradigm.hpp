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

#ifndef INCLUDE_OTF2XX_DEFINITIONS_IO_PARADIGM_HPP
#define INCLUDE_OTF2XX_DEFINITIONS_IO_PARADIGM_HPP

#include <otf2xx/common.hpp>
#include <otf2xx/fwd.hpp>
#include <otf2xx/reference.hpp>

#include <otf2xx/definition/string.hpp>

#include <otf2xx/definition/detail/io_paradigm_impl.hpp>
#include <otf2xx/definition/detail/referable_base.hpp>

namespace otf2
{
namespace definition
{

    class io_paradigm : public detail::referable_base<io_paradigm, detail::io_paradigm_impl>
    {
        using base = detail::referable_base<io_paradigm, detail::io_paradigm_impl>;
        using base::base;

    public:
        using paradigm_class_type = impl_type::paradigm_class_type;
        using paradigm_flag_type = impl_type::paradigm_flag_type;
        using paradigm_property_type = impl_type::paradigm_property_type;

        io_paradigm(reference_type ref, const otf2::definition::string& identification,
                    const otf2::definition::string& name, paradigm_class_type paradigmClass,
                    paradigm_flag_type paradigmFlags,
                    const std::vector<paradigm_property_type>& properties,
                    const std::vector<otf2::attribute_value>& values)
        : base(ref, new impl_type(identification, name, paradigmClass, paradigmFlags, properties,
                                  values))
        {
        }

        io_paradigm() = default;

        const otf2::definition::string& identification() const
        {
            assert(this->is_valid());
            return data_->identification();
        }

        const otf2::definition::string& name() const
        {
            assert(this->is_valid());
            return data_->name();
        }

        paradigm_class_type paradigm_class() const
        {
            assert(this->is_valid());
            return data_->paradigm_class();
        }

        paradigm_flag_type paradigm_flags() const
        {
            assert(this->is_valid());
            return data_->paradigm_flags();
        }

        std::size_t size() const
        {
            assert(this->is_valid());
            return data_->size();
        }

        const std::vector<paradigm_property_type>& properties() const
        {
            assert(this->is_valid());
            return data_->properties();
        }

        const std::vector<otf2::attribute_value>& values() const
        {
            assert(this->is_valid());
            return data_->values();
        }
    };
} // namespace definition
} // namespace otf2

#endif // INCLUDE_OTF2XX_DEFINITIONS_IO_PARADIGM_HPP
