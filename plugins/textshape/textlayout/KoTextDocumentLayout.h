/* This file is part of the KDE project
 * Copyright (C) 2006-2007, 2009 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006, 2011 Sebastian Sauer <mail@dipe.org>
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

#ifndef KOTEXTDOCUMENTLAYOUT_H
#define KOTEXTDOCUMENTLAYOUT_H

#include "kritatextlayout_export.h"

#include <QAbstractTextDocumentLayout>
#include <QList>
#include <QTextFrame>

class KoShape;
class KoStyleManager;
class KoChangeTracker;
class KoTextRangeManager;
class KoInlineTextObjectManager;
class KoViewConverter;
class KoImageCollection;
class KoShapeAnchor;
class KoTextLayoutRootArea;
class KoTextLayoutRootAreaProvider;
class KoTextLayoutObstruction;

class QRectF;
class QSizeF;

class KRITATEXTLAYOUT_EXPORT KoInlineObjectExtent
{
public:
    explicit KoInlineObjectExtent(qreal ascent = 0, qreal descent = 0);
    qreal m_ascent;
    qreal m_descent;
};


/**
 * Text layouter that allows text to flow in multiple root area and around
 * obstructions.
 */
class KRITATEXTLAYOUT_EXPORT KoTextDocumentLayout : public QAbstractTextDocumentLayout
{
    Q_OBJECT
public:
    /// This struct is a helper for painting of kotext texts.
    struct PaintContext {
        PaintContext()
            : viewConverter(0)
            , imageCollection(0)
            , showFormattingCharacters(false)
            , showSectionBounds(false)
            , showSpellChecking(false)
            , showSelections(true)
            , background(Qt::white)
        {
        }

        /// the QText context
        QAbstractTextDocumentLayout::PaintContext textContext;
        /// A view converter, when set, is used to find out when the zoom is so low that painting of text is unneeded
        const KoViewConverter *viewConverter;

        KoImageCollection *imageCollection;
        bool showFormattingCharacters;
        bool showTableBorders;
        bool showSectionBounds;
        bool showSpellChecking;
        bool showSelections;
        QColor background;
    };

    /// constructor
    explicit KoTextDocumentLayout(QTextDocument *doc, KoTextLayoutRootAreaProvider *provider = 0);
    virtual ~KoTextDocumentLayout();

    /// return the rootAreaProvider.
    KoTextLayoutRootAreaProvider *provider() const;

    /// return the currently set manager, or 0 if none is set.
    KoInlineTextObjectManager *inlineTextObjectManager() const;
    void setInlineTextObjectManager(KoInlineTextObjectManager *manager);

    /// return the currently set manager, or 0 if none is set.
    KoTextRangeManager *textRangeManager() const;
    void setTextRangeManager(KoTextRangeManager *manager);

    /// return the currently set changeTracker, or 0 if none is set.
    KoChangeTracker *changeTracker() const;
    void setChangeTracker(KoChangeTracker *tracker);

    /// return the currently set styleManager, or 0 if none is set.
    KoStyleManager *styleManager() const;
    void setStyleManager(KoStyleManager *manager);

    /// Returns the bounding rectangle of block.
    QRectF blockBoundingRect(const QTextBlock &block) const;
    /**
     * Returns the total size of the document. This is useful to display
     * widgets since they can use to information to update their scroll bars
     * correctly
     */
    virtual QSizeF documentSize() const;

    virtual QRectF frameBoundingRect(QTextFrame*) const;

    /// the default tab size for this document
    qreal defaultTabSpacing() const;

    /// set default tab size for this document
    void setTabSpacing(qreal spacing);

    /// set if this is for a word processor (slight changes in layout may occur)
    void setWordprocessingMode();

    /// is it for a word processor (slight changes in layout may occur)
    bool wordprocessingMode() const;

    /// are the tabs relative to indent or not
    bool relativeTabs(const QTextBlock &block) const;

    /// visualize inline objects during paint
    void showInlineObjectVisualization(bool show);

    /// Calc a bounding box rect of the selection
    QRectF selectionBoundingBox(QTextCursor &cursor) const;

    /// Draws the layout on the given painter with the given context.
    virtual void draw(QPainter * painter, const QAbstractTextDocumentLayout::PaintContext & context);

    /// reimplemented DO NOT CALL - USE HITTEST IN THE ROOTAREAS INSTEAD
    virtual int hitTest(const QPointF & point, Qt::HitTestAccuracy accuracy) const;

    /// reimplemented to always return 1
    virtual int pageCount() const;

    QList<KoShapeAnchor *> textAnchors() const;

    /**
     * Register the anchored obstruction  for run around
     *
     * We have the concept of Obstructions which text has to run around in various ways.
     * We maintain two collections of obstructions. The free which are tied to just a position
     * (tied to pages), and the anchored obstructions which are each anchored to a KoShapeAnchor
     *
     * The free obstructions are collected from the KoTextLayoutRootAreaProvider during layout
     *
     * The anchored obstructions are created in the FloatingAnchorStrategy and registered using
     * this method.
     */
    void registerAnchoredObstruction(KoTextLayoutObstruction *obstruction);


