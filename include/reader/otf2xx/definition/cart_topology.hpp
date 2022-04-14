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

#ifndef INCLUDE_OTF2XX_DEFINITIONS_CART_TOPOLOGY_HPP
#define INCLUDE_OTF2XX_DEFINITIONS_CART_TOPOLOGY_HPP

#include <otf2xx/fwd.hpp>
#include <otf2xx/reference.hpp>

#include <otf2xx/definition/cart_dimension.hpp>
#include <otf2xx/definition/comm.hpp>
#include <otf2xx/definition/string.hpp>

#include <otf2xx/definition/detail/cart_topology_impl.hpp>
#include <otf2xx/definition/detail/referable_base.hpp>

namespace otf2
{
namespace definition
{

    /**
     * \brief class for representing cart topology definitions
     *
     * Each topology is described by a global id, a reference to its name, a reference to a
     * communicator, the number of dimensions, and references to those dimensions. The topology type
     * is defined by the paradigm of the group referenced by the associated communicator.
     *
     */
    class cart_topology : public detail::referable_base<cart_topology, detail::cart_topology_impl>
    {
        using base = detail::referable_base<cart_topology, detail::cart_topology_impl>;
        using base::base;

    public:
        typedef impl_type::iterator iterator;

        cart_topology(reference_type ref, const otf2::definition::string& name,
                      const otf2::definition::comm& comm)
        : base(ref, new impl_type(name, comm))
        {
        }

        cart_topology() = default;

        /**
         * \brief Returns a string definition containing the name
         * \return otf2::definition::string containing the name
         */
        const otf2::definition::string& name() const
        {
            assert(this->is_valid());
            return data_->name();
        }

        /**
         * \brief Returns the communicator object used to create the topology.
         * \return otf2::definition::comm
         */
        const otf2::definition::comm& comm() const
        {
            assert(this->is_valid());
            return data_->comm();
        }

        /**
         * \brief returns the number of dimensions
         */
        std::size_t size() const
        {
            assert(this->is_valid());
            return this->data_->size();
        }

        /**
         * \brief add a metric member to this metric class
         */
        void add_dimension(const otf2::definition::cart_dimension& dim)
        {
            assert(this->is_valid());
            this->data_->add_dimension(dim);
        }

        /**
         * \brief returns the i-th dimension
         */
        const otf2::definition::cart_dimension& operator[](std::size_t i) const
        {
            assert(this->is_valid());
            return this->data_->operator[](i);
        }

        iterator begin() const
        {
            assert(this->is_valid());
            return this->data_->begin();
        }

        iterator end() const
        {
            assert(this->is_valid());
            return this->data_->end();
        }
    };
} // namespace definition
} // namespace otf2

#endif // INCLUDE_OTF2XX_DEFINITIONS_CART_TOPOLOGY_HPP
