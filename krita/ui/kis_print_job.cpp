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

#include "canvas/kis_canvas2.h"
#include "kis_config.h"
#include "kis_canvas_resource_provider.h"

#include <kis_image.h>

#include <KoColorSpaceRegistry.h>

#include <QPainter>

KisPrintJob::KisPrintJob(KisImageWSP image)
        : KoPrintJob(image.data())
        , m_image(image)
{
    m_printer.setFromTo(1, 1);
}

void KisPrintJob::startPrinting(RemovePolicy removePolicy)
{
    QPainter gc(&m_printer);

    if (!m_image) return;

    gc.setClipping(false);

    KisConfig cfg;
    QString printerProfileName = cfg.printerProfile();
    const KoColorProfile *printerProfile = KoColorSpaceRegistry::instance()->profileByName(printerProfileName);

    double scaleX = m_printer.resolution() / (72.0 * m_image->xRes());
    double scaleY = m_printer.resolution() / (72.0 * m_image->yRes());

    QRect r = m_image->bounds();

    gc.scale(scaleX, scaleY);
    m_image->renderToPainter(0, 0, r.x(), r.y(), r.width(), r.height(), gc, printerProfile);
    if (removePolicy == DeleteWhenDone)
        deleteLater();
}

QList<QWidget*> KisPrintJob::createOptionWidgets() const
{
    return QList<QWidget*>();
}
