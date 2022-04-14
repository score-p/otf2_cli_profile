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

#ifndef INCLUDE_OTF2XX_ATRTIBUTE_LIST_HPP
#define INCLUDE_OTF2XX_ATRTIBUTE_LIST_HPP

#include <otf2xx/definition/definitions.hpp>
#include <otf2xx/exception.hpp>

#include <otf2/OTF2_AttributeList.h>

#include <memory>

namespace otf2
{

namespace detail
{
    inline OTF2_AttributeList* OTF2_AttributeList_Clone(OTF2_AttributeList const* const list)
    {
        if (list == nullptr)
            return nullptr;

        OTF2_AttributeList* result = OTF2_AttributeList_New();

        for (uint32_t i = 0; i < OTF2_AttributeList_GetNumberOfElements(list); ++i)
        {
            OTF2_AttributeRef attr;
            OTF2_Type type;
            OTF2_AttributeValue value;

            check(OTF2_AttributeList_GetAttributeByIndex(list, i, &attr, &type, &value),
                  "Couldn't get value from attribute list");

            check(OTF2_AttributeList_AddAttribute(result, attr, type, value),
                  "Couldn't add attribute value to attribute list");
        }

        return result;
    }
} // namespace detail

namespace detail
{
    using attribute_type = otf2::definition::attribute::attribute_type;

    template <attribute_type Type>
    struct add_attribute;

    template <>
    struct add_attribute<otf2::definition::attribute::attribute_type::attribute>
    {
        void operator()(OTF2_AttributeList* list, const otf2::definition::attribute& attribute,
                        const otf2::definition::attribute& value)
        {
            check(OTF2_AttributeList_AddAttributeRef(list, attribute.ref(), value.ref()),
                  "Couldn't add value to attribute list.");
        }
    };

    template <>
    struct add_attribute<otf2::definition::attribute::attribute_type::comm>
    {
        void operator()(OTF2_AttributeList* list, const otf2::definition::attribute& attribute,
                        const otf2::definition::comm& value)
        {
            check(OTF2_AttributeList_AddCommRef(list, attribute.ref(), value.ref()),
                  "Couldn't add value to attribute list.");
        }
    };

    template <>
    struct add_attribute<otf2::definition::attribute::attribute_type::Double>
    {
        void operator()(OTF2_AttributeList* list, const otf2::definition::attribute& attribute,
                        double value)
        {
            check(OTF2_AttributeList_AddDouble(list, attribute.ref(), value),
                  "Couldn't add value to attribute list.");
        }
    };

    template <>
    struct add_attribute<otf2::definition::attribute::attribute_type::Float>
    {
        void operator()(OTF2_AttributeList* list, const otf2::definition::attribute& attribute,
                        float value)
        {
            check(OTF2_AttributeList_AddFloat(list, attribute.ref(), value),
                  "Couldn't add value to attribute list.");
        }
    };

    //    template <typename T>
    //    class
    //    add_attribute<otf2::definition::attribute::attribute_type::group>
    //    {
    //        void operator()(OTF2_AttributeList* list,
    //                        const otf2::definition::attribute& attribute,
    //                        const otf2::definition::group<T>& value)
    //        {
    //            check(OTF2_AttributeList_AddGroupRef(list, attribute.ref(),
    //                                                 value.ref()),
    //                  "Couldn't add value to attribute list.");
    //        }
    //    };

    template <>
    struct add_attribute<otf2::definition::attribute::attribute_type::int8>
    {
        void operator()(OTF2_AttributeList* list, const otf2::definition::attribute& attribute,
                        std::int8_t value)
        {
            check(OTF2_AttributeList_AddInt8(list, attribute.ref(), value),
                  "Couldn't add value to attribute list.");
        }
    };

    template <>
    struct add_attribute<otf2::definition::attribute::attribute_type::int16>
    {
        void operator()(OTF2_AttributeList* list, const otf2::definition::attribute& attribute,
                        std::int16_t value)
        {
            check(OTF2_AttributeList_AddInt16(list, attribute.ref(), value),
                  "Couldn't add value to attribute list.");
        }
    };

