/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISDETACHEDSHAPESVIEWCONVERTER_H
#define KISDETACHEDSHAPESVIEWCONVERTER_H

#include <kritaui_export.h>

#include "KisClonableViewConverter.h"

/**
 * @brief The KisDetachedShapesViewConverter class is a placeholder
 * class for vector layers for the time when they are not attached
 * to any image. Otherwise, a shape layer cannot reason about its
 * size.
 */
class KRITAUI_EXPORT KisDetachedShapesViewConverter : public KisClonableViewConverter
{
public:
    KisDetachedShapesViewConverter(qreal xRes, qreal yRes);

    KisClonableViewConverter* clone() const override;

    void zoom(qreal *zoomX, qreal *zoomY) const override;

    qreal documentToViewX(qreal documentX) const override;
    qreal documentToViewY(qreal documentY) const override;
    qreal viewToDocumentX(qreal viewX) const override;
    qreal viewToDocumentY(qreal viewY) const override;

    // This method shouldn't be used for image
    qreal zoom() const;

private:
    qreal m_xRes {1.0};
    qreal m_yRes {1.0};
};

#endif // KISDETACHEDSHAPESVIEWCONVERTER_H
