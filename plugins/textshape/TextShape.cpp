/* This file is part of the KDE project
 * Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008-2010 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Pierre Stirnweiss \pierre.stirnweiss_koffice@gadz.org>
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
#include <KoTextSharedLoadingData.h>

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

#include "Layout.h"
//#include "TextLayerShape.h"

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
        ,m_pageProvider(0)
        ,m_imageCollection(0)
{
    setShapeId(TextShape_SHAPEID);
    m_textShapeData = new KoTextShapeData();
    setUserData(m_textShapeData);
    KoTextDocumentLayout *lay = new KoTextDocumentLayout(m_textShapeData->document());
    lay->setLayout(new Layout(lay));
    lay->addShape(this);
    m_textShapeData->document()->setDocumentLayout(lay);

    KoTextDocument(m_textShapeData->document()).setInlineTextObjectManager(inlineTextObjectManager);
    setCollisionDetection(true);

    lay->connect(m_textShapeData, SIGNAL(relayout()), SLOT(scheduleLayout()));
}

TextShape::~TextShape()
{
    delete m_footnotes;
}

void TextShape::setDemoText(bool on)
{
    if (m_demoText == on)
        return;
    m_demoText = on;
    if (m_demoText) {
        QTextCursor cursor(m_textShapeData->document());
        QTextCharFormat cf;
        cf.setFontPointSize(12.);
        cursor.mergeCharFormat(cf);
        for (int i = 0; i < 10; ++i)
            cursor.insertText("Lorem ipsum dolor sit amet, consectetuer adipiscing elit, sed diam nonummy nibh euismod tincidunt ut laoreet dolore magna aliquam erat volutpat. Ut wisi enim ad minim veniam, quis nostrud exerci tation ullamcorper suscipit lobortis nisl ut aliquip ex ea commodo consequat. Duis autem vel eum iriure dolor in hendrerit in vulputate velit esse molestie consequat, vel illum dolore eu feugiat nulla facilisis at vero eros et accumsan et iusto odio dignissim qui blandit praesent luptatum zzril delenit augue duis dolore te feugait nulla facilisi.\n");
    }
}

void TextShape::paintComponent(QPainter &painter, const KoViewConverter &converter)
{
    applyConversion(painter, converter);
    if (background()) {
        QPainterPath p;
        p.addRect(QRectF(QPointF(), size()));
        background()->paint(painter, p);
    }
    QTextDocument *doc = m_textShapeData->document();
    Q_ASSERT(doc);
    KoTextDocumentLayout *lay = qobject_cast<KoTextDocumentLayout*>(doc->documentLayout());

    if (m_textShapeData->endPosition() < 0) { // not layouted yet.
        if (lay == 0) {
            kWarning(32500) << "Painting shape that doesn't have a kotext doc-layout, which can't work";
            return;
        }
        else if (! lay->hasLayouter()) {
            lay->setLayout(new Layout(lay));
        }
        if (!m_pageProvider) {
            return;
        }
    }
    Q_ASSERT(lay);

    if (m_pageProvider) {
        KoTextPage *page = m_pageProvider->page(this);
        if (page) {
            // this is used to not trigger repaints if layout during the painting is done
            // this enables to use the same shapes on different pages showing different page numbers
            m_paintRegion = painter.clipRegion();
            if (!m_textShapeData->page() || page->pageNumber() != m_textShapeData->page()->pageNumber()) {
                m_textShapeData->setPage(page);
                m_textShapeData->foul();
                lay->interruptLayout();
                m_textShapeData->fireResizeEvent();
            }

            if (lay) {
                while (m_textShapeData->isDirty() || lay->isInterrupted()){
                    m_textShapeData->foul();
                    lay->layout();
                }
            }
        }
    }

    QAbstractTextDocumentLayout::PaintContext pc;
    KoTextDocumentLayout::PaintContext context;
    context.textContext = pc;
    context.viewConverter = &converter;
    context.imageCollection = m_imageCollection;

    QRectF rect(0, 0, size().width(), size().height());
    rect.adjust(-5, 0, 5, 0);
    painter.setClipRect(rect, Qt::IntersectClip);
    painter.save();
    painter.translate(0, -m_textShapeData->documentOffset());
    lay->draw(&painter, context);
    painter.restore();

    if (m_footnotes) {
        painter.translate(0, size().height() - m_footnotes->size().height());
        m_footnotes->documentLayout()->draw(&painter, pc);
    }
    m_paintRegion = QRegion();
}

QPointF TextShape::convertScreenPos(const QPointF &point)
{
    QPointF p = absoluteTransformation(0).inverted().map(point);
    return p + QPointF(0.0, m_textShapeData->documentOffset());
}

void TextShape::shapeChanged(ChangeType type, KoShape *shape)
{
    Q_UNUSED(shape);
    if (type == PositionChanged || type == SizeChanged || type == CollisionDetected) {
        m_textShapeData->foul();
        KoTextDocumentLayout *lay = qobject_cast<KoTextDocumentLayout*>(m_textShapeData->document()->documentLayout());
        if (lay)
            lay->interruptLayout();
        m_textShapeData->fireResizeEvent();
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
        painter.drawLine(tl, rect.topRight());
        painter.drawLine(rect.bottomLeft(), rect.bottomRight());

        pen.setWidthF(onePixel.x());
        painter.setPen(pen);
        painter.drawLine(rect.topRight(), rect.bottomRight());
        painter.drawLine(tl, rect.bottomLeft());
        painter.restore();
    }

    if (m_demoText) return;
    if (m_textShapeData->endPosition() < 0) return;
    KoTextDocumentLayout *lay = qobject_cast<KoTextDocumentLayout*>(m_textShapeData->document()->documentLayout());
    if (showTextFrames && lay) {
        QList<KoShape *> shapes = lay->shapes();
        // Get the bottom of the text.
        bool moreText = false;
        qreal max = m_textShapeData->documentOffset() + size().height();
        qreal bottom = 0.0;
        //TODO optimize this; don't iterate over the whole doc, use findBlock instead.
        QTextBlock block = m_textShapeData->document()->begin();
        while (block.isValid()) {
            QTextLayout *tl = block.layout();
            if (tl == 0) {
                moreText = true;
                break;
            } else if (tl->lineCount() == 0) {
                moreText = true;
                break;
            } else {
                QTextLine line = tl->lineAt(tl->lineCount() - 1);
                bottom = qMax(bottom, line.position().y() + line.height());
                if (bottom > max) {
                    moreText = true;
                    break;
                }
            }
            block = block.next();
        }

        if (! moreText) { // draw bottom of text.  Makes it easier to see where the text ends
            QPalette palette = canvas->canvasWidget()->palette();
            QPen pen(palette.color(QPalette::Button));
            painter.setPen(pen);

            QPointF endPoint = converter.documentToView(QPointF(size().width(),
                               bottom - m_textShapeData->documentOffset()));
            endPoint.setX(endPoint.x() - 1);
            if (endPoint.y() > 0)
                painter.drawLine(QPointF(0, endPoint.y()), endPoint);
        } else if (shapes.count() <= 1 || shapes.last() == this) { // there is invisible text left.
            QPoint bottomRight = converter.documentToView(QPointF(size().width(), size().height())).toPoint();
            QPalette palette = canvas->canvasWidget()->palette();
            QPen pen(palette.color(QPalette::Link));
            painter.setPen(pen);
            QPoint topLeft = bottomRight - QPoint(15, 15);
            painter.drawRect(QRect(topLeft, QSize(13, 13)));
            pen.setWidth(2);
            painter.setPen(pen);
            painter.drawLine(topLeft.x() + 7, topLeft.y() + 3, topLeft.x() + 7, bottomRight.y() - 4);
            painter.drawLine(topLeft.x() + 3, topLeft.y() + 7, bottomRight.x() - 4, topLeft.y() + 7);
        }
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
    const bool saveMyText = index == 0 && !m_demoText; // only save the text once.

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
    else if (element.hasAttributeNS(KoXmlNS::presentation, "style-name")) {
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

bool TextShape::loadOdfFrameElement(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    return m_textShapeData->loadOdf(element, context, 0, this);
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
    synchronized(m_mutex) {
        m_waiter.wakeAll();
    }
}

void TextShape::update() const
{
    KoShapeContainer::update();
}

void TextShape::update(const QRectF &shape) const
{
    // this is done to avoid updates which are called during the paint event and not needed.
    if (!m_paintRegion.contains(shape.toRect())) {
        KoShape::update(shape);
    }
}

void TextShape::waitUntilReady(const KoViewConverter &, bool asynchronous) const
{
    KoTextDocumentLayout *lay = qobject_cast<KoTextDocumentLayout*>(m_textShapeData->document()->documentLayout());
    Q_ASSERT(lay);
    if (!lay->hasLayouter()) {
        lay->setLayout(new Layout(lay));
    }

    if (asynchronous) {
        synchronized(m_mutex) {
            if (m_textShapeData->isDirty()) {
                m_textShapeData->fireResizeEvent(); // triggers a relayout
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