    template <>
    struct add_attribute<otf2::definition::attribute::attribute_type::int32>
    {
        void operator()(OTF2_AttributeList* list, const otf2::definition::attribute& attribute,
                        std::int32_t value)
        {
            check(OTF2_AttributeList_AddInt32(list, attribute.ref(), value),
                  "Couldn't add value to attribute list.");
        }
    };

    template <>
    struct add_attribute<otf2::definition::attribute::attribute_type::int64>
    {
        void operator()(OTF2_AttributeList* list, const otf2::definition::attribute& attribute,
                        std::int64_t value)
        {
            check(OTF2_AttributeList_AddInt64(list, attribute.ref(), value),
                  "Couldn't add value to attribute list.");
        }
    };

    template <>
    struct add_attribute<otf2::definition::attribute::attribute_type::location>
    {
        void operator()(OTF2_AttributeList* list, const otf2::definition::attribute& attribute,
                        const otf2::definition::location& value)
        {
            check(OTF2_AttributeList_AddLocationRef(list, attribute.ref(), value.ref()),
                  "Couldn't add value to attribute list.");
        }
    };

    template <>
    struct add_attribute<otf2::definition::attribute::attribute_type::metric>
    {
        void operator()(OTF2_AttributeList* list, const otf2::definition::attribute& attribute,
                        const otf2::definition::metric_class& value)
        {
            check(OTF2_AttributeList_AddMetricRef(list, attribute.ref(), value.ref()),
                  "Couldn't add value to attribute list.");
        }
    };

    //    template <>
    //    class
    //    add_attribute<otf2::definition::attribute::attribute_type::metric>
    //    {
    //        void operator()(OTF2_AttributeList* list,
    //                        const otf2::definition::attribute& attribute,
    //                        const otf2::definition::metric_instance& value)
    //        {
    //            check(OTF2_AttributeList_AddMetricRef(list, attribute.ref(),
    //                                                  value.ref()),
    //                  "Couldn't add value to attribute list.");
    //        }
    //    };

    template <>
    struct add_attribute<otf2::definition::attribute::attribute_type::parameter>
    {
        void operator()(OTF2_AttributeList* list, const otf2::definition::attribute& attribute,
                        const otf2::definition::parameter& value)
        {
            check(OTF2_AttributeList_AddParameterRef(list, attribute.ref(), value.ref()),
                  "Couldn't add value to attribute list.");
        }
    };

    template <>
    struct add_attribute<otf2::definition::attribute::attribute_type::region>
    {
        void operator()(OTF2_AttributeList* list, const otf2::definition::attribute& attribute,
                        const otf2::definition::region& value)
        {
            check(OTF2_AttributeList_AddRegionRef(list, attribute.ref(), value.ref()),
                  "Couldn't add value to attribute list.");
        }
    };

    template <>
    struct add_attribute<otf2::definition::attribute::attribute_type::string>
    {
        void operator()(OTF2_AttributeList* list, const otf2::definition::attribute& attribute,
                        const otf2::definition::string& value)
        {
            check(OTF2_AttributeList_AddStringRef(list, attribute.ref(), value.ref()),
                  "Couldn't add value to attribute list.");
        }
    };

    template <>
    struct add_attribute<otf2::definition::attribute::attribute_type::uint8>
    {
        void operator()(OTF2_AttributeList* list, const otf2::definition::attribute& attribute,
                        std::uint8_t value)
        {
            check(OTF2_AttributeList_AddUint8(list, attribute.ref(), value),
                  "Couldn't add value to attribute list.");
        }
    };

    template <>
    struct add_attribute<otf2::definition::attribute::attribute_type::uint16>
    {
        void operator()(OTF2_AttributeList* list, const otf2::definition::attribute& attribute,
                        std::uint16_t value)
        {
            check(OTF2_AttributeList_AddUint16(list, attribute.ref(), value),
                  "Couldn't add value to attribute list.");
        }
    };

