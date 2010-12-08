/*
 * Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 * Copyright (c) 2010 Geoffry Song <goffrie@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _PERSPECTIVE_ASSISTANT_H_
#define _PERSPECTIVE_ASSISTANT_H_

#include "kis_painting_assistant.h"
#include <QObject>
#include <QPolygonF>

class PerspectiveAssistant : public KisPaintingAssistant
{
public:
    PerspectiveAssistant();
    virtual QPointF adjustPosition(const QPointF& point, const QPointF& strokeBegin);
    void drawAssistant(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter *converter);
private:
    QPointF project(const QPointF& pt, const QPointF& strokeBegin);
    // creates the convex hull, returns false if it's not a quadrilateral
    bool quad(QPolygonF& out) const;
};

class PerspectiveAssistantFactory : public KisPaintingAssistantFactory
{
public:
    PerspectiveAssistantFactory();
    virtual ~PerspectiveAssistantFactory();
    virtual QString id() const;
    virtual QString name() const;
    virtual KisPaintingAssistant* paintingAssistant(const QRectF& imageArea) const;
};

#endif
