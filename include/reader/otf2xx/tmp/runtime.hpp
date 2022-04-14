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

#ifndef INCLUDE_OTF2XX_TMP_RUNTIME_HPP
#define INCLUDE_OTF2XX_TMP_RUNTIME_HPP

#include <type_traits>
#include <utility>

namespace otf2
{
namespace tmp
{
    /**
     * @brief Call the given functor for every element in the sequence
     * There must be an appropriate get<Index>(seq) function available via ADL
     */
    template <class Seq, typename F>
    void foreach (Seq&& seq, F && f);

    ///////////////////////////////////////////////////////////////////////////
    // Implementation
    ///////////////////////////////////////////////////////////////////////////

    namespace detail
    {
        // Dummy as placeholder
        template <std::size_t idx, class T>
        void get(T&&);

        template <class Seq>
        struct foreach;

        template <template <typename...> class Seq, typename... Ts>
        struct foreach<Seq<Ts...>>
        {
            using indices = std::make_index_sequence<sizeof...(Ts)>;

            // Template required to enable ADL at instantiation point
            template <class ActSeq, class F, std::size_t... Idx>
            static void do_apply(ActSeq&& seq, F&& f, std::index_sequence<Idx...>)
            {
                // Pre C++17 expansion of `f(get<Idx>(std::forward<ActSeq>(seq))`
                // https://stackoverflow.com/questions/25680461/variadic-template-pack-expansion/25683817#25683817
                using expander = int[];
                (void)expander{ 0, ((void)f(get<Idx>(std::forward<ActSeq>(seq))), 0)... };
            }
            template <class ActSeq, class F>
            static void apply(ActSeq&& seq, F&& f)
            {
                do_apply(std::forward<ActSeq>(seq), std::forward<F>(f), indices{});
            }
        };
    } // namespace detail

    template <class Seq, typename F>
    void foreach (Seq&& seq, F && f)
    {
        using clean_seq = std::decay_t<Seq>;
        detail::foreach<clean_seq>::apply(std::forward<Seq>(seq), std::forward<F>(f));
    }

} // namespace tmp
} // namespace otf2

#endif // INCLUDE_OTF2XX_TMP_RUNTIME_HPP
