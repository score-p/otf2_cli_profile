/*
 * This file is part of otf2xx (https://github.com/tud-zih-energy/otf2xx)
 * otf2xx - A wrapper for the Open Trace Format 2 library
 *
 * Copyright (c) 2013-2018, Technische Universität Dresden, Germany
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

#ifndef INCLUDE_OTF2XX_REGISTRY_HPP
#define INCLUDE_OTF2XX_REGISTRY_HPP

#pragma once

#include <otf2xx/definition/definitions.hpp>
#include <otf2xx/fwd.hpp>
#include <otf2xx/reference.hpp>
#include <otf2xx/reference_generator.hpp>

#include <type_traits>
#include <utility>

#include <cassert>

namespace otf2
{

template <typename Definition>
class definition_holder
{
    static_assert(otf2::traits::is_referable_definition<Definition>::value, "Whoopsy.");

public:
    definition_holder(otf2::trace_reference_generator& refs) : refs_(refs)
    {
    }

public:
    const Definition& operator[](typename Definition::reference_type ref) const
    {
        return definitions_[ref];
    }

    Definition& operator[](typename Definition::reference_type ref)
    {
        return definitions_[ref];
    }

    void operator()(const Definition& def)
    {
        assert(def.ref() != Definition::reference_type::undefined());

        definitions_.add_definition(def);
        refs_.register_definition(def);
    }

    void operator()(otf2::definition::detail::weak_ref<Definition> ref)
    {
        assert(ref.ref() != Definition::reference_type::undefined());

        auto def = ref.lock();
        refs_.register_definition(def);
        definitions_.add_definition(std::move(def));
    }

    template <typename Arg, typename... Args>
    std::enable_if_t<!std::is_convertible<Arg, typename Definition::reference_type>::value &&
                         std::is_constructible<Definition, typename Definition::reference_type, Arg,
                                               Args...>::value,
                     Definition&>
    create(Arg&& arg, Args&&... args)
    {
        return definitions_.emplace(refs_.next<Definition>(), std::forward<Arg>(arg),
                                    std::forward<Args>(args)...);
    }

    template <typename RefType, typename... Args>
    std::enable_if_t<std::is_convertible<RefType, typename Definition::reference_type>::value,
                     Definition&>
    create(RefType&& ref, Args&&... args)
    {
        // TODO I fucking bet that some day there will be a definition, where this is well-formed in
        // the case you wanted to omit the ref FeelsBadMan
        auto& def = definitions_.emplace(ref, std::forward<Args>(args)...);
        refs_.register_definition(def);
        return def;
    }

    template <typename Arg, typename... Args>
    std::enable_if_t<!std::is_convertible<Arg, typename Definition::reference_type>::value &&
                         std::is_constructible<Definition, typename Definition::reference_type, Arg,
                                               Args...>::value,
                     Definition&>
    emplace(Arg&& arg, Args&&... args)
    {
        return definitions_.emplace(refs_.next<Definition>(), std::forward<Arg>(arg),
                                    std::forward<Args>(args)...);
    }

    template <typename RefType, typename... Args>
    std::enable_if_t<std::is_convertible<RefType, typename Definition::reference_type>::value,
                     Definition&>
    emplace(RefType&& ref, Args&&... args)
    {
        // TODO I fucking bet that some day there will be a definition, where this is well-formed in
        // the case you wanted to omit the ref FeelsBadMan

        if (!definitions_.count(ref))
        {
            auto& def = definitions_.emplace(ref, std::forward<Args>(args)...);
            refs_.register_definition(def);

            return def;
        }
        else
        {
            return definitions_[ref];
        }
    }

    bool has(typename Definition::reference_type ref) const
    {
        return definitions_.count(ref) > 0;
    }

    Definition& find(typename Definition::reference_type ref)
    {
        auto it = definitions_.find(ref);
        return it != definitions_.end() ? *it : definitions_[ref.undefined()];
    }

    const Definition& find(typename Definition::reference_type ref) const
    {
        auto it = definitions_.find(ref);
        return it != definitions_.end() ? *it : definitions_[ref.undefined()];
    }

    const otf2::definition::container<Definition>& data() const
    {
        return definitions_;
    }

    auto begin() const
    {
        return definitions_.begin();
    }

    auto end() const
    {
        return definitions_.end();
    }

protected:
    otf2::definition::container<Definition> definitions_;
    otf2::trace_reference_generator& refs_;
};

template <typename Definition, typename... KeyList>
class lookup_definition_holder : public definition_holder<Definition>
{
    template <class T, class Tuple>
    struct Index;

    template <class T, class... Types>
    struct Index<T, std::tuple<T, Types...>>
    {
        static const std::size_t value = 0;
    };

    template <class T, class U, class... Types>
    struct Index<T, std::tuple<U, Types...>>
    {
        static const std::size_t value = 1 + Index<T, std::tuple<Types...>>::value;
    };

    template <typename T, typename Tuple>
    struct has_type;

    template <typename T>
    struct has_type<T, std::tuple<>> : std::false_type
    {
    };

    template <typename T, typename U, typename... Ts>
    struct has_type<T, std::tuple<U, Ts...>> : has_type<T, std::tuple<Ts...>>
    {
    };

    template <typename T, typename... Ts>
    struct has_type<T, std::tuple<T, Ts...>> : std::true_type
    {
    };

    using key_list = std::tuple<KeyList...>;
    using base = definition_holder<Definition>;

public:
    lookup_definition_holder(otf2::trace_reference_generator& refs) : base(refs)
    {
    }

    using base::operator[];
    using base::operator();
    using base::create;
    using base::find;
    using base::has;

    template <typename Key>
    std::enable_if_t<has_type<Key, key_list>::value, const Definition&> operator[](Key key) const
    {
        return std::get<Index<Key, key_list>::value>(lookup_maps_).at(key.key);
    }

    template <typename Key>
    std::enable_if_t<has_type<Key, key_list>::value, Definition&> operator[](Key key)
    {
        return std::get<Index<Key, key_list>::value>(lookup_maps_).at(key.key);
    }

    template <typename Key>
    std::enable_if_t<has_type<Key, key_list>::value> operator()(Key key, const Definition& def)
    {
        assert(def.ref() != Definition::reference_type::undefined());

        std::get<Index<Key, key_list>::value>(lookup_maps_).emplace(key.key, def);
        this->definitions_.add_definition(def);
        this->refs_.register_definition(def);
    }

    template <typename Key>
    std::enable_if_t<has_type<Key, key_list>::value>
    operator()(Key key, otf2::definition::detail::weak_ref<Definition> ref)
    {
        assert(ref.ref() != Definition::reference_type::undefined());

        auto def = ref.lock();
        this->refs_.register_definition(def);
        this->definitions_.add_definition(def);
        std::get<Index<Key, key_list>::value>(lookup_maps_).emplace(key.key, std::move(def));
    }

    template <typename Key, typename... Args>
    std::enable_if_t<has_type<Key, key_list>::value, Definition&> emplace(Key key, Args&&... args)
    {
        if (!has(key))
        {
            return create(key, std::forward<Args>(args)...);
        }

        return find(key);
    }

    template <typename Key, typename... Args>
    std::enable_if_t<has_type<Key, key_list>::value, Definition&> create(Key key, Args&&... args)
    {
        auto result = std::get<Index<Key, key_list>::value>(lookup_maps_)
                          .emplace(std::piecewise_construct, std::forward_as_tuple(key.key),
                                   std::forward_as_tuple(this->refs_.template next<Definition>(),
                                                         std::forward<Args>(args)...));

        if (!result.second)
        {
            make_exception("Tried to create an already existing definition");
        }

        this->definitions_.add_definition(result.first->second);

        return result.first->second;
    }

    template <typename Key, typename RefType, typename... Args>
    std::enable_if_t<has_type<Key, key_list>::value &&
                         std::is_convertible<RefType, typename Definition::reference_type>::value,
                     Definition&>
    create(Key key, RefType ref, Args&&... args)
    {
        // TODO I fucking bet that some day there will be a definition, where this is well-formed in
        // the case you wanted to omit the ref FeelsBadMan
        auto result = std::get<Index<Key, key_list>::value>(lookup_maps_)
                          .emplace(std::piecewise_construct, std::forward_as_tuple(key.key),
                                   std::forward_as_tuple(ref, std::forward<Args>(args)...));

        auto& def = result.first->second;

        this->definitions_.add_definition(def);
        this->refs_.register_definition(def);
        return def;
    }

    template <typename Key>
    std::enable_if_t<has_type<Key, key_list>::value, bool> has(Key key) const
    {
        return std::get<Index<Key, key_list>::value>(lookup_maps_).count(key.key) > 0;
    }

    template <typename Key>
    std::enable_if_t<has_type<Key, key_list>::value, const Definition&> find(Key key) const
    {
        const auto& definitions = std::get<Index<Key, key_list>::value>(lookup_maps_);
        auto it = definitions.find(key.key);
        return it != definitions.end() ? it->second :
                                         (*this)[otf2::reference<Definition>::undefined()];
    }

    template <typename Key>
    std::enable_if_t<has_type<Key, key_list>::value, Definition&> find(Key key)
    {
        auto& definitions = std::get<Index<Key, key_list>::value>(lookup_maps_);
        auto it = definitions.find(key.key);
        return it != definitions.end() ? it->second :
                                         (*this)[otf2::reference<Definition>::undefined()];
    }

private:
    std::tuple<std::map<typename KeyList::key_type, Definition>...> lookup_maps_;
};

template <typename Property>
class property_holder
{
public:
    property_holder(otf2::trace_reference_generator&)
    {
    }

    void operator()(const Property& def)
    {
        properties_.emplace(def);
    }

    template <typename... Args>
    Property& create(Args&&... args)
    {
        return properties_.emplace(std::forward<Args>(args)...);
    }

    const otf2::definition::container<Property>& data() const
    {
        return properties_;
    }

    auto begin() const
    {
        return properties_.begin();
    }

    auto end() const
    {
        return properties_.end();
    }

private:
    otf2::definition::container<Property> properties_;
};

namespace detail
{
    template <typename Definition, template <typename> class DefinitionHolder,
              template <typename> class PropertyHolder, typename = void>
    struct holder_selection_helper;

    template <typename Definition, template <typename> class DefinitionHolder,
              template <typename> class PropertyHolder>
    struct holder_selection_helper<
        Definition, DefinitionHolder, PropertyHolder,
        typename std::enable_if<!traits::is_referable_definition<Definition>::value>::type>
    {
        using type = PropertyHolder<Definition>;
    };

    template <typename Definition, template <typename> class DefinitionHolder,
              template <typename> class PropertyHolder>
    struct holder_selection_helper<
        Definition, DefinitionHolder, PropertyHolder,
        typename std::enable_if<traits::is_referable_definition<Definition>::value>::type>
    {
        using type = DefinitionHolder<Definition>;
    };
} // namespace detail

template <typename Definition>
struct get_default_holder
{
    using type = typename detail::holder_selection_helper<Definition, definition_holder,
                                                          property_holder>::type;
};

template <typename Registry, typename Result, typename Definition, typename... Variants>
struct get_variant_helper
{
    const Registry& reg;
    get_variant_helper(const Registry& reg) : reg(reg)
    {
    }

    template <typename Key>
    Result operator()(const Key& key)
    {
        if (reg.template has<Definition>(key))
        {
            return reg.template get<Definition>(key);
        }
        return get_variant_helper<Registry, Result, Variants...>(reg)(key);
    }
};

template <typename Registry, typename Result, typename Definition>
struct get_variant_helper<Registry, Result, Definition>
{
    const Registry& reg;
    get_variant_helper(const Registry& reg) : reg(reg)
    {
    }

    template <typename Key>
    Result operator()(const Key& key)
    {
        return reg.template get<Definition>(key);
    }
};

template <template <typename> class GetHolderForDefinition>
class lookup_registry
{
    using self = lookup_registry<GetHolderForDefinition>;

    using holders =
        tmp::apply_t<tmp::transform_t<traits::usable_definitions, GetHolderForDefinition>,
                     std::tuple>;

    template <class Definition>
    auto& get_holder()
    {
        using holder = typename GetHolderForDefinition<Definition>::type;
        static_assert(tmp::contains<holders, holder>(), "Cannot get a holder for this definition!");
        return std::get<holder>(holders_);
    }

    template <class Definition>
    const auto& get_holder() const
    {
        using holder = typename GetHolderForDefinition<Definition>::type;
        static_assert(tmp::contains<holders, holder>(), "Cannot get a holder for this definition!");
        return std::get<holder>(holders_);
    }

    template <typename Holders>
    class construct_holders;

    template <typename... Holders>
    class construct_holders<std::tuple<Holders...>>
    {
    public:
        template <typename... Args>
        auto operator()(Args&&... args)
        {
            return std::make_tuple(Holders{ args... }...);
        }
    };

public:
    lookup_registry() : holders_(construct_holders<holders>()(refs_))
    {
    }

public:
    template <typename Definition>
    const auto& all() const
    {
        return get_holder<Definition>();
    }

    template <typename Definition, typename... Args>
    auto& create(Args&&... args)
    {
        return get_holder<Definition>().create(std::forward<Args>(args)...);
    }

    template <typename Definition, typename Key>
    const auto& get(const Key& key) const
    {
        return get_holder<Definition>()[key];
    }

    template <typename Definition, typename... Args>
    auto& emplace(Args&&... args)
    {
        return get_holder<Definition>().emplace(std::forward<Args>(args)...);
    }

    template <typename... Variants, typename Key>
    std::variant<Variants...> get_variant(const Key& key)
    {
        return get_variant_helper<self, std::variant<Variants...>, Variants...>(*this)(key);
    }

    template <typename... Variants, typename Key>
    std::variant<otf2::definition::weak_ref<Variants>...> get_variant_weak(const Key& key)
    {
        return get_variant_helper<self, std::variant<otf2::definition::weak_ref<Variants>...>,
                                  Variants...>(*this)(key);
    }

public:
    template <typename Definition, typename Key>
    auto& get(const Key& key)
    {
        return get_holder<Definition>()[key];
    }

    template <typename Definition, typename Key>
    bool has(const Key& key) const
    {
        return get_holder<Definition>().has(key);
    }

    template <typename Definition, typename Key>
    auto& find(const Key& key)
    {
        return get_holder<Definition>().find(key);
    }
    template <typename Definition, typename Key>
    const auto& find(const Key& key) const
    {
        return get_holder<Definition>().find(key);
    }

public:
    template <typename Definition, typename Key>
    void register_definition(Key&& key, Definition&& def)
    {
        get_holder<Definition>()(std::forward<Key>(key), std::forward<Definition>(def));
    }

    template <typename Definition>
    void register_definition(Definition&& def)
    {
        get_holder<std::remove_cv_t<std::remove_reference_t<Definition>>>()(
            std::forward<Definition>(def));
    }

public:
    const holders& get_holders() const
    {
        return holders_;
    }

private:
    trace_reference_generator refs_;

    holders holders_;
};

// template <typename Definition, typename... KeyList>
// class registry_view
// {
// public:
//     registry_view(otf2::registry& reg) : reg_(reg)
//     {
//     }
//
// public:
//     template <typename Key>
//     bool has(Key&& key) const
//     {
//         const auto& definitions = std::get<Index<Key, key_list>::value>(lookup_maps_);
//         return definitions.count(key);
//     }
//
//     template <typename Key>
//     bool get(Key&& key) const
//     {
//         const auto& definitions = std::get<Index<Key, key_list>::value>(lookup_maps_);
//         return definitions.at(key);
//     }
//
// private:
//     otf2::registry& reg_;
//     std::tuple<std::map<typename KeyList::key_type, Definition>...> lookup_maps_;
// };

} // namespace otf2

#endif // INCLUDE_OTF2XX_REGISTRY_HPP
