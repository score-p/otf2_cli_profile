/*
 * This file is part of otf2xx (https://github.com/tud-zih-energy/otf2xx)
 * otf2xx - A wrapper for the Open Trace Format 2 library
 *
 * Copyright (c) 2013-2017, Technische Universität Dresden, Germany
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

#ifndef INCLUDE_OTF2XX_ATTRIBUTE_VALUE_HPP
#define INCLUDE_OTF2XX_ATTRIBUTE_VALUE_HPP

#include <otf2xx/definition/definitions.hpp>
#include <otf2xx/exception.hpp>

#include <otf2/OTF2_AttributeValue.h>

#include <memory>

namespace otf2
{
class attribute_value
{
public:
    using type_type = otf2::common::type;

    attribute_value() : type_(type_type::none)
    {
        value_.uint64 = 0;
    }

    attribute_value(type_type type, OTF2_AttributeValue value) : value_(value), type_(type)
    {
    }

    attribute_value(const attribute_value&) = default;
    attribute_value(attribute_value&&) = default;

    attribute_value& operator=(const attribute_value&) = default;
    attribute_value& operator=(attribute_value&&) = default;

public:
    attribute_value(int8_t value) : type_(type_type::int8)
    {
        value_.int8 = value;
    }

    attribute_value(int16_t value) : type_(type_type::int16)
    {
        value_.int16 = value;
    }

    attribute_value(int32_t value) : type_(type_type::int32)
    {
        value_.int32 = value;
    }

    attribute_value(int64_t value) : type_(type_type::int64)
    {
        value_.int64 = value;
    }

    attribute_value(uint8_t value) : type_(type_type::uint8)
    {
        value_.uint8 = value;
    }

    attribute_value(uint16_t value) : type_(type_type::uint16)
    {
        value_.uint16 = value;
    }

    attribute_value(uint32_t value) : type_(type_type::uint32)
    {
        value_.uint32 = value;
    }

    attribute_value(uint64_t value) : type_(type_type::uint64)
    {
        value_.uint64 = value;
    }

    attribute_value(float value) : type_(type_type::Float)
    {
        value_.float32 = value;
    }

    attribute_value(double value) : type_(type_type::Double)
    {
        value_.float64 = value;
    }

    attribute_value(const otf2::definition::string& def) : type_(type_type::string)
    {
        value_.stringRef = def.ref();
    }

    attribute_value(const otf2::definition::location& def) : type_(type_type::location)
    {
        value_.locationRef = def.ref();
    }

    attribute_value(const otf2::definition::region& def) : type_(type_type::region)
    {
        value_.regionRef = def.ref();
    }

    template <class MemberType, otf2::common::group_type GroupType>
    attribute_value(const otf2::definition::group<MemberType, GroupType>& def)
    : type_(type_type::group)
    {
        value_.groupRef = def.ref();
    }

    attribute_value(const otf2::definition::metric_class& def) : type_(type_type::metric)
    {
        value_.metricRef = def.ref();
    }

    attribute_value(const otf2::definition::metric_instance& def) : type_(type_type::metric)
    {
        value_.metricRef = def.ref();
    }

    attribute_value(const otf2::definition::comm& def) : type_(type_type::comm)
    {
        value_.commRef = def.ref();
    }

    attribute_value(const otf2::definition::parameter& def) : type_(type_type::parameter)
    {
        value_.parameterRef = def.ref();
    }

    attribute_value(const otf2::definition::source_code_location& def)
    : type_(type_type::source_code_location)
    {
        value_.sourceCodeLocationRef = def.ref();
    }

    attribute_value(const otf2::definition::calling_context& def)
    : type_(type_type::calling_context)
    {
        value_.callingContextRef = def.ref();
    }

    attribute_value(const otf2::definition::interrupt_generator& def)
    : type_(type_type::interrupt_generator)
    {
        value_.interruptGeneratorRef = def.ref();
    }

    attribute_value(const otf2::definition::io_file& def) : type_(type_type::io_file)
    {
        value_.ioFileRef = def.ref();
    }

    attribute_value(const otf2::definition::io_directory& def) : type_(type_type::io_file)
    {
        value_.ioFileRef = def.ref();
    }

    attribute_value(const otf2::definition::io_handle& def) : type_(type_type::io_handle)
    {
        value_.ioHandleRef = def.ref();
    }

public:
    attribute_value& operator=(int8_t value)
    {
        type_ = type_type::int8;
        value_.int8 = value;

        return *this;
    }

    attribute_value& operator=(int16_t value)
    {
        type_ = type_type::int16;
        value_.int16 = value;

        return *this;
    }

    attribute_value& operator=(int32_t value)
    {
        type_ = type_type::int32;
        value_.int32 = value;

        return *this;
    }

    attribute_value& operator=(int64_t value)
    {
        type_ = type_type::int64;
        value_.int64 = value;

        return *this;
    }

    attribute_value& operator=(uint8_t value)
    {
        type_ = type_type::uint8;
        value_.uint8 = value;

        return *this;
    }

    attribute_value& operator=(uint16_t value)
    {
        type_ = type_type::uint16;
        value_.uint16 = value;

        return *this;
    }

    attribute_value& operator=(uint32_t value)
    {
        type_ = type_type::uint32;
        value_.uint32 = value;

        return *this;
    }

    attribute_value& operator=(uint64_t value)
    {
        type_ = type_type::uint64;
        value_.uint64 = value;

        return *this;
    }

    attribute_value& operator=(float value)
    {
        type_ = type_type::Float;
        value_.float32 = value;

        return *this;
    }

    attribute_value& operator=(double value)
    {
        type_ = type_type::Double;
        value_.float64 = value;

        return *this;
    }

    attribute_value& operator=(const otf2::definition::string& def)
    {
        type_ = type_type::string;
        value_.stringRef = def.ref();

        return *this;
    }

    attribute_value& operator=(const otf2::definition::location& def)
    {
        type_ = type_type::location;
        value_.locationRef = def.ref();

        return *this;
    }

    attribute_value& operator=(const otf2::definition::region& def)
    {
        type_ = type_type::region;
        value_.regionRef = def.ref();

        return *this;
    }

    template <class MemberType, otf2::common::group_type GroupType>
    attribute_value& operator=(const otf2::definition::group<MemberType, GroupType>& def)
    {
        type_ = type_type::group;
        value_.groupRef = def.ref();

        return *this;
    }

    attribute_value& operator=(const otf2::definition::metric_class& def)
    {
        type_ = type_type::metric;
        value_.metricRef = def.ref();

        return *this;
    }

    attribute_value& operator=(const otf2::definition::metric_instance& def)
    {
        type_ = type_type::metric;
        value_.metricRef = def.ref();

        return *this;
    }

    attribute_value& operator=(const otf2::definition::comm& def)
    {
        type_ = type_type::comm;
        value_.commRef = def.ref();

        return *this;
    }

    attribute_value& operator=(const otf2::definition::parameter& def)
    {
        type_ = type_type::parameter;
        value_.parameterRef = def.ref();

        return *this;
    }

    attribute_value& operator=(const otf2::definition::source_code_location& def)
    {
        type_ = type_type::source_code_location;
        value_.sourceCodeLocationRef = def.ref();

        return *this;
    }

    attribute_value& operator=(const otf2::definition::calling_context& def)
    {
        type_ = type_type::calling_context;
        value_.callingContextRef = def.ref();

        return *this;
    }

    attribute_value& operator=(const otf2::definition::interrupt_generator& def)
    {
        type_ = type_type::interrupt_generator;
        value_.interruptGeneratorRef = def.ref();

        return *this;
    }

    attribute_value& operator=(const otf2::definition::io_file& def)
    {
        type_ = type_type::io_file;
        value_.ioFileRef = def.ref();

        return *this;
    }

    attribute_value& operator=(const otf2::definition::io_directory& def)
    {
        type_ = type_type::io_file;
        value_.ioFileRef = def.ref();

        return *this;
    }

    attribute_value& operator=(const otf2::definition::io_handle& def)
    {
        type_ = type_type::io_handle;
        value_.ioHandleRef = def.ref();

        return *this;
    }

public:
    type_type type() const
    {
        return type_;
    }

    OTF2_AttributeValue value() const
    {
        return value_;
    }

private:
    OTF2_AttributeValue value_;
    type_type type_;
};
} // namespace otf2

#endif // INCLUDE_OTF2XX_ATTRIBUTE_VALUE_HPP
