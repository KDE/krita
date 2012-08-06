/*
 * Copyright (c) 2012 Shivaraman Aiyer <sra392@gmail.com>
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


#ifndef _MESH_ASSISTANT_H_
#define _MESH_ASSISTANT_H_

#include "kis_painting_assistant.h"
#include <QObject>
#include <QPolygonF>
#include <QLineF>
#include <QTransform>

class MeshAssistant : public KisPaintingAssistant
{
public:
    MeshAssistant();
    void initialize(char* file);
    virtual QPointF adjustPosition(const QPointF& point, const QPointF& strokeBegin);
    virtual QPointF buttonPosition() const;
    virtual int numHandles() const { return 3; }
protected:
    virtual QRect boundingRect() const;
    virtual void drawCache(QPainter& gc, const KisCoordinatesConverter *converter);
};

class MeshAssistantFactory : public KisPaintingAssistantFactory
{
public:
    MeshAssistantFactory();
    virtual ~MeshAssistantFactory();
    virtual QString id() const;
    virtual QString name() const;
    virtual KisPaintingAssistant* createPaintingAssistant() const;
};

#endif

