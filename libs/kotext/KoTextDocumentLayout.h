/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
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

#include <kotext_export.h>

#include <QAbstractTextDocumentLayout>
#include <QRectF>
#include <QSizeF>
#include <QList>

class KoTextDocumentLayout;
class KoShape;
class KoStyleManager;
class QTextLayout;
class QTextList;
class KoInlineTextObjectManager;

/**
 * KWords text layouter that allows text to flow in multiple frames and around
 * other KWord objects.
 */
class KOTEXT_EXPORT KoTextDocumentLayout : public QAbstractTextDocumentLayout {
    Q_OBJECT
public:
    class LayoutState;
    /// constructor
    explicit KoTextDocumentLayout(QTextDocument *document, KoTextDocumentLayout::LayoutState *layout = 0);
    virtual ~KoTextDocumentLayout();

    /**
     * While the text document is standalone, the text can refer to the character
     * and paragraph styles, and doing so is needed in doing proper text-layout.
     * Setting the stylemanager on this layouter is therefor required if there is one.
     */
    void setStyleManager(KoStyleManager *sm);

    /// set the layoutState for this document layout
    void setLayout(LayoutState *layout);
    /// return if this layoutstate has a proper layoutState object.
    bool hasLayouter() const;

    /**
     * Register the manager for inline objects which is needed to notify variables of layout changes.
     */
    void setInlineObjectTextManager(KoInlineTextObjectManager *iom);
    /// return the currently set manager, or 0 if none is set.
    KoInlineTextObjectManager *inlineObjectTextManager();

    /// Returns the bounding rectangle of block.
    QRectF blockBoundingRect ( const QTextBlock & block ) const;
    /**
     * Returns the total size of the document. This is useful to display
     * widgets since they can use to information to update their scroll bars
     * correctly
     */
    QSizeF documentSize () const;
    /// Draws the layout on the given painter with the given context.
    void draw ( QPainter * painter, const PaintContext & context );
    /// Returns the bounding rectacle of frame. Returns the bounding rectangle of frame.
    QRectF frameBoundingRect ( QTextFrame * frame ) const;
    /**
     * Returns the cursor postion for the given point with the accuracy
     * specified. Returns -1 to indicate failure if no valid cursor position
     * was found.
     * @param point the point in the document
     * @param accuracy if Qt::ExactHit this method will return -1 when not actaully hitting any text
     */
    int hitTest ( const QPointF & point, Qt::HitTestAccuracy accuracy ) const;
    /// reimplemented to always return 1
    int pageCount () const;

    /**
     * Actually do the layout of the text.
     * This method will layout the text into lines and shapes, chunk by chunk. It will
     * return quite quick and have requested for another layout if its unfinished.
     */
    void layout();

    /// stop layouting the text document until manually restarted.
    void interruptLayout();

    /// Add a shape to the list of shapes that the text can run into.
    void addShape(KoShape *shape);

    /// return the list of shapes that will be used to run all the text into.
    virtual QList<KoShape*> shapes() const;

    /// return the current styleManager.  Can be 0 if none set.
    KoStyleManager *styleManager() const;

    /**
     * This inner class is an interface that allows the KoTextDocumentLayout to do rough layout
     * while the LayoutState implementation can do all the boring details.
     */
    class KOTEXT_EXPORT LayoutState {
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
        virtual bool interrupted() = 0;
        /// return the width of the line to be layouted
        virtual double width() = 0;
        /// return the x position of the line to be layouted
        virtual double x() = 0;
        /// return the y position (top of text) of the line to be layouted
        virtual double y() = 0;
        /// return the cursor position (in the document) of the last character that has been positioned in the lay-out
        virtual int cursorPosition() const = 0;
        /// return the y offset of the document at start of shape.
        virtual double docOffsetInShape() const = 0;
        /// when a line is added, update internal vars.  Return true if line does not fit in shape
        virtual bool addLine(QTextLine &line) = 0;
        /// prepare for next paragraph; return false if there is no next parag.
        virtual bool nextParag() = 0;
        /// revert layout to the previous paragraph. Return false if there is no previous paragraph.
        virtual bool previousParag() = 0;
        /// Return the y position of the offset for the current shape (See KoTextShapeData::documentOffset() )
        virtual double documentOffsetInShape() = 0;
        /// paint the document
        virtual void draw(QPainter *painter, const PaintContext & context ) = 0;
        /**
         * After all shapes have been used and there is still text left, use the param shape to continue
         * layout.
         * @param shape the dummy shape to layout in.
         * @return true if the request for continued layout is honored, false otherwise.
         */
        virtual bool setFollowupShape(KoShape *shape) = 0;
        /// remove layout information from the current layout position to the end of the document.
        virtual void clearTillEnd() = 0;

        /// the index in the list of shapes (or frameset) of the shape we are currently layouting.
        int shapeNumber;
        /// the currently layoute shape
        KoShape *shape;
        /// The current paragraph layout.
        QTextLayout *layout;

    protected:
        friend class KoTextDocumentLayout;
        /// see KoTextDocumentLayout::setStyleManager()
        virtual void setStyleManager(KoStyleManager *sm) = 0;
        /// see KoTextDocumentLayout::styleManager()
        virtual KoStyleManager *styleManager() const = 0;
    };

    /**
     * We allow a text document to be shown in more then one shape; which brings up the need to figure out
     * which shape is used for a certain text.
     * @param position the position of the character in the text document we want to locate.
     * @return the shape the text is layed-out in.  Or 0 if there is no shape for that text character.
     */
    KoShape* shapeForPosition(int position) const;

    /// reimplemented from QAbstractTextDocumentLayout
    void documentChanged(int position, int charsRemoved, int charsAdded);

public slots:
    /// make sure we start a layout run
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

private:
    Q_PRIVATE_SLOT(d, void relayoutPrivate())

    class Private;
    Private * const d;
};

#endif
