/* This file is part of the KDE project
 * Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008-2010 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Pierre Stirnweiss \pierre.stirnweiss_calligra@gadz.org>
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

#include <KoCanvasBase.h>
#include <KoCanvasResourceProvider.h>
#include <KoChangeTracker.h>
#include <KoInlineTextObjectManager.h>
#include <KoTextRangeManager.h>
#include <KoOdfLoadingContext.h>
#include <KoOdfWorkaround.h>
#include <KoParagraphStyle.h>
#include <KoPostscriptPaintDevice.h>
#include <KoSelection.h>
#include <KoShapeLoadingContext.h>
#include <KoShapeBackground.h>
#include <KoShapePaintingContext.h>
#include <KoShapeSavingContext.h>
#include <KoText.h>
#include <KoTextDocument.h>
#include <KoTextDocumentLayout.h>
#include <KoTextPage.h>
#include <KoTextShapeContainerModel.h>
#include <KoPageProvider.h>
#include <KoViewConverter.h>
#include <KoXmlWriter.h>
#include <KoXmlReader.h>
#include <KoXmlNS.h>
#include <KoStyleStack.h>

#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QFont>
#include <QPainter>
#include <QPen>
#include <QTextLayout>

#include <QDebug>

#include <KoShapeContainer_p.h>
#include "kis_painting_tweaks.h"

TextShape::TextShape(KoInlineTextObjectManager *inlineTextObjectManager, KoTextRangeManager *textRangeManager)
    : KoShapeContainer(new KoTextShapeContainerModel())
    , KoFrameShape(KoXmlNS::draw, "text-box")
    , m_pageProvider(0)
    , m_imageCollection(0)
    , m_clip(true)
{
    setShapeId(TextShape_SHAPEID);
    m_textShapeData = new KoTextShapeData();
    setUserData(m_textShapeData);
    SimpleRootAreaProvider *provider = new SimpleRootAreaProvider(m_textShapeData, this);

    KoTextDocument(m_textShapeData->document()).setInlineTextObjectManager(inlineTextObjectManager);
    KoTextDocument(m_textShapeData->document()).setTextRangeManager(textRangeManager);

    m_layout = new KoTextDocumentLayout(m_textShapeData->document(), provider);
    m_textShapeData->document()->setDocumentLayout(m_layout);

    setCollisionDetection(true);

    QObject::connect(m_layout, SIGNAL(layoutIsDirty()), m_layout, SLOT(scheduleLayout()));
}

TextShape::TextShape(const TextShape &rhs)
    : KoShapeContainer(new KoShapeContainerPrivate(*reinterpret_cast<KoShapeContainerPrivate*>(rhs.d_ptr), this)),
      KoFrameShape(rhs),
      m_textShapeData(dynamic_cast<KoTextShapeData*>(rhs.m_textShapeData->clone())),
      m_pageProvider(0),
      m_imageCollection(0),
      m_clip(rhs.m_clip)
{
    reinterpret_cast<KoShapeContainerPrivate*>(rhs.d_ptr)->model = new KoTextShapeContainerModel();

    setShapeId(TextShape_SHAPEID);
    setUserData(m_textShapeData);

    SimpleRootAreaProvider *provider = new SimpleRootAreaProvider(m_textShapeData, this);
    m_layout = new KoTextDocumentLayout(m_textShapeData->document(), provider);
    m_textShapeData->document()->setDocumentLayout(m_layout);

    setCollisionDetection(true);
    QObject::connect(m_layout, SIGNAL(layoutIsDirty()), m_layout, SLOT(scheduleLayout()));

    updateDocumentData();
    m_layout->scheduleLayout();
}


TextShape::~TextShape()
{
}

KoShape *TextShape::cloneShape() const
{
    return new TextShape(*this);
}

void TextShape::paintComponent(QPainter &painter, const KoViewConverter &converter,
                               KoShapePaintingContext &paintContext)
{
    painter.save();
    applyConversion(painter, converter);
    KoBorder *border = this->border();

    if (border) {
        paintBorder(painter, converter);
    } else if (paintContext.showTextShapeOutlines) {
        // No need to paint the outlines if there is a real border.
        if (qAbs(rotation()) > 1) {
            painter.setRenderHint(QPainter::Antialiasing);
        }

        QPen pen(QColor(210, 210, 210)); // use cosmetic pen
        QPointF onePixel = converter.viewToDocument(QPointF(1.0, 1.0));
        QRectF rect(QPointF(0.0, 0.0), size() - QSizeF(onePixel.x(), onePixel.y()));
        painter.setPen(pen);
        painter.drawRect(rect);
    }
    painter.restore();

    if (m_textShapeData->isDirty()) { // not layouted yet.
        return;
    }

    QTextDocument *doc = m_textShapeData->document();
    Q_ASSERT(doc);
    KoTextDocumentLayout *lay = qobject_cast<KoTextDocumentLayout *>(doc->documentLayout());
    Q_ASSERT(lay);
    lay->showInlineObjectVisualization(paintContext.showInlineObjectVisualization);

    applyConversion(painter, converter);

    if (background()) {
        QPainterPath p;
        p.addRect(QRectF(QPointF(), size()));
        background()->paint(painter, converter, paintContext, p);
    }

    // this enables to use the same shapes on different pages showing different page numbers
    if (m_pageProvider) {
        KoTextPage *page = m_pageProvider->page(this);
        if (page) {
            // this is used to not trigger repaints if layout during the painting is done
            m_paintRegion = KisPaintingTweaks::safeClipRegion(painter);
            if (!m_textShapeData->rootArea()->page() || page->pageNumber() != m_textShapeData->rootArea()->page()->pageNumber()) {
                m_textShapeData->rootArea()->setPage(page); // takes over ownership of the page
            } else {
                delete page;
            }
        }
    }

    KoTextDocumentLayout::PaintContext pc;

    QAbstractTextDocumentLayout::Selection selection;
    KoTextEditor *textEditor = KoTextDocument(m_textShapeData->document()).textEditor();
    selection.cursor = *(textEditor->cursor());
    QPalette palette = pc.textContext.palette;
    selection.format.setBackground(palette.brush(QPalette::Highlight));
    selection.format.setForeground(palette.brush(QPalette::HighlightedText));
    pc.textContext.selections.append(selection);

    pc.textContext.selections += KoTextDocument(doc).selections();
    pc.viewConverter = &converter;
    pc.imageCollection = m_imageCollection;
    pc.showFormattingCharacters = paintContext.showFormattingCharacters;
    pc.showTableBorders = paintContext.showTableBorders;
    pc.showSectionBounds = paintContext.showSectionBounds;
    pc.showSpellChecking = paintContext.showSpellChecking;
    pc.showSelections = paintContext.showSelections;

    // When clipping the painter we need to make sure not to cutoff cosmetic pens which
    // may used to draw e.g. table-borders for user convenience when on screen (but not
    // on e.g. printing). Such cosmetic pens are special cause they will always have the
    // same pen-width (1 pixel) independent of zoom-factor or painter transformations and
    // are not taken into account in any border-calculations.
    QRectF clipRect = outlineRect();
    qreal cosmeticPenX = 1 * 72. / painter.device()->logicalDpiX();
    qreal cosmeticPenY = 1 * 72. / painter.device()->logicalDpiY();
    painter.setClipRect(clipRect.adjusted(-cosmeticPenX, -cosmeticPenY, cosmeticPenX, cosmeticPenY), Qt::IntersectClip);

    painter.save();
    painter.translate(0, -m_textShapeData->documentOffset());
    m_textShapeData->rootArea()->paint(&painter, pc); // only need to draw ourselves
    painter.restore();

    m_paintRegion = QRegion();
}

QPointF TextShape::convertScreenPos(const QPointF &point) const
{
    QPointF p = absoluteTransformation(0).inverted().map(point);
    return p + QPointF(0.0, m_textShapeData->documentOffset());
}

QPainterPath TextShape::outline() const
{
    QPainterPath path;
    path.addRect(QRectF(QPointF(0, 0), size()));
    return path;
}

QRectF TextShape::outlineRect() const
{
    if (m_textShapeData->rootArea()) {
        QRectF rect = m_textShapeData->rootArea()->boundingRect();
        rect.moveTop(rect.top() - m_textShapeData->rootArea()->top());
        if (m_clip) {
            rect.setHeight(size().height());
        }
        return rect | QRectF(QPointF(0, 0), size());
    }
    return QRectF(QPointF(0, 0), size());
}

void TextShape::shapeChanged(ChangeType type, KoShape *shape)
{
    Q_UNUSED(shape);
    KoShapeContainer::shapeChanged(type, shape);
    if (type == PositionChanged || type == SizeChanged || type == CollisionDetected) {
        m_textShapeData->setDirty();
    }
}

void TextShape::saveOdf(KoShapeSavingContext &context) const
{
    KoXmlWriter &writer = context.xmlWriter();

    QString textHeight = additionalAttribute("fo:min-height");
    const_cast<TextShape *>(this)->removeAdditionalAttribute("fo:min-height");
    writer.startElement("draw:frame");
    // if the TextShape is wrapped in a shrink to fit container we need to save the geometry of the container as
    // the geomerty of the shape might have been changed.
    if (ShrinkToFitShapeContainer *stf = dynamic_cast<ShrinkToFitShapeContainer *>(this->parent())) {
        stf->saveOdfAttributes(context, OdfSize | OdfPosition | OdfTransformation);
        saveOdfAttributes(context, OdfAdditionalAttributes | OdfMandatories | OdfCommonChildElements);
    } else {
        saveOdfAttributes(context, OdfAllAttributes);
    }

    writer.startElement("draw:text-box");
    if (!textHeight.isEmpty()) {
        writer.addAttribute("fo:min-height", textHeight);
    }
    KoTextDocumentLayout *lay = qobject_cast<KoTextDocumentLayout *>(m_textShapeData->document()->documentLayout());
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
    } else if (vAlign == Qt::AlignVCenter) {
        verticalAlign = "middle";
    }
    style.addProperty("draw:textarea-vertical-align", verticalAlign);

    KoTextShapeData::ResizeMethod resize = m_textShapeData->resizeMethod();
    if (resize == KoTextShapeData::AutoGrowWidth || resize == KoTextShapeData::AutoGrowWidthAndHeight) {
        style.addProperty("draw:auto-grow-width", "true");
    }
    if (resize != KoTextShapeData::AutoGrowHeight && resize != KoTextShapeData::AutoGrowWidthAndHeight) {
        style.addProperty("draw:auto-grow-height", "false");
    }
    if (resize == KoTextShapeData::ShrinkToFitResize) {
        style.addProperty("draw:fit-to-size", "true");
    }

    m_textShapeData->saveStyle(style, context);

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
    } else if (verticalAlign == "justify") {
        // not yet supported
        alignment = Qt::AlignVCenter;
    } else if (verticalAlign == "middle") {
        alignment = Qt::AlignVCenter;
    }

    m_textShapeData->setVerticalAlignment(alignment);

    const QString fitToSize = styleStack.property(KoXmlNS::draw, "fit-to-size");
    KoTextShapeData::ResizeMethod resize = KoTextShapeData::NoResize;
    if (fitToSize == "true" || fitToSize == "shrink-to-fit") { // second is buggy value from impress
        resize = KoTextShapeData::ShrinkToFitResize;
    } else {
        // An explicit svg:width or svg:height defined do change the default value (means those value
        // used if not explicit defined otherwise) for auto-grow-height and auto-grow-height. So
        // they are mutable exclusive.
        // It is not clear (means we did not test and took care of it) what happens if both are
        // defined and are in conflict with each other or how the fit-to-size is related to this.

        QString autoGrowWidth = styleStack.property(KoXmlNS::draw, "auto-grow-width");
        if (autoGrowWidth.isEmpty()) {
            autoGrowWidth = element.hasAttributeNS(KoXmlNS::svg, "width") ? "false" : "true";
        }

        QString autoGrowHeight = styleStack.property(KoXmlNS::draw, "auto-grow-height");
        if (autoGrowHeight.isEmpty()) {
            autoGrowHeight = element.hasAttributeNS(KoXmlNS::svg, "height") ? "false" : "true";
        }

        if (autoGrowWidth == "true") {
            resize = autoGrowHeight == "true" ? KoTextShapeData::AutoGrowWidthAndHeight : KoTextShapeData::AutoGrowWidth;
        } else if (autoGrowHeight == "true") {
            resize = KoTextShapeData::AutoGrowHeight;
        }
    }

    m_textShapeData->setResizeMethod(resize);
}

bool TextShape::loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    m_textShapeData->document()->setUndoRedoEnabled(false);
    loadOdfAttributes(element, context, OdfAllAttributes);

    // this cannot be done in loadStyle as that fills the style stack incorrectly and therefore
    // it results in wrong data being loaded.
    m_textShapeData->loadStyle(element, context);

#ifndef NWORKAROUND_ODF_BUGS
    KoTextShapeData::ResizeMethod method = m_textShapeData->resizeMethod();
    if (KoOdfWorkaround::fixAutoGrow(method, context)) {
        KoTextDocumentLayout *lay = qobject_cast<KoTextDocumentLayout *>(m_textShapeData->document()->documentLayout());
        Q_ASSERT(lay);
        if (lay) {
            SimpleRootAreaProvider *provider = dynamic_cast<SimpleRootAreaProvider *>(lay->provider());
            if (provider) {
                provider->m_fixAutogrow = true;
            }
        }
    }
#endif

    bool answer = loadOdfFrame(element, context);
    m_textShapeData->document()->setUndoRedoEnabled(true);
    return answer;
}

bool TextShape::loadOdfFrame(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    // If the loadOdfFrame from the base class for draw:text-box fails, check
    // for table:table, because that is a legal child of draw:frame in ODF 1.2.
    if (!KoFrameShape::loadOdfFrame(element, context)) {
        const KoXmlElement &possibleTableElement(KoXml::namedItemNS(element, KoXmlNS::table, "table"));
        if (possibleTableElement.isNull()) {
            return false;
        } else {
            return loadOdfFrameElement(possibleTableElement, context);
        }
    }

    return true;
}

bool TextShape::loadOdfFrameElement(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    bool ok = m_textShapeData->loadOdf(element, context, 0, this);
    if (ok) {
        ShrinkToFitShapeContainer::tryWrapShape(this, element, context);
    }
    return ok;
}

void TextShape::update() const
{
    KoShapeContainer::update();
}

void TextShape::updateAbsolute(const QRectF &shape) const
{
    // this is done to avoid updates which are called during the paint event and not needed.
    //if (!m_paintRegion.contains(shape.toRect())) {
        //KoShape::update(shape);
    //}

    // FIXME: just a hack to remove outdated call from the deprecated shape
    KoShape::updateAbsolute(shape);
}

void TextShape::waitUntilReady(const KoViewConverter &, bool asynchronous) const
{
    Q_UNUSED(asynchronous);
    KoTextDocumentLayout *lay = qobject_cast<KoTextDocumentLayout *>(m_textShapeData->document()->documentLayout());
    Q_ASSERT(lay);
    if (m_textShapeData->isDirty()) {
        // Do a simple layout-call which will make sure to relayout till things are done. If more
        // layouts are scheduled then we don't need to wait for them here but can just continue.
        lay->layout();
    }
}

KoImageCollection *TextShape::imageCollection()
{
    return m_imageCollection;
}

void TextShape::updateDocumentData()
{
    if (m_layout) {
        KoTextDocument document(m_textShapeData->document());
        m_layout->setStyleManager(document.styleManager());
        m_layout->setInlineTextObjectManager(document.inlineTextObjectManager());
        m_layout->setTextRangeManager(document.textRangeManager());
        m_layout->setChangeTracker(document.changeTracker());
    }
}
