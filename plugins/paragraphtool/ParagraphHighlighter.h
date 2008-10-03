/* This file is part of the KDE project
 * Copyright (C) 2008 Florian Merz <florianmerz@gmx.de>
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

#ifndef PARAGRAPHHIGHLIGHTER_H
#define PARAGRAPHHIGHLIGHTER_H

#include <QObject>
#include <QTextBlock>
#include <QTextCursor>
#include <QPointF>

class KoCanvasBase;
class KoPointerEvent;
class KoShape;
class KoViewConverter;

class QKeyEvent;
class QPainter;

/**
 * This tool can be used to highlight a paragraph of text
 */
class ParagraphHighlighter : public QObject
{
    Q_OBJECT
public:
    explicit ParagraphHighlighter(QObject *parent, KoCanvasBase *canvas);
    ~ParagraphHighlighter();

    // reimplemented from superclass
    virtual void paint(QPainter &painter, const KoViewConverter &converter);

    // reimplemented from superclass
    virtual void mousePressEvent(KoPointerEvent *event);

    // reimplemented from superclass
    virtual void mouseReleaseEvent(KoPointerEvent *event);

    // reimplemented from superclass
    virtual void mouseMoveEvent(KoPointerEvent *event);

    // reimplemented from superclass
    virtual void keyPressEvent(QKeyEvent *event);

    // reimplemented from superclass
    virtual void keyReleaseEvent(QKeyEvent *event);

    void scheduleRepaint();
    bool needsRepaint() const;
    QRectF dirtyRectangle();

    QTextBlock textBlock() const;
    bool hasActiveTextBlock() const;
    void activateTextBlockAt(const QPointF &point);
    void activateTextBlock(QTextBlock textBlock);
    void deactivateTextBlock();

protected:
    void addShapes();

private:
    KoCanvasBase *m_canvas;
    QList<KoShape*> m_shapes;
    QTextCursor m_cursor;
    QPointF m_mousePosition;
    QRectF m_storedRepaintRectangle;

    bool m_needsRepaint;
};

#endif

