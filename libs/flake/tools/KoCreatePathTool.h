/* This file is part of the KDE project
 *
 * Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008-2009 Jan Hambrecht <jaham@gmx.net>
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
#ifndef KOCREATEPATHTOOL_H
#define KOCREATEPATHTOOL_H


#include <KoTool.h>

#include <QMap>

class KoPathShape;
class KoPathPoint;

#define KoCreatePathTool_ID "CreatePathTool"

/**
 * Tool for creating path shapes.
 */
class FLAKE_EXPORT KoCreatePathTool : public KoTool
{
    Q_OBJECT
public:
    /**
     * Constructor for the tool that allows you to create new paths by hand.
     * @param canvas the canvas this tool will be working for.
     */
    explicit KoCreatePathTool(KoCanvasBase * canvas);
    virtual ~KoCreatePathTool();

    void paint(QPainter &painter, const KoViewConverter &converter);

    void mousePressEvent(KoPointerEvent *event);
    void mouseDoubleClickEvent(KoPointerEvent *event);
    void mouseMoveEvent(KoPointerEvent *event);
    void mouseReleaseEvent(KoPointerEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);

public slots:
    virtual void activate(bool temporary = false);
    virtual void deactivate();
    virtual void resourceChanged(int key, const QVariant & res);

protected:
    /// add path shape to document
    virtual void addPathShape();
    /// reimplemented
    virtual QMap<QString, QWidget *> createOptionWidgets();

    KoPathShape *m_shape;

private slots:
    void angleDeltaChanged(int value);

private:
    void repaintActivePoint();

    /// returns the nearest existing path point
    KoPathPoint* endPointAtPosition( const QPointF &position );

    /// Connects given path with the ones we hit when starting/finishing
    bool connectPaths( KoPathShape *pathShape, KoPathPoint *pointAtStart, KoPathPoint *pointAtEnd );

    KoPathPoint *m_activePoint;
    KoPathPoint *m_firstPoint;
    int m_handleRadius;
    bool m_mouseOverFirstPoint;
    bool m_pointIsDragged;
    KoPathPoint *m_existingStartPoint; ///< an existing path point we started a new path at
    KoPathPoint *m_existingEndPoint;   ///< an existing path point we finished a new path at
    KoPathPoint *m_hoveredPoint; ///< an existing path end point the mouse is hovering on

    class AngleSnapStrategy;
    AngleSnapStrategy *m_angleSnapStrategy;
    int m_angleSnappingDelta;
    KoCanvasBase * const m_canvas;
};
#endif

