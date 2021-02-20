/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
        MIRROR_OPTION,
        SHARPNESS_OPTION
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
                              qreal alignForZoom,
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
