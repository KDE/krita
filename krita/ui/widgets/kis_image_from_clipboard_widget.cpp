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
#include <kundo2command.h>
#include <QFile>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>


#include <kcolorcombo.h>
#include <kcomponentdata.h>
#include <kfiledialog.h>
#include <kstandarddirs.h>
#include <kglobal.h>

#include <kis_debug.h>

#include <KoIcon.h>
#include <KoCompositeOp.h>
#include <KoUnitDoubleSpinBox.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorProfile.h>
#include <KoColorSpace.h>
#include <KoID.h>
#include <KoColor.h>
#include <KoUnit.h>
#include <KoColorModelStandardIds.h>

#include <kis_fill_painter.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_group_layer.h>
#include <kis_paint_layer.h>
#include <kis_paint_device.h>
#include <kis_painter.h>

#include "kis_clipboard.h"
#include "kis_doc2.h"
#include "widgets/kis_cmb_idlist.h"
#include "widgets/squeezedcombobox.h"


KisImageFromClipboard::KisImageFromClipboard(QWidget* parent, KisDoc2* doc, qint32 defWidth, qint32 defHeight, double resolution, const QString& defColorModel, const QString& defColorDepth, const QString& defColorProfile, const QString& imageName)
: KisCustomImageWidget(parent, doc, defWidth, defHeight, resolution, defColorModel, defColorDepth, defColorProfile, imageName)
{
    setObjectName("KisImageFromClipboard");
    
    // create clipboard preview and show it   
    createClipboardPreview();
    grpClipboard->show();
    
    connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(clipboardDataChanged()));
    connect(QApplication::clipboard(), SIGNAL(selectionChanged()), this, SLOT(clipboardDataChanged()));
    connect(QApplication::clipboard(), SIGNAL(changed(QClipboard::Mode)), this, SLOT(clipboardDataChanged()));
    
    disconnect(createButton, SIGNAL(clicked()), 0, 0); //disable normal signal
    connect(createButton, SIGNAL(clicked()), this, SLOT(createImage()));
}

KisImageFromClipboard::~KisImageFromClipboard()
{
}

void KisImageFromClipboard::createImage()
{
    createNewImage();
    
    KisImageWSP image = m_doc->image();
    if (image && image->root() && image->root()->firstChild()) {
        KisLayer * layer = dynamic_cast<KisLayer*>(image->root()->firstChild().data());

        KisPaintDeviceSP clip = KisClipboard::instance()->clip(QRect());
        if (clip) {
            QRect r = clip->exactBounds();
            KisPainter painter;
            painter.begin(layer->paintDevice());
            painter.setCompositeOp(COMPOSITE_COPY);
            painter.bitBlt(0, 0, clip, r.x(), r.y(), r.width(), r.height());
        }

    }

    emit documentSelected();
}


void KisImageFromClipboard::clipboardDataChanged()
{
    createClipboardPreview();
}


void KisImageFromClipboard::createClipboardPreview()
{
    QClipboard *cb = QApplication::clipboard();
    QImage qimage = cb->image();
    const QMimeData *cbData = cb->mimeData();
    QByteArray mimeType("application/x-krita-selection");
    
    if ((cbData && cbData->hasFormat(mimeType)) || !qimage.isNull()) {
        QImage* clipboardImage = new QImage(qimage); // qimage needs to be on the heap
        QGraphicsPixmapItem *item = new QGraphicsPixmapItem( QPixmap::fromImage(*clipboardImage));
        
        QGraphicsScene *clipboardScene = new QGraphicsScene();      
        clipboardScene->addItem(item);
        
        clipPreview->setScene(clipboardScene);
        clipPreview->show();             
        createButton->setEnabled(true);
        
        doubleWidth->setValue(clipboardImage->width());
        doubleHeight->setValue(clipboardImage->height());
    } else {
        createButton->setEnabled(false);
        clipPreview->setScene(new QGraphicsScene());
    }
    
    
}



