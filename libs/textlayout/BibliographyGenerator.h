/* This file is part of the KDE project
 * Copyright (C) 2011 Smit Patel <smitpatel24@gmail.com>

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
#ifndef BIBLIOGRAPHYGENERATOR_H
#define BIBLIOGRAPHYGENERATOR_H

#include <QObject>
#include <QList>
#include <QTextBlock>

#include <KoBibliographyInfo.h>
#include "textlayout_export.h"

#include <QAbstractTextDocumentLayout>
class KoInlineTextObjectManager;
class KoTextDocumentLayout;
class QTextFrame;

class BibliographyGenerator : public QObject
{
    Q_OBJECT
public:
    explicit BibliographyGenerator(QTextDocument *bibDocument, QTextBlock block, KoBibliographyInfo *bibInfo);
    virtual ~BibliographyGenerator();


public slots:
    void generate();

private:
    //QString resolvePageNumber(const QTextBlock &headingBlock);

    QTextDocument *m_document;
    QTextDocument *m_bibDocument;
    KoBibliographyInfo *m_bibInfo;
    QTextBlock m_block;
    KoTextDocumentLayout *m_documentLayout;
    qreal m_maxTabPosition;
};

class TEXTLAYOUT_EXPORT BibDocumentLayout : public QAbstractTextDocumentLayout
{
    Q_OBJECT
public:
    /// constructor
    explicit BibDocumentLayout(QTextDocument *doc);
    virtual ~BibDocumentLayout();

    /// Returns the bounding rectangle of block.
    virtual QRectF blockBoundingRect(const QTextBlock & block) const;
    /**
     * Returns the total size of the document. This is useful to display
     * widgets since they can use to information to update their scroll bars
     * correctly
     */
    virtual QSizeF documentSize() const;

    /// Draws the layout on the given painter with the given context.
    virtual void draw(QPainter * painter, const QAbstractTextDocumentLayout::PaintContext & context);

    virtual QRectF frameBoundingRect(QTextFrame*) const;

    /// reimplemented DO NOT CALL - USE HITTEST IN THE ROOTAREAS INSTEAD
    virtual int hitTest(const QPointF & point, Qt::HitTestAccuracy accuracy) const;

    /// reimplemented to always return 1
    virtual int pageCount() const;

    /// reimplemented from QAbstractTextDocumentLayout
    virtual void documentChanged(int position, int charsRemoved, int charsAdded);
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
