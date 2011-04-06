/* This file is part of the KDE project
 * Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008-2010 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Pierre Stirnweiss \pierre.stirnweiss_koffice@gadz.org>
 * Copyright (C) 2010 KO GmbH <cbo@kogmbh.com>
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
#include "TextShape.h"
#include "ShrinkToFitShapeContainer.h"
#include <KoTextSharedLoadingData.h>
#include "SimpleRootAreaProvider.h"

#include <KoTextLayoutRootArea.h>
#include <KoTextEditor.h>

#define synchronized(T) QMutex T; \
    for(Finalizer finalizer(T); finalizer.loop(); finalizer.inc())

struct Finalizer {
    Finalizer(QMutex &lock) : l(&lock), b(1) {
        l->lock();
    }
    ~Finalizer() {
        l->unlock();
    }
    QMutex *l;
    short b;
    short loop() {
        return b;
    }
    void inc() {
        --b;
    }
};

#include <KoCanvasBase.h>
#include <KoResourceManager.h>
#include <KoChangeTracker.h>
#include <KoInlineTextObjectManager.h>
#include <KoOdfLoadingContext.h>
#include <KoOdfStylesReader.h>
#include <KoParagraphStyle.h>
#include <KoPostscriptPaintDevice.h>
#include <KoSelection.h>
#include <KoShapeBackground.h>
#include <KoShapeLoadingContext.h>
#include <KoShapeManager.h>
#include <KoShapeSavingContext.h>
#include <KoText.h>
#include <KoTextDocument.h>
#include <KoTextDocumentLayout.h>
#include <KoTextEditor.h>
#include <KoTextPage.h>
#include <KoTextShapeContainerModel.h>
#include <KoPageProvider.h>
#include <KoViewConverter.h>
#include <KoXmlWriter.h>
#include <KoXmlReader.h>
#include <KoXmlNS.h>

#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QFont>
#include <QPainter>
#include <QPen>
#include <QTextLayout>
#include <QThread>

#include <kdebug.h>

TextShape::TextShape(KoInlineTextObjectManager *inlineTextObjectManager)
        : KoShapeContainer(new KoTextShapeContainerModel())
        , KoFrameShape(KoXmlNS::draw, "text-box")
        , m_footnotes(0)
        , m_demoText(false)
        , m_pageProvider(0)
        , m_imageCollection(0)
{
    setShapeId(TextShape_SHAPEID);
    m_textShapeData = new KoTextShapeData();
    setUserData(m_textShapeData);
    SimpleRootAreaProvider *provider = new SimpleRootAreaProvider(m_textShapeData, this);

    KoTextDocument(m_textShapeData->document()).setInlineTextObjectManager(inlineTextObjectManager);

    KoTextDocumentLayout *lay = new KoTextDocumentLayout(m_textShapeData->document(), provider);
    m_textShapeData->document()->setDocumentLayout(lay);

    setCollisionDetection(true);

    QObject::connect(lay, SIGNAL(layoutIsDirty()), lay, SLOT(layout()), Qt::QueuedConnection);
}

TextShape::~TextShape()
{
    delete m_footnotes;
}

void TextShape::paintComponent(QPainter &painter, const KoViewConverter &converter)
{
    kDebug();

    if (m_textShapeData->isDirty()) { // not layouted yet.
        return;
    }

    QTextDocument *doc = m_textShapeData->document();
    Q_ASSERT(doc);
    KoTextDocumentLayout *lay = qobject_cast<KoTextDocumentLayout*>(doc->documentLayout());
    Q_ASSERT(lay);

    applyConversion(painter, converter);

    if (background()) {
        QPainterPath p;
        p.addRect(QRectF(QPointF(), size()));
        background()->paint(painter, p);
    }

/*TODO FIXME pagenumber support done here
    if (m_pageProvider) {
        KoTextPage *page = m_pageProvider->page(this);
        if (page) {
            // this is used to not trigger repaints if layout during the painting is done
            // this enables to use the same shapes on different pages showing different page numbers
            m_paintRegion = painter.clipRegion();
            if (!m_textShapeData->page() || page->pageNumber() != m_textShapeData->page()->pageNumber()) {
                m_textShapeData->setPage(page);
                m_textShapeData->setDirty();
                lay->interruptLayout();
                m_textShapeData->fireResizeEvent();
            }

            if (lay) {
                while (m_textShapeData->isDirty() || lay->isInterrupted()){
                    m_textShapeData->setDirty();
                    lay->layout();
                }
            }
        }
    }
*/
    KoTextEditor *textEditor = KoTextDocument(m_textShapeData->document()).textEditor();

    KoTextDocumentLayout::PaintContext pc;
    QAbstractTextDocumentLayout::Selection selection;
    selection.cursor = *(textEditor->cursor());
    QPalette palette = pc.textContext.palette;
    selection.format.setBackground(palette.brush(QPalette::Highlight));
    selection.format.setForeground(palette.brush(QPalette::HighlightedText));

    pc.textContext.selections.append(selection);
    pc.viewConverter = &converter;
    pc.imageCollection = m_imageCollection;

    painter.setClipRect(outlineRect(), Qt::IntersectClip);

    painter.save();
    painter.translate(0, -m_textShapeData->documentOffset());
    m_textShapeData->rootArea()->paint(&painter, pc); // only need to draw ourselves
    painter.restore();

    if (m_footnotes) {
        painter.translate(0, size().height() - m_footnotes->size().height());
        m_footnotes->documentLayout()->draw(&painter, pc.textContext);
    }
    m_paintRegion = QRegion();
}

