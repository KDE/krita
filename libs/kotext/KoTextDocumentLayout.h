/* This file is part of the KDE project
 * Copyright (C) 2006-2007, 2009 Thomas Zander <zander@kde.org>
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

#include <QAbstractTextDocumentLayout>
#include <QRectF>
#include <QSizeF>
#include <QList>
#include <QTextFrame>
#include <QTextTableCell>

class KoTextDocumentLayout;
class KoShape;
class KoStyleManager;
class QTextLayout;
class KoInlineTextObjectManager;
class KoViewConverter;
class KoImageCollection;

/**
 * KWords text layouter that allows text to flow in multiple frames and around
 * other KWord objects.
 */
class KOTEXT_EXPORT KoTextDocumentLayout : public QAbstractTextDocumentLayout
{
    Q_OBJECT
public:
    class LayoutState;

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
    explicit KoTextDocumentLayout(QTextDocument *document, KoTextDocumentLayout::LayoutState *layout = 0);
    virtual ~KoTextDocumentLayout();

    /// set the layoutState for this document layout
    void setLayout(LayoutState *layout);

    /// return if this document layout has a proper layoutState object.
    bool hasLayouter() const;

    /**
     * Register the manager for inline objects which is needed to notify variables of layout changes.
     */
    void setInlineTextObjectManager(KoInlineTextObjectManager *manager);

    /// return the currently set manager, or 0 if none is set.
    KoInlineTextObjectManager *inlineTextObjectManager();

    /// Returns the bounding rectangle of block.
    virtual QRectF blockBoundingRect(const QTextBlock & block) const;
    /**
     * Returns the total size of the document. This is useful to display
     * widgets since they can use to information to update their scroll bars
     * correctly
     */
    virtual QSizeF documentSize() const;

    /**
     * Expands the give rect in the horizontal dimension,
     * so that everything in the document will be visible
     * Notably this can give a negative x coord
     * the vertical dimensions are never touched or relied upon
     */
    QRectF expandVisibleRect(const QRectF &rect) const;

    /// Draws the layout on the given painter with the given context.
    virtual void draw(QPainter * painter, const QAbstractTextDocumentLayout::PaintContext & context);

    /// Draws the layout on the given painter with the given context, and pass the zoom.
    void draw(QPainter * painter, const KoTextDocumentLayout::PaintContext & context);

    /// Returns the bounding rectacle of frame. Returns the bounding rectangle of frame.
    virtual QRectF frameBoundingRect(QTextFrame * frame) const;

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

    /**
     * Actually do the layout of the text.
     * This method will layout the text into lines and shapes, chunk by chunk. It will
     * return quite quick and have requested for another layout if its unfinished.
     */
    void layout();

    /// stop layouting the text document until manually restarted.
    void interruptLayout();

    /// get the isInterrupted state from the LayoutState
    bool isInterrupted() const;

    /// Add a shape to the list of shapes that the text can run into.
    void addShape(KoShape *shape);

    /// return the list of shapes that will be used to run all the text into.
    virtual QList<KoShape*> shapes() const;

