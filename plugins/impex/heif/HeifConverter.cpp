/*
 *  Copyright (c) 2005 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
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

#include "HeifConverter.h"

#include <QApplication>
#include <QMessageBox>
#include <QDomDocument>

#include <QFileInfo>

#include <KoColorSpaceRegistry.h>
#include <KoCompositeOpRegistry.h>
#include <KoColorSpaceTraits.h>
#include <KoColorModelStandardIds.h>
#include <KoColor.h>

#include <KisDocument.h>
#include <kis_group_layer.h>
#include <kis_image.h>
#include <kis_paint_device.h>
#include <kis_paint_layer.h>
#include <kis_transaction.h>
#include "kis_iterator_ng.h"

#include "kis_kra_savexml_visitor.h"

struct HeifConverter::Private {
    Private()
        : doc(0)
    {}

    bool showNotifications;
    KisImageSP image;
    KisDocument *doc;
    QString errorMessage;
};

HeifConverter::HeifConverter(KisDocument *doc, bool showNotifications)
    : d(new Private)
{
    d->doc = doc;
    d->showNotifications = showNotifications;
}

HeifConverter::~HeifConverter()
{
}

KisImageBuilder_Result HeifConverter::buildImage(const QString &filename)
{
    // Open the file
    // Read it
    // Convert the libheif image data to layer(s) in d->image
    return KisImageBuilder_RESULT_OK;
}

KisImageSP HeifConverter::image()
{
    return d->image;
}

QString HeifConverter::errorMessage() const
{
    return d->errorMessage;
}

KisImageBuilder_Result HeifConverter::buildFile(const QString &filename, KisImageSP image)
{
    // open the file
    // Iterate over the layer's pixels
    // KisSequentialIterator it(image->rootLayer()->projection(), paintRegion);
    // while (it.nextPixel()) {
    //
    // }
    // write the data to the file
    return KisImageBuilder_RESULT_OK;
}


void HeifConverter::cancel()
{
    warnKrita << "WARNING: Cancelling of an EXR loading is not supported!";
}


