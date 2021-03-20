/*
 *  SPDX-FileCopyrightText: 2010 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2010 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

    /**
     * Returns a virtual bounding rect of a paint device. E.g. when a
     * paint device has non-transparent default pixel, its virtual bounds
     * extend much wider than the actual data it contains.
     *
     * This bounds rectangle should be used in all the cases when
     * one wants to process all the non-existing pixels with default
     * value, which may still be visible to the user.
     *
     * The returned rect usually equals to the bounds of the image,
     * except of a few special cases for selections.
     *
     * Example:
     *
     * KisPaintDevice adds `defaultBounds->bounds()` to its `extent()`
     * and `exactBounds()` when its default pixel is non-transparent.
     */
    virtual QRect bounds() const = 0;

    /**
     * Returns the rectangle of the official image size. This rect is
     * used for wrapping the device in wrap-around mode and in some
     * specific operations.
     *
     * NOTE: don't use it uless you know what you are doing,
     *       most probably you want to use `bounds()` instead!
     */
    virtual QRect imageBorderRect() const;

    virtual bool wrapAroundMode() const = 0;
    virtual int currentLevelOfDetail() const = 0;
    virtual int currentTime() const = 0;
    virtual bool externalFrameActive() const = 0;

    /**
     * Return an abstract pointer to the source object,
     * where default bounds takes its data from. It the
     * cookie is nullptr, then the default bounds is not
     * connected to anything. One can also compare if two
     * default bounds are connected to the same source by
     * comparing two pointers.
     *
     * NOTE: It is intended to be used for debugging
     *       purposes only!
     */
    virtual void* sourceCookie() const = 0;
};


#endif // KIS_DEFAULT_BOUNDS_BASE_H
