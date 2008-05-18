/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
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

#define synchronized(T) QMutex T; \
 for(Finalizer finalizer(T); finalizer.loop(); finalizer.inc())

struct Finalizer {
    Finalizer(QMutex &lock) : l(&lock), b(1) { l->lock(); }
    ~Finalizer() { l->unlock(); }
    QMutex *l;
    short b;
    short loop() { return b; }
    void inc() { --b; }
};

#include "Layout.h"

#include <KoTextDocumentLayout.h>
#include <KoInlineTextObjectManager.h>
#include <KoTextShapeContainerModel.h>
#include <KoViewConverter.h>
#include <KoPostscriptPaintDevice.h>
#include <KoCanvasBase.h>
#include <KoCanvasResourceProvider.h>
#include <KoShapeManager.h>
#include <KoText.h>
#include <KoSelection.h>
#include <KoStyleManager.h>
#include <KoParagraphStyle.h>
#include <KoShapeSavingContext.h>
#include <KoXmlWriter.h>
#include <KoXmlReader.h>
#include <KoXmlNS.h>

#include <QTextLayout>
#include <QFont>
#include <QPen>
#include <QPainter>
#include <QAbstractTextDocumentLayout>
#include <kdebug.h>


TextShape::TextShape()
: KoShapeContainer(new KoTextShapeContainerModel())
, KoFrameShape(KoXmlNS::draw, "text-box")
, m_footnotes(0)
, m_demoText(false)
{
    setShapeId(TextShape_SHAPEID);
    m_textShapeData = new KoTextShapeData();
    setUserData(m_textShapeData);
    KoTextDocumentLayout *lay = new KoTextDocumentLayout(m_textShapeData->document());
    lay->setLayout(new Layout(lay));
    lay->addShape(this);
    m_textShapeData->document()->setDocumentLayout(lay);

    lay->setInlineObjectTextManager(new KoInlineTextObjectManager(lay));
    setCollisionDetection(true);

    lay->connect(m_textShapeData, SIGNAL(relayout()), SLOT(scheduleLayout()));
}

TextShape::~TextShape() {
    delete m_footnotes;
}

KoShape * TextShape::cloneShape() const
{
    // TODO implement cloning
    return 0;
}

void TextShape::setDemoText(bool on) {
    if(on) {
        QTextCursor cursor (m_textShapeData->document());
        for (int i=0; i < 10; i ++)
            cursor.insertText("Lorem ipsum dolor sit amet, consectetuer adipiscing elit, sed diam nonummy nibh euismod tincidunt ut laoreet dolore magna aliquam erat volutpat. Ut wisi enim ad minim veniam, quis nostrud exerci tation ullamcorper suscipit lobortis nisl ut aliquip ex ea commodo consequat. Duis autem vel eum iriure dolor in hendrerit in vulputate velit esse molestie consequat, vel illum dolore eu feugiat nulla facilisis at vero eros et accumsan et iusto odio dignissim qui blandit praesent luptatum zzril delenit augue duis dolore te feugait nulla facilisi.\n");
    }
    else if(m_demoText) {
        m_textShapeData->document()->clear();
        KoTextDocumentLayout *lay = dynamic_cast<KoTextDocumentLayout*> (m_textShapeData->document()->documentLayout());
        if(lay && lay->styleManager()) {
            QTextBlock block = m_textShapeData->document()->begin();
            lay->styleManager()->defaultParagraphStyle()->applyStyle(block);
        }
    }
    m_demoText = on;
}

void TextShape::paintComponent(QPainter &painter, const KoViewConverter &converter) {
    painter.fillRect(converter.documentToView(QRectF(QPointF(0.0,0.0), size())), background());
    QTextDocument *doc = m_textShapeData->document();
    KoTextDocumentLayout *lay = dynamic_cast<KoTextDocumentLayout*> (doc->documentLayout());
    if(m_textShapeData->endPosition() < 0) { // not layouted yet.
        if(lay == 0)
            kWarning(32500) << "Painting shape that doesn't have a kotext doc-layout, which can't work\n";
        else if(! lay->hasLayouter())
            lay->setLayout(new Layout(lay));
        return;
    }
    Q_ASSERT(lay);
    applyConversion(painter, converter);
    QAbstractTextDocumentLayout::PaintContext pc;
    KoTextDocumentLayout::PaintContext context;
    context.textContext = pc;
    context.viewConverter = &converter;

    painter.setClipRect(QRectF(QPointF(0, 0), size()), Qt::IntersectClip);
    painter.save();
    painter.translate(0, -m_textShapeData->documentOffset());
    lay->draw( &painter, context);
    painter.restore();

    if(m_footnotes) {
        painter.translate(0, size().height() - m_footnotes->size().height());
        m_footnotes->documentLayout()->draw( &painter, pc);
    }
}

QPointF TextShape::convertScreenPos(const QPointF &point) {
    QPointF p = matrix().inverted().map(point);
    return p + QPointF(0.0, m_textShapeData->documentOffset());
}

void TextShape::shapeChanged(ChangeType type) {
    if(type == PositionChanged || type == SizeChanged || type == CollisionDetected) {
        m_textShapeData->foul();
        KoTextDocumentLayout *lay = dynamic_cast<KoTextDocumentLayout*> (m_textShapeData->document()->documentLayout());
        if(lay)
            lay->interruptLayout();
        m_textShapeData->fireResizeEvent();
    }
}

