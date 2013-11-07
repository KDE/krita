/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_ASSERT_H
#define __KIS_ASSERT_H

#include <QtGlobal>
#include <krita_export.h>

KRITAIMAGE_EXPORT void kis_assert(const char *assertion, const char *file, int line);

#define KIS_ASSERT(cond) ((!(cond)) ? kis_assert(#cond,__FILE__,__LINE__) : qt_noop())
#define KIS_ASSERT_RECOVER(cond) if (!(cond) && (kis_assert(#cond,__FILE__,__LINE__), true))
#define KIS_ASSERT_RECOVER_BREAK(cond) KIS_ASSERT_RECOVER(cond) { break; }
#define KIS_ASSERT_RECOVER_RETURN(cond) KIS_ASSERT_RECOVER(cond) { return; }
#define KIS_ASSERT_RECOVER_RETURN_VALUE(cond, val) KIS_ASSERT_RECOVER(cond) { return (val); }

#endif /* __KIS_ASSERT_H */
