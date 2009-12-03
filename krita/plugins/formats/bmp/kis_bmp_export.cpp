/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kis_bmp_export.h"

#include <QCheckBox>
#include <QSlider>

#include <kapplication.h>
#include <kdialog.h>
#include <kgenericfactory.h>

#include <KoColorSpace.h>
#include <KoFilterChain.h>

#include <kis_paint_device.h>
#include <kis_doc2.h>
#include <kis_image.h>
#include <kis_paint_layer.h>

typedef KGenericFactory<KisBMPExport> KisBMPExportFactory;
K_EXPORT_COMPONENT_FACTORY(libkritabmpexport, KisBMPExportFactory("kofficefilters"))

KisBMPExport::KisBMPExport(QObject *parent, const QStringList&) : KoFilter(parent)
{
}

KisBMPExport::~KisBMPExport()
{
}

bool hasVisibleWidgets()
{
    QWidgetList wl = QApplication::allWidgets();
    foreach(QWidget* w, wl) {
        if (w->isVisible()) return true;
    }
    return false;
}

KoFilter::ConversionStatus KisBMPExport::convert(const QByteArray& from, const QByteArray& to)
{
    dbgFile << "BMP export! From:" << from << ", To:" << to << "";

    KisDoc2 *output = dynamic_cast<KisDoc2*>(m_chain->inputDocument());
    QString filename = m_chain->outputFile();

    if (!output)
        return KoFilter::CreationError;

    if (filename.isEmpty()) return KoFilter::FileNotFound;

    if (from != "application/x-krita")
        return KoFilter::NotImplemented;

    KUrl url;
    url.setPath(filename);

    QRect rc = output->image()->bounds();
    output->image()->lock();
    output->image()->refreshGraph();
    QImage image = output->image()->projection()->convertToQImage(0, 0, 0, rc.width(), rc.height());
    output->image()->unlock();
    image.save(url.toLocalFile());
    return KoFilter::OK;
}

#include "kis_bmp_export.moc"

