/*
 * Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 * Copyright (c) 2010 Geoffry Song <goffrie@gmail.com>
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

#ifndef _PERSPECTIVE_ASSISTANT_H_
#define _PERSPECTIVE_ASSISTANT_H_

#include "kis_abstract_perspective_grid.h"
#include "kis_painting_assistant.h"
#include <QObject>
#include <QPolygonF>
#include <QLineF>
#include <QTransform>

class PerspectiveAssistant : public KisAbstractPerspectiveGrid, public KisPaintingAssistant
{
    Q_OBJECT
public:
    PerspectiveAssistant(QObject * parent = 0);
    QPointF adjustPosition(const QPointF& point, const QPointF& strokeBegin) override;
    void endStroke() override;
    QPointF buttonPosition() const override;
    int numHandles() const override { return 4; }
    void drawAssistant(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter* converter, bool cached = true,KisCanvas2* canvas=0, bool assistantVisible=true, bool previewVisible=true) override;

    bool contains(const QPointF& point) const override;
    qreal distance(const QPointF& point) const override;


    bool isAssistantComplete() const override;

protected:
    void drawCache(QPainter& gc, const KisCoordinatesConverter *converter,  bool assistantVisible=true) override;
private:
    QPointF project(const QPointF& pt, const QPointF& strokeBegin);
    // creates the convex hull, returns false if it's not a quadrilateral
    bool quad(QPolygonF& out) const;
    // finds the transform from perspective coordinates (a unit square) to the document
    bool getTransform(QPolygonF& polyOut, QTransform& transformOut) const;

    // which direction to snap to (in transformed coordinates)
    QLineF m_snapLine;
    // cached information
    mutable QTransform m_cachedTransform;
    mutable QPolygonF m_cachedPolygon;
    mutable QPointF m_cachedPoints[4];
    mutable bool m_cacheValid;
};

class PerspectiveAssistantFactory : public KisPaintingAssistantFactory
{
public:
    PerspectiveAssistantFactory();
    ~PerspectiveAssistantFactory() override;
    QString id() const override;
    QString name() const override;
    KisPaintingAssistant* createPaintingAssistant() const override;
};

#endif
