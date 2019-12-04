/*
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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
 *
 *  adopted from here http://web.archive.org/web/20090728150504/http://www.snippetcenter.org/en/a-fast-atan2-function-s1868.aspx
 */


#ifndef _KIS_IMAGE_FAST_
#define _KIS_IMAGE_FAST_

#include <QtGlobal>

#include "kritaimage_export.h"

/**
 * This namespace contains fast but inaccurate version of mathematical function.
 */
namespace KisFastMath {

    /// atan2 replacement
    KRITAIMAGE_EXPORT qreal atan2(qreal y, qreal x);
}

#endif
