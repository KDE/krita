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


#include <KoToolBase.h>

#include <QList>

class KoPathShape;
class KoPathPoint;

class KoCreatePathToolPrivate;

#define KoCreatePathTool_ID "CreatePathTool"

/**
 * Tool for creating path shapes.
 */
class FLAKE_EXPORT KoCreatePathTool : public KoToolBase
{
    Q_OBJECT
public:
    /**
     * Constructor for the tool that allows you to create new paths by hand.
     * @param canvas the canvas this tool will be working for.
     */
    explicit KoCreatePathTool(KoCanvasBase * canvas);
    virtual ~KoCreatePathTool();

    /// reimplemented
    virtual void paint(QPainter &painter, const KoViewConverter &converter);

    /// reimplemented
    virtual void mousePressEvent(KoPointerEvent *event);
    /// reimplemented
    virtual void mouseDoubleClickEvent(KoPointerEvent *event);
    /// reimplemented
    virtual void mouseMoveEvent(KoPointerEvent *event);
    /// reimplemented
    virtual void mouseReleaseEvent(KoPointerEvent *event);
    /// reimplemented
    virtual void keyPressEvent(QKeyEvent *event);

public slots:
    /// reimplemented
    virtual void activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes);
    /// reimplemented
    virtual void deactivate();
    /// reimplemented
    virtual void documentResourceChanged(int key, const QVariant & res);

protected:
    /**
     * Add path shape to document.
     * This method can be overridden and change the behaviour of the tool. In that case the subclass takes ownership of pathShape.
     * It gets only called, if there are two or more points in the path.
     */
    virtual void addPathShape(KoPathShape* pathShape);

protected:
    /**
      * This method is called to paint the path. Decorations are drawn by KoCreatePathTool afterwards.
      */
    virtual void paintPath(KoPathShape& pathShape, QPainter &painter, const KoViewConverter &converter);

    /// reimplemented
    virtual QList<QWidget *> createOptionWidgets();

private:
    Q_DECLARE_PRIVATE(KoCreatePathTool);
    Q_PRIVATE_SLOT(d_func(), void angleDeltaChanged(int));
};
#endif

