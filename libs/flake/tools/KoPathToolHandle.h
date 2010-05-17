/* This file is part of the KDE project
 * Copyright (C) 2006,2008 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2006,2007 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2007,2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KOPATHTOOLHANDLE_H
#define KOPATHTOOLHANDLE_H

#include <KoPathPoint.h>
#include "KoInteractionStrategy.h"

#include <QList>

class KoPathTool;
class KoParameterShape;
class KoViewConverter;
class KoPointerEvent;
class QPainter;
class KoPathShape;

class KoPathToolHandle
{
public:
    KoPathToolHandle(KoPathTool *tool);
    virtual ~KoPathToolHandle();
    virtual void paint(QPainter &painter, const KoViewConverter &converter) = 0;
    virtual void repaint() const = 0;
    virtual KoInteractionStrategy * handleMousePress(KoPointerEvent *event) = 0;
    // test if handle is still valid
    virtual bool check(const QList<KoPathShape*> &selectedShapes) = 0;

protected:
    KoPathTool *m_tool;
};

class PointHandle : public KoPathToolHandle
{
public:
    PointHandle(KoPathTool *tool, KoPathPoint *activePoint, KoPathPoint::PointType activePointType);
    void paint(QPainter &painter, const KoViewConverter &converter);
    void repaint() const;
    KoInteractionStrategy *handleMousePress(KoPointerEvent *event);
    virtual bool check(const QList<KoPathShape*> &selectedShapes);
    KoPathPoint *activePoint() const;
    KoPathPoint::PointType activePointType() const;
private:
    KoPathPoint *m_activePoint;
    KoPathPoint::PointType m_activePointType;
};

class ParameterHandle : public KoPathToolHandle
{
public:
    ParameterHandle(KoPathTool *tool, KoParameterShape *parameterShape, int handleId);
    void paint(QPainter &painter, const KoViewConverter &converter);
    void repaint() const;
    KoInteractionStrategy *handleMousePress(KoPointerEvent *event);
    virtual bool check(const QList<KoPathShape*> &selectedShapes);
protected:
    KoParameterShape *m_parameterShape;
    int m_handleId;
};

class ConnectionHandle : public ParameterHandle
{
public:
    ConnectionHandle(KoPathTool *tool, KoParameterShape *parameterShape, int handleId);

    // XXX: Later: create a paint even to distinguish a connection
    // handle  from another handle type
    KoInteractionStrategy *handleMousePress(KoPointerEvent *event);
};

#endif // KOPATHTOOLHANDLE_H
