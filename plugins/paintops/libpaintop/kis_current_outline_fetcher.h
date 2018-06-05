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

#ifndef __KIS_CURRENT_OUTLINE_FETCHER_H
#define __KIS_CURRENT_OUTLINE_FETCHER_H

#include <kritapaintop_export.h>

#include <QFlags>
#include <QScopedPointer>
#include <QPainterPath>
#include <QPointF>

#include <kis_paintop_settings.h>

class KisPaintInformation;



class PAINTOP_EXPORT KisCurrentOutlineFetcher
{
public:
    enum Option {
        NO_OPTION,
        SIZE_OPTION,
        ROTATION_OPTION,
        MIRROR_OPTION
    };

    Q_DECLARE_FLAGS(Options, Option);

public:
    KisCurrentOutlineFetcher(Options optionsAvailable);
    ~KisCurrentOutlineFetcher();

    void setDirty();

    QPainterPath fetchOutline(const KisPaintInformation &info,
                              const KisPaintOpSettingsSP settings,
                              const QPainterPath &originalOutline,
                              const KisPaintOpSettings::OutlineMode &mode,
                              qreal additionalScale = 1.0,
                              qreal additionalRotation = 0.0,
                              bool tilt = false, qreal tiltcenterx = 1.0, qreal tiltcentery = 1.0) const;

private:
    Q_DISABLE_COPY(KisCurrentOutlineFetcher);
    struct Private;
    const QScopedPointer<Private> d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KisCurrentOutlineFetcher::Options);

#endif /* __KIS_CURRENT_OUTLINE_FETCHER_H */
