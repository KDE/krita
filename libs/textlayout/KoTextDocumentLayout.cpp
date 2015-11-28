/* This file is part of the KDE project
 * Copyright (C) 2006-2007, 2009-2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2010 Johannes Simon <johannes.simon@gmail.com>
 * Copyright (C) 2011-2013 KO GmbH <cbo@kogmbh.com>
 * Copyright (C) 2011-2013 C.Boemann <cbo@boemann.dk>
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

#include "KoTextDocumentLayout.h"

#include "styles/KoStyleManager.h"
#include "KoTextBlockData.h"
#include "KoInlineTextObjectManager.h"
#include "KoTextLayoutRootArea.h"
#include "KoTextLayoutRootAreaProvider.h"
#include "KoTextLayoutObstruction.h"
#include "FrameIterator.h"
#include "InlineAnchorStrategy.h"
#include "FloatingAnchorStrategy.h"
#include "AnchorStrategy.h"
#include "IndexGeneratorManager.h"

#include <KoShapeAnchor.h>
#include <KoAnchorInlineObject.h>
#include <KoAnchorTextRange.h>
#include <KoTextRangeManager.h>
#include <KoTextPage.h>
#include <KoPostscriptPaintDevice.h>
#include <KoShape.h>
#include <KoShapeContainer.h>
#include <KoAnnotation.h>
#include <KoTextDocument.h>
#include <KoUnit.h>
#include <KoParagraphStyle.h>
#include <KoTableStyle.h>

#include <TextLayoutDebug.h>
#include <QTextBlock>
#include <QTextTable>
#include <QTimer>
#include <QList>

extern int qt_defaultDpiY();


KoInlineObjectExtent::KoInlineObjectExtent(qreal ascent, qreal descent)
    : m_ascent(ascent),
      m_descent(descent)
{
}

class Q_DECL_HIDDEN KoTextDocumentLayout::Private
{
public:
    Private(KoTextDocumentLayout *)
       : styleManager(0)
       , changeTracker(0)
       , inlineTextObjectManager(0)
       , textRangeManager(0)
       , provider(0)
       , layoutPosition(0)
       , anchoringRootArea(0)
       , anchoringIndex(0)
       , anAnchorIsPlaced(false)
       , anchoringSoftBreak(INT_MAX)
       , allowPositionInlineObject(true)
       , continuationObstruction(0)
       , referencedLayout(0)
       , defaultTabSizing(0)
       , y(0)
       , isLayouting(false)
       , layoutScheduled(false)
       , continuousLayout(true)
       , layoutBlocked(false)
       , changesBlocked(false)
       , restartLayout(false)
       , wordprocessingMode(false)
       , showInlineObjectVisualization(false)
    {
    }
    KoStyleManager *styleManager;

    KoChangeTracker *changeTracker;

    KoInlineTextObjectManager *inlineTextObjectManager;
    KoTextRangeManager *textRangeManager;
    KoTextLayoutRootAreaProvider *provider;
    KoPostscriptPaintDevice *paintDevice;
    QList<KoTextLayoutRootArea *> rootAreaList;
    FrameIterator *layoutPosition;

    QHash<int, KoInlineObjectExtent> inlineObjectExtents; // maps text-position to whole-line-height of an inline object
    int inlineObjectOffset;
    QList<KoShapeAnchor *> textAnchors; // list of all inserted inline objects
    QList<KoShapeAnchor *> foundAnchors; // anchors found in an iteration run
    KoTextLayoutRootArea *anchoringRootArea;
    int anchoringIndex; // index of last not positioned inline object inside textAnchors
    bool anAnchorIsPlaced;
    int anchoringSoftBreak;
    QRectF anchoringParagraphRect;
    QRectF anchoringParagraphContentRect;
    QRectF anchoringLayoutEnvironmentRect;
    bool allowPositionInlineObject;

    QHash<KoShape*,KoTextLayoutObstruction*> anchoredObstructions; // all obstructions created because KoShapeAnchor from m_textAnchors is in text
    QList<KoTextLayoutObstruction*> freeObstructions; // obstructions affecting the current rootArea, and not anchored to text
    KoTextLayoutObstruction *continuationObstruction;

    KoTextDocumentLayout *referencedLayout;

    QHash<KoInlineObject *, KoTextLayoutRootArea *> rootAreaForInlineObject;

    qreal defaultTabSizing;
    qreal y;
    bool isLayouting;
    bool layoutScheduled;
    bool continuousLayout;
    bool layoutBlocked;
    bool changesBlocked;
    bool restartLayout;
    bool wordprocessingMode;
    bool showInlineObjectVisualization;
};


// ------------------- KoTextDocumentLayout --------------------
KoTextDocumentLayout::KoTextDocumentLayout(QTextDocument *doc, KoTextLayoutRootAreaProvider *provider)
        : QAbstractTextDocumentLayout(doc),
        d(new Private(this))
{
    d->paintDevice = new KoPostscriptPaintDevice();
    d->provider = provider;
    setPaintDevice(d->paintDevice);

    d->styleManager = KoTextDocument(document()).styleManager();
    d->changeTracker = KoTextDocument(document()).changeTracker();
    d->inlineTextObjectManager = KoTextDocument(document()).inlineTextObjectManager();
    d->textRangeManager = KoTextDocument(document()).textRangeManager();

    setTabSpacing(MM_TO_POINT(23)); // use same default as open office

    d->layoutPosition = new FrameIterator(doc->rootFrame());
}

KoTextDocumentLayout::~KoTextDocumentLayout()
{
    delete d->paintDevice;
    delete d->layoutPosition;
    qDeleteAll(d->freeObstructions);
    qDeleteAll(d->anchoredObstructions);
    qDeleteAll(d->textAnchors);
    delete d;
}

KoTextLayoutRootAreaProvider *KoTextDocumentLayout::provider() const
{
    return d->provider;
}

void KoTextDocumentLayout::setWordprocessingMode()
{
    d->wordprocessingMode = true;
}

bool KoTextDocumentLayout::wordprocessingMode() const
{
    return d->wordprocessingMode;
}


bool KoTextDocumentLayout::relativeTabs(const QTextBlock &block) const
{
    return KoTextDocument(document()).relativeTabs() 
                && KoTextDocument(block.document()).relativeTabs();
}

KoInlineTextObjectManager *KoTextDocumentLayout::inlineTextObjectManager() const
{
    return d->inlineTextObjectManager;
}

void KoTextDocumentLayout::setInlineTextObjectManager(KoInlineTextObjectManager *manager)
{
    d->inlineTextObjectManager = manager;
}

KoTextRangeManager *KoTextDocumentLayout::textRangeManager() const
{
    return d->textRangeManager;
}

void KoTextDocumentLayout::setTextRangeManager(KoTextRangeManager *manager)
{
    d->textRangeManager = manager;
}

KoChangeTracker *KoTextDocumentLayout::changeTracker() const
{
    return d->changeTracker;
}

void KoTextDocumentLayout::setChangeTracker(KoChangeTracker *tracker)
{
    d->changeTracker = tracker;
}

KoStyleManager *KoTextDocumentLayout::styleManager() const
{
    return d->styleManager;
}

void KoTextDocumentLayout::setStyleManager(KoStyleManager *manager)
{
    d->styleManager = manager;
}

QRectF KoTextDocumentLayout::blockBoundingRect(const QTextBlock &block) const
{
    QTextLayout *layout = block.layout();
    return layout->boundingRect();
}

QSizeF KoTextDocumentLayout::documentSize() const
{
    return QSizeF();
}

QRectF KoTextDocumentLayout::selectionBoundingBox(QTextCursor &cursor) const
{
    QRectF retval;
    Q_FOREACH (const KoTextLayoutRootArea *rootArea, d->rootAreaList) {
        if (!rootArea->isDirty()) {
            QRectF areaBB  = rootArea->selectionBoundingBox(cursor);
            if (areaBB.isValid()) {
                retval |= areaBB;
            }
        }
    }
    return retval;
}


void KoTextDocumentLayout::draw(QPainter *painter, const QAbstractTextDocumentLayout::PaintContext &context)
{
    // WARNING Text shapes ask their root area directly to paint.
    // It saves a lot of extra traversal, that is quite costly for big
    // documents
    Q_UNUSED(painter);
    Q_UNUSED(context);
}


int KoTextDocumentLayout::hitTest(const QPointF &point, Qt::HitTestAccuracy accuracy) const
{
    Q_UNUSED(point);
    Q_UNUSED(accuracy);
    Q_ASSERT(false); //we should no longer call this method.
    // There is no need and is just slower than needed
    // call rootArea->hitTest() directly
    // root area is available through KoTextShapeData
    return -1;
}

int KoTextDocumentLayout::pageCount() const
{
    return 1;
}

void KoTextDocumentLayout::setTabSpacing(qreal spacing)
{
    d->defaultTabSizing = spacing;
}

qreal KoTextDocumentLayout::defaultTabSpacing() const
{
    return d->defaultTabSizing;
}

// this method is called on every char inserted or deleted, on format changes, setting/moving of variables or objects.
void KoTextDocumentLayout::documentChanged(int position, int charsRemoved, int charsAdded)
{
    if (d->changesBlocked) {
        return;
    }

    int from = position;
    const int to = from + charsAdded;
    while (from < to) { // find blocks that have been added
        QTextBlock block = document()->findBlock(from);
        if (! block.isValid())
            break;
        if (from == block.position() && block.textList()) {
            KoTextBlockData data(block);
            data.setCounterWidth(-1);
        }

        from = block.position() + block.length();
    }

    // Mark the to the position corresponding root-areas as dirty. If there is no root-area for the position then we
    // don't need to mark anything dirty but still need to go on to force a scheduled relayout.
    if (!d->rootAreaList.isEmpty()) {
        KoTextLayoutRootArea *fromArea;
        if (position) {
            fromArea = rootAreaForPosition(position-1);
        } else {
            fromArea = d->rootAreaList.at(0);
        }
        int startIndex = fromArea ? qMax(0, d->rootAreaList.indexOf(fromArea)) : 0;
        int endIndex = startIndex;
        if (charsRemoved != 0 || charsAdded != 0) {
            // If any characters got removed or added make sure to also catch other root-areas that may be
            // affected by this change. Note that adding, removing or formatting text will always charsRemoved>0
            // and charsAdded>0 cause they are changing a range of characters. One case where both is zero is if
            // the content of a variable changed (see KoVariable::setValue which calls publicDocumentChanged). In
            // those cases we only need to relayout the root-area dirty where the variable is on.
            KoTextLayoutRootArea *toArea = fromArea ? rootAreaForPosition(position + qMax(charsRemoved, charsAdded) + 1) : 0;
            if (toArea) {
                if (toArea != fromArea) {
                    endIndex = qMax(startIndex, d->rootAreaList.indexOf(toArea));
                } else {
                    endIndex = startIndex;
                }
            } else {
                endIndex = d->rootAreaList.count() - 1;
            }
            // The previous and following root-area of that range are selected too cause they can also be affect by
            // changes done to the range of root-areas.
            if (startIndex >= 1)
                --startIndex;
            if (endIndex + 1 < d->rootAreaList.count())
                ++endIndex;
        }
        // Mark all selected root-areas as dirty so they are relayouted.
        for(int i = startIndex; i <= endIndex; ++i) {
            if (d->rootAreaList.size() > i && d->rootAreaList[i])
                d->rootAreaList[i]->setDirty();
        }
    }

    // Once done we emit the layoutIsDirty signal. The consumer (e.g. the TextShape) will then layout dirty
    // root-areas and if needed following ones which got dirty cause content moved to them. Also this will
    // created new root-areas using KoTextLayoutRootAreaProvider::provide if needed.
    emitLayoutIsDirty();
}

KoTextLayoutRootArea *KoTextDocumentLayout::rootAreaForPosition(int position) const
{
    QTextBlock block = document()->findBlock(position);
    if (!block.isValid())
        return 0;
    QTextLine line = block.layout()->lineForTextPosition(position - block.position());
    if (!line.isValid())
        return 0;

    foreach (KoTextLayoutRootArea *rootArea, d->rootAreaList) {
        QRectF rect = rootArea->boundingRect(); // should already be normalized()
        if (rect.width() <= 0.0 && rect.height() <= 0.0) // ignore the rootArea if it has a size of QSizeF(0,0)
            continue;
        QPointF pos = line.position();
        qreal x = pos.x();
        qreal y = pos.y();

        //0.125 needed since Qt Scribe works with fixed point
        if (x + 0.125 >= rect.x() && x<= rect.right() && y + line.height() + 0.125 >= rect.y() && y <= rect.bottom()) {
            return rootArea;
        }
    }
    return 0;
}

KoTextLayoutRootArea *KoTextDocumentLayout::rootAreaForPoint(const QPointF &point) const
{
    Q_FOREACH (KoTextLayoutRootArea *rootArea, d->rootAreaList) {
        if (!rootArea->isDirty()) {
            if (rootArea->boundingRect().contains(point)) {
                return rootArea;
            }
        }
    }
    return 0;
}

void KoTextDocumentLayout::showInlineObjectVisualization(bool show)
{
    d->showInlineObjectVisualization = show;
}

void KoTextDocumentLayout::drawInlineObject(QPainter *painter, const QRectF &rect, QTextInlineObject object, int position, const QTextFormat &format)
{
    Q_ASSERT(format.isCharFormat());
    if (d->inlineTextObjectManager == 0)
        return;
    QTextCharFormat cf = format.toCharFormat();
    if (d->showInlineObjectVisualization) {
        QColor color = cf.foreground().color();
        // initial idea was to use Qt::gray (#A0A0A4)
        // for non-black text on non-white background it was derived to use
        // the text color with a transparency of 0x5F, so white-gray (0xFF-0xA0)
        color.setAlpha(0x5F);
        cf.setBackground(QBrush(color));
    }
    KoInlineObject *obj = d->inlineTextObjectManager->inlineTextObject(cf);
    if (obj)
        obj->paint(*painter, paintDevice(), document(), rect, object, position, cf);
}

QList<KoShapeAnchor *> KoTextDocumentLayout::textAnchors() const
{
    return d->textAnchors;
}

void KoTextDocumentLayout::registerAnchoredObstruction(KoTextLayoutObstruction *obstruction)
{
    d->anchoredObstructions.insert(obstruction->shape(), obstruction);
}

qreal KoTextDocumentLayout::maxYOfAnchoredObstructions(int firstCursorPosition, int lastCursorPosition) const
{
    qreal y = 0.0;
    int index = 0;

    while (index < d->anchoringIndex) {
        Q_ASSERT(index < d->textAnchors.count());
        KoShapeAnchor *anchor = d->textAnchors[index];
        if (anchor->flowWithText()) {
            if (anchor->textLocation()->position() >= firstCursorPosition
                        && anchor->textLocation()->position() <= lastCursorPosition) {
                y = qMax(y, anchor->shape()->boundingRect().bottom() - anchor->shape()->parent()->boundingRect().y());
            }
        }
        ++index;
    }
    return y;
}

int KoTextDocumentLayout::anchoringSoftBreak() const
{
    return d->anchoringSoftBreak;
}
void KoTextDocumentLayout::positionAnchoredObstructions()
{
    if (!d->anchoringRootArea)
        return;
    KoTextPage *page = d->anchoringRootArea->page();
    if (!page)
        return;
    if (d->anAnchorIsPlaced)
        return;

    // The specs define 3 different anchor modes using the
    // draw:wrap-influence-on-position. We only implement the
    // once-successive and decided against supporting the other
    // two modes cause;
    // 1. The first mode, once-concurrently, is only for backward-compatibility
    //    with pre OpenOffice.org 1.1. No other application supports that. It
    //    should never have been added to the specs.
    // 2. The iterative mode is undocumented and it's absolute unclear how to
    //    implement it in a way that we would earn 100% the same results OO.org
    //    produces. In fact by looking at the OO.org source-code there seem to
    //    be lot of extra-conditions, assumptions and OO.org related things going
    //    on to handle that mode. We tried to support that mode once and it did
    //    hit us bad, our source-code become way more worse, layouting slower and
    //    the result was still different from OO.org. So, we decided it's not
    //    worth it.
    // 3. The explanation provided at http://lists.oasis-open.org/archives/office/200409/msg00018.html
    //    why the specs support those 3 anchor modes is, well, poor. It just doesn't
    //    make sense. The specs should be fixed.
    // 4. The only support mode, the once-successive, is the one (only) support by
    //    MSOffice. It's clear, logical, easy and needs to be supported by all
    //    major office-suites that like to be compatible with MSOffice and OO.org.
    if (d->anchoringIndex < d->textAnchors.count()) {
        KoShapeAnchor *textAnchor = d->textAnchors[d->anchoringIndex];
        AnchorStrategy *strategy = static_cast<AnchorStrategy *>(textAnchor->placementStrategy());

        strategy->setPageRect(page->rect());
        strategy->setPageContentRect(page->contentRect());
        strategy->setPageNumber(page->pageNumber());

        if (strategy->moveSubject()) {
            ++d->anchoringIndex;
            d->anAnchorIsPlaced = true;
        }
    }
}

void KoTextDocumentLayout::setAnchoringParagraphRect(const QRectF &paragraphRect)
{
    d->anchoringParagraphRect = paragraphRect;
}

void KoTextDocumentLayout::setAnchoringParagraphContentRect(const QRectF &paragraphContentRect)
{
    d->anchoringParagraphContentRect = paragraphContentRect;
}

void KoTextDocumentLayout::setAnchoringLayoutEnvironmentRect(const QRectF &layoutEnvironmentRect)
{
    d->anchoringLayoutEnvironmentRect = layoutEnvironmentRect;
}

void KoTextDocumentLayout::allowPositionInlineObject(bool allow)
{
    d->allowPositionInlineObject = allow;
}

// This method is called by qt every time  QTextLine.setWidth()/setNumColumns() is called
void KoTextDocumentLayout::positionInlineObject(QTextInlineObject item, int position, const QTextFormat &format)
{
    // Note: "item" used to be what was positioned. We don't actually use qtextinlineobjects anymore
    // for our inline objects, but get the id from the format.
    Q_UNUSED(item);
    //We are called before layout so that we can position objects
    Q_ASSERT(format.isCharFormat());
    if (d->inlineTextObjectManager == 0)
        return;
    if (!d->allowPositionInlineObject)
        return;
    QTextCharFormat cf = format.toCharFormat();
    KoInlineObject *obj = d->inlineTextObjectManager->inlineTextObject(cf);
    // We need some special treatment for anchors as they need to position their object during
    // layout and not this early
    KoAnchorInlineObject *anchorObject = dynamic_cast<KoAnchorInlineObject *>(obj);
    if (anchorObject && d->anchoringRootArea->associatedShape()) {
        // The type can only be KoShapeAnchor::AnchorAsCharacter since it's inline
        KoShapeAnchor *anchor = anchorObject->anchor();
        d->foundAnchors.append(anchor);

        // if there is no anchor strategy set then create one
        if (!anchor->placementStrategy()) {
            anchor->setPlacementStrategy(new InlineAnchorStrategy(anchorObject, d->anchoringRootArea));
            d->textAnchors.append(anchor);
            anchorObject->updatePosition(document(), position, cf); // by extension calls updateContainerModel
        }
        static_cast<AnchorStrategy *>(anchor->placementStrategy())->setParagraphRect(d->anchoringParagraphRect);
        static_cast<AnchorStrategy *>(anchor->placementStrategy())->setParagraphContentRect(d->anchoringParagraphContentRect);
        static_cast<AnchorStrategy *>(anchor->placementStrategy())->setLayoutEnvironmentRect(d->anchoringLayoutEnvironmentRect);
    }
    else if (obj) {
        obj->updatePosition(document(), position, cf);
    }
}

// This method is called by KoTextLauoutArea every time it encounters a KoAnchorTextRange
void KoTextDocumentLayout::positionAnchorTextRanges(int pos, int length, const QTextDocument *effectiveDocument)
{
    if (!d->allowPositionInlineObject)
        return;

    if (!textRangeManager()) {
        return;
    }
    QHash<int, KoTextRange *> ranges = textRangeManager()->textRangesChangingWithin(effectiveDocument, pos, pos+length, pos, pos+length);

    Q_FOREACH (KoTextRange *range, ranges.values()) {
        KoAnchorTextRange *anchorRange = dynamic_cast<KoAnchorTextRange *>(range);
        if (anchorRange) {
            // We need some special treatment for anchors as they need to position their object during
            // layout and not this early
            KoShapeAnchor *anchor = anchorRange->anchor();
            d->foundAnchors.append(anchor);

            // At the beginAnchorCollecting the strategy is cleared, so this if will be entered
            // every time we layout a page (though not every time for the inner repeats due to anchors)
            if (!anchor->placementStrategy()) {
                int index = d->textAnchors.count();
                anchor->setPlacementStrategy(new FloatingAnchorStrategy(anchorRange, d->anchoringRootArea));

                // The purpose of following code-block is to be sure that our paragraph-anchors are
                // properly sorted by their z-index so the FloatingAnchorStrategy::checkStacking
                // logic stack in the proper order. Bug 274512 has a testdoc for this attached.
                if (index > 0 &&
                    anchor->anchorType() == KoShapeAnchor::AnchorParagraph &&
                    (anchor->horizontalRel() == KoShapeAnchor::HParagraph || anchor->horizontalRel() == KoShapeAnchor::HParagraphContent) &&
                    (anchor->horizontalPos() == KoShapeAnchor::HLeft || anchor->horizontalPos() == KoShapeAnchor::HRight)) {
                    QTextBlock anchorBlock = document()->findBlock(anchorRange->position());
                    for(int i = index - 1; i >= 0; --i) {
                        KoShapeAnchor *a = d->textAnchors[i];
                        if (a->anchorType() != anchor->anchorType())
                            break;
                        if (a->horizontalPos() != anchor->horizontalPos())
                            break;
                        if (document()->findBlock(a->textLocation()->position()) != anchorBlock)
                            break;
                        if (a->shape()->zIndex() < anchor->shape()->zIndex())
                            break;
                        --index;
                    }
                }
                d->textAnchors.insert(index, anchor);
                anchorRange->updateContainerModel();
            }
            static_cast<AnchorStrategy *>(anchor->placementStrategy())->setParagraphRect(d->anchoringParagraphRect);
            static_cast<AnchorStrategy *>(anchor->placementStrategy())->setParagraphContentRect(d->anchoringParagraphContentRect);
            static_cast<AnchorStrategy *>(anchor->placementStrategy())->setLayoutEnvironmentRect(d->anchoringLayoutEnvironmentRect);
        }
        KoAnnotation *annotation = dynamic_cast<KoAnnotation *>(range);
        if (annotation) {
            int position = range->rangeStart();
            QTextBlock block = range->document()->findBlock(position);
            QTextLine line = block.layout()->lineForTextPosition(position - block.position());
            QPointF refPos(line.cursorToX(position - block.position()), line.y());

            KoShape *refShape = d->anchoringRootArea->associatedShape();
            //KoTextShapeData *refTextShapeData;
            //refPos += QPointF(refTextShapeData->leftPadding(), -refTextShapeData->documentOffset() + refTextShapeData->topPadding());

            refPos += QPointF(0, -d->anchoringRootArea->top());
            refPos = refShape->absoluteTransformation(0).map(refPos);

            //FIXME we need a more precise position than anchorParagraph Rect
            emit foundAnnotation(annotation->annotationShape(), refPos);
        }
    }
}

void KoTextDocumentLayout::beginAnchorCollecting(KoTextLayoutRootArea *rootArea)
{
    for(int i = 0; i<d->textAnchors.size(); i++ ) {
        d->textAnchors[i]->setPlacementStrategy(0);
    }

    qDeleteAll(d->anchoredObstructions);
    d->anchoredObstructions.clear();
    d->textAnchors.clear();

    d->anchoringIndex = 0;
    d->anAnchorIsPlaced = false;
    d->anchoringRootArea = rootArea;
    d->allowPositionInlineObject = true;
    d->anchoringSoftBreak = INT_MAX;
}

void KoTextDocumentLayout::resizeInlineObject(QTextInlineObject item, int position, const QTextFormat &format)
{
    // Note: This method is called by qt during layout AND during paint
    Q_ASSERT(format.isCharFormat());
    if (d->inlineTextObjectManager == 0)
        return;
    QTextCharFormat cf = format.toCharFormat();
    KoInlineObject *obj = d->inlineTextObjectManager->inlineTextObject(cf);

    if (!obj) {
        return;
    }

    if (d->isLayouting) {
        d->rootAreaForInlineObject[obj] = d->anchoringRootArea;
    }
    KoTextLayoutRootArea *rootArea = d->rootAreaForInlineObject.value(obj);

    if (rootArea == 0 || rootArea->associatedShape() == 0)
        return;

    QTextDocument *doc = document();
    QVariant v;
    v.setValue(rootArea->page());
    doc->addResource(KoTextDocument::LayoutTextPage, KoTextDocument::LayoutTextPageUrl, v);
    obj->resize(doc, item, position, cf, paintDevice());
    registerInlineObject(item);
}

void KoTextDocumentLayout::emitLayoutIsDirty()
{
    emit layoutIsDirty();
}

void KoTextDocumentLayout::layout()
{
    if (d->layoutBlocked) {
        return;
    }

    if (IndexGeneratorManager::instance(document())->generate()) {
        return;
    }

    Q_ASSERT(!d->isLayouting);
    d->isLayouting = true;

    bool finished;
    do {
        // Try to layout as long as d->restartLayout==true. This can happen for example if
        // a schedule layout call interrupts the layouting and asks for a new layout run.
        finished = doLayout();
    } while (d->restartLayout);

    Q_ASSERT(d->isLayouting);
    d->isLayouting = false;

    if (finished) {
        // We are only finished with layouting if continuousLayout()==true.
        emit finishedLayout();
    }
}

RootAreaConstraint constraintsForPosition(QTextFrame::iterator it, bool previousIsValid)
{
    RootAreaConstraint constraints;
    constraints.masterPageName = QString::null;
    constraints.visiblePageNumber = -1;
    constraints.newPageForced = false;
    QTextBlock block = it.currentBlock();
    QTextTable *table = qobject_cast<QTextTable*>(it.currentFrame());
    if (block.isValid()) {
        constraints.masterPageName = block.blockFormat().stringProperty(KoParagraphStyle::MasterPageName);
        if (block.blockFormat().hasProperty(KoParagraphStyle::PageNumber)) {
            constraints.visiblePageNumber = block.blockFormat().intProperty(KoParagraphStyle::PageNumber);
        }
        constraints.newPageForced = block.blockFormat().intProperty(KoParagraphStyle::BreakBefore) == KoText::PageBreak;
    }
    if (table) {
        constraints.masterPageName = table->frameFormat().stringProperty(KoTableStyle::MasterPageName);
        if (table->frameFormat().hasProperty(KoTableStyle::PageNumber)) {
            constraints.visiblePageNumber = table->frameFormat().intProperty(KoTableStyle::PageNumber);
        }
        constraints.newPageForced = table->frameFormat().intProperty(KoTableStyle::BreakBefore) == KoText::PageBreak;
    }

    if (!constraints.masterPageName.isEmpty()) {
        constraints.newPageForced = true;
    }
    if (previousIsValid && !constraints.newPageForced) {
        it--;
        block = it.currentBlock();
        table = qobject_cast<QTextTable*>(it.currentFrame());
        if (block.isValid()) {
            constraints.newPageForced = block.blockFormat().intProperty(KoParagraphStyle::BreakAfter) == KoText::PageBreak;
        }
        if (table) {
            constraints.newPageForced = table->frameFormat().intProperty(KoTableStyle::BreakAfter) == KoText::PageBreak;
        }
    }

    return constraints;
}

bool KoTextDocumentLayout::doLayout()
{
    delete d->layoutPosition;
    d->layoutPosition = new FrameIterator(document()->rootFrame());
    d->y = 0;
    d->layoutScheduled = false;
    d->restartLayout = false;
    FrameIterator *transferedFootNoteCursor = 0;
    KoInlineNote *transferedContinuedNote = 0;
    int footNoteAutoCount = 0;
    KoTextLayoutRootArea *rootArea = 0;

    d->rootAreaList.clear();

    int currentAreaNumber = 0;
    do {
        if (d->restartLayout) {
            return false; // Abort layouting to restart from the beginning.
        }

        // Build our request for our rootArea provider
        RootAreaConstraint constraints = constraintsForPosition(d->layoutPosition->it, currentAreaNumber > 0);

        // Request a new root-area. If NULL is returned then layouting is finished.
        bool newRootArea = false;
        rootArea = d->provider->provide(this, constraints, currentAreaNumber, &newRootArea);
        if (!rootArea) {
            // Out of space ? Nothing more to do
            break;
        }

        d->rootAreaList.append(rootArea);
        bool shouldLayout = false;

        if (rootArea->top() != d->y) {
            shouldLayout = true;
        }
        else if (rootArea->isDirty()) {
            shouldLayout = true;
        }
        else if (!rootArea->isStartingAt(d->layoutPosition)) {
            shouldLayout = true;
        }
        else if (newRootArea) {
            shouldLayout = true;
        }

        if (shouldLayout) {
            QRectF rect = d->provider->suggestRect(rootArea);
            d->freeObstructions = d->provider->relevantObstructions(rootArea);

            rootArea->setReferenceRect(rect.left(), rect.right(), d->y + rect.top(), d->y + rect.bottom());

            beginAnchorCollecting(rootArea);

            // Layout all that can fit into that root area
            bool finished;
            FrameIterator *tmpPosition = 0;
            do {
                rootArea->setFootNoteCountInDoc(footNoteAutoCount);
                rootArea->setFootNoteFromPrevious(transferedFootNoteCursor, transferedContinuedNote);
                d->foundAnchors.clear();
                delete tmpPosition;
                tmpPosition = new FrameIterator(d->layoutPosition);
                finished = rootArea->layoutRoot(tmpPosition);
                if (d->anAnchorIsPlaced) {
                    d->anAnchorIsPlaced = false;
                } else {
                    ++d->anchoringIndex;
                }
            } while (d->anchoringIndex < d->textAnchors.count());

            foreach (KoShapeAnchor *anchor, d->textAnchors) {
                if (!d->foundAnchors.contains(anchor)) {
                    d->anchoredObstructions.remove(anchor->shape());
                    d->anchoringSoftBreak = qMin(d->anchoringSoftBreak, anchor->textLocation()->position());
                }
            }

            if (d->textAnchors.count() > 0) {
                delete tmpPosition;
                tmpPosition = new FrameIterator(d->layoutPosition);
                finished = rootArea->layoutRoot(tmpPosition);
            }
            delete d->layoutPosition;
            d->layoutPosition = tmpPosition;

            d->provider->doPostLayout(rootArea, newRootArea);
            updateProgress(d->layoutPosition->it);

            if (finished && !rootArea->footNoteCursorToNext()) {
                d->provider->releaseAllAfter(rootArea);
                // We must also delete them from our own list too
                int newsize = d->rootAreaList.indexOf(rootArea) + 1;
                while (d->rootAreaList.size() > newsize) {
                    d->rootAreaList.removeLast();
                }
                return true; // Finished layouting
            }

            if (d->layoutPosition->it == document()->rootFrame()->end()) {
                return true; // Finished layouting
            }

            if (!continuousLayout()) {
                return false; // Let's take a break. We are not finished layouting yet.
            }
        } else {
            // Drop following rootAreas
            delete d->layoutPosition;
            d->layoutPosition = new FrameIterator(rootArea->nextStartOfArea());
            if (d->layoutPosition->it == document()->rootFrame()->end() && !rootArea->footNoteCursorToNext()) {
                d->provider->releaseAllAfter(rootArea);
                // We must also delete them from our own list too
                int newsize = d->rootAreaList.indexOf(rootArea) + 1;
                while (d->rootAreaList.size() > newsize) {
                    d->rootAreaList.removeLast();
                }
                return true; // Finished layouting
            }
        }
        transferedFootNoteCursor = rootArea->footNoteCursorToNext();
        transferedContinuedNote = rootArea->continuedNoteToNext();
        footNoteAutoCount += rootArea->footNoteAutoCount();

        d->y = rootArea->bottom() + qreal(50); // (post)Layout method(s) just set this
                                               // 50 just to separate pages
        currentAreaNumber++;
    } while (transferedFootNoteCursor || d->layoutPosition->it != document()->rootFrame()->end());

    return true; // Finished layouting
}

void KoTextDocumentLayout::scheduleLayout()
{
    // Compress multiple scheduleLayout calls into one executeScheduledLayout.
    if (d->layoutScheduled) {
        return;
    }
    d->layoutScheduled = true;
    QTimer::singleShot(0, this, SLOT(executeScheduledLayout()));
}

void KoTextDocumentLayout::executeScheduledLayout()
{
    // Only do the actual layout if it wasn't done meanwhile by someone else.
    if (!d->layoutScheduled) {
        return;
    }
    d->layoutScheduled = false;
    if (d->isLayouting) {
        // Since we are already layouting ask for a restart to be sure to also include
        // root-areas that got dirty and are before the currently processed root-area.
        d->restartLayout = true;
    } else {
        layout();
    }
}

bool KoTextDocumentLayout::continuousLayout() const
{
    return d->continuousLayout;
}

void KoTextDocumentLayout::setContinuousLayout(bool continuous)
{
    d->continuousLayout = continuous;
}

void KoTextDocumentLayout::setBlockLayout(bool block)
{
    d->layoutBlocked = block;
}

bool KoTextDocumentLayout::layoutBlocked() const
{
    return d->layoutBlocked;
}

void KoTextDocumentLayout::setBlockChanges(bool block)
{
    d->changesBlocked = block;
}

bool KoTextDocumentLayout::changesBlocked() const
{
    return d->changesBlocked;
}

KoTextDocumentLayout* KoTextDocumentLayout::referencedLayout() const
{
    return d->referencedLayout;
}

void KoTextDocumentLayout::setReferencedLayout(KoTextDocumentLayout *layout)
{
    d->referencedLayout = layout;
}

QRectF KoTextDocumentLayout::frameBoundingRect(QTextFrame*) const
{
    return QRectF();
}

void KoTextDocumentLayout::clearInlineObjectRegistry(const QTextBlock &block)
{
    d->inlineObjectExtents.clear();
    d->inlineObjectOffset = block.position();
}

void KoTextDocumentLayout::registerInlineObject(const QTextInlineObject &inlineObject)
{
    KoInlineObjectExtent pos(inlineObject.ascent(),inlineObject.descent());
    d->inlineObjectExtents.insert(d->inlineObjectOffset + inlineObject.textPosition(), pos);
}

KoInlineObjectExtent KoTextDocumentLayout::inlineObjectExtent(const QTextFragment &fragment)
{
    if (d->inlineObjectExtents.contains(fragment.position()))
        return d->inlineObjectExtents[fragment.position()];
    return KoInlineObjectExtent();
}

void KoTextDocumentLayout::setContinuationObstruction(KoTextLayoutObstruction *continuationObstruction)
{
    if (d->continuationObstruction) {
        delete d->continuationObstruction;
    }
    d->continuationObstruction = continuationObstruction;
}

QList<KoTextLayoutObstruction *> KoTextDocumentLayout::currentObstructions()
{
    if (d->continuationObstruction) {
        // () is needed so we append to a local list and not anchoredObstructions
        return (d->freeObstructions + d->anchoredObstructions.values()) << d->continuationObstruction;
    } else {
        return d->freeObstructions + d->anchoredObstructions.values();
    }
}

QList<KoTextLayoutRootArea *> KoTextDocumentLayout::rootAreas() const
{
    return d->rootAreaList;
}

void KoTextDocumentLayout::removeRootArea(KoTextLayoutRootArea *rootArea)
{
    int indexOf = rootArea ? qMax(0, d->rootAreaList.indexOf(rootArea)) : 0;
    for(int i = d->rootAreaList.count() - 1; i >= indexOf; --i)
        d->rootAreaList.removeAt(i);
}

QList<KoShape*> KoTextDocumentLayout::shapes() const
{
    QList<KoShape*> listOfShapes;
    foreach (KoTextLayoutRootArea *rootArea, d->rootAreaList) {
        if (rootArea->associatedShape())
            listOfShapes.append(rootArea->associatedShape());
    }
    return listOfShapes;
}

void KoTextDocumentLayout::updateProgress(const QTextFrame::iterator &it)
{
    QTextBlock block = it.currentBlock();
    if (block.isValid()) {
        int percent = block.position() / qreal(document()->rootFrame()->lastPosition()) * 100.0;
        emit layoutProgressChanged(percent);
    } else if (it.currentFrame()) {
        int percent = it.currentFrame()->firstPosition() / qreal(document()->rootFrame()->lastPosition()) * 100.0;
        emit layoutProgressChanged(percent);
    }
}
