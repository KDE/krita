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

#include "KoReportScreenRenderer.h"
#include "renderobjects.h"
#include <KoPageFormat.h>
#include <kdebug.h>
#include <KoUnit.h>

KoReportScreenRenderer::KoReportScreenRenderer()
{

}

KoReportScreenRenderer::~KoReportScreenRenderer()
{
}

//void KRScreenRender::setPainter(QPainter * pPainter)
//{
//    context.painter = pPainter;
//}

bool KoReportScreenRenderer::render(const KoReportRendererContext& context, ORODocument *document, int page)
{
    if (!document)
        return false;

    if (!context.painter)
        return false;

    OROPage *p = document->page(page);

    // Render Page Objects
    for (int i = 0; i < p->primitives(); i++) {
        OROPrimitive *prim = p->primitive(i);

        if (prim->type() == OROTextBox::TextBox) {
            OROTextBox *tb = dynamic_cast<OROTextBox*>(prim);

            QPointF ps = tb->position();
            QSizeF sz = tb->size();
            QRectF rc = QRectF(ps.x(), ps.y(), sz.width(), sz.height());

            context.painter->save();
            //Background

            QColor bg = tb->textStyle().backgroundColor;
            bg.setAlpha((tb->textStyle().backgroundOpacity / 100) * 255);

            context.painter->setBackground(bg);
            context.painter->fillRect(rc, bg);

            //Text
            context.painter->setBackgroundMode(Qt::TransparentMode);
            context.painter->setFont(tb->textStyle().font);
            context.painter->setPen(tb->textStyle().foregroundColor);
            context.painter->drawText(rc.adjusted(2, 2, 0, 0), tb->flags(), tb->text());

            //outer line
            context.painter->setPen(QPen(tb->lineStyle().lineColor, tb->lineStyle().weight, tb->lineStyle().style));
            context.painter->drawRect(rc);

            //Reset back to defaults for next element
            context.painter->restore();
        }
        else if (prim->type() == OROLine::Line) {
            OROLine * ln = dynamic_cast<OROLine*>(prim);
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
        }
        else if (prim->type() == ORORect::Rect) {
            ORORect * re = dynamic_cast<ORORect*>(prim);

            QPointF ps = re->position();
            QSizeF sz = re->size();
            QRectF rc = QRectF(ps.x(), ps.y(), sz.width(), sz.height());

            context.painter->save();
            context.painter->setPen(re->pen());
            context.painter->setBrush(re->brush());
            context.painter->drawRect(rc);
            context.painter->restore();
        }
        else if (prim->type() == OROEllipse::Ellipse) {
            OROEllipse * re = dynamic_cast<OROEllipse*>(prim);

            QPointF ps = re->position();
            QSizeF sz = re->size();
            QRectF rc = QRectF(ps.x(), ps.y(), sz.width(), sz.height());

            context.painter->save();
            context.painter->setPen(re->pen());
            context.painter->setBrush(re->brush());
            context.painter->drawEllipse(rc);
            context.painter->restore();
        }
        else if (prim->type() == OROImage::Image) {
            OROImage * im = dynamic_cast<OROImage*>(prim);
            QPointF ps = im->position();
            QSizeF sz = im->size();
            QRectF rc = QRectF(ps.x(), ps.y(), sz.width(), sz.height());

            QImage img = im->image();
            if (im->scaled())
                img = img.scaled(rc.size().toSize(), (Qt::AspectRatioMode) im->aspectRatioMode(),
                                 (Qt::TransformationMode) im->transformationMode());

            QRectF sr = QRectF(QPointF(0.0, 0.0), rc.size().boundedTo(img.size()));
            context.painter->drawImage(rc.topLeft(), img, sr);
        }
        else if (prim->type() == OROPicture::Picture) {
            OROPicture * im = dynamic_cast<OROPicture*>(prim);
            QPointF ps = im->position();
            QSizeF sz = im->size();
            QRectF rc = QRectF(ps.x(), ps.y(), sz.width(), sz.height());
            context.painter->save();
            context.painter->drawPicture(rc.topLeft(), *(im->picture()));
            context.painter->restore();
        }
        else if (prim->type() == OROCheck::Check) {
            OROCheck * chk = dynamic_cast<OROCheck*>(prim);
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
            }
            else if (chk->checkType() == "Dot") {
                //Radio Style
                context.painter->drawEllipse(rc);

                if (chk->value()) {
                    QBrush lb(chk->foregroundColor());
                    context.painter->setBrush(lb);
                    context.painter->setPen(Qt::NoPen);
                    context.painter->drawEllipse(rc.center(), sz.width() / 2 - ox, sz.height() / 2 - oy);
                }
            }
            else {
                //Tickbox Style
                context.painter->drawRoundedRect(rc, sz.width() / 10 , sz.height() / 10);

                if (chk->value()) {
                    QPen lp;
                    lp.setColor(chk->foregroundColor());
                    lp.setWidth(ox > oy ? oy : ox);
                    context.painter->setPen(lp);
                    context.painter->drawLine(
                        QPointF(ox, sz.height() / 2) + ps,
                        QPointF(sz.width() / 2, sz.height() - oy) + ps);
                    context.painter->drawLine(
                        QPointF(sz.width() / 2, sz.height() - oy) + ps,
                        QPointF(sz.width() - ox, oy) + ps);
                }
            }

            context.painter->restore();
        }
        else {
            kWarning() << "unrecognized primitive type";
        }
    }

    return true;
}
