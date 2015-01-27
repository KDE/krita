/*
 * Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 * Copyright (c) 2010 Geoffry Song <goffrie@gmail.com>
 * Copyright (c) 2014 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _VANISHINGPOINT_ASSISTANT_H_
#define _VANISHINGPOINT_ASSISTANT_H_

#include "kis_painting_assistant.h"
#include <QObject>
#include <QPolygonF>
#include <QLineF>
#include <QTransform>
/* Design:
 *The idea behind the vanishing point ruler is that in a perspective deformed landscape, a set of parallel
 *lines al share a single vanishing point.
 *Therefore, a perspective can contain an theoretical infinite of vanishing points.
 *It's a pity if we only allowed an artist to access 1, 2 or 3 of these at any given time, as other
 *solutions for perspective tools do.
 *Hence a vanishing point ruler.
 *
 *This ruler is relatively simple compared to the other perspective ruler:
 *It has only one vanishing point that is required to draw.
 *However, it does have it's own weaknesses in how to determine onto which of these infinite rulers to snap.
 *Furthermore, it has four extra handles for adding a perspective ruler to a preexisting perspective.
 */
//class VanishingPoint;

class VanishingPointAssistant : public KisPaintingAssistant
{
public:
    VanishingPointAssistant();
    virtual QPointF adjustPosition(const QPointF& point, const QPointF& strokeBegin);
    //virtual void endStroke();
    virtual QPointF buttonPosition() const;
    virtual int numHandles() const { return 1; }
protected:
    virtual void drawAssistant(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter* converter, bool  cached = true,KisCanvas2* canvas=0, bool assistantVisible=true, bool previewVisible=true);
    virtual void drawCache(QPainter& gc, const KisCoordinatesConverter *converter,  bool assistantVisible=true);
private:
    QPointF project(const QPointF& pt, const QPointF& strokeBegin);
};

class VanishingPointAssistantFactory : public KisPaintingAssistantFactory
{
public:
    VanishingPointAssistantFactory();
    virtual ~VanishingPointAssistantFactory();
    virtual QString id() const;
    virtual QString name() const;
    virtual KisPaintingAssistant* createPaintingAssistant() const;
};

#endif
