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

/*
 * otf2xx - reference.hpp
 *
 * Copyright (c) 2014 TU Dresden
 *
 * All rights reserved.
 *
 * author: Mario Bielert <mario.bielert@tu-dresden.de>
 *
 */

#ifndef INCLUDE_OTF2XX_REFERENCE_GENERATOR_HPP
#define INCLUDE_OTF2XX_REFERENCE_GENERATOR_HPP

#include <otf2xx/fwd.hpp>
#include <otf2xx/reference.hpp>
#include <otf2xx/tmp/typelist.hpp>
#include <otf2xx/traits/definition.hpp>
#include <otf2xx/traits/reference.hpp>

#include <otf2xx/exception.hpp>

#include <cassert>
#include <type_traits>

namespace otf2
{

/**
 * @brief gives a free reference number for a set of definitions
 *
 * This class generates free reference numbers for definitons. For this
 * task, it needs to know every used reference number first.
 *
 * Therefore you need to register every definition.
 *
 * \note The algorithm for generating a number is undefined.
 *
 * \tparam Definition The definition for which it should generate numbers
 */
template <typename RefType>
class reference_generator
{
public:
    typedef RefType ref_type;

    template <typename Definition>
    void register_definition(const Definition& def)
    {
        static_assert(std::is_constructible<typename Definition::reference_type, RefType>::value,
                      "Trying to register a definition with a different id space");

        register_reference(static_cast<RefType>(def.ref()));
    }

    void register_reference(ref_type ref)
    {
        if (old_max == ref_type::undefined())
        {
            old_max = ref.get();
            return;
        }

        using std::max;

        old_max = max(ref.get(), old_max);
    }

    template <typename RefType2 = ref_type>
    RefType2 next()
    {
        static_assert(std::is_constructible<RefType2, ref_type>::value,
                      "Trying to get a reference for a definition with a different id space");

        if (ref_type::undefined() == old_max + 1)
        {
            make_exception("Cannot generate a new unused reference number");
        }

        return ++old_max;
    }

    template <typename RefType2 = ref_type>
    RefType2 peak()
    {
        static_assert(std::is_constructible<RefType2, ref_type>::value,
                      "Trying to get a reference for a definition with a different id space");
        return old_max + 1;
    }

private:
    typename ref_type::ref_type old_max = -1;
};

class trace_reference_generator
{
    template <typename Tag>
    struct make_generator
    {
        using type = reference_generator<otf2::reference_impl<Tag, Tag>>;
    };

    using generators =
        tmp::apply_t<tmp::transform_t<traits::referable_definitions_base, make_generator>,
                     std::tuple>;

    template <class Definition>
    auto& get_generator()
    {
        using generator = typename make_generator<typename Definition::tag_type>::type;
        static_assert(tmp::contains<generators, generator>(),
                      "Cannot get a generator for this definition!");
        return std::get<generator>(ref_generators_);
    }

public:
    template <class Definition>
    void register_definition(const Definition& def)
    {
        get_generator<Definition>().register_definition(def);
    }

    template <typename Definition>
    void operator()(const Definition& def)
    {
        register_definition(def);
    }

    template <typename Definition>
    typename Definition::reference_type next()
    {
        // TMP-code-obfuscator was here
        return get_generator<Definition>().template next<typename Definition::reference_type>();
    }

    template <typename Definition>
    typename Definition::reference_type peak()
    {
        // TMP-code-obfuscator was here
        return get_generator<Definition>().template peak<typename Definition::reference_type>();
    }

private:
    /// std::tuple of reference_generator for each definition (tag)
    generators ref_generators_;
};

} // namespace otf2

#endif // INCLUDE_OTF2XX_REFERENCE_GENERATOR_HPP
