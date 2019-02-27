/* This file is part of the Calligra project
 * Copyright (C) 2005 Thomas Zander <zander@kde.org>
 * Copyright (C) 2005 C. Boemann <cbo@boemann.dk>
 * Copyright (C) 2007 Boudewijn Rempt <boud@valdyas.org>
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

#include "widgets/kis_image_from_clipboard_widget.h"
#include "widgets/kis_custom_image_widget.h"

#include <QMimeData>
#include <QPushButton>
#include <QSlider>
#include <QComboBox>
#include <QRect>
#include <QApplication>
#include <QClipboard>
#include <QDesktopWidget>
#include <QFile>

#include <kis_debug.h>

#include <kis_icon.h>
#include <KoCompositeOpRegistry.h>
#include <KoColorProfile.h>
#include <KoID.h>
#include <KoColor.h>
#include <KoColorModelStandardIds.h>

#include <kis_fill_painter.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_group_layer.h>
#include <kis_paint_layer.h>
#include <kis_paint_device.h>
#include <kis_painter.h>

#include "kis_import_catcher.h"
#include "kis_clipboard.h"
#include "KisDocument.h"
#include "widgets/kis_cmb_idlist.h"
#include <KisSqueezedComboBox.h>


KisImageFromClipboard::KisImageFromClipboard(QWidget* parent, qint32 defWidth, qint32 defHeight, double resolution, const QString& defColorModel, const QString& defColorDepth, const QString& defColorProfile, const QString& imageName)
    : KisCustomImageWidget(parent, defWidth, defHeight, resolution, defColorModel, defColorDepth, defColorProfile, imageName)
{
    setObjectName("KisImageFromClipboard");

    // create clipboard preview and show it   
    createClipboardPreview();

    grpClipboard->show();
    imageGroupSpacer->changeSize(20, 40, QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(clipboardDataChanged()));
    connect(QApplication::clipboard(), SIGNAL(selectionChanged()), this, SLOT(clipboardDataChanged()));
    connect(QApplication::clipboard(), SIGNAL(changed(QClipboard::Mode)), this, SLOT(clipboardDataChanged()));


    disconnect(newDialogConfirmationButtonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), 0, 0); //disable normal signal
    connect(newDialogConfirmationButtonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(createImage()));
    setNumberOfLayers(1);
}

KisImageFromClipboard::~KisImageFromClipboard()
{
}

void KisImageFromClipboard::createImage()
{
    KisDocument *doc = createNewImage();
    
    KisImageSP image = doc->image();
    if (image && image->root() && image->root()->firstChild()) {
        KisLayer * layer = qobject_cast<KisLayer*>(image->root()->firstChild().data());

        KisPaintDeviceSP clip = KisClipboard::instance()->clip(QRect(), true);
        if (clip) {
            KisImportCatcher::adaptClipToImageColorSpace(clip, image);

            QRect r = clip->exactBounds();
            KisPainter::copyAreaOptimized(QPoint(), clip, layer->paintDevice(), r);

            layer->setDirty();
        }
    }
    doc->setModified(true);
    emit m_openPane->documentSelected(doc);
}


void KisImageFromClipboard::clipboardDataChanged()
{
    createClipboardPreview();
}


void KisImageFromClipboard::createClipboardPreview()
{
    QClipboard *cb = QApplication::clipboard();
    const QMimeData *cbData = cb->mimeData();
    if (cbData->hasImage()) {
        QImage qimage = cb->image();

        QByteArray mimeType("application/x-krita-selection");

        if ((cbData && cbData->hasFormat(mimeType)) || !qimage.isNull()) {
            lblPreview->setPixmap(QPixmap::fromImage(qimage.scaled(75, 75, Qt::KeepAspectRatio)));
            lblPreview->show();
            newDialogConfirmationButtonBox->button(QDialogButtonBox::Ok)->setEnabled(true);

            doubleWidth->setValue(qimage.width());
            doubleHeight->setValue(qimage.height());
        }
    }
    else {
        newDialogConfirmationButtonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        lblPreview->hide();
    }
    
    
}



