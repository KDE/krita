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

#ifndef PARAGRAPHBASE_H
#define PARAGRAPHBASE_H

#include <KoParagraphStyle.h>

#include <QObject>
#include <QPainter>
#include <QRectF>
#include <QTextBlock>
#include <QTextBlockFormat>
#include <QTextCursor>
#include <QList>

class KoCanvasBase;
class KoShape;
class KoViewConverter;

class QTextLayout;

class ParagraphBase : public QObject
{
    Q_OBJECT
public:
    explicit ParagraphBase(QObject *parent, KoCanvasBase *canvas);
    ~ParagraphBase();

    // activate the paragraph at the specified position on the canvas
    void activateTextBlockAt(const QPointF &point);

    void activateTextBlock(QTextBlock newBlock);

    // deactivate the current text block
    void deactivateTextBlock();

    bool hasActiveTextBlock() const;

    QTextBlock textBlock() const;

    QTextBlockFormat blockFormat() const {
        return textBlock().blockFormat();
    }
    QTextCharFormat charFormat() const {
        return textBlock().charFormat();
    }
    QTextLayout *textLayout() const {
        return textBlock().layout();
    }

    qreal shapeTop(const KoShape *shape) const;
    qreal shapeBottom(const KoShape *shape) const;

    bool needsRepaint() const;

public slots:
    void scheduleRepaint();

protected:
    bool m_needsRepaint;

    KoCanvasBase *canvas() { return m_canvas; }
    QTextCursor cursor() { return m_cursor; }
    KoParagraphStyle *paragraphStyle() { return m_paragraphStyle; }

    QList<KoShape*> shapes() { return m_shapes; }
    virtual void addShapes();
    bool shapeContainsBlock(const KoShape *shape);

private:
    KoCanvasBase *m_canvas;
    QTextCursor m_cursor;
    KoParagraphStyle *m_paragraphStyle;

    QList<KoShape*> m_shapes;
};

#endif

