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

#ifndef _SPLINE_ASSISTANT_H_
#define _SPLINE_ASSISTANT_H_

#include "kis_painting_assistant.h"
#include <QObject>

class SplineAssistant : public KisPaintingAssistant
{
public:
    SplineAssistant();
    virtual QPointF adjustPosition(const QPointF& point, const QPointF& strokeBegin);
    virtual QPointF buttonPosition() const;
    virtual int numHandles() const { return 4; }
protected:
    virtual void drawCache(QPainter& gc, const KisCoordinatesConverter *converter);
private:
    QPointF project(const QPointF& pt) const;
};

class SplineAssistantFactory : public KisPaintingAssistantFactory
{
public:
    SplineAssistantFactory();
    virtual ~SplineAssistantFactory();
    virtual QString id() const;
    virtual QString name() const;
    virtual KisPaintingAssistant* createPaintingAssistant() const;
};

#endif