    template <>
    struct add_attribute<otf2::definition::attribute::attribute_type::uint32>
    {
        void operator()(OTF2_AttributeList* list, const otf2::definition::attribute& attribute,
                        std::uint32_t value)
        {
            check(OTF2_AttributeList_AddUint32(list, attribute.ref(), value),
                  "Couldn't add value to attribute list.");
        }
    };

    template <>
    struct add_attribute<otf2::definition::attribute::attribute_type::uint64>
    {
        void operator()(OTF2_AttributeList* list, const otf2::definition::attribute& attribute,
                        std::uint64_t value)
        {
            check(OTF2_AttributeList_AddUint64(list, attribute.ref(), value),
                  "Couldn't add value to attribute list.");
        }
    };

    template <>
    struct add_attribute<otf2::definition::attribute::attribute_type::io_file>
    {
        void operator()(OTF2_AttributeList* list, const otf2::definition::attribute& attribute,
                        const otf2::definition::io_file& value)
        {
            check(OTF2_AttributeList_AddIoFileRef(list, attribute.ref(), value.ref()),
                  "Couldn't add value to attribute list.");
        }
    };

    template <>
    struct add_attribute<otf2::definition::attribute::attribute_type::io_handle>
    {
        void operator()(OTF2_AttributeList* list, const otf2::definition::attribute& attribute,
                        const otf2::definition::io_handle& value)
        {
            check(OTF2_AttributeList_AddIoHandleRef(list, attribute.ref(), value.ref()),
                  "Couldn't add value to attribute list.");
        }
    };
} // namespace detail
namespace detail
{
    using attribute_type = otf2::definition::attribute::attribute_type;

    template <attribute_type Type>
    struct get_attribute;

    template <>
    struct get_attribute<otf2::definition::attribute::attribute_type::attribute>
    {
        template <bool Test = false>
        void operator()(OTF2_AttributeList*, const otf2::definition::attribute&)
        {
            static_assert(Test, "Not implemented yet.");
        }
    };

    template <>
    struct get_attribute<otf2::definition::attribute::attribute_type::comm>
    {
        template <bool Test = false>
        void operator()(OTF2_AttributeList*, const otf2::definition::attribute&)
        {
            static_assert(Test, "Not implemented yet.");
        }
    };

    template <>
    struct get_attribute<otf2::definition::attribute::attribute_type::Double>
    {
        double operator()(OTF2_AttributeList* list, const otf2::definition::attribute& attribute)
        {
            double res;
            check(OTF2_AttributeList_GetDouble(list, attribute.ref(), &res),
                  "Couldn't get value from attribute list.");

            return res;
        }
    };

    template <>
    struct get_attribute<otf2::definition::attribute::attribute_type::Float>
    {
        float operator()(OTF2_AttributeList* list, const otf2::definition::attribute& attribute)
        {
            float res;
            check(OTF2_AttributeList_GetFloat(list, attribute.ref(), &res),
                  "Couldn't get value from attribute list.");
            return res;
        }
    };

    //    template <typename T>
    //    class
    //    get_attribute<otf2::definition::attribute::attribute_type::group>
    //    {
    //        void operator()(OTF2_AttributeList* list,
    //                        const otf2::definition::attribute& attribute,
    //                        const otf2::definition::group<T>& value)
    //        {
    //            check(OTF2_AttributeList_AddGroupRef(list, attribute.ref(),
    //                                                 value.ref()),
    //                  "Couldn't get value from attribute list.");
    //        }
    //    };

    template <>
    struct get_attribute<otf2::definition::attribute::attribute_type::int8>
    {
        std::int8_t operator()(OTF2_AttributeList* list,
                               const otf2::definition::attribute& attribute)
        {
            std::int8_t res;
            check(OTF2_AttributeList_GetInt8(list, attribute.ref(), &res),
                  "Couldn't get value from attribute list.");
            return res;
        }
    };

