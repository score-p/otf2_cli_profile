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

#ifndef INCLUDE_OTF2XX_DEFINITIONS_SYSTEM_TREE_NODE_HPP
#define INCLUDE_OTF2XX_DEFINITIONS_SYSTEM_TREE_NODE_HPP

#include <otf2xx/exception.hpp>

#include <otf2xx/fwd.hpp>
#include <otf2xx/reference.hpp>

#include <otf2xx/definition/string.hpp>

#include <otf2xx/definition/detail/referable_base.hpp>
#include <otf2xx/definition/detail/system_tree_node_impl.hpp>

namespace otf2
{
namespace definition
{

    /**
     * \brief class for representing system tree node definitions
     */
    class system_tree_node
    : public detail::referable_base<system_tree_node, detail::system_tree_node_impl>
    {
        using base = detail::referable_base<system_tree_node, detail::system_tree_node_impl>;
        using base::base;

    public:
        system_tree_node(reference<system_tree_node> ref, const otf2::definition::string& name,
                         const otf2::definition::string& class_name,
                         const otf2::definition::system_tree_node& parent)
        : base(ref, new impl_type(name, class_name, parent.get(), parent.ref()))
        {
        }

        system_tree_node(reference<system_tree_node> ref, const otf2::definition::string& name,
                         const otf2::definition::string& class_name)
        : base(ref, new impl_type(name, class_name))
        {
        }

        system_tree_node() = default;

        /**
         * \brief returns the name of the system tree node definion as a string definition
         *
         * \returns a string definiton containing the name
         *
         */
        const otf2::definition::string& name() const
        {
            assert(this->is_valid());
            return data_->name();
        }

        /**
         * \brief sets the name of the system tree node definition
         *
         * \param newname a string definition containing the new name for this node
         *
         */
        void name(const otf2::definition::string& newname)
        {
            assert(this->is_valid());
            data_->name() = newname;
        }

        /**
         * \brief returns the class name of the system tree node definion as a string definition
         *
         * \returns a string definiton containing the class name
         *
         */
        const otf2::definition::string& class_name() const
        {
            assert(this->is_valid());
            return data_->class_name();
        }

        /**
         * \brief returns the parent
         * \returns otf2::definition::system_tree_node
         */
        otf2::definition::system_tree_node parent() const
        {
            assert(this->is_valid());
            auto p = data_->parent();
            if (p.first != nullptr)
            {
                return otf2::definition::system_tree_node{ p.second, p.first };
            }
            else
            {
                return {};
            }
        }
    };

} // namespace definition

} // namespace otf2

#endif // INCLUDE_OTF2XX_DEFINITIONS_SYSTEM_TREE_NODE_HPP
