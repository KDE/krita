/*
 *  Copyright (c) 2005 Bart Coppens <kde@bartcoppens.be>
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "kis_custom_brush_widget.h"

#include <kis_debug.h>
#include <QLabel>
#include <QImage>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>

#include <QDateTime>

#include <QPixmap>
#include <QShowEvent>

#include <KoResourcePaths.h>

#include "kis_image.h"
#include "kis_layer.h"
#include "kis_paint_device.h"
#include "kis_gbr_brush.h"
#include "kis_imagepipe_brush.h"
#include <kis_fixed_paint_device.h>

#include "KisBrushServerProvider.h"
#include "kis_paint_layer.h"
#include "kis_group_layer.h"
#include <kis_selection.h>
#include <KoProperties.h>
#include "kis_iterator_ng.h"

KisCustomBrushWidget::KisCustomBrushWidget(QWidget *parent, const QString& caption, KisImageWSP image)
    : KisWdgCustomBrush(parent)
    , m_image(image)
{
    setWindowTitle(caption);
    preview->setScaledContents(false);
    preview->setFixedSize(preview->size());
    preview->setStyleSheet("border: 2px solid #222; border-radius: 4px; padding: 5px; font: normal 10px;");

    m_rServer = KisBrushServerProvider::instance()->brushServer();

    m_brush = 0;

    connect(this, SIGNAL(accepted()), SLOT(slotAddPredefined()));
    connect(brushStyle, SIGNAL(activated(int)), this, SLOT(slotUpdateCurrentBrush(int)));
    connect(colorAsMask, SIGNAL(toggled(bool)), this, SLOT(slotUpdateUseColorAsMask(bool)));


    colorAsMask->setChecked(true); // use color as mask by default. This is by far the most common way to make tip.
    spacingWidget->setSpacing(true, 1.0);
    connect(spacingWidget, SIGNAL(sigSpacingChanged()), SLOT(slotSpacingChanged()));
}

KisCustomBrushWidget::~KisCustomBrushWidget()
{
}


KisBrushSP KisCustomBrushWidget::brush()
{
    return m_brush;
}

void KisCustomBrushWidget::showEvent(QShowEvent *)
{
    slotUpdateCurrentBrush(0);
}

void KisCustomBrushWidget::updatePreviewImage()
{
    QImage brushImage = m_brush ? m_brush->brushTipImage() : QImage();

    if (!brushImage.isNull()) {
        brushImage = brushImage.scaled(preview->size(), Qt::KeepAspectRatio);
    }

    preview->setPixmap(QPixmap::fromImage(brushImage));
}

void KisCustomBrushWidget::slotUpdateCurrentBrush(int)
{
    if (brushStyle->currentIndex() == 0) {
        comboBox2->setEnabled(false);
    } else {
        comboBox2->setEnabled(true);
    }
    if (m_image) {
        createBrush();
        updatePreviewImage();
    }
}

void KisCustomBrushWidget::slotSpacingChanged()
{
    if (m_brush) {
        m_brush->setSpacing(spacingWidget->spacing());
        m_brush->setAutoSpacing(spacingWidget->autoSpacingActive(), spacingWidget->autoSpacingCoeff());
    }
}

void KisCustomBrushWidget::slotUpdateUseColorAsMask(bool useColorAsMask)
{
    if (m_brush) {
        static_cast<KisGbrBrush*>(m_brush.data())->setUseColorAsMask(useColorAsMask);
        updatePreviewImage();
    }
}


void KisCustomBrushWidget::slotAddPredefined()
{
    QString dir = KoResourcePaths::saveLocation("data", ResourceType::Brushes);
    QString extension;

    if (brushStyle->currentIndex() == 0) {
        extension = ".gbr";
    }
    else {
        extension = ".gih";
    }

    QString name = nameLineEdit->text();
    QString tempFileName;
    {
        QFileInfo fileInfo;
        fileInfo.setFile(dir + name + extension);

        int i = 1;
        while (fileInfo.exists()) {
            fileInfo.setFile(dir + name + QString("%1").arg(i) + extension);
            i++;
        }

        tempFileName = fileInfo.filePath();
    }

    // Add it to the brush server, so that it automatically gets to the mediators, and
    // so to the other brush choosers can pick it up, if they want to
    if (m_rServer && m_brush) {
        qDebug() << "m_brush" << m_brush;
        KisGbrBrushSP resource = m_brush->clone().dynamicCast<KisGbrBrush>();
        resource->setFilename(tempFileName);

        if (nameLineEdit->text().isEmpty()) {
            resource->setName(QDateTime::currentDateTime().toString("yyyy-MM-ddThh:mm"));
        }
        else {
            resource->setName(name);
        }

        if (colorAsMask->isChecked()) {
            resource->makeMaskImage();
        }
        m_rServer->addResource(resource.dynamicCast<KisBrush>());
        emit sigNewPredefinedBrush(resource);
    }

    close();
}

void KisCustomBrushWidget::createBrush()
{
    if (!m_image)
        return;

    if (brushStyle->currentIndex() == 0) {
        KisSelectionSP selection = m_image->globalSelection();
        // create copy of the data
        m_image->lock();
        KisPaintDeviceSP dev = new KisPaintDevice(*m_image->projection());
        m_image->unlock();

        if (!selection) {
            m_brush = KisBrushSP(new KisGbrBrush(dev, 0, 0, m_image->width(), m_image->height()));
        }
        else {
            // apply selection mask
            QRect r = selection->selectedExactRect();
            dev->crop(r);

            KisHLineIteratorSP pixelIt = dev->createHLineIteratorNG(r.x(), r.top(), r.width());
            KisHLineConstIteratorSP maskIt = selection->projection()->createHLineIteratorNG(r.x(), r.top(), r.width());

            for (qint32 y = r.top(); y <= r.bottom(); ++y) {

                do {
                    dev->colorSpace()->applyAlphaU8Mask(pixelIt->rawData(), maskIt->oldRawData(), 1);
                } while (pixelIt->nextPixel() && maskIt->nextPixel());

                pixelIt->nextRow();
                maskIt->nextRow();
            }

            QRect rc = dev->exactBounds();
            m_brush = KisBrushSP(new KisGbrBrush(dev, rc.x(), rc.y(), rc.width(), rc.height()));
        }

    }
    else {
        // For each layer in the current image, create a new image, and add it to the list
        QVector< QVector<KisPaintDevice*> > devices;
        devices.push_back(QVector<KisPaintDevice*>());
        int w = m_image->width();
        int h = m_image->height();

        m_image->lock();

        // We only loop over the rootLayer. Since we actually should have a layer selection
        // list, no need to elaborate on that here and now
        KoProperties properties;
        properties.setProperty("visible", true);
        QList<KisNodeSP> layers = m_image->root()->childNodes(QStringList("KisLayer"), properties);
        KisNodeSP node;
        Q_FOREACH (KisNodeSP node, layers) {
            devices[0].push_back(node->projection().data());
        }

        QVector<KisParasite::SelectionMode> modes;

        switch (comboBox2->currentIndex()) {
        case 0: modes.push_back(KisParasite::Constant); break;
        case 1: modes.push_back(KisParasite::Random); break;
        case 2: modes.push_back(KisParasite::Incremental); break;
        case 3: modes.push_back(KisParasite::Pressure); break;
        case 4: modes.push_back(KisParasite::Angular); break;
        default: modes.push_back(KisParasite::Incremental);
        }

        m_brush = KisBrushSP(new KisImagePipeBrush(m_image->objectName(), w, h, devices, modes));
        m_image->unlock();
    }

    static_cast<KisGbrBrush*>(m_brush.data())->setUseColorAsMask(colorAsMask->isChecked());
    m_brush->setSpacing(spacingWidget->spacing());
    m_brush->setAutoSpacing(spacingWidget->autoSpacingActive(), spacingWidget->autoSpacingCoeff());
    m_brush->setFilename(TEMPORARY_FILENAME);
    m_brush->setName(TEMPORARY_BRUSH_NAME);
    m_brush->setValid(true);
}