QPointF TextShape::convertScreenPos(const QPointF &point) const
{
    QPointF p = absoluteTransformation(0).inverted().map(point);
    return p + QPointF(0.0, m_textShapeData->documentOffset());
}

QRectF TextShape::outlineRect() const
{
    if (m_textShapeData->rootArea()) {
        QRectF rect = m_textShapeData->rootArea()->boundingRect();
        rect.moveTop(-m_textShapeData->documentOffset());
        return rect | QRectF(QPointF(0, 0), size());
    }
    return QRectF(QPointF(0,0), size());
}

void TextShape::shapeChanged(ChangeType type, KoShape *shape)
{
    Q_UNUSED(shape);
    kDebug() << type;
    if (type == PositionChanged || type == SizeChanged || type == CollisionDetected) {
        m_textShapeData->setDirty();
    }
}

void TextShape::paintDecorations(QPainter &painter, const KoViewConverter &converter, const KoCanvasBase *canvas)
{
    bool showTextFrames = canvas->resourceManager()->boolResource(KoText::ShowTextFrames);

    if (showTextFrames) {
        painter.save();
        applyConversion(painter, converter);
        if (qAbs(rotation()) > 1)
            painter.setRenderHint(QPainter::Antialiasing);

        // Make the border not overwrite already drawn things like
        // paragraph borders. DestinationOver and DestinationAtop
        // gives slightly different results, but non are really good
        // at low zooms.
        painter.setCompositionMode(QPainter::CompositionMode_DestinationAtop);

        QPen pen(QColor(210, 210, 210));
        QPointF onePixel = converter.viewToDocument(QPointF(1.0, 1.0));

        QPointF tl(0.0, 0.0);
        QRectF rect(tl, size());
        pen.setWidthF(onePixel.y());
        painter.setPen(pen);
        painter.drawLine(rect.topLeft(), rect.topRight());
        painter.drawLine(rect.bottomLeft(), rect.bottomRight());

        pen.setWidthF(onePixel.x());
        painter.setPen(pen);
        painter.drawLine(rect.topRight(), rect.bottomRight());
        painter.drawLine(rect.topLeft(), rect.bottomLeft());
        painter.restore();
    }
}

