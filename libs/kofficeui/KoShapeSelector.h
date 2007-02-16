/*
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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

#ifndef KOSHAPESELECTOR_H
#define KOSHAPESELECTOR_H

#include <KoCanvasBase.h>
#include <KoViewConverter.h>
#include <KoShapeControllerBase.h>

#include <QDockWidget>
#include <QRectF>

class KoTool;
class KoShape;
class KoViewConverter;
class QUndoCommand;
class KoCanvasController;
class KoShapeManager;
class QKeyEvent;
class QPainter;

/**
 * The shape selector shows a widget that holds templates and clipboard data
 * for the user to easilly move that between apps and maintain functionality.
 */
class KoShapeSelector : public QDockWidget {
    Q_OBJECT
public:
    explicit KoShapeSelector(QWidget *parent = 0);
    ~KoShapeSelector();

private slots:
    void loadShapeTypes();

private:
    void itemSelected();
    void add(KoShape *item);

private:
    /// \internal
    class DummyViewConverter : public KoViewConverter {
        QPointF documentToView (const QPointF &documentPoint) const;
        QPointF viewToDocument (const QPointF &viewPoint) const;
        QRectF documentToView (const QRectF &documentRect) const;
        QRectF viewToDocument (const QRectF &viewRect) const;
        void zoom (double *zoomX, double *zoomY) const;
        double documentToViewX (double documentX) const;
        double documentToViewY (double documentY) const;
        double viewToDocumentX (double viewX) const;
        double viewToDocumentY (double viewY) const;
    };

    class DummyShapeController : public KoShapeControllerBase {
    public:
        void addShape( KoShape* ) {}
        void removeShape( KoShape* ) {}
    };

    /// \internal
    class Canvas : public QWidget, public KoCanvasBase {
        public:
            explicit Canvas(KoShapeSelector *parent);
            void gridSize (double *horizontal, double *vertical) const;
            bool snapToGrid() const { return false; }
            void addCommand (QUndoCommand *command);
            KoShapeManager * shapeManager () const { return m_parent->m_shapeManager; }
            void updateCanvas (const QRectF &rc);
            KoToolProxy *toolProxy () { return 0; }
            KoViewConverter * viewConverter() { return &m_converter; }
            QWidget *canvasWidget () { return m_parent; }
            KoUnit unit() { return KoUnit(KoUnit::Millimeter); }

        protected: // event handlers
            void mouseMoveEvent(QMouseEvent *e);
            void mousePressEvent(QMouseEvent *e);
            void mouseReleaseEvent(QMouseEvent *e);
            void paintEvent(QPaintEvent * e);
            void dragEnterEvent(QDragEnterEvent *e);
            void dropEvent(QDropEvent *e);
            bool event(QEvent *e);

        private:
            DummyShapeController m_shapeController;
            DummyViewConverter m_converter;
            KoShapeSelector *m_parent;
            bool m_emitItemSelected;
    };

    friend class Canvas;

    KoShapeManager *m_shapeManager;
    Canvas *m_canvas;
};

#endif
