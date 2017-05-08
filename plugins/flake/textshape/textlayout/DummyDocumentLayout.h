/* This file is part of the KDE project
 * Copyright (C) 2011 C. Boemann <cbo@kogmbh.com>
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

#ifndef DUMMYDOCUMENTLAYOUT_H
#define DUMMYDOCUMENTLAYOUT_H

#include "kritatextlayout_export.h"

#include <QAbstractTextDocumentLayout>


/**
 * Dummy TextLayouter that does nothing really, but without it the Table of Contents/Bibliography
 * can not be layout.TextLayouter
 * The real layout of the ToC/Bib still happens by the KoTextLayoutArea as part of
 * KoTextDocumentLayout of the main document
 *
 * You really shouldn't add anything to this class
 */

class KRITATEXTLAYOUT_EXPORT DummyDocumentLayout : public QAbstractTextDocumentLayout
{
    Q_OBJECT
public:
    /// constructor
    explicit DummyDocumentLayout(QTextDocument *doc);
    ~DummyDocumentLayout() override;

    /// Returns the bounding rectangle of block.
    QRectF blockBoundingRect(const QTextBlock & block) const override;
    /**
     * Returns the total size of the document. This is useful to display
     * widgets since they can use to information to update their scroll bars
     * correctly
     */
    QSizeF documentSize() const override;

    /// Draws the layout on the given painter with the given context.
    void draw(QPainter * painter, const QAbstractTextDocumentLayout::PaintContext & context) override;

    QRectF frameBoundingRect(QTextFrame*) const override;

    /// reimplemented DO NOT CALL - USE HITTEST IN THE ROOTAREAS INSTEAD
    int hitTest(const QPointF & point, Qt::HitTestAccuracy accuracy) const override;

    /// reimplemented to always return 1
    int pageCount() const override;

    /// reimplemented from QAbstractTextDocumentLayout
    void documentChanged(int position, int charsRemoved, int charsAdded) override;
/*
protected:
    /// reimplemented
    virtual void drawInlineObject(QPainter *painter, const QRectF &rect, QTextInlineObject object, int position, const QTextFormat &format);
    /// reimplemented
    virtual void positionInlineObject(QTextInlineObject item, int position, const QTextFormat &format);
    /// reimplemented
    virtual void resizeInlineObject(QTextInlineObject item, int position, const QTextFormat &format);
*/
};

#endif
