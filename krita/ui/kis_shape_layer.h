/*
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KIS_SHAPE_LAYER_H_
#define KIS_SHAPE_LAYER_H_

#include <KoShapeContainer.h>

#include <kis_external_layer_iface.h>

class QRect;
class QPainter;
class QIcon;
class QRect;
class QDomDocument;
class QDomElement;

class KoViewConverter;

/**
   A KisShapeLayer contains any number of non-krita flakes, such as
   path shapes, text shapes and anything else people come up with.
*/
class KisShapeLayer : public KoShapeContainer, public KisExternalLayer
{
    // KoShape overrides

    bool isSelectable() const { return false; }

    // KoShapeContainer implemenation

    void paintComponent(QPainter &painter, const KoViewConverter &converter);

    // KisExternalLayer implementation

    QIcon icon() const;

    KisPaintDeviceSP prepareProjection(KisPaintDeviceSP projection, const QRect& r);

    bool saveToXML(QDomDocument doc, QDomElement elem);

};

#endif
