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
#include "InlineObjectExtend.h"
#include "KoTextLayoutObstruction.h"
#include "FrameIterator.h"

#include <KoTextAnchor.h>
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


InlineObjectExtend::InlineObjectExtend(qreal ascent, qreal descent)
        : m_ascent(ascent),
        m_descent(descent)
{
}

struct KoTextDocumentLayout::Private
{
    Private(KoTextDocumentLayout *)
       : styleManager(0)
       , changeTracker(0)
       , textAnchorIndex(0)
       , defaultTabSizing(0)
       , y(0)
    {
    }
    KoStyleManager *styleManager;

    KoChangeTracker *changeTracker;

    KoInlineTextObjectManager *inlineTextObjectManager;
    KoTextLayoutRootAreaProvider *provider;
    KoPostscriptPaintDevice *paintDevice;
    QList<KoTextLayoutRootArea *> rootAreaList;
    FrameIterator *layoutPosition;

    QHash<int, InlineObjectExtend> inlineObjectExtends; // maps text-position to whole-line-height of an inline object
    QList<KoTextAnchor *> textAnchors; // list of all inserted inline objects
    int textAnchorIndex; // index of last not positioned inline object inside m_textAnchors

    QHash<KoShape*,KoTextLayoutObstruction*> obstructions; // all obstructions created in positionInlineObjects because KoTextAnchor from m_textAnchors is in text
    QList<KoTextLayoutObstruction*> currentLineObstructions; // obstructions for current page

    qreal defaultTabSizing;
    qreal y;

    KoTextDocumentLayout::ResizeMethod resizeMethod;
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
    unregisterAllObstructions();

    delete d;
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
        retval |= rootArea->selectionBoundingBox(cursor);
    }
    return retval;
}


void KoTextDocumentLayout::draw(QPainter *painter, const QAbstractTextDocumentLayout::PaintContext &context)
{
    PaintContext pc;
    pc.textContext = context;
    draw(painter, pc);
}

void KoTextDocumentLayout::draw(QPainter *painter, const KoTextDocumentLayout::PaintContext &context)
{
    // WARNING Text shapes ask their root area directly to paint.
    // It saves a lot of extra traversal, that is quite costly for big
    // documents
    Q_UNUSED(painter);
    Q_UNUSED(context);
}


int KoTextDocumentLayout::hitTest(const QPointF &point, Qt::HitTestAccuracy accuracy) const
{
    int position = hitTestIterated(document()->rootFrame()->begin(),
                        document()->rootFrame()->end(), point, accuracy);
    if (accuracy != Qt::ExactHit && position == -1)
        return document()->rootFrame()->lastPosition();
    return position;
}

int KoTextDocumentLayout::hitTestIterated(QTextFrame::iterator begin, QTextFrame::iterator end, const QPointF &point, Qt::HitTestAccuracy accuracy) const
{
    int position = -1;
    bool basicallyFound = false;
    QTextFrame::iterator it = begin;
    for (it = begin; it != end; ++it) {
        QTextBlock block = it.currentBlock();
        QTextTable *table = qobject_cast<QTextTable*>(it.currentFrame());
        QTextFrame *subFrame = it.currentFrame();

        if (table) {
            QTextTableCell cell;// TODO =  m_state->hitTestTable(table, point);
            if (cell.isValid()) {
                position = hitTestIterated(cell.begin(), cell.end(), point,
                                accuracy);
                if (position == -1)
                    position = cell.lastPosition();
                return position;
            }
            continue;
        } else if (subFrame) {
        } else {
            if (!block.isValid())
                continue;
        }
        if (basicallyFound) // a subsequent table or lines have now had their chance
            return position;

        // kDebug(32500) <<"hitTest[" << point.x() <<"," << point.y() <<"]";
        QTextLayout *layout = block.layout();
        if (point.y() > layout->boundingRect().bottom()) {
            // just skip this block.
            continue;
        }

        for (int i = 0; i < layout->lineCount(); i++) {
            QTextLine line = layout->lineAt(i);
            // kDebug(32500) <<" + line[" << line.textStart() <<"]:" << line.y() <<"-" << line.height();
            if (point.y() > line.y() + line.height()) {
                position = block.position() + line.textStart() + line.textLength();
                continue;
            }
            if (accuracy == Qt::ExactHit && point.y() < line.y()) { // between lines
                return -1;
            }
            if (accuracy == Qt::ExactHit && // left or right of line
                    (point.x() < line.naturalTextRect().left() || point.x() > line.naturalTextRect().right())) {
                return -1;
            }
            if (point.x() > line.x() + line.naturalTextWidth() && layout->textOption().textDirection() == Qt::RightToLeft) {
                // totally right of RTL text means the position is the start of the text.
                //TODO how about the other side?
                return block.position() + line.textStart();
            }
            if (point.x() > line.x() + line.naturalTextWidth()) {
                // right of line
                basicallyFound = true;
                position = block.position() + line.textStart() + line.textLength();
                continue;
            }
            return block.position() + line.xToCursor(point.x());
        }
    }
    return position;
}