void TextShape::saveOdf(KoShapeSavingContext &context) const
{
    KoXmlWriter & writer = context.xmlWriter();

    QString textHeight = additionalAttribute("fo:min-height");
    const_cast<TextShape*>(this)->removeAdditionalAttribute("fo:min-height");
    writer.startElement("draw:frame");
    saveOdfAttributes(context, OdfAllAttributes);
    writer.startElement("draw:text-box");
    if (! textHeight.isEmpty())
        writer.addAttribute("fo:min-height", textHeight);
    KoTextDocumentLayout *lay = qobject_cast<KoTextDocumentLayout*>(m_textShapeData->document()->documentLayout());
    int index = -1;
    if (lay) {
        int i = 0;
        foreach (KoShape *shape, lay->shapes()) {
            if (shape == this) {
                index = i;
            } else if (index >= 0) {
                writer.addAttribute("draw:chain-next-name", shape->name());
                break;
            }
            ++i;
        }
    }
    const bool saveMyText = index == 0; // only save the text once.

    m_textShapeData->saveOdf(context, 0, 0, saveMyText ? -1 : 0);
    writer.endElement(); // draw:text-box
    saveOdfCommonChildElements(context);
    writer.endElement(); // draw:frame
}

QString TextShape::saveStyle(KoGenStyle &style, KoShapeSavingContext &context) const
{
    Qt::Alignment vAlign(m_textShapeData->verticalAlignment());
    QString verticalAlign = "top";
    if (vAlign == Qt::AlignBottom) {
        verticalAlign = "bottom";
    }
    else if ( vAlign == Qt::AlignVCenter ) {
        verticalAlign = "middle";
    }
    style.addProperty("draw:textarea-vertical-align", verticalAlign);

    KoTextDocumentLayout *lay = qobject_cast<KoTextDocumentLayout*>(m_textShapeData->document()->documentLayout());
    Q_ASSERT(lay);
    KoTextDocumentLayout::ResizeMethod resize = lay->resizeMethod();
    if (resize == KoTextDocumentLayout::AutoGrowWidth || resize == KoTextDocumentLayout::AutoGrowWidthAndHeight)
        style.addProperty("draw:auto-grow-width", "true");
    if (resize != KoTextDocumentLayout::AutoGrowHeight && resize != KoTextDocumentLayout::AutoGrowWidthAndHeight)
        style.addProperty("draw:auto-grow-height", "false");
    if (resize == KoTextDocumentLayout::ShrinkToFitResize)
        style.addProperty("draw:fit-to-size", "true");

    return KoShape::saveStyle(style, context);
}

void TextShape::loadStyle(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    KoShape::loadStyle(element, context);
    KoStyleStack &styleStack = context.odfLoadingContext().styleStack();
    styleStack.setTypeProperties("graphic");
    QString verticalAlign(styleStack.property(KoXmlNS::draw, "textarea-vertical-align"));
    Qt::Alignment alignment(Qt::AlignTop);
    if (verticalAlign == "bottom") {
        alignment = Qt::AlignBottom;
    }
    else if (verticalAlign == "justify") {
        // not yet supported
        alignment = Qt::AlignVCenter;
    }
    else if (verticalAlign == "middle") {
        alignment = Qt::AlignVCenter;
    }

    m_textShapeData->setVerticalAlignment(alignment);

    const QString autoGrowWidth = styleStack.property(KoXmlNS::draw, "auto-grow-width");
    const QString autoGrowHeight = styleStack.property(KoXmlNS::draw, "auto-grow-height");
    const QString fitToSize = styleStack.property(KoXmlNS::draw, "fit-to-size");
    KoTextDocumentLayout::ResizeMethod resize = KoTextDocumentLayout::NoResize;
    if (fitToSize == "true") {
        resize = KoTextDocumentLayout::ShrinkToFitResize;
    }
    else if (autoGrowWidth == "true") {
        resize = autoGrowHeight != "false" ? KoTextDocumentLayout::AutoGrowWidthAndHeight : KoTextDocumentLayout::AutoGrowWidth;
    }
    else if (autoGrowHeight != "false") {
        resize = KoTextDocumentLayout::AutoGrowHeight;
    }
    KoTextDocumentLayout *lay = qobject_cast<KoTextDocumentLayout*>(m_textShapeData->document()->documentLayout());
    Q_ASSERT(lay);
    lay->setResizeMethod(resize);
}

