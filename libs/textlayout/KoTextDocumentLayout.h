/* This file is part of the KDE project
 * Copyright (C) 2006-2007, 2009 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006, 2011 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2011 Casper Boemann <cbo@kogmbh.com>
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

#ifndef KOTEXTDOCUMENTLAYOUT_H
#define KOTEXTDOCUMENTLAYOUT_H

#include "kotext_export.h"

#include "KoTextDocument.h"
#include "InlineObjectExtend.h"

#include <QAbstractTextDocumentLayout>
#include <QRectF>
#include <QSizeF>
#include <QList>
#include <QTextFrame>
#include <QTextTableCell>

class KoTextDocumentLayout;
class KoShape;
class KoStyleManager;
class KoChangeTracker;
class QTextLayout;
class KoInlineTextObjectManager;
class KoViewConverter;
class KoImageCollection;
class KoTextAnchor;
class KoTextLayoutRootArea;
class KoTextLayoutRootAreaProvider;

/**
 * KWords text layouter that allows text to flow in multiple frames and around
 * other KWord objects.
 */
class KOTEXT_EXPORT KoTextDocumentLayout : public QAbstractTextDocumentLayout
{
    Q_OBJECT
public:
    /// This struct is a helper for painting of kotext texts.
    struct PaintContext {
        PaintContext() : viewConverter(0), imageCollection(0) { }
        /// the QText context
        QAbstractTextDocumentLayout::PaintContext textContext;
        /// A view converter, when set, is used to find out when the zoom is so low that painting of text is unneeded
        const KoViewConverter *viewConverter;

        KoImageCollection *imageCollection;
    };

    /// constructor
    explicit KoTextDocumentLayout(QTextDocument *doc, KoTextLayoutRootAreaProvider *provider = 0);
    virtual ~KoTextDocumentLayout();

    /// return the rootAreaProvider.
    KoTextLayoutRootAreaProvider *provider() const;

    /// return the currently set manager, or 0 if none is set.
    KoInlineTextObjectManager *inlineTextObjectManager() const;

    /// return the currently set changeTracker, or 0 if none is set.
    KoChangeTracker *changeTracker() const;

    /// return the currently set styleManager, or 0 if none is set.
    KoStyleManager *styleManager() const;

    /// Returns the bounding rectangle of block.
    QRectF blockBoundingRect(const QTextBlock & block) const;
    /**
     * Returns the total size of the document. This is useful to display
     * widgets since they can use to information to update their scroll bars
     * correctly
     */
    virtual QSizeF documentSize() const;

    virtual QRectF frameBoundingRect(QTextFrame*) const;

    /// set default tab size for this document
    void setTabSpacing(qreal spacing);

    /// are the tabs relative to indent or not
    bool relativeTabs() const;

    /// Calc a bounding box rect of the selection
    QRectF selectionBoundingBox(QTextCursor &cursor) const;

    /// Draws the layout on the given painter with the given context.
    virtual void draw(QPainter * painter, const QAbstractTextDocumentLayout::PaintContext & context);

    /// Draws the layout on the given painter with the given context, and pass the zoom.
    void draw(QPainter * painter, const KoTextDocumentLayout::PaintContext & context);

    /**
     * Returns the cursor postion for the given point with the accuracy
     * specified. Returns -1 to indicate failure if no valid cursor position
     * was found.
     * @param point the point in the document
     * @param accuracy if Qt::ExactHit this method will return -1 when not actaully hitting any text
     */
    virtual int hitTest(const QPointF & point, Qt::HitTestAccuracy accuracy) const;

    /// reimplemented to always return 1
    virtual int pageCount() const;

    /// Add a shape to the list of shapes that the text can occupy.
    void addShape(KoShape *shape);

    // add inline object
    void insertInlineObject(KoTextAnchor * textAnchor);
    // remove all inline objects which document position is bigger or equal to resetPosition
    void resetInlineObject(int resetPosition);
    // remove inline object
    void removeInlineObject(KoTextAnchor * textAnchor);

    InlineObjectExtend inlineObjectExtend(const QTextFragment&);

    /// Registers the shape as being relevant for run around at this moment in time
    void registerObstruction(KoShape *shape);

    /// Updates the registration of the shape for run around
    void updateObstruction(KoShape *shape);

    /// Clear all registrations of shapest for run around
    void unregisterAllObstructions();


    /**
     * We allow a text document to be distributed onto a sequence of KoTextLayoutRootArea;
     * which brings up the need to figure out which KoTextLayoutRootArea is used for a certain
     * text.
     * @param position the position of the character in the text document we want to locate.
     * @return the KoTextLayoutRootArea the text is laid-out in. Or 0 if there is no shape for that text character.
     */
    KoTextLayoutRootArea *rootAreaForPosition(int position) const;

    /// reimplemented from QAbstractTextDocumentLayout
    virtual void documentChanged(int position, int charsRemoved, int charsAdded);



    QList<KoTextLayoutRootArea *> rootAreas() const;
    QList<KoShape*> shapes() const;

signals:
    /**
     * Signal is emitted every time a layout run has completely finished (all text is positioned).
     */
    void finishedLayout();

    /**
     * Signal is emitted when emitLayoutIsDirty() is called which happens at
     * least when a root area is marked as dirty.
     * @see emitLayoutIsDirty
     */
    void layoutIsDirty();

public slots:
    /**
     * Does the layout of the text.
     * This method will layout the text into sections,tables and textlines,
     * chunk by chunk.
     * It may interrupt itself, @see contiuousLayout
     * calling this method when the layout is not dirty, doesn't take that much
     * time as it doesn't do much, although it does check every root area
     */
    virtual void layout();

    /**
     * Schedules a \a layout call for later using a QTimer::singleShot. Multiple calls
     * to this slot will be compressed into one layout-call to prevent calling layouting
     * to much.
     */
    virtual void scheduleLayout();

    void emitLayoutIsDirty();

protected:
    /// reimplemented
    virtual void drawInlineObject(QPainter *painter, const QRectF &rect, QTextInlineObject object, int position, const QTextFormat &format);
    /// reimplemented
    virtual void positionInlineObject(QTextInlineObject item, int position, const QTextFormat &format);
    /// reimplemented
    virtual void resizeInlineObject(QTextInlineObject item, int position, const QTextFormat &format);

    /// same as hitTest but for a range specified by an iterator
    int hitTestIterated(QTextFrame::iterator begin, QTextFrame::iterator end, const QPointF & point, Qt::HitTestAccuracy accuracy) const;

    /// should we continue layout when done with current root area
    bool continuousLayout();

    void registerInlineObject(const QTextInlineObject &inlineObject);
    
    // Fill m_currentLineObstructions list with actual obstructions for current page
    void refreshCurrentPageObstructions();

private:
    class Private;
    Private * const d;
};

#endif
