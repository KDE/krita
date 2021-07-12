/*
 *  SPDX-FileCopyrightText: 2005 Bart Coppens <kde@bartcoppens.be>
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
#include "kis_image_barrier_locker.h"

#include <kstandardguiitem.h>

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
    connect(preserveAlpha, SIGNAL(toggled(bool)), this, SLOT(slotUpdateCurrentBrush()));
    connect(comboBox2, SIGNAL(currentIndexChanged(int)), this, SLOT(slotUpdateCurrentBrush(int)));


    colorAsMask->setChecked(true); // use color as mask by default. This is by far the most common way to make tip.
    spacingWidget->setSpacing(true, 1.0);
    connect(spacingWidget, SIGNAL(sigSpacingChanged()), SLOT(slotSpacingChanged()));

    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Save), KStandardGuiItem::save());
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Cancel), KStandardGuiItem::cancel());
}

KisCustomBrushWidget::~KisCustomBrushWidget()
{
}


KisBrushSP KisCustomBrushWidget::brush()
{
    return m_brush;
}

void KisCustomBrushWidget::setImage(KisImageWSP image){
    m_image = image;
    createBrush();
    updatePreviewImage();
}

void KisCustomBrushWidget::showEvent(QShowEvent *)
{
    slotUpdateCurrentBrush(0);
}

void KisCustomBrushWidget::updatePreviewImage()
{
    QImage brushImage = m_brush ? m_brush->brushTipImage() : QImage();

    if (!brushImage.isNull()) {
        int w = preview->size().width() - 10; // 10 for the padding...
        brushImage = brushImage.scaled(w, w, Qt::KeepAspectRatio);
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
    preserveAlpha->setEnabled(useColorAsMask);
    slotUpdateCurrentBrush();
}

void KisCustomBrushWidget::slotUpdateSaveButton()
{
    QString suffix = ".gbr";
    if (brushStyle->currentIndex() != 0) {
        suffix = ".gih";
    }
    if (QFileInfo(m_rServer->saveLocation() + "/" + nameLineEdit->text().split(" ").join("_")
                  + suffix).exists()) {
        buttonBox->button(QDialogButtonBox::Save)->setText(i18n("Overwrite"));
    } else {
        buttonBox->button(QDialogButtonBox::Save)->setText(i18n("Save"));
    }
}


void KisCustomBrushWidget::slotAddPredefined()
{
    QString dir = KoResourcePaths::saveLocation("data", ResourceType::Brushes);

    QString name = nameLineEdit->text();

    if (nameLineEdit->text().isEmpty()) {
        name = (QDateTime::currentDateTime().toString("yyyy-MM-ddThh:mm"));
    }

    // Add it to the brush server, so that it automatically gets to the mediators, and
    // so to the other brush choosers can pick it up, if they want to
    if (m_rServer && m_brush) {


        if (m_brush->clone().dynamicCast<KisGbrBrush>()) {
            KisGbrBrushSP resource = m_brush->clone().dynamicCast<KisGbrBrush>();
            resource->setName(name);
            resource->setFilename(resource->name().split(" ").join("_") + resource->defaultFileExtension());
            m_rServer->addResource(resource.dynamicCast<KisBrush>());
            emit sigNewPredefinedBrush(resource);
        }
        else {
            KisImagePipeBrushSP resource = m_brush->clone().dynamicCast<KisImagePipeBrush>();
            resource->setName(name);
            resource->setFilename(resource->name().split(" ").join("_") + resource->defaultFileExtension());
            m_rServer->addResource(resource.dynamicCast<KisBrush>());
            emit sigNewPredefinedBrush(resource);
        }
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
        m_image->barrierLock();
        KisPaintDeviceSP dev = new KisPaintDevice(*m_image->projection());
        m_image->unlock();

        if (!selection) {
            m_brush = KisBrushSP(new KisGbrBrush(dev, 0, 0, m_image->width(), m_image->height()));
        }
        else {
            // apply selection mask
            QRect r = selection->selectedExactRect();

            KisHLineIteratorSP pixelIt = dev->createHLineIteratorNG(r.x(), r.top(), r.width());
            KisHLineConstIteratorSP maskIt = selection->projection()->createHLineIteratorNG(r.x(), r.top(), r.width());

            for (qint32 y = r.top(); y <= r.bottom(); ++y) {

                do {
                    dev->colorSpace()->applyAlphaU8Mask(pixelIt->rawData(), maskIt->oldRawData(), 1);
                } while (pixelIt->nextPixel() && maskIt->nextPixel());

                pixelIt->nextRow();
                maskIt->nextRow();
            }
            m_brush = KisBrushSP(new KisGbrBrush(dev, r.x(), r.y(), r.width(), r.height()));
        }

    }
    else {
        // For each layer in the current image, create a new image, and add it to the list
        QVector< QVector<KisPaintDevice*> > devices;
        devices.push_back(QVector<KisPaintDevice*>());
        int w = m_image->width();
        int h = m_image->height();

        KisImageBarrierLocker locker(m_image);

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
    }
    if (colorAsMask->isChecked()) {
        static_cast<KisGbrBrush*>(m_brush.data())->makeMaskImage(preserveAlpha->isChecked());
        static_cast<KisGbrBrush*>(m_brush.data())->setBrushApplication(preserveAlpha->isChecked() ? LIGHTNESSMAP : ALPHAMASK);
    } else {
        static_cast<KisGbrBrush*>(m_brush.data())->setBrushApplication(IMAGESTAMP);
    }
    m_brush->setSpacing(spacingWidget->spacing());
    m_brush->setAutoSpacing(spacingWidget->autoSpacingActive(), spacingWidget->autoSpacingCoeff());
    m_brush->setFilename(TEMPORARY_FILENAME);
    m_brush->setName(TEMPORARY_BRUSH_NAME);
    m_brush->setValid(true);
}