    /**
     * This inner class is an interface that allows the KoTextDocumentLayout to do rough layout
     * while the LayoutState implementation can do all the boring details.
     */
    class KOTEXT_EXPORT LayoutState
    {
    public:
        LayoutState() : shapeNumber(-1), shape(0), layout(0) {}
        virtual ~LayoutState() {}
        /// start layouting, return false when there is nothing to do
        virtual bool start() = 0;
        /// end layouting
        virtual void end() = 0;
        /// Asks the layout to stop and restart from the beginning.
        virtual void reset() = 0;
        /// returns true if reset has been called.
        virtual bool isInterrupted() const = 0;
        /// return the number of columns of the line to be layouted
        virtual int numColumns() {
            return 0;
        } // if numColumns() returns 0, use width() instead
        /// return the width of the line to be layouted
        virtual qreal width() = 0;
        /// return the x position of the line to be layouted
        virtual qreal x() = 0;
        /// return the y position (top of text) of the line to be layouted
        virtual qreal y() = 0;
        /// return the cursor position (in the document) of the last character that has been positioned in the lay-out
        virtual int cursorPosition() const = 0;
        /// return the y offset of the document at start of shape.
        virtual qreal docOffsetInShape() const = 0;
        /// when a line is added, update internal vars. Inner shapes possibly intersect and split line into more parts. Set processingLine true, when current line is part of line and not last part of line. So baseline will be same for all parts of line. Return true if line does not fit in shape.
        /// expand a bounding rect by excessive indents (indents outside the shape)
        virtual QRectF expandVisibleRect(const QRectF &rect) const = 0;
        virtual bool addLine(QTextLine &line, bool processingLine = false) = 0;
        /// prepare for next paragraph; return false if there is no next parag.
        virtual bool nextParag() = 0;
        /// revert layout to the previous paragraph. Return false if there is no previous paragraph.
        virtual bool previousParag() = 0;
        /// Return the y position of the offset for the current shape (See KoTextShapeData::documentOffset() )
        virtual qreal documentOffsetInShape() = 0;
        /**
         * Paint the document.
         * Paint the whole document, at least within the cliprect as set on the painter.
         * @param painter the painter to draw to.
         * @param context a set of variables able to alter the way things are painted.
         */
        virtual void draw(QPainter *painter, const KoTextDocumentLayout::PaintContext & context) = 0;
        /**
         * After all shapes have been used and there is still text left, use the param shape to continue
         * layout.
         * @param shape the dummy shape to layout in.
         * @return true if the request for continued layout is honored, false otherwise.
         */
        virtual bool setFollowupShape(KoShape *shape) = 0;
        /// remove layout information from the current layout position to the end of the document.
        virtual void clearTillEnd() = 0;
        /// called by the KoTextDocumentLayout to notify the LayoutState of a successfully resized inline object
        virtual void registerInlineObject(const QTextInlineObject &inlineObject) = 0;
        /// called by the KoTextDocumentLayout to find out which if any table cell is hit. Returns 0 when no hit
        virtual QTextTableCell hitTestTable(QTextTable *table, const QPointF &point) = 0;
        /// Inner shapes possibly intersect and split line into more parts. This returns max part height.
        virtual qreal maxLineHeight() const = 0;
        /// the index in the list of shapes (or frameset) of the shape we are currently layouting.
        int shapeNumber;
        /// the shape that is currently being laid out
        KoShape *shape;
        /// The current paragraph layout.
        QTextLayout *layout;
    };

    /**
     * We allow a text document to be shown in more than one shape; which brings up the need to figure out
     * which shape is used for a certain text.
     * @param position the position of the character in the text document we want to locate.
     * @return the shape the text is laid-out in.  Or 0 if there is no shape for that text character.
     */
    KoShape* shapeForPosition(int position) const;

    /// reimplemented from QAbstractTextDocumentLayout
    virtual void documentChanged(int position, int charsRemoved, int charsAdded);

    /**
     * Sets the document's resizing method. @see KoTextDocument::setResizeMethod
     */
    void setResizeMethod(KoTextDocument::ResizeMethod mode);

    /**
     * Returns the document's resizing method. @see KoTextDocument::resizeMethod
     */
    KoTextDocument::ResizeMethod resizeMethod() const;

signals:
    void shapeAdded(KoShape *shape);
    /**
     * Signal is emitted every time a layout run has finished and all text is positioned.
     */
    void finishedLayout();

public slots:
    /// make sure we start a layout run (returns immediately)
    void scheduleLayout();

protected:
    /// the currently set LayoutState
    LayoutState *m_state;

    /// make sure we start a layout run
    virtual void relayout();

    /// reimplemented
    virtual void drawInlineObject(QPainter *painter, const QRectF &rect, QTextInlineObject object, int position, const QTextFormat &format);
    /// reimplemented
    virtual void positionInlineObject(QTextInlineObject item, int position, const QTextFormat &format);
    /// reimplemented
    virtual void resizeInlineObject(QTextInlineObject item, int position, const QTextFormat &format);

    /// make sure we start a layout run (returns immediately)
    /// This method keeps the state data and does not interrupt a runnign layout.
    void scheduleLayoutWithoutInterrupt();

    /// same as hitTest but for a range specified by an iterator
    int hitTestIterated(QTextFrame::iterator begin, QTextFrame::iterator end, const QPointF & point, Qt::HitTestAccuracy accuracy) const;


private:
    // Takes care of auto-resizing the text shape
    Q_PRIVATE_SLOT(d, void adjustSize())
    Q_PRIVATE_SLOT(d, void relayoutPrivate())
    Q_PRIVATE_SLOT(d, void postLayoutHook())

    class Private;
    Private * const d;
};

#endif
