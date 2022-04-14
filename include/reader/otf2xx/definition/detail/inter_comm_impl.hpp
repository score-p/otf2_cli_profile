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

#ifndef INCLUDE_OTF2XX_DEFINITIONS_DETAIL_INTER_COMM_HPP
#define INCLUDE_OTF2XX_DEFINITIONS_DETAIL_INTER_COMM_HPP

#include <otf2xx/exception.hpp>

#include <otf2xx/fwd.hpp>
#include <otf2xx/reference.hpp>

#include <otf2xx/definition/detail/ref_counted.hpp>
#include <otf2xx/intrusive_ptr.hpp>

#include <otf2xx/definition/group.hpp>
#include <otf2xx/definition/string.hpp>

#include <sstream>
#include <tuple>
#include <variant>

namespace otf2
{
namespace definition
{
    namespace detail
    {

        class inter_comm_impl : public ref_counted
        {
        public:
            using tag_type = detail::comm_base;

        private:
            using reference_type = otf2::reference_impl<comm_base, tag_type>::ref_type;

        public:
            using comm_flag_type = otf2::common::comm_flag_type;
            using group_type =
                std::variant<otf2::definition::comm_group, otf2::definition::comm_self_group>;

            inter_comm_impl(const otf2::definition::string& name, const group_type& groupA,
                            const group_type& groupB, inter_comm_impl* common_communicator,
                            reference_type pref, comm_flag_type flags,
                            std::int64_t retain_count = 0)
            : ref_counted(retain_count), name_(name), groupA_(groupA), groupB_(groupB),
              common_inter_communicator_(common_communicator), common_communicator_(), pref_(pref),
              flags_(flags)
            {
            }

            inter_comm_impl(const otf2::definition::string& name, const group_type& groupA,
                            const group_type& groupB, comm_impl* common_communicator,
                            reference_type ccref, comm_flag_type flags,
                            std::int64_t retain_count = 0)
            : ref_counted(retain_count), name_(name), groupA_(groupA), groupB_(groupB),
              common_inter_communicator_(), common_communicator_(common_communicator), pref_(ccref),
              flags_(flags)
            {
            }

            inter_comm_impl(const otf2::definition::string& name, const group_type& groupA,
                            const group_type& groupB, comm_flag_type flags,
                            std::int64_t retain_count = 0)
            : ref_counted(retain_count), name_(name), groupA_(groupA), groupB_(groupB),
              common_inter_communicator_(), common_communicator_(),
              pref_(otf2::reference_impl<comm_base, tag_type>::undefined()), flags_(flags)
            {
            }

            otf2::definition::string& name()
            {
                return name_;
            }

            const group_type& groupA() const
            {
                return groupA_;
            }

            const group_type& groupB() const
            {

                return groupB_;
            }

            std::tuple<inter_comm_impl*, comm_impl*, reference_type> common_communicator() const
            {
                return std::make_tuple(common_inter_communicator_.get(), common_communicator_.get(),
                                       pref_);
            }

            comm_flag_type flags() const
            {
                return flags_;
            }

        private:
            otf2::definition::string name_;
            group_type groupA_;
            group_type groupB_;
            otf2::intrusive_ptr<inter_comm_impl> common_inter_communicator_;
            otf2::intrusive_ptr<comm_impl> common_communicator_;
            reference_type pref_;
            comm_flag_type flags_;
        };
    } // namespace detail
} // namespace definition
} // namespace otf2

#endif // INCLUDE_OTF2XX_DEFINITIONS_DETAIL_INTER_COMM_HPP