    template <>
    struct get_attribute<otf2::definition::attribute::attribute_type::int16>
    {
        std::int16_t operator()(OTF2_AttributeList* list,
                                const otf2::definition::attribute& attribute)
        {
            std::int16_t res;
            check(OTF2_AttributeList_GetInt16(list, attribute.ref(), &res),
                  "Couldn't get value from attribute list.");
            return res;
        }
    };

    template <>
    struct get_attribute<otf2::definition::attribute::attribute_type::int32>
    {
        std::int32_t operator()(OTF2_AttributeList* list,
                                const otf2::definition::attribute& attribute)
        {
            std::int32_t res;
            check(OTF2_AttributeList_GetInt32(list, attribute.ref(), &res),
                  "Couldn't get value from attribute list.");
            return res;
        }
    };

    template <>
    struct get_attribute<otf2::definition::attribute::attribute_type::int64>
    {
        std::int64_t operator()(OTF2_AttributeList* list,
                                const otf2::definition::attribute& attribute)
        {
            std::int64_t res;
            check(OTF2_AttributeList_GetInt64(list, attribute.ref(), &res),
                  "Couldn't get value from attribute list.");
            return res;
        }
    };

    template <>
    struct get_attribute<otf2::definition::attribute::attribute_type::location>
    {
        template <bool Test = false>
        void operator()(OTF2_AttributeList*, const otf2::definition::attribute&)
        {
            static_assert(Test, "Not implemented yet.");
        }
    };

    template <>
    struct get_attribute<otf2::definition::attribute::attribute_type::metric>
    {
        template <bool Test = false>
        void operator()(OTF2_AttributeList*, const otf2::definition::attribute&)
        {
            static_assert(Test, "Not implemented yet.");
        }
    };

    //    template <>
    //    class
    //    get_attribute<otf2::definition::attribute::attribute_type::metric>
    //    {
    //        void operator()(OTF2_AttributeList* list,
    //                        const otf2::definition::attribute& attribute,
    //                        const otf2::definition::metric_instance& value)
    //        {
    //            check(OTF2_AttributeList_AddMetricRef(list, attribute.ref(),
    //                                                  value.ref()),
    //                  "Couldn't get value from attribute list.");
    //        }
    //    };

    template <>
    struct get_attribute<otf2::definition::attribute::attribute_type::parameter>
    {
        template <bool Test = false>
        void operator()(OTF2_AttributeList*, const otf2::definition::attribute&)
        {
            static_assert(Test, "Not implemented yet.");
        }
    };

    template <>
    struct get_attribute<otf2::definition::attribute::attribute_type::region>
    {
        template <bool Test = false>
        void operator()(OTF2_AttributeList*, const otf2::definition::attribute&)
        {
            static_assert(Test, "Not implemented yet.");
        }
    };

    template <>
    struct get_attribute<otf2::definition::attribute::attribute_type::string>
    {
        template <bool Test = false>
        void operator()(OTF2_AttributeList*, const otf2::definition::attribute&)
        {
            static_assert(Test, "Not implemented yet.");
        }
    };

    template <>
    struct get_attribute<otf2::definition::attribute::attribute_type::uint8>
    {
        std::uint8_t operator()(OTF2_AttributeList* list,
                                const otf2::definition::attribute& attribute)
        {
            std::uint8_t res;
            check(OTF2_AttributeList_GetUint8(list, attribute.ref(), &res),
                  "Couldn't get value from attribute list.");
            return res;
        }
    };

    template <>
    struct get_attribute<otf2::definition::attribute::attribute_type::uint16>
    {
        std::uint16_t operator()(OTF2_AttributeList* list,
                                 const otf2::definition::attribute& attribute)
        {
            std::uint16_t res;
            check(OTF2_AttributeList_GetUint16(list, attribute.ref(), &res),
                  "Couldn't get value from attribute list.");
            return res;
        }
    };

