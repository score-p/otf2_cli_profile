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

#ifndef INCLUDE_OTF2XX_EXCEPTION_HPP
#define INCLUDE_OTF2XX_EXCEPTION_HPP

#include <otf2/OTF2_ErrorCodes.h>

#include <sstream>
#include <stdexcept>
#include <string>

namespace otf2
{

struct exception : std::runtime_error
{
    explicit exception(const std::string& arg) : std::runtime_error(arg)
    {
    }
};

namespace detail
{

    template <typename Arg, typename... Args>
    class make_exception
    {
    public:
        void operator()(std::stringstream& msg, Arg&& arg, Args&&... args)
        {
            msg << std::forward<Arg>(arg);
            make_exception<Args...>()(msg, std::forward<Args>(args)...);
        }
    };

    template <typename Arg>
    class make_exception<Arg>
    {
    public:
        void operator()(std::stringstream& msg, Arg&& arg)
        {
            msg << std::forward<Arg>(arg);
        }
    };
} // namespace detail

template <typename... Args>
[[noreturn]] inline void make_exception(Args&&... args)
{
    std::stringstream msg;

    detail::make_exception<Args...>()(msg, std::forward<Args>(args)...);

    throw exception(msg.str());
}

template <typename... Args>
inline void make_otf2_exception(OTF2_ErrorCode code, Args&&... args)
{
    make_exception(OTF2_Error_GetName(code), ": ", OTF2_Error_GetDescription(code), "\n",
                   std::forward<Args>(args)...);
}

template <typename... Args>
void inline check(OTF2_ErrorCode code, Args&&... args)
{
    if (code != OTF2_SUCCESS)
    {
        make_otf2_exception(code, std::forward<Args>(args)...);
    }
}

} // namespace otf2

#endif // INCLUDE_OTF2XX_EXCEPTION_HPP
