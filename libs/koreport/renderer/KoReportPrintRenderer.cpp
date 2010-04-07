/*
 * OpenRPT report writer and rendering engine
 * Copyright (C) 2001-2007 by OpenMFG, LLC (info@openmfg.com)
 * Copyright (C) 2007-2008 by Adam Pigg (adam@piggz.co.uk)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "KoReportPrintRenderer.h"
#include "renderobjects.h"
#include <KoPageFormat.h>
#include <kdebug.h>
#include <QPainter>

KoReportPrintRenderer::KoReportPrintRenderer()
{
}

KoReportPrintRenderer::~KoReportPrintRenderer()
{
}

bool KoReportPrintRenderer::setupPrinter( ORODocument * document, QPrinter * pPrinter)
{
    if (document == 0 || pPrinter == 0)
        return false;

    pPrinter->setCreator("KoReport Print Renderer");
    pPrinter->setDocName(document->title());
    pPrinter->setFullPage(true);
    pPrinter->setOrientation((document->pageOptions().isPortrait() ? QPrinter::Portrait : QPrinter::Landscape));
    pPrinter->setPageOrder(QPrinter::FirstPageFirst);

    if (document->pageOptions().getPageSize().isEmpty())
        pPrinter->setPageSize(QPrinter::Custom);
    else
        pPrinter->setPageSize(KoPageFormat::printerPageSize(KoPageFormat::formatFromString(document->pageOptions().getPageSize())));

    return true;
}

bool KoReportPrintRenderer::render(const KoReportRendererContext &context, ORODocument *document, int page)
{
    if (document == 0 || context.printer == 0 || context.painter == 0)
        return false;

    setupPrinter(document, context.printer);

    bool endWhenComplete = false;


    if (!context.painter->isActive()) {
        endWhenComplete = true;
        if (!context.painter->begin(context.printer))
            return false;
    }

    int fromPage = context.printer->fromPage();
    if (fromPage > 0)
        fromPage -= 1;
    int toPage = context.printer->toPage();
    if (toPage == 0 || toPage > document->pages())
        toPage = document->pages();
    for (int copy = 0; copy < context.printer->numCopies(); copy++) {
        for (int page = fromPage; page < toPage; page++) {
            if (page > 0)
                context.printer->newPage();

            OROPage * p = document->page(page);
            if (context.printer->pageOrder() == QPrinter::LastPageFirst)
                p = document->page(toPage - 1 - page);


            // Render Page Objects
            for (int i = 0; i < p->primitives(); i++) {
                OROPrimitive * prim = p->primitive(i);
                kDebug() << "Rendering object" << i << "type" << prim->type();
                if (prim->type() == OROTextBox::TextBox) {
                    kDebug() << "Text Box";
                    OROTextBox * tb = (OROTextBox*) prim;

                    QPointF ps = tb->position();
                    QSizeF sz = tb->size();
                    QRectF rc = QRectF(ps.x(), ps.y(), sz.width(), sz.height());

                    context.painter->save();
                    //Background

                    QColor bg = tb->textStyle().backgroundColor;
                    bg.setAlpha(tb->textStyle().backgroundOpacity);

                    //_painter->setBackgroundMode(Qt::OpaqueMode);
                    context.painter->setBackground(bg);
                    context.painter->fillRect(rc, bg);

                    //Text
                    context.painter->setBackgroundMode(Qt::TransparentMode);
                    context.painter->setFont(tb->textStyle().font);
                    context.painter->setPen(tb->textStyle().foregroundColor);
                    context.painter->drawText(rc, tb->flags(), tb->text());

                    //outer line
                    context.painter->setPen(QPen(tb->lineStyle().lineColor, tb->lineStyle().weight, tb->lineStyle().style));
                    context.painter->drawRect(rc);

                    //Reset back to defaults for next element
                    context.painter->restore();

                } else if (prim->type() == OROLine::Line) {
                    kDebug() << "Line";
                    OROLine * ln = (OROLine*) prim;
                    QPointF s = ln->startPoint();
                    QPointF e = ln->endPoint();
                    //QPen pen ( _painter->pen() );
                    QPen pen(ln->lineStyle().lineColor, ln->lineStyle().weight, ln->lineStyle().style);

                    context.painter->save();
                    context.painter->setRenderHint(QPainter::Antialiasing, true);
                    context.painter->setPen(pen);
                    context.painter->drawLine(QLineF(s.x(), s.y(), e.x(), e.y()));
                    context.painter->setRenderHint(QPainter::Antialiasing, false);
                    context.painter->restore();
                } else if (prim->type() == OROImage::Image) {
                    kDebug() << "Image";
                    OROImage * im = (OROImage*) prim;
                    QPointF ps = im->position();
                    QSizeF sz = im->size();
                    QRectF rc = QRectF(ps.x(), ps.y(), sz.width(), sz.height());

                    QImage img = im->image();
                    if (im->scaled())
                        img = img.scaled(rc.size().toSize(), (Qt::AspectRatioMode) im->aspectRatioMode(), (Qt::TransformationMode) im->transformationMode());

                    QRectF sr = QRectF(QPointF(0.0, 0.0), rc.size().boundedTo(img.size()));
                    context.painter->drawImage(rc.topLeft(), img, sr);
                } else if (prim->type() == ORORect::Rect) {
                    kDebug() << "Rect";
                    ORORect * re = (ORORect*) prim;

                    QPointF ps = re->position();
                    QSizeF sz = re->size();
                    QRectF rc = QRectF(ps.x(), ps.y(), sz.width(), sz.height());

                    context.painter->save();
                    context.painter->setPen(re->pen());
                    context.painter->setBrush(re->brush());
                    context.painter->drawRect(rc);
                    context.painter->restore();
                } else if (prim->type() == OROEllipse::Ellipse) {
                    OROEllipse * re = (OROEllipse*) prim;

                    QPointF ps = re->position();
                    QSizeF sz = re->size();
                    QRectF rc = QRectF(ps.x(), ps.y(), sz.width(), sz.height());

                    context.painter->save();
                    context.painter->setPen(re->pen());
                    context.painter->setBrush(re->brush());
                    context.painter->drawEllipse(rc);
                    context.painter->restore();
                } else if (prim->type() == OROPicture::Picture) {
                    OROPicture * im = (OROPicture*) prim;
                    QPointF ps = im->position();
                    QSizeF sz = im->size();
                    QRectF rc = QRectF(ps.x(), ps.y(), sz.width(), sz.height());
                    context.painter->drawPicture(rc.topLeft(), *(im->picture()));
                } else if (prim->type() == OROCheck::Check) {
                    OROCheck * chk = (OROCheck*) prim;
                    QPointF ps = chk->position();
                    QSizeF sz = chk->size();
                    QRectF rc = QRectF(ps.x(), ps.y(), sz.width(), sz.height());

                    context.painter->save();

                    context.painter->setBackgroundMode(Qt::OpaqueMode);
                    context.painter->setRenderHint(QPainter::Antialiasing);

                    context.painter->setPen(chk->foregroundColor());

                    if (chk->lineStyle().style == Qt::NoPen || chk->lineStyle().weight <= 0) {
                        context.painter->setPen(QPen(QColor(224, 224, 224)));
                    } else {
                        context.painter->setPen(QPen(chk->lineStyle().lineColor, chk->lineStyle().weight, chk->lineStyle().style));
                    }

                    qreal ox = sz.width() / 5;
                    qreal oy = sz.height() / 5;

                    //Checkbox Style
                    if (chk->checkType() == "Cross") {
                        context.painter->drawRoundedRect(rc, sz.width() / 10 , sz.height() / 10);

                        if (chk->value()) {
                            QPen lp;
                            lp.setColor(chk->foregroundColor());
                            lp.setWidth(ox > oy ? oy : ox);
                            context.painter->setPen(lp);
                            context.painter->drawLine(QPointF(ox, oy) + ps, QPointF(sz.width() - ox, sz.height() - oy) + ps);
                            context.painter->drawLine(QPointF(ox, sz.height() - oy) + ps, QPoint(sz.width() - ox, oy) + ps);
                        }
                    } else if (chk->checkType() == "Dot") {
                        //Radio Style
                        context.painter->drawEllipse(rc);

                        if (chk->value()) {
                            QBrush lb(chk->foregroundColor());
                            context.painter->setBrush(lb);
                            context.painter->setPen(Qt::NoPen);
                            context.painter->drawEllipse(rc.center(), sz.width() / 2 - ox, sz.height() / 2 - oy);
                        }
                    } else {
                        //Tickbox Style
                        context.painter->drawRoundedRect(rc, sz.width() / 10 , sz.height() / 10);

                        if (chk->value()) {
                            QPen lp;
                            lp.setColor(chk->foregroundColor());
                            lp.setWidth(ox > oy ? oy : ox);
                            context.painter->setPen(lp);
                            context.painter->drawLine(QPointF(ox, sz.height() / 2) + ps, QPointF(sz.width() / 2, sz.height() - oy) + ps);
                            context.painter->drawLine(QPointF(sz.width() / 2, sz.height() - oy) + ps, QPointF(sz.width() - ox, oy) + ps);
                        }
                    }

                    context.painter->restore();

                } else {
                    qDebug("unrecognized primitive type");
                }
            }
        }
    }

    if (endWhenComplete)
        context.painter->end();

    return true;
}



//#include <math.h>
//#include <QFontDatabase>

