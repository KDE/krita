/*
 * Calligra Report Engine
 * Copyright (C) 2010 by Adam Pigg (adam@piggz.co.uk)
 * Copyright (C) 2012 by Dag Andersen (danders@get2net.dk)
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

#include "KoOdtFrameReportRenderer.h"
#include "odtframe/KoOdtFrameReportDocument.h"
#include "odtframe/KoOdtFrameReportTextBox.h"
#include "odtframe/KoOdtFrameReportImage.h"
#include "odtframe/KoOdtFrameReportPicture.h"
#include "odtframe/KoOdtFrameReportLine.h"
#include "odtframe/KoOdtFrameReportCheckBox.h"
#include "renderobjects.h"

#include <kdebug.h>

KoOdtFrameReportRenderer::KoOdtFrameReportRenderer()
{

}

KoOdtFrameReportRenderer::~KoOdtFrameReportRenderer()
{
}

bool KoOdtFrameReportRenderer::render(const KoReportRendererContext& context, ORODocument* document, int /*page*/)
{
    int uid = 1;
    KoOdtFramesReportDocument doc;
    doc.setPageOptions(document->pageOptions());
    for (int page = 0; page < document->pages(); page++) {
        OROPage *p = document->page(page);
        for (int i = 0; i < p->primitives(); i++) {
            OROPrimitive *prim = p->primitive(i);
            if (prim->type() == OROTextBox::TextBox) {
                KoOdtFrameReportPrimitive *sp = new KoOdtFrameReportTextBox(static_cast<OROTextBox*>(prim));
                sp->setUID(uid++);
                doc.addPrimitive(sp);
            } else if (prim->type() == OROImage::Image) {
                KoOdtFrameReportPrimitive *sp = new KoOdtFrameReportImage(static_cast<OROImage*>(prim));
                sp->setUID(uid++);
                doc.addPrimitive(sp);
            } else if (prim->type() == OROPicture::Picture) {
                KoOdtFrameReportPrimitive *sp = new KoOdtFrameReportPicture(static_cast<OROPicture*>(prim));
                sp->setUID(uid++);
                doc.addPrimitive(sp);
            } else if (prim->type() == OROLine::Line) {
                KoOdtFrameReportPrimitive *sp = new KoOdtFrameReportLine(static_cast<OROLine*>(prim));
                sp->setUID(uid++);
                doc.addPrimitive(sp);
            } else if (prim->type() == OROCheck::Check) {
                KoOdtFrameReportPrimitive *sp = new KoOdtFrameReportCheckBox(static_cast<OROCheck*>(prim));
                sp->setUID(uid++);
                doc.addPrimitive(sp);
            } else {
                kDebug() << "unhandled primitive type."<<prim->type();
            }
        }
    }
    return doc.saveDocument(context.destinationUrl.path()) == QFile::NoError;
}