    /**
     * Anchors are special InlineObjects that we detect in positionInlineObject()
     * We save those for later so we can position them during layout instead.
     * During KoTextLayoutArea::layout() we call positionAnchoredObstructions()
     */
    /// remove all anchors and associated obstructions and set up for collecting new ones
    void beginAnchorCollecting(KoTextLayoutRootArea *rootArea);

    /// allow  positionInlineObject() to do anything (incl saving anchors)
    void allowPositionInlineObject(bool allow);

    /// Sets the paragraph rect that will be applied to anchorStrategies being created in
    /// positionInlineObject()
    void setAnchoringParagraphRect(const QRectF &paragraphRect);

    /// Sets the paragraph content rect that will be applied to anchorStrategies being created in
    /// positionInlineObject()
    void setAnchoringParagraphContentRect(const QRectF &paragraphContentRect);

    /// Sets the layoutEnvironment rect that will be applied to anchorStrategies being created in
    /// positionInlineObject()
    void setAnchoringLayoutEnvironmentRect(const QRectF &layoutEnvironmentRect);

    /// Calculates the maximum y of anchored obstructions
    qreal maxYOfAnchoredObstructions(int firstCursorPosition, int lastCursorPosition) const;

    int anchoringSoftBreak() const;

    /// Positions all anchored obstructions
    /// the paragraphRect should be in textDocument coords and not global/document coords
    void positionAnchoredObstructions();

    /// remove inline object
    void removeInlineObject(KoShapeAnchor *textAnchor);

    void clearInlineObjectRegistry(const QTextBlock& block);

    KoInlineObjectExtent inlineObjectExtent(const QTextFragment&);

    /**
     * We allow a text document to be distributed onto a sequence of KoTextLayoutRootArea;
     * which brings up the need to figure out which KoTextLayoutRootArea is used for a certain
     * text.
     * @param position the position of the character in the text document we want to locate.
     * @return the KoTextLayoutRootArea the text is laid-out in. Or 0 if there is no shape for that text character.
     */
    KoTextLayoutRootArea *rootAreaForPosition(int position) const;


    KoTextLayoutRootArea *rootAreaForPoint(const QPointF &point) const;

    /**
     * Remove the root-areas \p rootArea from the list of \a rootAreas() .
     * \param rootArea root-area to remove. If NULL then all root-areas are removed.
     */
    void removeRootArea(KoTextLayoutRootArea *rootArea = 0);

    /// reimplemented from QAbstractTextDocumentLayout
    virtual void documentChanged(int position, int charsRemoved, int charsAdded);

    void setContinuationObstruction(KoTextLayoutObstruction *continuationObstruction);

    /// Return a list of obstructions intersecting current root area (during layout)
    QList<KoTextLayoutObstruction *> currentObstructions();

    QList<KoTextLayoutRootArea *> rootAreas() const;
    QList<KoShape*> shapes() const;

    /// Set should layout be continued when done with current root area
    void setContinuousLayout(bool continuous);

    /// Set \a layout() to be blocked (no layouting will happen)
    void setBlockLayout(bool block);
    bool layoutBlocked() const;

    /// Set \a documentChanged() to be blocked (changes will not result in root-areas being marked dirty)
    void setBlockChanges(bool block);
    bool changesBlocked() const;

    KoTextDocumentLayout* referencedLayout() const;
    void setReferencedLayout(KoTextDocumentLayout *layout);

    /**
     * To be called during layout by KoTextLayoutArea - similar to how qt calls positionInlineObject
     *
     * It searches for anchor text ranges in the given span
     */
    void positionAnchorTextRanges(int pos, int length, const QTextDocument *effectiveDocument);

Q_SIGNALS:
    /**
     * Signal that is emitted during layouting to inform about the progress done so far.
     */
    void layoutProgressChanged(int percent);

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

    void foundAnnotation(KoShape *annotationShape, const QPointF &refPosition);

public Q_SLOTS:
    /**
     * Does the layout of the text.
     * This method will layout the text into sections, tables and textlines,
     * chunk by chunk.
     * It may interrupt itself, @see contiuousLayout
     * calling this method when the layout is not dirty, doesn't take that much
     * time as it doesn't do much, although it does check every root area
     */
    virtual void layout();

    /**
     * Schedules a \a layout call for later using a QTimer::singleShot. Multiple calls
     * to this slot will be compressed into one layout-call to prevent calling layouting
     * to much. Also if meanwhile \a layout was called then the scheduled layout won't
     * be executed.
     */
    virtual void scheduleLayout();

    /**
     * Emits the \a layoutIsDirty signal.
     */
    void emitLayoutIsDirty();

private Q_SLOTS:
    /// Called by \a scheduleLayout to start a \a layout run if not done already meanwhile.
    void executeScheduledLayout();

protected:
    /// reimplemented
    virtual void drawInlineObject(QPainter *painter, const QRectF &rect, QTextInlineObject object, int position, const QTextFormat &format);
    /// reimplemented
    virtual void positionInlineObject(QTextInlineObject item, int position, const QTextFormat &format);
    /// reimplemented
    virtual void resizeInlineObject(QTextInlineObject item, int position, const QTextFormat &format);

    /// should we continue layout when done with current root area
    bool continuousLayout() const;

    void registerInlineObject(const QTextInlineObject &inlineObject);

private:
    class Private;
    Private * const d;

    bool doLayout();
    void updateProgress(const QTextFrame::iterator &it);
};

#endif
