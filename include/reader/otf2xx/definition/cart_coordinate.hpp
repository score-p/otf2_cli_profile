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

#ifndef INCLUDE_OTF2XX_DEFINITIONS_CART_COORDINATE_HPP
#define INCLUDE_OTF2XX_DEFINITIONS_CART_COORDINATE_HPP

#include <otf2xx/common.hpp>
#include <otf2xx/fwd.hpp>
#include <otf2xx/reference.hpp>

#include <otf2xx/definition/cart_topology.hpp>

#include <otf2xx/definition/detail/base.hpp>
#include <otf2xx/definition/detail/cart_coordinate_impl.hpp>

namespace otf2
{
namespace definition
{

    /**
     * \brief class for representing cart topology definitions
     * Defines the coordinate of the location referenced by the given rank (w.r.t. the communicator
     * associated to the topology) in the referenced topology.
     */
    class cart_coordinate : public detail::base<detail::cart_coordinate_impl>
    {
        using base = detail::base<detail::cart_coordinate_impl>;
        using base::base;

    public:
        cart_coordinate(const otf2::definition::cart_topology& topology, std::uint32_t rank,
                        std::vector<std::uint32_t> coordinates)
        : base(new impl_type(topology, rank, std::move(coordinates)))
        {
        }

        cart_coordinate() = default;

        /**
         * \brief returns the cart topology to which this one is a supplementary definition.
         * \returns otf2::definition::system_tree_node
         */
        const otf2::definition::cart_topology& topology() const
        {
            assert(this->is_valid());
            return data_->topology();
        }

        /**
         * \brief The rank w.r.t. the communicator associated to the topology referencing this
         * coordinate.
         */
        std::uint32_t rank() const
        {

            assert(this->is_valid());
            return data_->rank();
        }

        /**
         * \brief returns the coordinate
         * \returns Coordinates, indexed by dimension
         */
        const std::vector<std::uint32_t>& coordinates() const
        {
            assert(this->is_valid());
            return data_->coordinates();
        }
    };

} // namespace definition

} // namespace otf2

#endif // INCLUDE_OTF2XX_DEFINITIONS_CART_COORDINATE_HPP
