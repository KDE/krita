/*
 *  SPDX-FileCopyrightText: 2007 Thomas Zander <zander@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_IMAGE_VIEW_CONVERTER_H
#define KIS_IMAGE_VIEW_CONVERTER_H

#include <kritaui_export.h>

#include "kis_types.h"
#include "KisClonableViewConverter.h"
#include "KisImageResolutionProxy.h"

/**
 * ViewConverter to convert from flake-internal points to
 * krita-internal pixels and back. You can use this class wherever
 * the flake tools or shapes come in contact with the krita-image.
 *
 * For usage remember that the document here is the flake-points. And
 * the view is the krita-pixels.
 */
class KRITAUI_EXPORT KisImageViewConverter : public KisClonableViewConverter
{
public:
    /**
     * constructor
     * @param image the image this viewConverter works for.
     */
    KisImageViewConverter();
    KisImageViewConverter(const KisImageWSP image);
    KisImageViewConverter(KisImageResolutionProxySP proxy);
    KisImageViewConverter(const KisImageViewConverter &rhs);
    ~KisImageViewConverter();

    KisClonableViewConverter* clone() const override;

    void setImage(KisImageWSP image);

    /// reimplemented from superclass
    void zoom(qreal *zoomX, qreal *zoomY) const override;

    qreal documentToViewX(qreal documentX) const override;
    qreal documentToViewY(qreal documentY) const override;
    qreal viewToDocumentX(qreal viewX) const override;
    qreal viewToDocumentY(qreal viewY) const override;

    // This method shouldn't be used for image
    qreal zoom() const;

private:
    qreal effectiveXRes() const;
    qreal effectiveYRes() const;

private:
    KisImageResolutionProxySP m_proxy;
};

using KisImageViewConverterSP = QSharedPointer<KisImageViewConverter>;

#endif
