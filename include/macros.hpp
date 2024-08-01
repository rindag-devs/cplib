/*
 * This file is part of CPLib.
 *
 * CPLib is free software: you can redistribute it and/or modify it under the terms of the GNU
 * Lesser General Public License as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * CPLib is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License along with CPLib. If
 * not, see <https://www.gnu.org/licenses/>.
 */

#ifndef CPLIB_MACROS_HPP_
#define CPLIB_MACROS_HPP_

#define CPLIB_VERSION "0.0.1-SNAPSHOT"

#if (_WIN32 || __WIN32__ || __WIN32 || _WIN64 || __WIN64__ || __WIN64 || WINNT || __WINNT || \
     __WINNT__ || __CYGWIN__)
#if !defined(_MSC_VER) || _MSC_VER > 1400
#define NOMINMAX 1
#include <windows.h>  // IWYU pragma: export
#else
#include <unistd.h>  // IWYU pragma: export
#endif
#include <fcntl.h>  // IWYU pragma: export
#include <io.h>     // IWYU pragma: export
#define ON_WINDOWS
#if defined(_MSC_VER) && _MSC_VER > 1400
#pragma warning(disable : 4127)
#pragma warning(disable : 4146)
#pragma warning(disable : 4458)
#endif
#else
#include <fcntl.h>   // IWYU pragma: export
#include <unistd.h>  // IWYU pragma: export
#endif

#if defined(__GNUC__)
#define CPLIB_PRINTF_LIKE(n, m) __attribute__((format(printf, n, m)))
#else
#define CPLIB_PRINTF_LIKE(n, m) /* If only */
#endif                          /* __GNUC__ */

#endif
