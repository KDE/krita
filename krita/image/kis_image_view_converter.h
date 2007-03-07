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

#include <krita_export.h>

#include "kis_image.h"
#include <KoViewConverter.h>

/**
 * ViewConverter to convert from flake-internal points to krita-internal pixels and back.
 * You can use this class whereever the flake tools or shapes come in contact with the
 * krita-image.
 * For usage remember that the document here is the flake-points. And the view is the
 * krita-pixels.
 */
class KRITAIMAGE_EXPORT KisImageViewConverter : public KoViewConverter {
public:
    /**
     * constructor
     * @param image the image this viewConver works for.
     */
    KisImageViewConverter( const KisImage *image );
    /// convert from flake to krita units
    QPointF documentToView( const QPointF &documentPoint ) const;
    /// convert from krita to flake units
    QPointF viewToDocument( const QPointF &viewPoint ) const;
    /// convert from flake to krita units
    QRectF documentToView( const QRectF &documentRect ) const;
    /// convert from krita to flake units
    QRectF viewToDocument( const QRectF &viewRect ) const;
    /// convert from flake to krita units
    inline double documentToViewX( double documentX ) const { return documentX * m_image->xRes(); }
    /// convert from flake to krita units
    inline double documentToViewY( double documentY ) const { return documentY * m_image->yRes(); }
    /// convert from krita to flake units
    inline double viewToDocumentX( double viewX ) const { return viewX / m_image->xRes(); }
    /// convert from krita to flake units
    inline double viewToDocumentY( double viewY ) const { return viewY / m_image->yRes(); }
    /// reimplemented from superclass
    void zoom(double *zoomX, double *zoomY) const;

private:
    const KisImage *m_image;
};

#endif