void TextShape::paintDecorations(QPainter &painter, const KoViewConverter &converter, const KoCanvasBase *canvas) {
    bool showTextFrames = canvas->resourceProvider()->boolResource(KoText::ShowTextFrames);

    if(showTextFrames) {
        painter.save();
        applyConversion(painter, converter);
        if(qAbs(rotation()) > 1)
            painter.setRenderHint(QPainter::Antialiasing);

        QPen pen(QColor(210, 210, 210));
        QPointF onePixel = converter.viewToDocument( QPointF(1.0, 1.0));
        pen.setWidthF( onePixel.y() );
        painter.setPen(pen);

        QPointF tl(0.0, 0.0);
        QRectF rect(tl, size());
        painter.drawLine(tl, rect.topRight());
        painter.drawLine(rect.bottomLeft(), rect.bottomRight());

        pen.setWidthF( onePixel.x() );
        painter.setPen(pen);
        painter.drawLine(rect.topRight(), rect.bottomRight());
        painter.drawLine(tl, rect.bottomLeft());
        painter.restore();
    }

    if(m_demoText) return;
    if(m_textShapeData->endPosition() < 0) return;
    KoTextDocumentLayout *lay = dynamic_cast<KoTextDocumentLayout*> (m_textShapeData->document()->documentLayout());
    if(showTextFrames && lay) {
        QList< KoShape * > shapes = lay->shapes();
        // Get the bottom of the text.
        bool moreText = false;
        double max = m_textShapeData->documentOffset()+size().height();
        double bottom = 0.0;
        QTextBlock block = m_textShapeData->document()->begin();
        while(block.isValid()) {
            QTextLayout *tl = block.layout();
            if(tl == 0) {
                moreText = true;
                break;
            }
            else if(tl->lineCount() == 0) {
                moreText = true;
                break;
            }
            else {
                QTextLine line = tl->lineAt(tl->lineCount()-1);
                bottom = qMax(bottom, line.position().y() + line.height());
                if(bottom > max) {
                    moreText = true;
                    break;
                }
            }
            block = block.next();
        }

        if(! moreText) { // draw bottom of text.  Makes it easier to see where the text ends
            QPen pen(Qt::blue); // TODO make configurable?
            painter.setPen(pen);

            QPointF endPoint = converter.documentToView(QPointF(size().width(),
                        bottom - m_textShapeData->documentOffset()));
            endPoint.setX(endPoint.x() -1);
            if(endPoint.y() > 0)
                painter.drawLine(QPointF(0, endPoint.y()), endPoint);
        }
        else if(shapes.count() <= 1 || shapes.last() == this) { // there is invisible text left.
            QPoint bottomRight = converter.documentToView(QPointF(size().width(), size().height())).toPoint();
            QPen pen(Qt::red); // TODO make configurable?
            painter.setPen(pen);
            QPoint topLeft = bottomRight - QPoint(15,15);
            painter.drawRect(QRect(topLeft, QSize(13, 13)));
            pen.setWidth(2);
            painter.setPen(pen);
            painter.drawLine(topLeft.x() + 7, topLeft.y() + 3, topLeft.x() + 7, bottomRight.y() - 4);
            painter.drawLine(topLeft.x() + 3, topLeft.y() + 7, bottomRight.x() - 4, topLeft.y() + 7);
        }
    }
}

void TextShape::saveOdf(KoShapeSavingContext & context) const
{
    KoXmlWriter & writer = context.xmlWriter();
    //fo:min-height="120pt" draw:chain-next-name="Framesetje-2"

    writer.startElement( "draw:frame" );
    saveOdfAttributes( context, OdfAllAttributes );
    writer.startElement( "draw:text-box" );
    m_textShapeData->saveOdf(context);
    writer.endElement(); // draw:text-box
    writer.endElement(); // draw:frame
}

bool TextShape::loadOdf( const KoXmlElement & element, KoShapeLoadingContext &context )
{
    loadOdfAttributes( element, context, OdfAllAttributes );
    //HACK ? I'm not sure at all if this code should be here, but it works.
    if ((element.tagName() == "text") && (element.namespaceURI() == KoXmlNS::office))
        return loadOdfFrameElement(element, context);
    else
        return loadOdfFrame( element, context );
}

bool TextShape::loadOdfFrameElement( const KoXmlElement & element, KoShapeLoadingContext & context )
{
    return m_textShapeData->loadOdf(element, context);
}

void TextShape::init( QMap<QString, KoDataCenter *>  dataCenterMap )
{
    KoStyleManager * styleManager = dynamic_cast<KoStyleManager *>( dataCenterMap["StyleManager"] );

    KoTextDocumentLayout *lay = dynamic_cast<KoTextDocumentLayout*> ( m_textShapeData->document()->documentLayout() );
    if ( lay ) {
        lay->setStyleManager( styleManager );
    }
}

QTextDocument *TextShape::footnoteDocument() {
    if(m_footnotes == 0) {
        m_footnotes = new QTextDocument();
        m_footnotes->setUseDesignMetrics(true);
        m_footnotes->documentLayout()->setPaintDevice( new KoPostscriptPaintDevice() );
        m_footnotes->setDefaultFont(QFont("Sans Serif", 12, QFont::Normal, false));
        m_footnotes->setPageSize(size());
    }
    return m_footnotes;
}

void TextShape::markLayoutDone() {
    synchronized(m_mutex) {
        m_waiter.wakeAll();
    }
}

void TextShape::waitUntilReady() const {
    synchronized(m_mutex) {
        if(m_textShapeData->isDirty()) {
            m_textShapeData->fireResizeEvent(); // triggers a relayout
            m_waiter.wait(&m_mutex);
        }
    }
}

