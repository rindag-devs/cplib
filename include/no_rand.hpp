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

#ifndef CPLIB_NO_RAND_HPP_
#define CPLIB_NO_RAND_HPP_

// IWYU pragma: no_include <features.h>

#include <cstdlib>

/* cplib_embed_ignore start */
#include "utils.hpp"
/* cplib_embed_ignore end */

#ifdef __GLIBC__
#define CPLIB_RAND_THROW_STATEMENT noexcept(true)
#else
#define CPLIB_RAND_THROW_STATEMENT
#endif

#if defined(__GNUC__) && !defined(__clang__)
__attribute__((error("Don not use `rand`, use `rnd.next` instead")))
#endif
#ifdef _MSC_VER
#pragma warning(disable : 4273)
#endif
inline auto
rand() CPLIB_RAND_THROW_STATEMENT->int {
  cplib::panic("Don not use `rand`, use `rnd.next` instead");
}

#if defined(__GNUC__) && !defined(__clang__)
__attribute__((error("Don not use `srand`, you should use `cplib::Random` for random generator")))
#endif
#ifdef _MSC_VER
#pragma warning(disable : 4273)
#endif
inline auto
srand(unsigned int) CPLIB_RAND_THROW_STATEMENT->void {
  cplib::panic("Don not use `srand`, you should use `cplib::Random` for random generator");
}

#endif