bool TextShape::loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    m_textShapeData->document()->setUndoRedoEnabled(false);
    loadOdfAttributes(element, context, OdfAllAttributes);

    // load the (text) style of the frame
    const KoXmlElement *style = 0;
    if (element.hasAttributeNS(KoXmlNS::draw, "style-name")) {
        style = context.odfLoadingContext().stylesReader().findStyle(
                    element.attributeNS(KoXmlNS::draw, "style-name"), "graphic",
                    context.odfLoadingContext().useStylesAutoStyles());
        if (!style) {
            kDebug(32500) << "graphic style not found:" << element.attributeNS(KoXmlNS::draw, "style-name");
        }
    }
    if (element.hasAttributeNS(KoXmlNS::presentation, "style-name")) {
        style = context.odfLoadingContext().stylesReader().findStyle(
                    element.attributeNS(KoXmlNS::presentation, "style-name"), "presentation",
                    context.odfLoadingContext().useStylesAutoStyles());
        if (!style) {
            kDebug(32500) << "presentation style not found:" << element.attributeNS(KoXmlNS::presentation, "style-name");
        }
    }

    if (style) {
        KoParagraphStyle paragraphStyle;
        paragraphStyle.loadOdf(style, context);

        QTextDocument *document = m_textShapeData->document();
        QTextCursor cursor(document);
        QTextBlock block = cursor.block();
        paragraphStyle.applyStyle(block, false);

    }

    bool answer = loadOdfFrame(element, context);
    m_textShapeData->document()->setUndoRedoEnabled(true);
    return answer;
}

bool TextShape::loadOdfFrame(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    // if the loadOdfFrame from the base class for draw:text-box failes check for table:table
    if (!KoFrameShape::loadOdfFrame(element, context)) {
        const KoXmlElement & frameElement(KoXml::namedItemNS(element, KoXmlNS::table, "table"));
        if (frameElement.isNull()) {
            return false;
        }
        else {
            return loadOdfFrameElement(frameElement, context);
        }
    }
    return true;
}

//tryWrapShape(KoShape *shape, const KoXmlElement &element, KoShapeLoadingContext &context);
bool TextShape::loadOdfFrameElement(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    bool ok = m_textShapeData->loadOdf(element, context, 0, this);
//FIXME    if (ok)
//        ShrinkToFitShapeContainer::tryWrapShape(this, element, context);
    return ok;
}

QTextDocument *TextShape::footnoteDocument()
{
    if (m_footnotes == 0) {
        m_footnotes = new QTextDocument();
        m_footnotes->setUseDesignMetrics(true);
        m_footnotes->documentLayout()->setPaintDevice(new KoPostscriptPaintDevice());
        m_footnotes->setDefaultFont(QFont("Sans Serif", 12, QFont::Normal, false));
        m_footnotes->setPageSize(size());
    }
    return m_footnotes;
}

void TextShape::markLayoutDone()
{
    kDebug();
    synchronized(m_mutex) {
        m_waiter.wakeAll();
    }
}

void TextShape::update() const
{
    kDebug();
    KoShapeContainer::update();
}

void TextShape::update(const QRectF &shape) const
{
    kDebug() << shape;
    // this is done to avoid updates which are called during the paint event and not needed.
    if (!m_paintRegion.contains(shape.toRect())) {
        KoShape::update(shape);
    }
}

void TextShape::waitUntilReady(const KoViewConverter &, bool asynchronous) const
{
    kDebug();
    KoTextDocumentLayout *lay = qobject_cast<KoTextDocumentLayout*>(m_textShapeData->document()->documentLayout());
    Q_ASSERT(lay);

    if (asynchronous) {
        synchronized(m_mutex) {
            if (m_textShapeData->isDirty()) {
                if (QThread::currentThread() != QApplication::instance()->thread()) {
                    // only wait if this is called in the non-main thread.
                    // this avoids locks due to the layout code expecting the GUI thread to be free while layouting.
                    m_waiter.wait(&m_mutex);
                }
            }
        }
    }
    else {
        KoTextDocumentLayout *lay = qobject_cast<KoTextDocumentLayout*>(m_textShapeData->document()->documentLayout());
        if (lay) {
            while (m_textShapeData->isDirty()){
                lay->layout();
            }
        }
    }
}
