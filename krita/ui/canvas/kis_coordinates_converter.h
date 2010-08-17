/*
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

#ifndef KIS_COORDINATES_CONVERTER_H
#define KIS_COORDINATES_CONVERTER_H

#include "krita_export.h"
#include "kis_types.h"


class QSize;
class QRectF;
class QPoint;
class QTransform;
class KoViewConverter;


/**
 * Automatic generation of QRectF transformations.
 * They will use QPointF methods for transforming rectangles.
 */
#define DEFINE_RECT_METHOD(name)                                        \
    QRectF KisCoordinatesConverter::name(const QRectF &rc) const        \
    { return QRectF(name(rc.topLeft()), name(rc.bottomRight())); }

#define DECLARE_RECT_METHOD(name)               \
    QRectF name(const QRectF &rc) const



class KRITAUI_EXPORT KisCoordinatesConverter
{
public:
    KisCoordinatesConverter(KoViewConverter *viewConverter);
    ~KisCoordinatesConverter();

    void setImage(KisImageWSP image);
    void setDocumentOrigin(const QPoint &origin);
    void setDocumentOffset(const QPoint &offset);

    QPoint documentOrigin() const;
    QPoint documentOffset() const;

    QPointF imageToViewport(const QPointF &pt) const;
    QPointF viewportToImage(const QPointF &pt) const;

    QPointF widgetToViewport(const QPointF &pt) const;
    QPointF viewportToWidget(const QPointF &pt) const;

    QPointF widgetToDocument(const QPointF &pt) const;
    QPointF documentToWidget(const QPointF &pt) const;

    QPointF imageToDocument(const QPointF &pt) const;
    QPointF documentToImage(const QPointF &pt) const;

    DECLARE_RECT_METHOD(imageToViewport);
    DECLARE_RECT_METHOD(viewportToImage);

    DECLARE_RECT_METHOD(widgetToViewport);
    DECLARE_RECT_METHOD(viewportToWidget);

    DECLARE_RECT_METHOD(widgetToDocument);
    DECLARE_RECT_METHOD(documentToWidget);

    DECLARE_RECT_METHOD(imageToDocument);
    DECLARE_RECT_METHOD(documentToImage);

    QTransform imageToWidgetTransform() const;
    QTransform documentToWidgetTransform() const;
    QTransform flakeToWidgetTransform() const;
    QTransform viewportToWidgetTransform() const;
    QTransform checkersToWidgetTransform() const;

    QRectF imageRectInWidgetPixels() const;
    QRectF imageRectInViewportPixels() const;

    void imageScale(qreal *scaleX, qreal *scaleY) const;
private:
    struct Private;
    Private * const m_d;
};

#endif /* KIS_COORDINATES_CONVERTER_H */
