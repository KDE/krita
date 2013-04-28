/*
 *  Copyright (c) 2010 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kis_drop_button.h"

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QDebug>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include <kis_types.h>
#include <kis_image.h>
#include <kis_paint_device.h>
#include <kis_doc2.h>
#include <kis_group_layer.h>
#include <kis_paint_layer.h>
#include <kis_node.h>
#include <kis_mimedata.h>

KisDropButton::KisDropButton(QWidget *parent)
  : KisToolButton(parent)
{
    setAcceptDrops(true);
}


void KisDropButton::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasImage()
            || event->mimeData()->hasFormat("application/x-krita-node")) {
        event->accept();
    } else {
        event->ignore();
    }
}

void KisDropButton::dropEvent(QDropEvent *event)
{
    qDebug() << "DROP EVENT" << event->mimeData()->formats();

    KisNodeSP node;

    const KisMimeData *mimedata = qobject_cast<const KisMimeData*>(event->mimeData());

    if (mimedata) {
        qDebug() << "internal move";
        node = mimedata->node();
    }
    else if (event->mimeData()->hasFormat("application/x-krita-node") ) {

        qDebug() << "going to deserialize the dropped node";

        QByteArray ba = event->mimeData()->data("application/x-krita-node");

        KisPart2 part;
        KisDoc2 tmpDoc(&part);
        part.setDocument(&tmpDoc);

        tmpDoc.loadNativeFormatFromStore(ba);

        node = tmpDoc.image()->rootLayer()->firstChild();
    }
    else if (event->mimeData()->hasImage()) {

        qDebug() << "got an image";

        QImage qimage = qvariant_cast<QImage>(event->mimeData()->imageData());
        KisPaintDeviceSP device = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
        device->convertFromQImage(qimage, 0);
        node = new KisPaintLayer(0, "node creaed from dropped image", OPACITY_OPAQUE_U8, device);
    }

    if (node) {
        if (event->keyboardModifiers() & Qt::ShiftModifier) {
            replaceFromNode(node);
        }
        else {
            createFromNode(node);
        }

    }
}
