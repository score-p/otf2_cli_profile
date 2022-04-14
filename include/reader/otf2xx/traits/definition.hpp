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

#ifndef INCLUDE_OTF2XX_TRAITS_DEFINITION_HPP
#define INCLUDE_OTF2XX_TRAITS_DEFINITION_HPP

#include <otf2xx/definition/pre_fwd.hpp>
#include <otf2xx/tmp/algorithm.hpp>
#include <otf2xx/tmp/typelist.hpp>
#include <type_traits>

namespace otf2
{
namespace traits
{
    // clang-format off

    /// All definitions that can be referred to and have a unique id space (Tag of reference<>)
    using referable_definitions_base = tmp::typelist<
        otf2::definition::attribute,
        otf2::definition::detail::comm_base,
        otf2::definition::detail::group_base,
        otf2::definition::location,
        otf2::definition::location_group,
        otf2::definition::parameter,
        otf2::definition::region,
        otf2::definition::string,
        otf2::definition::rma_win,
        otf2::definition::system_tree_node,
        otf2::definition::detail::metric_base,
        otf2::definition::metric_member,
        otf2::definition::source_code_location,
        otf2::definition::call_path,
        otf2::definition::calling_context,
        otf2::definition::interrupt_generator,
        otf2::definition::cart_topology,
        otf2::definition::cart_dimension,
        otf2::definition::marker,
        otf2::definition::detail::io_file_base,
        otf2::definition::io_handle,
        otf2::definition::io_paradigm
    >;

    /// Definitions that can be referred to but don't have a unique id space
    /// They will use an id from the space of one of the types in @ref referable_definitions_base
    using referable_definitions_ext = tmp::typelist<
        otf2::definition::comm,
        otf2::definition::inter_comm,
        otf2::definition::metric_class,
        otf2::definition::metric_instance,
        otf2::definition::io_file,
        otf2::definition::io_regular_file,
        otf2::definition::io_directory
    >;

    /// Definitions without a reference
    using unreferable_definitions = tmp::typelist<
        otf2::definition::mapping_table,
        otf2::definition::io_pre_created_handle_state,
        otf2::definition::system_tree_node_domain,
        otf2::definition::metric_class_recorder,
        otf2::definition::cart_coordinate,
        otf2::definition::call_path_parameter
    >;

    using all_definitions = tmp::concat_t<
        referable_definitions_base,
        referable_definitions_ext,
        unreferable_definitions
    >;

    /// all definitions, which can be used as part of a otf2::registry
    using usable_definitions = tmp::typelist<
        otf2::definition::attribute,
        otf2::definition::comm,
        otf2::definition::inter_comm,
        otf2::definition::location,
        otf2::definition::location_group,
        otf2::definition::parameter,
        otf2::definition::region,
        otf2::definition::string,
        otf2::definition::system_tree_node,
        otf2::definition::system_tree_node_domain,
        otf2::definition::metric_member,
        otf2::definition::source_code_location,
        otf2::definition::call_path,
        otf2::definition::call_path_parameter,
        otf2::definition::calling_context,
        otf2::definition::interrupt_generator,
        otf2::definition::marker,
        otf2::definition::io_handle,
        otf2::definition::io_paradigm,
        otf2::definition::metric_class,
        otf2::definition::metric_class_recorder,
        otf2::definition::metric_instance,
        otf2::definition::io_regular_file,
        otf2::definition::io_directory,
        otf2::definition::io_pre_created_handle_state,
        otf2::definition::location_property,
        otf2::definition::location_group_property,
        otf2::definition::system_tree_node_property,
        otf2::definition::calling_context_property,
        otf2::definition::rma_win,
        otf2::definition::cart_topology,
        otf2::definition::cart_dimension,
        otf2::definition::cart_coordinate,
        otf2::definition::io_file_property,
        otf2::definition::locations_group,
        otf2::definition::regions_group,
        otf2::definition::comm_locations_group,
        otf2::definition::comm_group,
        otf2::definition::comm_self_group
    >;

    // clang-format on

    using referable_definitions =
        tmp::concat_t<referable_definitions_base, referable_definitions_ext>;

    template <typename Type>
    struct is_definition : tmp::contains<all_definitions, Type>
    {
    };

    template <typename T, otf2::common::group_type GroupType>
    struct is_definition<otf2::definition::group<T, GroupType>> : std::true_type
    {
    };

    template <typename Definition>
    struct is_definition<otf2::definition::property<Definition>> : std::true_type
    {
    };

    template <>
    struct is_definition<otf2::definition::clock_properties> : std::true_type
    {
    };

    template <typename Type>
    struct is_referable_definition : tmp::contains<referable_definitions, Type>
    {
    };

    template <typename T, otf2::common::group_type GroupType>
    struct is_referable_definition<otf2::definition::group<T, GroupType>> : std::true_type
    {
    };
} // namespace traits
} // namespace otf2

#endif // INCLUDE_OTF2XX_TRAITS_DEFINITION_HPP
