/* This file is part of the KDE project
 * Copyright (C) 2006-2007, 2009-2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2010 Johannes Simon <johannes.simon@gmail.com>
 * Copyright (C) 2011 KO GmbH <cbo@kogmbh.com>
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
#include "styles/KoParagraphStyle.h"
#include "styles/KoCharacterStyle.h"
#include "styles/KoListStyle.h"
#include "styles/KoStyleManager.h"
#include "KoTextBlockData.h"
#include "KoTextBlockBorderData.h"
#include "KoInlineTextObjectManager.h"
#include "KoTextLayoutRootArea.h"
#include "KoTextLayoutRootAreaProvider.h"
#include "KoTextLayoutObstruction.h"
#include "FrameIterator.h"
#include "InlineAnchorStrategy.h"
#include "FloatingAnchorStrategy.h"
#include "AnchorStrategy.h"
#include "IndexGeneratorManager.h"

#include <KoTextAnchor.h>
#include <KoTextPage.h>
#include <KoInsets.h>
#include <KoPostscriptPaintDevice.h>
#include <KoShape.h>

#include <kdebug.h>
#include <QTextBlock>
#include <QTextTable>
#include <QTextTableCell>
#include <QTextList>
#include <QTimer>
#include <QList>

extern int qt_defaultDpiY();


KoInlineObjectExtent::KoInlineObjectExtent(qreal ascent, qreal descent)
    : m_ascent(ascent),
      m_descent(descent)
{
}

class KoTextDocumentLayout::Private
{
public:
    Private(KoTextDocumentLayout *)
       : styleManager(0)
       , changeTracker(0)
       , inlineTextObjectManager(0)
       , provider(0)
       ,layoutPosition(0)
       ,anchoringRootArea(0)
       , anchoringIndex(0)
       , allowPositionInlineObject(true)
       , referencedLayout(0)
       , defaultTabSizing(0)
       , y(0)
       , isLayouting(false)
       , layoutScheduled(false)
       , continuousLayout(true)
       , layoutBlocked(false)
       , restartLayout(false)
    {
    }
    KoStyleManager *styleManager;

    KoChangeTracker *changeTracker;

    KoInlineTextObjectManager *inlineTextObjectManager;
    KoTextLayoutRootAreaProvider *provider;
    KoPostscriptPaintDevice *paintDevice;
    QList<KoTextLayoutRootArea *> rootAreaList;
    FrameIterator *layoutPosition;

    QHash<int, KoInlineObjectExtent> inlineObjectExtents; // maps text-position to whole-line-height of an inline object
    int inlineObjectOffset;
    QList<KoTextAnchor *> textAnchors; // list of all inserted inline objects
    KoTextLayoutRootArea *anchoringRootArea;
    int anchoringIndex; // index of last not positioned inline object inside textAnchors
    QRectF anchoringParagraphRect;
    bool allowPositionInlineObject;

    QHash<KoShape*,KoTextLayoutObstruction*> anchoredObstructions; // all obstructions created in positionInlineObjects because KoTextAnchor from m_textAnchors is in text
    QList<KoTextLayoutObstruction*> freeObstructions; // obstructions affecting the current rootArea, and not anchored

    KoTextDocumentLayout *referencedLayout;

    qreal defaultTabSizing;
    qreal y;
    bool isLayouting;
    bool layoutScheduled;
    bool continuousLayout;
    bool layoutBlocked;
    bool restartLayout;
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

    setTabSpacing(MM_TO_POINT(23)); // use same default as open office

    d->layoutPosition = new FrameIterator(doc->rootFrame());
}

KoTextDocumentLayout::~KoTextDocumentLayout()
{
    delete d->paintDevice;
    delete d->layoutPosition;
    qDeleteAll(d->rootAreaList);
    qDeleteAll(d->freeObstructions);
    qDeleteAll(d->anchoredObstructions);
    qDeleteAll(d->textAnchors);
    delete d;
}

KoTextLayoutRootAreaProvider *KoTextDocumentLayout::provider() const
{
    return d->provider;
}

bool KoTextDocumentLayout::relativeTabs() const
{
    return KoTextDocument(document()).relativeTabs();
}

KoInlineTextObjectManager *KoTextDocumentLayout::inlineTextObjectManager() const
{
    return d->inlineTextObjectManager;
}

KoChangeTracker *KoTextDocumentLayout::changeTracker() const
{
    return d->changeTracker;
}

KoStyleManager *KoTextDocumentLayout::styleManager() const
{
    return d->styleManager;
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
    foreach(const KoTextLayoutRootArea *rootArea, d->rootAreaList) {
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

qreal KoTextDocumentLayout::defaultTabSpacing()
{
    return d->defaultTabSizing;
}

// this method is called on every char inserted or deleted, on format changes, setting/moving of variables or objects.
void KoTextDocumentLayout::documentChanged(int position, int charsRemoved, int charsAdded)
{
    Q_UNUSED(charsAdded);
    Q_UNUSED(charsRemoved);

    int from = position;
    const int to = from + charsAdded;
    while (from < to) { // find blocks that have been added
        QTextBlock block = document()->findBlock(from);
        if (! block.isValid())
            break;
        if (from == block.position() && block.textList()) {
            KoTextBlockData *data = dynamic_cast<KoTextBlockData*>(block.userData());
            if (data)
                data->setCounterWidth(-1); // invalidate whole list.
        }

        from = block.position() + block.length();
    }

    // Mark the to the position corresponding root-areas as dirty. If there is no root-area for the position then we
    // don't need to mark anything dirty but still need to go on to force a scheduled relayout.
    if (!d->rootAreaList.isEmpty()) {
        KoTextLayoutRootArea *fromArea = rootAreaForPosition(position);
        int startIndex = fromArea ? qMax(0, d->rootAreaList.indexOf(fromArea)) : 0;
        int endIndex = startIndex;
        if (charsRemoved != 0 || charsAdded != 0) {
            // If any characters got removed or added make sure to also catch other root-areas that may be
            // affected by this change. Note that adding, removing or formatting text will always charsRemoved>0
            // and charsAdded>0 cause they are changing a range of characters. One case where both is zero is if
            // the content of a variable changed (see KoVariable::setValue which calls publicDocumentChanged). In
            // those cases we only need to relayout the root-area dirty where the variable is on.
            KoTextLayoutRootArea *toArea = fromArea ? rootAreaForPosition(position + qMax(charsRemoved, charsAdded)) : 0;
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
        if (x + 0.125 >= rect.x() && x<= rect.right() && y + 0.125 >= rect.y() && y <= rect.bottom()) {
            return rootArea;
        }
    }
    return 0;
}

void KoTextDocumentLayout::drawInlineObject(QPainter *painter, const QRectF &rect, QTextInlineObject object, int position, const QTextFormat &format)
{
    Q_ASSERT(format.isCharFormat());
    if (d->inlineTextObjectManager == 0)
        return;
    QTextCharFormat cf = format.toCharFormat();
    KoInlineObject *obj = d->inlineTextObjectManager->inlineTextObject(cf);
    if (obj)
        obj->paint(*painter, paintDevice(), document(), rect, object, position, cf);
}

void KoTextDocumentLayout::registerAnchoredObstruction(KoTextLayoutObstruction *obstruction)
{
    d->anchoredObstructions.insert(obstruction->shape(), obstruction);
}

void KoTextDocumentLayout::positionAnchoredObstructions()
{
    if (!d->anchoringRootArea)
        return;
    KoTextPage *page = d->anchoringRootArea->page();
    if (!page)
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
        KoTextAnchor *textAnchor = d->textAnchors[d->anchoringIndex];
        AnchorStrategy *strategy = static_cast<AnchorStrategy *>(textAnchor->anchorStrategy());

        strategy->setPageRect(page->rect());
        strategy->setPageNumber(page->pageNumber());

        if (strategy->moveSubject()) {
            //d->anchoringRootArea->setDirty(); // make sure we do the layout to flow around
            ++d->anchoringIndex;
        }
    }
}

void KoTextDocumentLayout::setAnchoringParagraphRect(const QRectF &paragraphRect)
{
    d->anchoringParagraphRect = paragraphRect;
}

void KoTextDocumentLayout::allowPositionInlineObject(bool allow)
{
    d->allowPositionInlineObject = allow;
}

// This method is called by qt every time  QTextLine.setWidth()/setNumColums() is called
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
    KoTextAnchor *anchor = dynamic_cast<KoTextAnchor*>(obj);
    if (anchor && d->anchoringRootArea->associatedShape()) {
        // if there is no anchor strategy set then create one
        if (!anchor->anchorStrategy()) {
            if (anchor->behavesAsCharacter()) {
                anchor->setAnchorStrategy(new InlineAnchorStrategy(anchor, d->anchoringRootArea));
            } else {
                anchor->setAnchorStrategy(new FloatingAnchorStrategy(anchor, d->anchoringRootArea));
            }
            d->textAnchors.append(anchor);
            anchor->updatePosition(document(), position, cf);
        }
        static_cast<AnchorStrategy *>(anchor->anchorStrategy())->setParagraphRect(d->anchoringParagraphRect);
    }
    else if (obj) {
        obj->updatePosition(document(), position, cf);
    }
}

void KoTextDocumentLayout::beginAnchorCollecting(KoTextLayoutRootArea *rootArea)
{
    for(int i = 0; i<d->textAnchors.size(); i++ ) {
        d->textAnchors[i]->setAnchorStrategy(0);
    }

    qDeleteAll(d->anchoredObstructions);
    d->anchoredObstructions.clear();
    d->textAnchors.clear();

    d->anchoringIndex = 0;
    d->anchoringRootArea = rootArea;
    d->allowPositionInlineObject = true;
}

void KoTextDocumentLayout::resizeInlineObject(QTextInlineObject item, int position, const QTextFormat &format)
{
    Q_ASSERT(format.isCharFormat());
    if (d->inlineTextObjectManager == 0)
        return;
    if(!d->anchoringRootArea->associatedShape())
        return;
    QTextCharFormat cf = format.toCharFormat();
    KoInlineObject *obj = d->inlineTextObjectManager->inlineTextObject(cf);
    if (obj) {
        QTextDocument *doc = document();
        QVariant v;
        v.setValue(d->anchoringRootArea->page());
        doc->addResource(KoTextDocument::LayoutTextPage, QUrl("kotext://layoutTextPage"), v);
        obj->resize(doc, item, position, cf, paintDevice());
        registerInlineObject(item);
    }
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

bool KoTextDocumentLayout::doLayout()
{
    delete d->layoutPosition;
    d->layoutPosition = new FrameIterator(document()->rootFrame());
    d->y = 0;
    d->layoutScheduled = false;
    d->restartLayout = false;

    foreach (KoTextLayoutRootArea *rootArea, d->rootAreaList) {
        if (d->restartLayout) {
            return false; // Abort layouting to restart from the beginning.
        }
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

        if (shouldLayout) {
            QSizeF size = d->provider->suggestSize(rootArea);
            d->freeObstructions = d->provider->relevantObstructions(rootArea);

            rootArea->setReferenceRect(0, size.width(), d->y, d->y + size.height());

            beginAnchorCollecting(rootArea);

            // Layout all that can fit into that root area
            bool finished;
            FrameIterator *tmpPosition = 0;
            do {
                delete tmpPosition;
                tmpPosition = new FrameIterator(d->layoutPosition);
                finished = rootArea->layoutRoot(tmpPosition);
            } while (d->anchoringIndex < d->textAnchors.count());
            if (d->textAnchors.count() > 0) {
                delete tmpPosition;
                tmpPosition = new FrameIterator(d->layoutPosition);
                rootArea->layoutRoot(tmpPosition);
            }

            delete d->layoutPosition;
            d->layoutPosition = tmpPosition;

            d->provider->doPostLayout(rootArea, false);
            updateProgress(rootArea->startTextFrameIterator());

            if (finished) {
                d->provider->releaseAllAfter(rootArea);
                // We must also delete them from our own list too
                int newsize = d->rootAreaList.indexOf(rootArea) + 1;
                while (d->rootAreaList.size() > newsize) {
                    d->rootAreaList.removeLast();
                }
                return true; // Finished layouting
            }

            if (!continuousLayout()) {
                return false; // Let's take a break. We are not finished layouting yet.
            }
        } else {
            delete d->layoutPosition;
            d->layoutPosition = new FrameIterator(rootArea->nextStartOfArea());
            if (d->layoutPosition->it == document()->rootFrame()->end()) {
                d->provider->releaseAllAfter(rootArea);
                // We must also delete them from our own list too
                int newsize = d->rootAreaList.indexOf(rootArea) + 1;
                while (d->rootAreaList.size() > newsize) {
                    d->rootAreaList.removeLast();
                }
                return true; // Finished layouting
            }
        }
        d->y = rootArea->bottom() + qreal(50); // (post)Layout method(s) just set this
                                               // 50 just to seperate pages
    }

    while (d->layoutPosition->it != document()->rootFrame()->end()) {
        if (d->restartLayout) {
            return false; // Abort layouting to restart from the beginning.
        }

        // Request a new root-area. If NULL is returned then layouting is finished.
        KoTextLayoutRootArea *rootArea = d->provider->provide(this);

        if (rootArea) {
            d->rootAreaList.append(rootArea);
            QSizeF size = d->provider->suggestSize(rootArea);
            d->freeObstructions = d->provider->relevantObstructions(rootArea);

            rootArea->setReferenceRect(0, size.width(), d->y, d->y + size.height());

            beginAnchorCollecting(rootArea);

            // Layout all that can fit into that root area
            FrameIterator *tmpPosition = 0;
            do {
                delete tmpPosition;
                tmpPosition = new FrameIterator(d->layoutPosition);
                rootArea->layoutRoot(tmpPosition);
            } while (d->anchoringIndex < d->textAnchors.count());
            if (d->textAnchors.count() > 0) {
                delete tmpPosition;
                tmpPosition = new FrameIterator(d->layoutPosition);
                rootArea->layoutRoot(tmpPosition);
            }
            delete d->layoutPosition;
            d->layoutPosition = tmpPosition;

            d->provider->doPostLayout(rootArea, true);
            updateProgress(rootArea->startTextFrameIterator());

            if (d->layoutPosition->it == document()->rootFrame()->end()) {
                return true; // Finished layouting
            }
            if (!continuousLayout()) {
                return false; // Let's take a break. We are not finished layouting yet.
            }
        } else {
            break; // with no more space there is nothing else we can do
        }
        d->y = rootArea->bottom() + qreal(50); // (post)Layout method(s) just set this
                                               // 50 just to seperate pages
    }

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

bool KoTextDocumentLayout::continuousLayout()
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

void KoTextDocumentLayout::clearInlineObjectRegistry(QTextBlock block)
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

QList<KoTextLayoutObstruction *> KoTextDocumentLayout::currentObstructions()
{
    return d->freeObstructions + d->anchoredObstructions.values();
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

#include <KoTextDocumentLayout.moc>
