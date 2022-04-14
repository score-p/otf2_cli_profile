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

#ifndef INCLUDE_OTF2XX_INTRUSIVE_PTR_HPP
#define INCLUDE_OTF2XX_INTRUSIVE_PTR_HPP

#include <utility>

namespace otf2
{

/**
 * \brief shared_ptr using the contained class for ref counting
 *
 * This is similar to shared_ptr but requires the contained type to define
 * the functions `retain()` to increase the ref counter and
 * `release()` to decrease it and return the new value.
 *
 * This is very similar to boost::intrusive_ptr.
 *
 * \tparam T contained type
 */
template <class T>
class intrusive_ptr
{
public:
    intrusive_ptr() : data_(nullptr)
    {
    }

    explicit intrusive_ptr(T* data) : data_(data)
    {
        if (data_)
            data_->retain();
    }

    intrusive_ptr(const intrusive_ptr& other) : intrusive_ptr(other.get())
    {
    }

    intrusive_ptr(intrusive_ptr&& other) : intrusive_ptr()
    {
        this->swap(other);
    }

    ~intrusive_ptr()
    {
        if (data_ && data_->release() == 0u)
            delete data_;
    }

    intrusive_ptr& operator=(const intrusive_ptr& other)
    {
        if (data_ == other.data_)
            return *this;
        intrusive_ptr(other).swap(*this);
        return *this;
    }

    intrusive_ptr& operator=(intrusive_ptr&& other)
    {
        intrusive_ptr(std::move(other)).swap(*this);
        return *this;
    }

    void swap(intrusive_ptr& other)
    {
        std::swap(data_, other.data_);
    }

    /**
     * \brief Returns whether the contained pointer is not nullptr
     *
     * \returns true or false
     */
    explicit operator bool() const
    {
        return data_ != nullptr;
    }

    T& operator*() const
    {
        return *data_;
    }

    T* operator->() const
    {
        return data_;
    }

    T* get() const
    {
        return data_;
    }

private:
    T* data_;
};

template <class T>
inline bool operator==(const intrusive_ptr<T>& a, const intrusive_ptr<T>& b)
{
    return a.get() == b.get();
}

template <class T>
inline bool operator!=(const intrusive_ptr<T>& a, const intrusive_ptr<T>& b)
{
    return !(a == b);
}
template <class T>
inline void swap(intrusive_ptr<T>& a, intrusive_ptr<T>& b)
{
    a.swap(b);
}
} // namespace otf2

#endif // INCLUDE_OTF2XX_INTRUSIVE_PTR_HPP
