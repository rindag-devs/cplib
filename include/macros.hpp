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

#define CPLIB_STARTUP_TEXT       \
  "cplib (CPLib) " CPLIB_VERSION \
  "\n"                           \
  "https://github.com/rindag-devs/cplib/ by Rindag Devs, copyright(c) 2023-2026\n"

#if defined(_WIN32) || defined(__WIN32__) || defined(__WIN32) || defined(_WIN64) || \
    defined(__WIN64__) || defined(__WIN64) || defined(WINNT) || defined(__WINNT) || \
    defined(__WINNT__) || defined(__CYGWIN__)
#define CPLIB_ON_WINDOWS
#endif

#endif
