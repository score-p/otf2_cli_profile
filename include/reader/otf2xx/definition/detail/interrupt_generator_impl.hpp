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

#ifndef INCLUDE_OTF2XX_DEFINITIONS_DETAIL_INTERRUPT_GENERATOR_HPP
#define INCLUDE_OTF2XX_DEFINITIONS_DETAIL_INTERRUPT_GENERATOR_HPP

#include <otf2xx/exception.hpp>

#include <otf2xx/fwd.hpp>

#include <otf2xx/definition/detail/ref_counted.hpp>

#include <otf2xx/definition/region.hpp>
#include <otf2xx/definition/source_code_location.hpp>

#include <memory>

namespace otf2
{
namespace definition
{
    namespace detail
    {

        class interrupt_generator_impl : public ref_counted
        {
        public:
            using tag_type = interrupt_generator;

            using interrupt_generator_mode_type = otf2::common::interrupt_generator_mode_type;
            using base_type = otf2::common::base_type;

            interrupt_generator_impl(const otf2::definition::string& name,
                                     interrupt_generator_mode_type interrupt_generator_mode,
                                     base_type base, std::int64_t exponent, std::uint64_t period,
                                     std::int64_t retain_count = 0)
            : ref_counted(retain_count), name_(name),
              interrupt_generator_mode_(interrupt_generator_mode), base_(base), exponent_(exponent),
              period_(period)
            {
            }

            const otf2::definition::string& name() const
            {
                return name_;
            }

            interrupt_generator_mode_type interrupt_generator_mode() const
            {
                return interrupt_generator_mode_;
            }

            base_type period_base() const
            {
                return base_;
            }

            /**
             * \brief returns the exponent
             * \returns std::int64_t
             */
            std::int64_t period_exponent() const
            {
                return exponent_;
            }

            /**
             * \brief returns the period
             * \returns std::uint64_t
             */
            std::uint64_t period() const
            {
                return period_;
            }

        private:
            otf2::definition::string name_;
            interrupt_generator_mode_type interrupt_generator_mode_;
            base_type base_;
            std::int64_t exponent_;
            std::uint64_t period_;
        };
    } // namespace detail
} // namespace definition
} // namespace otf2

#endif // INCLUDE_OTF2XX_DEFINITIONS_DETAIL_INTERRUPT_GENERATOR_HPP
