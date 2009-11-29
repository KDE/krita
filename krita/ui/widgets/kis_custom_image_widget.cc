/* This file is part of the KOffice project
 * Copyright (C) 2005 Thomas Zander <zander@kde.org>
 * Copyright (C) 2005 Casper Boemann <cbr@boemann.dk>
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

#include "widgets/kis_custom_image_widget.h"


#include <QPushButton>
#include <QSlider>
#include <QComboBox>
#include <QRect>
#include <QApplication>
#include <QClipboard>
#include <QDesktopWidget>

#include <kcolorcombo.h>
#include <kis_debug.h>

#include <KoCompositeOp.h>
#include <KoUnitDoubleSpinBox.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorProfile.h>
#include <KoColorSpace.h>
#include <KoID.h>
#include <KoColor.h>
#include <KoUnit.h>
#include <KoColorModelStandardIds.h>

#include "kis_clipboard.h"
#include "kis_doc2.h"

#include "widgets/kis_cmb_idlist.h"
#include "widgets/squeezedcombobox.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_group_layer.h"
#include "kis_paint_layer.h"
#include "kis_paint_device.h"
#include "kis_painter.h"

KisCustomImageWidget::KisCustomImageWidget(QWidget *parent, KisDoc2 *doc, qint32 defWidth, qint32 defHeight, bool clipAvailable, double resolution, const QString & defColorSpaceName, const QString & imageName)
        : WdgNewImage(parent)
{
    Q_UNUSED(defColorSpaceName);
    setObjectName("KisCustomImageWidget");
    m_doc = doc;

    txtName->setText(imageName);

    m_widthUnit = KoUnit(KoUnit::Pixel, resolution);
    doubleWidth->setValue(defWidth);
    doubleWidth->setDecimals(0);
    m_width = m_widthUnit.fromUserValue(defWidth);
    cmbWidthUnit->addItems(KoUnit::listOfUnitName(false));
    cmbWidthUnit->setCurrentIndex(KoUnit::Pixel);

    m_heightUnit = KoUnit(KoUnit::Pixel, resolution);
    doubleHeight->setValue(defHeight);
    doubleHeight->setDecimals(0);
    m_height = m_heightUnit.fromUserValue(defHeight);
    cmbHeightUnit->addItems(KoUnit::listOfUnitName(false));
    cmbHeightUnit->setCurrentIndex(KoUnit::Pixel);

    doubleResolution->setValue(72.0 * resolution);
    doubleResolution->setDecimals(0);

    connect(doubleResolution, SIGNAL(valueChanged(double)),
            this, SLOT(resolutionChanged(double)));
    connect(cmbWidthUnit, SIGNAL(activated(int)),
            this, SLOT(widthUnitChanged(int)));
    connect(doubleWidth, SIGNAL(valueChanged(double)),
            this, SLOT(widthChanged(double)));
    connect(cmbHeightUnit, SIGNAL(activated(int)),
            this, SLOT(heightUnitChanged(int)));
    connect(doubleHeight, SIGNAL(valueChanged(double)),
            this, SLOT(heightChanged(double)));
    connect(m_createButton, SIGNAL(clicked()), this, SLOT(buttonClicked()));
    m_createButton -> setDefault(true);

    chkFromClipboard->setChecked(clipAvailable);
    chkFromClipboard->setEnabled(clipAvailable);

    colorSpaceSelector->setCurrentColorModel(RGBAColorModelID);
    colorSpaceSelector->setCurrentColorDepth(Integer8BitsColorDepthID);

    connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(clipboardDataChanged()));
    connect(QApplication::clipboard(), SIGNAL(selectionChanged()), this, SLOT(clipboardDataChanged()));
    connect(QApplication::clipboard(), SIGNAL(changed(QClipboard::Mode)), this, SLOT(clipboardDataChanged()));

    connect(bnScreenSize, SIGNAL(clicked()), this, SLOT(screenSizeClicked()));
    connect(colorSpaceSelector, SIGNAL(selectionChanged(bool)), m_createButton, SLOT(setEnabled(bool)));
}

void KisCustomImageWidget::resolutionChanged(double res)
{
    if (m_widthUnit.indexInList(false) == KoUnit::Pixel) {
        m_widthUnit = KoUnit(KoUnit::Pixel, res / 72.0);
        m_width = m_widthUnit.fromUserValue(doubleWidth->value());
    }

    if (m_heightUnit.indexInList(false) == KoUnit::Pixel) {
        m_heightUnit = KoUnit(KoUnit::Pixel, res / 72.0);
        m_height = m_heightUnit.fromUserValue(doubleHeight->value());
    }
}


void KisCustomImageWidget::widthUnitChanged(int index)
{
    doubleWidth->blockSignals(true);

    if (index == KoUnit::Pixel) {
        doubleWidth->setDecimals(0);
        m_widthUnit = KoUnit(KoUnit::Pixel, doubleResolution->value() / 72.0);
    } else {
        doubleWidth->setDecimals(2);
        m_widthUnit = KoUnit((KoUnit::Unit)cmbWidthUnit->currentIndex());
    }

    doubleWidth->setValue(KoUnit::ptToUnit(m_width, m_widthUnit));

    doubleWidth->blockSignals(false);
}

void KisCustomImageWidget::widthChanged(double value)
{
    m_width = m_widthUnit.fromUserValue(value);
}

void KisCustomImageWidget::heightUnitChanged(int index)
{
    doubleHeight->blockSignals(true);

    if (index == KoUnit::Pixel) {
        doubleHeight->setDecimals(0);
        m_heightUnit = KoUnit(KoUnit::Pixel, doubleResolution->value() / 72.0);
    } else {
        doubleHeight->setDecimals(2);
        m_heightUnit = KoUnit((KoUnit::Unit)cmbHeightUnit->currentIndex());
    }

    doubleHeight->setValue(KoUnit::ptToUnit(m_height, m_heightUnit));

    doubleHeight->blockSignals(false);
}

void KisCustomImageWidget::heightChanged(double value)
{
    m_height = m_heightUnit.fromUserValue(value);
}

void KisCustomImageWidget::buttonClicked()
{
    const KoColorSpace * cs = colorSpaceSelector->currentColorSpace();

    QColor qc = cmbColor->color();

    qint32 width, height;
    double resolution;
    resolution =  doubleResolution->value() / 72.0;  // internal resolution is in pixels per pt

    // XXX: Added explicit casts to get rid of warning
    width = static_cast<qint32>(0.5  + KoUnit::ptToUnit(m_width, KoUnit(KoUnit::Pixel, resolution)));
    height = static_cast<qint32>(0.5 + KoUnit::ptToUnit(m_height, KoUnit(KoUnit::Pixel, resolution)));
    m_doc->newImage(txtName->text(), width, height, cs, KoColor(qc, cs), txtDescription->toPlainText(), resolution);

    KisImageWSP image = m_doc->image();
    if (image && image->root() && image->root()->firstChild()) {
        KisLayer * layer = dynamic_cast<KisLayer*>(image->root()->firstChild().data());
        if (layer) {
            layer->setOpacity(backgroundOpacity());
        }
        if (chkFromClipboard->isChecked()) {
            KisPaintDeviceSP clip = KisClipboard::instance()->clip();
            if (clip) {
                QRect r = clip->exactBounds();
                KisPainter gc;
                gc.begin(layer->paintDevice());
                gc.setCompositeOp(COMPOSITE_COPY);
                gc.bitBlt(0, 0, clip, r.x(), r.y(), r.width(), r.height());
                gc.end();
            }
        }
        layer->setDirty(QRect(0, 0, width, height));
    }

    emit documentSelected();
}

quint8 KisCustomImageWidget::backgroundOpacity() const
{
    qint32 opacity = sliderOpacity->value();

    if (!opacity)
        return 0;

    return (opacity * 255) / 100;
}

void KisCustomImageWidget::clipboardDataChanged()
{
    QClipboard *cb = QApplication::clipboard();
    QImage qimage = cb->image();
    const QMimeData *cbData = cb->mimeData();
    QByteArray mimeType("application/x-krita-selection");

    if ((cbData && cbData->hasFormat(mimeType)) || !qimage.isNull()) {
        KisClipboard * cb = KisClipboard::instance();
        QSize sz = cb->clipSize();
        if (sz.isValid() && sz.width() != 0 && sz.height() != 0) {
            chkFromClipboard->setChecked(true);
            chkFromClipboard->setEnabled(true);
            doubleWidth->setValue(sz.width());
            doubleWidth->setDecimals(0);
            doubleHeight->setValue(sz.height());
            doubleHeight->setDecimals(0);
        } else {
            chkFromClipboard->setChecked(false);
            chkFromClipboard->setEnabled(false);

        }
    }

}

void KisCustomImageWidget::screenSizeClicked()
{
    QSize sz = QApplication::desktop()->availableGeometry(this).size();
    doubleWidth->setValue(sz.width());
    doubleWidth->setDecimals(0);
    doubleHeight->setValue(sz.height());
    doubleHeight->setDecimals(0);
}

#include "kis_custom_image_widget.moc"
