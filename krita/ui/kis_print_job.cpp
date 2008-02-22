/*
 *  This file is part of Krita
 *
 *  Copyright (c) 2007 Thomas Zander <zander@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_print_job.h"

#include "kis_view2.h"
#include "kis_canvas2.h"
#include "kis_config.h"
#include "kis_resource_provider.h"

#include <kis_image.h>

#include <KoColorSpaceRegistry.h>

#include <QPainter>

KisPrintJob::KisPrintJob(KisView2 *view)
    : KoPrintJob(view),
    m_view(view)
{
    m_printer.setFromTo(1, 1);
}

void KisPrintJob::startPrinting(RemovePolicy removePolicy)
{
    QPainter gc(&m_printer);

    KisImageSP img = m_view->image();
    if (!img) return;

    gc.setClipping(false);

    KisConfig cfg;
    QString printerProfileName = cfg.printerProfile();
    const KoColorProfile *printerProfile = KoColorSpaceRegistry::instance()->profileByName(printerProfileName);

    double scaleX = m_printer.resolution() / (72.0 * img->xRes());
    double scaleY = m_printer.resolution() / (72.0 * img->yRes());

    QRect r = img->bounds();

    gc.scale(scaleX, scaleY);
    img->renderToPainter(0, 0, r.x(), r.y(), r.width(), r.height(), gc, printerProfile );
    if (removePolicy == DeleteWhenDone)
        deleteLater();
}

QList<QWidget*> KisPrintJob::createOptionWidgets() const
{
    return QList<QWidget*>();
}
