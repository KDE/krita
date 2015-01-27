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

#ifndef _INFINITERULER_ASSISTANT_H_
#define _INFINITERULER_ASSISTANT_H_

#include "kis_painting_assistant.h"
#include <QObject>
#include <QPolygonF>
#include <QLineF>
#include <QTransform>
/* Design:
 */
class InfiniteRuler;

class InfiniteRulerAssistant : public KisPaintingAssistant
{
public:
    InfiniteRulerAssistant();
    virtual QPointF adjustPosition(const QPointF& point, const QPointF& strokeBegin);
    //virtual void endStroke();
    virtual QPointF buttonPosition() const;
    virtual int numHandles() const { return 2; }
protected:
    virtual void drawAssistant(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter* converter, bool  cached = true,KisCanvas2* canvas=0, bool assistantVisible=true, bool previewVisible=true);
    virtual void drawCache(QPainter& gc, const KisCoordinatesConverter *converter,  bool assistantVisible=true);
private:
    QPointF project(const QPointF& pt, const QPointF& strokeBegin);
};

class InfiniteRulerAssistantFactory : public KisPaintingAssistantFactory
{
public:
    InfiniteRulerAssistantFactory();
    virtual ~InfiniteRulerAssistantFactory();
    virtual QString id() const;
    virtual QString name() const;
    virtual KisPaintingAssistant* createPaintingAssistant() const;
};

#endif
