/*
 * Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 * Copyright (c) 2010 Geoffry Song <goffrie@gmail.com>
 * Copyright (c) 2014 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 * Copyright (c) 2017 Scott Petrovic <scottpetrovic@gmail.com>
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

#ifndef _FISHEYEPOINT_ASSISTANT_H_
#define _FISHEYEPOINT_ASSISTANT_H_

#include "kis_painting_assistant.h"
#include "Ellipse.h"
#include <QObject>
#include <QPolygonF>
#include <QLineF>
#include <QTransform>
//class FisheyePoint;

class FisheyePointAssistant : public KisPaintingAssistant
{
public:
    FisheyePointAssistant();
    QPointF adjustPosition(const QPointF& point, const QPointF& strokeBegin) override;
    //virtual void endStroke();
    QPointF buttonPosition() const override;
    int numHandles() const override { return 3; }

    bool isAssistantComplete() const override;

protected:
    QRect boundingRect() const override;
    void drawAssistant(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter* converter, bool  cached = true,KisCanvas2* canvas=0, bool assistantVisible=true, bool previewVisible=true) override;
    void drawCache(QPainter& gc, const KisCoordinatesConverter *converter,  bool assistantVisible=true) override;
private:
    QPointF project(const QPointF& pt, const QPointF& strokeBegin);
    mutable Ellipse e;
    mutable Ellipse extraE;
};

class FisheyePointAssistantFactory : public KisPaintingAssistantFactory
{
public:
    FisheyePointAssistantFactory();
    ~FisheyePointAssistantFactory() override;
    QString id() const override;
    QString name() const override;
    KisPaintingAssistant* createPaintingAssistant() const override;
};

#endif
