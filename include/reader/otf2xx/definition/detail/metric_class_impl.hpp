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

#ifndef INCLUDE_OTF2XX_DEFINITIONS_DETAIL_METRIC_CLASS_HPP
#define INCLUDE_OTF2XX_DEFINITIONS_DETAIL_METRIC_CLASS_HPP

#include <otf2xx/common.hpp>
#include <otf2xx/fwd.hpp>

#include <otf2xx/definition/detail/ref_counted.hpp>

#include <otf2xx/definition/metric_member.hpp>
#include <otf2xx/definition/string.hpp>

#include <memory>
#include <vector>

namespace otf2
{
namespace definition
{
    namespace detail
    {

        class metric_class_impl : public ref_counted
        {
        public:
            using tag_type = metric_base;

            typedef otf2::common::metric_occurence metric_occurence;
            typedef otf2::common::recorder_kind recorder_kind_type;

            typedef std::vector<otf2::definition::metric_member>::const_iterator iterator;

            metric_class_impl(metric_occurence occurence, recorder_kind_type recorder_kind,
                              std::int64_t retain_count = 0)
            : ref_counted(retain_count), occurence_(occurence), recorder_kind_(recorder_kind)
            {
            }

            std::size_t size() const
            {
                return members_.size();
            }

            void add_member(otf2::definition::metric_member&& member)
            {
                members_.emplace_back(std::move(member));
            }

            void add_member(const otf2::definition::metric_member& member)
            {
                members_.push_back(member);
            }

            metric_occurence occurence() const
            {
                return occurence_;
            }

            const otf2::definition::metric_member& operator[](std::size_t i) const
            {
                return members_[i];
            }

            recorder_kind_type recorder_kind() const
            {
                return recorder_kind_;
            }

            iterator begin() const
            {
                return members_.begin();
            }

            iterator end() const
            {
                return members_.end();
            }

        private:
            metric_occurence occurence_;
            recorder_kind_type recorder_kind_;
            std::vector<otf2::definition::metric_member> members_;
        };
    } // namespace detail
} // namespace definition
} // namespace otf2

#endif // INCLUDE_OTF2XX_DEFINITIONS_DETAIL_METRIC_CLASS_HPP
