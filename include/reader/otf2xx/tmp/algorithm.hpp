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

#ifndef INCLUDE_OTF2XX_TMP_ALGORITHM_HPP
#define INCLUDE_OTF2XX_TMP_ALGORITHM_HPP

#include <otf2xx/tmp/typelist.hpp>
#include <type_traits>

namespace otf2
{
namespace tmp
{
    /**
     * Note: All template parameters TList take only typelists
     *       All others take any variadic sequence type (e.g. std::tuple)
     */

    /** @brief Standard void_t implementation to support e.g. detection idiom */
    template <class...>
    using void_t = void;

    /** @brief Return true, when the type ToSearch is contained in the sequence */
    template <class Seq, typename ToSearch>
    struct contains;

    /** @brief Return the size (number of arguments) of a sequence */
    template <class Seq>
    struct size;

    /** @brief Concatenate multiple typelists into one */
    template <typename... TLists>
    struct concat;
    template <typename... TLists>
    using concat_t = typename concat<TLists...>::type;

    /**
     * @brief Apply all elements from Seq to TargetSeq:
     * Seq<Foo, Bar> -> TargetSeq<Foo, Bar>
     */
    template <class Seq, template <typename...> class TargetSeq>
    struct apply;
    template <class Seq, template <typename...> class TargetSeq>
    using apply_t = typename apply<Seq, TargetSeq>::type;

    /**
     * @brief Apply the functor F to every element of TList:
     * typelist<a1, a2,...> -> typelist<F<a1>::type, F<a2>::type, ...>
     */
    template <class TList, template <typename> class F>
    struct transform;
    template <class TList, template <typename> class F>
    using transform_t = typename transform<TList, F>::type;

    /** @brief Remove all elements from TList not matching the predicate Cond */
    template <class TList, template <typename...> class Cond>
    struct filter;
    template <class TList, template <typename...> class Cond>
    using filter_t = typename filter<TList, Cond>::type;

    ///////////////////////////////////////////////////////////////////////////
    // Implementation
    ///////////////////////////////////////////////////////////////////////////

    namespace detail
    {
        /** @brief A list of bools */
        template <bool...>
        struct bool_list;

        /** @brief Return true, when all values are true */
        template <bool... bools>
        using all = std::is_same<bool_list<true, bools...>, bool_list<bools..., true>>;

        /** @brief Return true, when any value is true */
        template <bool... bools>
        using any = std::integral_constant<bool, !all<(!bools)...>::value>;
    } // namespace detail

    template <typename ToSearch, template <typename...> class Seq, typename... Ts>
    struct contains<Seq<Ts...>, ToSearch> : detail::any<std::is_same<Ts, ToSearch>::value...>
    {
    };

    template <template <typename...> class Seq, typename... Ts>
    struct size<Seq<Ts...>> : std::integral_constant<std::size_t, sizeof...(Ts)>
    {
    };

    template <>
    struct concat<>
    {
        using type = typelist<>;
    };

    template <typename... Args1>
    struct concat<typelist<Args1...>>
    {
        using type = typelist<Args1...>;
    };

    template <typename... Args1, typename... Args2, typename... Seqs>
    struct concat<typelist<Args1...>, typelist<Args2...>, Seqs...>
    : concat<typelist<Args1..., Args2...>, Seqs...>
    {
    };

    template <template <typename...> class Seq, template <typename...> class TargetSeq,
              typename... Ts>
    struct apply<Seq<Ts...>, TargetSeq>
    {
        using type = TargetSeq<Ts...>;
    };

    template <template <typename> class F, typename... Ts>
    struct transform<typelist<Ts...>, F>
    {
        using type = typelist<typename F<Ts>::type...>;
    };

    namespace detail
    {
        template <bool>
        struct return_if
        {
            template <typename T>
            using type = typelist<T>;
        };
        template <>
        struct return_if<false>
        {
            template <typename T>
            using type = typelist<>;
        };
    } // namespace detail

    template <template <typename...> class Cond, typename... Ts>
    struct filter<typelist<Ts...>, Cond>
    {
        // Convert each element T into either typelist<T> or typelist<> depending on Cond
        // Then concatenate all of those
        using type = concat_t<typename detail::return_if<Cond<Ts>::value>::template type<Ts>...>;
    };

} // namespace tmp
} // namespace otf2

#endif // INCLUDE_OTF2XX_TMP_ALGORITHM_HPP
