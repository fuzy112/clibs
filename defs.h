/* Copyright © 2024  Zhengyi Fu <i@fuzy.me> */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* You should have received a copy of the GNU General Public License */
/* along with this program.  If not, see <https://www.gnu.org/licenses/>. */

#ifndef DEFS_H
#define DEFS_H

#ifdef __cplusplus
/* clang-format off */
#define C_DECL extern "C"
#define C_DECL_BEGIN extern "C" {
#define C_DECL_END }
/* clang-format on */
#else
#define C_DECL extern
#define C_DECL_BEGIN
#define C_DECL_END
#endif
#endif /*! DEFS_H */
