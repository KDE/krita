/*
 * SPDX-FileCopyrightText: 2005 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2005 C. Boemann <cbo@boemann.dk>
 * SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "widgets/kis_image_from_clipboard_widget.h"
#include "widgets/kis_custom_image_widget.h"

#include <QApplication>
#include <QClipboard>
#include <QComboBox>
#include <QDesktopWidget>
#include <QFile>
#include <QMimeData>
#include <QPushButton>
#include <QRect>
#include <QSlider>
#include <QTimer>

#include <KisPart.h>
#include <KoColor.h>
#include <KoColorModelStandardIds.h>
#include <KoColorProfile.h>
#include <KoCompositeOpRegistry.h>
#include <KoID.h>
#include <KisSqueezedComboBox.h>

#include <kis_debug.h>
#include <kis_fill_painter.h>
#include <kis_group_layer.h>
#include <kis_icon.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_paint_device.h>
#include <kis_paint_layer.h>
#include <kis_painter.h>

#include <KisCursorOverrideLock.h>
#include "KisDocument.h"
#include "kis_clipboard.h"
#include "kis_import_catcher.h"
#include "widgets/kis_cmb_idlist.h"

KisImageFromClipboardWidget::KisImageFromClipboardWidget(QWidget* parent, qint32 defWidth, qint32 defHeight, double resolution, const QString& defColorModel, const QString& defColorDepth, const QString& defColorProfile, const QString& imageName)
    : KisCustomImageWidget(parent, defWidth, defHeight, resolution, defColorModel, defColorDepth, defColorProfile, imageName)
{
    setObjectName("KisImageFromClipboard");

    lblPreview->hide();
    grpClipboard->show();

    setNumberOfLayers(1);

    disconnect(newDialogConfirmationButtonBox, &QDialogButtonBox::accepted, nullptr,
               nullptr); // disable normal signal
    connect(newDialogConfirmationButtonBox, &QDialogButtonBox::accepted, this, &KisImageFromClipboardWidget::createImage);
}

KisImageFromClipboardWidget::~KisImageFromClipboardWidget()
{
}

void KisImageFromClipboardWidget::createImage()
{
    newDialogConfirmationButtonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    KisPaintDeviceSP clip = KisClipboard::instance()->clip(QRect(), true);
    if (!clip) {
        enableImageCreation(QImage());
        return;
    }

    KisDocument *doc = createNewImage();

    if (doc) {
        KisImageSP image = doc->image();
        if (image && image->root() && image->root()->firstChild()) {
            KisNodeSP node = image->root()->firstChild();
            while (node && (!dynamic_cast<KisPaintLayer*>(node.data()) || node->userLocked())) {
                node = node->nextSibling();
            }

            if (!node) {
                KisPaintLayerSP newLayer = new KisPaintLayer(image, image->nextLayerName(), OPACITY_OPAQUE_U8);
                image->addNode(newLayer);
                node = newLayer;
            }

            KIS_SAFE_ASSERT_RECOVER_RETURN(node);

            KisPaintLayer * layer = dynamic_cast<KisPaintLayer*>(node.data());
            KIS_SAFE_ASSERT_RECOVER_RETURN(layer);

            layer->setOpacity(OPACITY_OPAQUE_U8);
            const QRect r = clip->exactBounds();
            KisImportCatcher::adaptClipToImageColorSpace(clip, image);

            KisPainter::copyAreaOptimized(QPoint(), clip, layer->paintDevice(), r);
            layer->setDirty();
        }
        doc->setModified(true);
        Q_EMIT m_openPane->documentSelected(doc);
        m_openPane->accept();
    }

    newDialogConfirmationButtonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}

void KisImageFromClipboardWidget::clipboardDataChanged()
{
    createClipboardPreview();
}

void KisImageFromClipboardWidget::showEvent(QShowEvent *event)
{
    KisCustomImageWidget::showEvent(event);

    connect(KisClipboard::instance(), &KisClipboard::clipChanged, this, &KisImageFromClipboardWidget::clipboardDataChanged, Qt::UniqueConnection);

    createClipboardPreview();
}


void KisImageFromClipboardWidget::createClipboardPreview()
{
    if (!KisClipboard::instance()->hasClip()) {
        enableImageCreation(QImage());
    }

    KisCursorOverrideLock cursorLock(Qt::BusyCursor);

    QImage qimage = QApplication::clipboard()->image();
    enableImageCreation(qimage);
}

void KisImageFromClipboardWidget::enableImageCreation(const QImage &qimage)
{
    if (qimage.isNull()) {
        doubleWidth->setValue(0);
        doubleHeight->setValue(0);
        newDialogConfirmationButtonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        lblPreview->hide();
        tabWidget->setEnabled(false);

        lblDocumentInfo->setText(i18n("The clipboard is empty or does not have an image in it."));
    } else {
        QSize previewSize = QSize(75, 75) * devicePixelRatioF();
        QPixmap preview = QPixmap::fromImage(qimage.scaled(previewSize, Qt::KeepAspectRatio));
        preview.setDevicePixelRatio(devicePixelRatioF());
        lblPreview->setPixmap(preview);
        lblPreview->show();
        newDialogConfirmationButtonBox->button(QDialogButtonBox::Ok)->setEnabled(true);

        doubleWidth->setValue(qimage.width());
        doubleHeight->setValue(qimage.height());

        tabWidget->setEnabled(true);
    }
}
