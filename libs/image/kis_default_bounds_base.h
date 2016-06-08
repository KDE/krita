/*
 *  Copyright (c) 2010 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
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
#ifndef KIS_DEFAULT_BOUNDS_BASE_H
#define KIS_DEFAULT_BOUNDS_BASE_H

#include <QRect>
#include "kis_shared.h"
#include "kis_shared_ptr.h"
#include "kritaimage_export.h"
class KisDefaultBoundsBase;

typedef KisSharedPtr<KisDefaultBoundsBase> KisDefaultBoundsBaseSP;

class KRITAIMAGE_EXPORT KisDefaultBoundsBase : public KisShared
{
public:
    virtual ~KisDefaultBoundsBase();

    virtual QRect bounds() const = 0;
    virtual bool wrapAroundMode() const = 0;
    virtual int currentLevelOfDetail() const = 0;
    virtual int currentTime() const = 0;
    virtual bool externalFrameActive() const = 0;
};


#endif // KIS_DEFAULT_BOUNDS_BASE_H
