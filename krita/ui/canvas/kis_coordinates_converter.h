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


class QRect;
class QRectF;
class QPoint;
class KoViewConverter;

class KRITAUI_EXPORT KisCoordinatesConverter
{
public:
    KisCoordinatesConverter(KisImageWSP image, KoViewConverter *viewConverter);
    ~KisCoordinatesConverter();

    void setDocumentOrigin(const QPoint &origin);
    void setDocumentOffset(const QPoint &offset);

    QRectF imageToWidget(const QRect &imageRect);
    QRect widgetToImage(const QRectF &widgetRect);

    QRectF imageToViewport(const QRect &imageRect);
    QRect viewportToImage(const QRectF &viewportRect);

    QRectF widgetToViewport(const QRectF &widgetRect);
    QRectF viewportToWidget(const QRectF &viewportRect);

    void imageScale(qreal *scaleX, qreal *scaleY);
private:
    struct Private;
    Private * const m_d;
};

#endif /* KIS_COORDINATES_CONVERTER_H */
