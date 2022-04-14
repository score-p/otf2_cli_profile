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

#ifndef INCLUDE_OTF2XX_TRAITS_REFERENCE_HPP
#define INCLUDE_OTF2XX_TRAITS_REFERENCE_HPP

#include <otf2xx/definition/fwd.hpp>
#include <otf2xx/traits/traits.hpp>

#include <type_traits>

extern "C"
{
#include <otf2/OTF2_GeneralDefinitions.h>
#include <otf2/OTF2_Marker.h>
}

namespace otf2
{
namespace traits
{

    template <typename Type>
    struct reference_type;

    // Type traits for Definitions tags
    // only add specializations for base / tag types
    template <>
    struct reference_type<definition::location> : identity<OTF2_LocationRef>
    {
    };

    template <>
    struct reference_type<definition::location_group> : identity<OTF2_LocationGroupRef>
    {
    };

    template <>
    struct reference_type<definition::system_tree_node> : identity<OTF2_SystemTreeNodeRef>
    {
    };

    template <>
    struct reference_type<definition::string> : identity<OTF2_StringRef>
    {
    };

    template <>
    struct reference_type<definition::detail::group_base> : identity<OTF2_GroupRef>
    {
    };

    template <>
    struct reference_type<definition::detail::comm_base> : identity<OTF2_CommRef>
    {
    };

    template <>
    struct reference_type<definition::rma_win> : identity<OTF2_RmaWinRef>
    {
    };

    template <>
    struct reference_type<definition::attribute> : identity<OTF2_AttributeRef>
    {
    };

    template <>
    struct reference_type<definition::parameter> : identity<OTF2_ParameterRef>
    {
    };

    template <>
    struct reference_type<definition::region> : identity<OTF2_RegionRef>
    {
    };

    template <>
    struct reference_type<definition::call_path> : identity<OTF2_CallpathRef>
    {
    };

    template <>
    struct reference_type<definition::cart_dimension> : identity<OTF2_CartDimensionRef>
    {
    };

    template <>
    struct reference_type<definition::cart_topology> : identity<OTF2_CartTopologyRef>
    {
    };

    template <>
    struct reference_type<definition::calling_context> : identity<OTF2_CallingContextRef>
    {
    };

    template <>
    struct reference_type<definition::source_code_location> : identity<OTF2_SourceCodeLocationRef>
    {
    };

    template <>
    struct reference_type<definition::interrupt_generator> : identity<OTF2_InterruptGeneratorRef>
    {
    };

    template <>
    struct reference_type<definition::detail::metric_base> : identity<OTF2_MetricRef>
    {
    };

    template <>
    struct reference_type<definition::marker> : identity<OTF2_MarkerRef>
    {
    };

    template <>
    struct reference_type<definition::metric_member> : identity<OTF2_MetricMemberRef>
    {
    };

    template <>
    struct reference_type<definition::detail::io_file_base> : identity<OTF2_IoFileRef>
    {
    };

    template <>
    struct reference_type<definition::io_handle> : identity<OTF2_IoHandleRef>
    {
    };

    template <>
    struct reference_type<definition::io_paradigm> : identity<OTF2_IoParadigmRef>
    {
    };

    template <typename T, typename... Args>
    struct reference_type_var : reference_type<T>
    {
        static_assert(std::is_same<typename reference_type<T>::type,
                                   typename reference_type_var<Args...>::type>::value,
                      "Can only use this for compatible definitions");
    };

    template <typename T>
    struct reference_type_var<T> : reference_type<T>
    {
    };

} // namespace traits
} // namespace otf2

#endif // INCLUDE_OTF2XX_TRAITS_REFERENCE_HPP
