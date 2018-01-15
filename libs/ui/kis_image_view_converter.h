/*
 *  Copyright (c) 2007 Thomas Zander <zander@kde.org>
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

#ifndef KIS_IMAGE_VIEW_CONVERTER_H
#define KIS_IMAGE_VIEW_CONVERTER_H

#include <kritaui_export.h>

#include "kis_image.h"
#include "kis_types.h"
#include <KoViewConverter.h>

class QTransform;


/**
 * ViewConverter to convert from flake-internal points to
 * krita-internal pixels and back. You can use this class wherever
 * the flake tools or shapes come in contact with the krita-image.
 *
 * For usage remember that the document here is the flake-points. And
 * the view is the krita-pixels.
 */
class KRITAUI_EXPORT KisImageViewConverter : public KoViewConverter
{
public:
    /**
     * constructor
     * @param image the image this viewConverter works for.
     */
    KisImageViewConverter(const KisImageWSP image);

    void setImage(KisImageWSP image);

    QTransform documentToView() const;
    QTransform viewToDocument() const;

    using KoViewConverter::documentToView;
    using KoViewConverter::viewToDocument;

    /// reimplemented from superclass
    void zoom(qreal *zoomX, qreal *zoomY) const override;

    qreal documentToViewX(qreal documentX) const override;
    qreal documentToViewY(qreal documentY) const override;
    qreal viewToDocumentX(qreal viewX) const override;
    qreal viewToDocumentY(qreal viewY) const override;

    // This method shouldn't be used for image
    qreal zoom() const;

private:
    KisImageWSP m_image;
};

#endif
