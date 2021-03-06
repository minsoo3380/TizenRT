/****************************************************************************
 *
 * Copyright 2018 Samsung Electronics All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 *
 ****************************************************************************/
//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// template<> struct char_traits<char16_t>

// typedef char16_t       char_type;
// typedef uint_least16_t int_type;
// typedef streamoff      off_type;
// typedef u16streampos   pos_type;
// typedef mbstate_t      state_type;

#include <string>
#include <type_traits>
#include <cstdint>
#include "test_macros.h"
#include "libcxx_tc_common.h"

int tc_libcxx_strings_char_traits_specializations_char16_t_types(void)
{
#ifndef _LIBCPP_HAS_NO_UNICODE_CHARS
    static_assert((std::is_same<std::char_traits<char16_t>::char_type, char16_t>::value), "");
    static_assert((std::is_same<std::char_traits<char16_t>::int_type, std::uint_least16_t>::value), "");
    static_assert((std::is_same<std::char_traits<char16_t>::off_type, std::streamoff>::value), "");
    static_assert((std::is_same<std::char_traits<char16_t>::pos_type, std::u16streampos>::value), "");
    static_assert((std::is_same<std::char_traits<char16_t>::state_type, std::mbstate_t>::value), "");
#endif  // _LIBCPP_HAS_NO_UNICODE_CHARS
    TC_SUCCESS_RESULT();
    return 0;
}
