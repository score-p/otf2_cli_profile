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

#ifndef INCLUDE_OTF2XX_DEFINITIONS_DETAIL_GROUP_HPP
#define INCLUDE_OTF2XX_DEFINITIONS_DETAIL_GROUP_HPP

#include <otf2xx/common.hpp>
#include <otf2xx/definition/fwd.hpp>

#include <otf2xx/definition/detail/ref_counted.hpp>

#include <otf2xx/definition/string.hpp>

#include <otf2xx/traits/definition.hpp>

#include <memory>
#include <vector>

namespace otf2
{
namespace definition
{
    namespace detail
    {
        template <class MemberType,
                  otf2::common::group_type GroupType = otf2::common::group_type::unknown>
        class group_impl : public ref_counted
        {
            static_assert(otf2::traits::is_definition<MemberType>::value,
                          "The MemberType has to be a otf2::definition.");

            typedef std::vector<MemberType> members_type;

        public:
            typedef otf2::common::group_type group_type;
            typedef otf2::common::group_flag_type group_flag_type;
            typedef otf2::common::paradigm_type paradigm_type;
            typedef MemberType value_type;

        public:
            using tag_type = group_base;

            group_impl(const otf2::definition::string& name, paradigm_type paradigm,
                       group_flag_type group_flag, std::int64_t retain_count = 0)
            : ref_counted(retain_count), name_(name), paradigm_(paradigm), group_flag_(group_flag)
            {
            }

            otf2::definition::string& name()
            {
                return name_;
            }

            group_type type() const
            {
                return GroupType;
            }

            paradigm_type paradigm() const
            {
                return paradigm_;
            }

            group_flag_type group_flag() const
            {
                return group_flag_;
            }

            std::vector<std::uint64_t> members() const
            {
                std::vector<std::uint64_t> result(members_.size());

                for (std::size_t i = 0; i < members_.size(); i++)
                {
                    result[i] = members_[i].ref();
                }

                return result;
            }

            std::size_t size() const
            {
                return members_.size();
            }

            const value_type& operator[](std::size_t i) const
            {
                return members_[i];
            }

            void add_member(value_type&& member)
            {
                members_.emplace_back(std::forward(member));
            }

            void add_member(const value_type& member)
            {
                members_.push_back(member);
            }

            void remove_member(const value_type& def)
            {
                for (auto it = members_.begin(); it != members_.end();)
                {
                    if (it->ref() == def.ref())
                    {
                        it = members_.erase(it);
                    }
                    else
                    {
                        ++it;
                    }
                }
            }

        private:
            otf2::definition::string name_;
            members_type members_;
            paradigm_type paradigm_;
            group_flag_type group_flag_;
        };
    } // namespace detail
} // namespace definition
} // namespace otf2

#endif // INCLUDE_OTF2XX_DEFINITIONS_DETAIL_GROUP_HPP
