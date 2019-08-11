/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __KIS_FILL_SANITY_CHECKS_H
#define __KIS_FILL_SANITY_CHECKS_H

#define ENABLE_FILL_SANITY_CHECKS
//#define ENABLE_CHECKS_FOR_TESTING

#ifdef ENABLE_FILL_SANITY_CHECKS

#ifdef ENABLE_CHECKS_FOR_TESTING
#include <stdexcept>
#define SANITY_ASSERT_MSG(cond, msg) ((!(cond)) ? throw std::invalid_argument(msg) : qt_noop())
#else
#define SANITY_ASSERT_MSG(cond, msg) KIS_SAFE_ASSERT_RECOVER_NOOP((cond))
#endif /* ENABLE_CHECKS_FOR_TESTING */

#else

#define SANITY_ASSERT_MSG(cond, msg)

#endif /* ENABLE_FILL_SANITY_CHECKS */


#endif /* __KIS_FILL_SANITY_CHECKS_H */