int KoTextDocumentLayout::pageCount() const
{
    return 1;
}

void KoTextDocumentLayout::setTabSpacing(qreal spacing)
{
    d->defaultTabSizing = spacing * qt_defaultDpiY() / 72.;
}


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

//TODO FIXME make corresponding root area as dirty and then do layout
// right now we are just marking all as dirty
    foreach (KoTextLayoutRootArea *rootArea, d->rootAreaList) {
        rootArea->setDirty();
    }
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

void KoTextDocumentLayout::positionInlineObject(QTextInlineObject item, int position, const QTextFormat &format)
{
    Q_ASSERT(format.isCharFormat());
    if (d->inlineTextObjectManager == 0)
        return;
    QTextCharFormat cf = format.toCharFormat();
    KoInlineObject *obj = d->inlineTextObjectManager->inlineTextObject(cf);
    if (obj)
        obj->updatePosition(document(), item, position, cf);

#if 0
    KoTextAnchor *anchor = dynamic_cast<KoTextAnchor*>(inlineTextObjectManager()->inlineTextObject(f.toCharFormat()));
    if (anchor) { // special case anchors as positionInlineObject is called before layout; which is no good.
        KoShape *parent = anchor->shape()->parent();
        if (parent) {
            KWPage page = m_frameSet->pageManager()->page(parent);
            QRectF pageRect(0,page.offsetInDocument(),page.width(),page.height());
            QRectF pageContentRect = parent->boundingRect();
            int pageNumber = m_frameSet->pageManager()->pageNumber(parent);

            anchor->setPageRect(pageRect);
            //TODO get the right position for headers and footers
            anchor->setPageContentRect(pageContentRect);
            anchor->setPageNumber(pageNumber);

            // if there is no anchor strategy set send the textAnchor into the layout to create anchor strategy and position it
            if (!anchor->anchorStrategy()) {
                //place anchored object outside the page view, and let the layout position it right. It is better than make it invisible.
                anchor->shape()->setPosition(QPointF(0,100000000));
                m_state->insertInlineObject(anchor);
            }
        }
    }
#endif
}

void KoTextDocumentLayout::resizeInlineObject(QTextInlineObject item, int position, const QTextFormat &format)
{
    Q_ASSERT(format.isCharFormat());
    if (d->inlineTextObjectManager == 0)
        return;
    QTextCharFormat cf = format.toCharFormat();
    KoInlineObject *obj = d->inlineTextObjectManager->inlineTextObject(cf);
    if (obj) {
        obj->resize(document(), item, position, cf, paintDevice());
        registerInlineObject(item);
    }
}

void KoTextDocumentLayout::emitLayoutIsDirty()
{
    emit layoutIsDirty();
}

void KoTextDocumentLayout::layout()
{
    delete d->layoutPosition;
    d->layoutPosition = new FrameIterator(document()->rootFrame());
    d->y = 0;

    foreach (KoTextLayoutRootArea *rootArea, d->rootAreaList) {
        bool shouldLayout = false;

        if (rootArea->top() != d->y) {
            shouldLayout = true;
        }
        if (rootArea->isDirty()) {
            shouldLayout = true;
        }
        if (rootArea->isStartingAt(d->layoutPosition)) {
            shouldLayout = true;
        }

        if (shouldLayout) {
            rootArea->setReferenceRect(rootArea->left(),
                                          rootArea->right(),
                                          d->y,
                                          d->y + rootArea->maximumAllowedBottom());
            //layout all that can fit into that root area
            if (rootArea->layout(d->layoutPosition)) {
                d->provider->releaseAllAfter(rootArea);
                emit finishedLayout();
                return;
            }
            if (!continuousLayout()) {
                return; // let's take a break
            }
        }
        d->y = rootArea->bottom(); // layout method just set this
    }

    while (d->layoutPosition->it != document()->rootFrame()->end()) {
        // request a Root Area
        KoTextLayoutRootArea *rootArea = d->provider->provide(this);

        if (rootArea) {
            d->rootAreaList.append(rootArea);
            rootArea->setReferenceRect(rootArea->left(), rootArea->right(),
                                            d->y, d->y + rootArea->maximumAllowedBottom());
            //layout all that can fit into that root area
            rootArea->layout(d->layoutPosition);

            if (d->layoutPosition->it == document()->rootFrame()->end()) {
                emit finishedLayout();
                return;
            }
            if (!continuousLayout()) {
                return; // let's take a break
            }
        } else {
            return; // with no more space there is nothing else we can do
        }
        d->y = rootArea->bottom(); // layout method just set this
    }
}

