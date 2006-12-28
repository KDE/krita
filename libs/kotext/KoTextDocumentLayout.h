/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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

#include <koffice_export.h>

#include <QAbstractTextDocumentLayout>
#include <QRectF>
#include <QSizeF>
#include <QList>

class LayoutState;
class LayoutStatePrivate;
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
    /// constructor
    KoTextDocumentLayout(QTextDocument *document);
    virtual ~KoTextDocumentLayout();

    /**
     * While the text document is standalone, the text can refer to the character
     * and paragraph styles, and doing so is needed in doing proper text-layout.
     * Setting the stylemanager on this layouter is therefor required if there is one.
     */
    void setStyleManager(KoStyleManager *sm);

    void setInlineObjectTextManager(KoInlineTextObjectManager *iom);

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

    void interruptLayout();

    void addShape(KoShape *shape);

    virtual QList<KoShape*> shapes() const { return m_shapes; }

    class KOTEXT_EXPORT LayoutState {
    public:
        LayoutState(KoTextDocumentLayout *parent);
        /// start layouting, return false when there is nothing to do
        bool start();
        /// end layouting
        void end();
        void reset();
        /// returns true if reset has been called.
        bool interrupted();
        double width();
        double x();
        double y();
        /// return the y offset of the document at start of shape.
        double docOffsetInShape() const;
        /// when a line is added, update internal vars.  Return true if line does not fit in shape
        bool addLine(const QTextLine &line);
        /// prepare for next paragraph; return false if there is no next parag.
        bool nextParag();
        double documentOffsetInShape();

        int shapeNumber;
        KoShape *shape;
        QTextLayout *layout;

    private:
        friend class KoTextDocumentLayout;
        void setStyleManager(KoStyleManager *sm);
        void updateBorders();
        double topMargin();
        double listIndent();
        void cleanupShapes();
        void cleanupShape(KoShape *daShape);
        void nextShape();

        void resetPrivate();

        LayoutStatePrivate *d;

    };

    /// reimplemented from QAbstractTextDocumentLayout
    void documentChanged(int position, int charsRemoved, int charsAdded);

protected:
    LayoutState *m_state;

private slots:
    void relayout();

private:
    /// reimplemented
    void drawInlineObject(QPainter *painter, const QRectF &rect, QTextInlineObject object, int position, const QTextFormat &format);
    /// reimplemented
    void positionInlineObject(QTextInlineObject item, int position, const QTextFormat &format);
    /// reimplemented
    void resizeInlineObject(QTextInlineObject item, int position, const QTextFormat &format);

    /// paints paragraph specific decorations.
    void decorateParagraph(QPainter *painter, const QTextBlock &block);

    virtual void scheduleLayout();

private:
    KoStyleManager *m_styleManager;
    KoInlineTextObjectManager *m_inlineTextObjectManager;
    int m_lastKnownFrameCount;
    bool m_moreFramesRequested;
    QList<KoShape *> m_shapes;
    bool m_scheduled;
};

class ListItemsPrivate;
/// \internal helper class for calculating text-lists prefixes and indents
class ListItemsHelper {
public:
    ListItemsHelper(QTextList *textList, const QFont &font);
    ~ListItemsHelper();
    void recalculate();
    static bool needsRecalc(QTextList *textList);

private:
    ListItemsPrivate *d;
};

#endif
