/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
