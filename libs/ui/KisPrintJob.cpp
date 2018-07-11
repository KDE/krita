/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#include "KisPrintJob.h"

#include <QWidget>
#include <QPainter>

#include <KoColorSpaceRegistry.h>

#include <kis_image.h>

#include "canvas/kis_canvas2.h"
#include "kis_config.h"
#include "kis_canvas_resource_provider.h"

KisPrintJob::KisPrintJob(KisImageWSP image)
        : QObject(image.data())
        , m_image(image)
{
    m_printer.setFromTo(1, 1);
}

KisPrintJob::~KisPrintJob()
{
}

QAbstractPrintDialog::PrintDialogOptions KisPrintJob::printDialogOptions() const
{
    return QAbstractPrintDialog::PrintToFile |
           QAbstractPrintDialog::PrintPageRange |
           QAbstractPrintDialog::PrintCollateCopies |
           QAbstractPrintDialog::DontUseSheet |
           QAbstractPrintDialog::PrintShowPageSize;
}

bool KisPrintJob::canPrint()
{
    if (! printer().isValid()) {
        return false;
    }

    QPainter testPainter(&printer());
    if (testPainter.isActive()) {
        return true;
    }

    return false;
}

void KisPrintJob::startPrinting(RemovePolicy removePolicy)
{
    QPainter gc(&m_printer);

    if (!m_image) return;

    gc.setClipping(false);

    KisConfig cfg(true);
    QString printerProfileName = cfg.printerProfile();
    const KoColorProfile *printerProfile = KoColorSpaceRegistry::instance()->profileByName(printerProfileName);

    double scaleX = m_printer.resolution() / (72.0 * m_image->xRes());
    double scaleY = m_printer.resolution() / (72.0 * m_image->yRes());

    QRect r = m_image->bounds();

    gc.scale(scaleX, scaleY);

    QImage image = m_image->convertToQImage(0, 0, r.width(), r.height(), printerProfile);
    gc.drawImage(r.x(), r.y(), image, 0, 0, r.width(), r.height());
    if (removePolicy == DeleteWhenDone)
        deleteLater();
}