    template <>
    struct get_attribute<otf2::definition::attribute::attribute_type::uint32>
    {
        std::uint32_t operator()(OTF2_AttributeList* list,
                                 const otf2::definition::attribute& attribute)
        {
            std::uint32_t res;
            check(OTF2_AttributeList_GetUint32(list, attribute.ref(), &res),
                  "Couldn't get value from attribute list.");
            return res;
        }
    };

    template <>
    struct get_attribute<otf2::definition::attribute::attribute_type::uint64>
    {
        std::uint64_t operator()(OTF2_AttributeList* list,
                                 const otf2::definition::attribute& attribute)
        {
            std::uint64_t res;
            check(OTF2_AttributeList_GetUint64(list, attribute.ref(), &res),
                  "Couldn't get value from attribute list.");
            return res;
        }
    };

    template <>
    struct get_attribute<otf2::definition::attribute::attribute_type::io_file>
    {
        template <bool Test = false>
        void operator()(OTF2_AttributeList*, const otf2::definition::attribute&)
        {
            static_assert(Test, "Not implemented yet.");
        }
    };

    template <>
    struct get_attribute<otf2::definition::attribute::attribute_type::io_handle>
    {
        template <bool Test = false>
        void operator()(OTF2_AttributeList*, const otf2::definition::attribute&)
        {
            static_assert(Test, "Not implemented yet.");
        }
    };
} // namespace detail

class attribute_list
{
public:
    using attribute_type = otf2::definition::attribute::attribute_type;

    attribute_list() : attribute_list(nullptr, false)
    {
    }

    explicit attribute_list(OTF2_AttributeList* list, bool take_ownership = true)
    : attribute_list_(list), owned_(take_ownership)
    {
    }

    attribute_list(const attribute_list&) = delete;
    attribute_list& operator=(const attribute_list&) = delete;

    attribute_list(attribute_list&& other)
    : attribute_list_(other.attribute_list_), owned_(other.owned_)
    {
        other.owned_ = false;
    }

    attribute_list& operator=(attribute_list&& other)
    {
        std::swap(attribute_list_, other.attribute_list_);
        std::swap(other.owned_, owned_);

        return *this;
    }

    template <attribute_type Type, typename T>
    void add(const otf2::definition::attribute& attribute, T value)
    {
        if (attribute_list_ == nullptr)
        {
            allocate_list();
        }
        else if (!owned_)
        {
            attribute_list_ = detail::OTF2_AttributeList_Clone(attribute_list_);
            owned_ = true;
        }

        detail::add_attribute<Type>()(this->get(), attribute, value);
    }

    bool has(const otf2::definition::attribute& attribute) const
    {
        return attribute_list_ != nullptr ?
                   OTF2_AttributeList_TestAttributeByID(attribute_list_, attribute.ref().get()) :
                   false;
    }

    template <attribute_type Type>
    auto get(const otf2::definition::attribute& attribute) const
    {
        return detail::get_attribute<Type>()(this->get(), attribute);
    }

    ~attribute_list()
    {
        if (attribute_list_ != nullptr && owned_)
            OTF2_AttributeList_Delete(attribute_list_);
    }

    OTF2_AttributeList* get() const
    {
        return attribute_list_;
    }

    attribute_list clone() const
    {
        if (attribute_list_ == nullptr ||
            OTF2_AttributeList_GetNumberOfElements(attribute_list_) == 0)
        {
            return attribute_list();
        }
        return attribute_list(detail::OTF2_AttributeList_Clone(attribute_list_));
    }

private:
    void allocate_list()
    {
        assert(attribute_list_ == nullptr);

        attribute_list_ = OTF2_AttributeList_New();

        if (attribute_list_ == nullptr)
        {
            make_exception("Couldn't create a new attribute list.");
        }

        owned_ = true;
    }

private:
    OTF2_AttributeList* attribute_list_;
    bool owned_;
};
} // namespace otf2

#endif // INCLUDE_OTF2XX_ATRTIBUTE_LIST_HPP