void KoTextDocumentLayout::interruptLayout()
{
    //TODO make sure the currentLayout process is stopped
}



bool KoTextDocumentLayout::continuousLayout()
{
    return true;
}

QRectF KoTextDocumentLayout::frameBoundingRect(QTextFrame*) const
{
    return QRectF();
}

void KoTextDocumentLayout::registerInlineObject(const QTextInlineObject &inlineObject)
{
    InlineObjectExtend pos(inlineObject.ascent(),inlineObject.descent());
   //TODO d->inlineObjectExtends.insert(m_block.position() + inlineObject.textPosition(), pos);
}

void KoTextDocumentLayout::unregisterAllObstructions()
{
    qDeleteAll(d->obstructions);
    d->obstructions.clear();
}

InlineObjectExtend KoTextDocumentLayout::inlineObjectExtend(const QTextFragment &fragment)
{
    if (d->inlineObjectExtends.contains(fragment.position()))
        return d->inlineObjectExtends[fragment.position()];
    return InlineObjectExtend();
}

void KoTextDocumentLayout::resetInlineObject(int resetPosition)
{
    QList<KoTextAnchor *>::iterator iterBeginErase = d->textAnchors.end();
    QList<KoTextAnchor *>::iterator iter;
    for (iter = d->textAnchors.begin(); iter != d->textAnchors.end(); iter++) {

        // if the position of anchor is bigger than resetPosition than remove the anchor from layout
        if ((*iter)->positionInDocument() >= resetPosition) {
            (*iter)->anchorStrategy()->reset();

            // delete obstruction
            if (d->obstructions.contains((*iter)->shape())) {
                KoTextLayoutObstruction *obstruction = d->obstructions.value((*iter)->shape());
                d->obstructions.remove((*iter)->shape());
                //TODO m_textLine.updateObstruction(obstruction);
                refreshCurrentPageObstructions();
                delete obstruction;
            }
            (*iter)->setAnchorStrategy(0);

            if (iterBeginErase == d->textAnchors.end()) {
                iterBeginErase = iter;
            }
        }
    }

    d->textAnchors.erase(iterBeginErase,d->textAnchors.end());

    // update m_textAnchorIndex if necesary
    if (d->textAnchorIndex > d->textAnchors.size()) {
        d->textAnchorIndex = d->textAnchors.size();
    }
}

void KoTextDocumentLayout::refreshCurrentPageObstructions()
{
/*    m_currentLineObstructions.clear();

    TextShape *textShape = dynamic_cast<TextShape*>(shape);
    if (textShape == 0) {
        return;
    }

    // add current page children obstructions to m_currentLineObstructions
    foreach(KoShape *childShape, textShape->shapes()) {
        if (m_obstructions.contains(childShape)) {
            m_currentLineObstructions.append(m_obstructions.value(childShape));
        }
    }
*/
}

KoTextLayoutRootArea *KoTextDocumentLayout::rootAreaForPosition(int position) const
{
    Q_UNUSED(position);
    return 0;
}
void KoTextDocumentLayout::setResizeMethod(KoTextDocumentLayout::ResizeMethod method)
{
    if (d->resizeMethod == method)
        return;
    d->resizeMethod = method;
    //TODO scheduleLayout();
}

KoTextDocumentLayout::ResizeMethod KoTextDocumentLayout::resizeMethod() const
{
    return d->resizeMethod;
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


#include <KoTextDocumentLayout.moc>
