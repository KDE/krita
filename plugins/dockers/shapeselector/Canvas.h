/*
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef CANVAS_H
#define CANVAS_H

#include "ZoomHandler.h"

#include <KoCanvasBase.h>
#include <KoShapeControllerBase.h>

class QUndoCommand;
class ShapeSelector;
class KoShapeManager;

class DummyShapeController : public KoShapeControllerBase {
public:
    void addShape( KoShape* ) {}
    void removeShape( KoShape* ) {}
};

class Canvas : public QWidget, public KoCanvasBase {
    Q_OBJECT
public:
    explicit Canvas(ShapeSelector *parent);
    void gridSize (double *horizontal, double *vertical) const;
    bool snapToGrid() const { return false; }
    void addCommand (QUndoCommand *command);
    KoShapeManager * shapeManager() const;
    void updateCanvas (const QRectF &rc);
    KoToolProxy *toolProxy () const { return 0; }
    const KoViewConverter * viewConverter() const { return &m_converter; }
    QWidget *canvasWidget ();
    KoUnit unit() const { return KoUnit(KoUnit::Millimeter); }
    void updateInputMethodInfo() {}

signals:
    void resized(const QSize &newSize);

protected: // event handlers
    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);
    virtual void tabletEvent(QTabletEvent *e);
    virtual void paintEvent(QPaintEvent * e);
    virtual void dragEnterEvent(QDragEnterEvent *e);
    virtual void dropEvent(QDropEvent *e);
    virtual bool event(QEvent *e);
    virtual void resizeEvent (QResizeEvent *e);

private:
    DummyShapeController m_shapeController;
    ZoomHandler m_converter;
    ShapeSelector *m_parent;
    bool m_emitItemSelected;
};

#endif
