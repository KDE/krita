/*
 *  SPDX-FileCopyrightText: 1999 Matthias Elter <me@kde.org>
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  SPDX-FileCopyrightText: 2018 Emmet & Eoin O'Neill <emmetoneill.pdx@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_tool_colorsampler.h"

#include <boost/thread/locks.hpp>
#include <QMessageBox>
#include <kis_cursor.h>
#include <KisDocument.h>
#include <kis_canvas2.h>
#include <KisReferenceImagesLayer.h>
#include <KoCanvasBase.h>
#include <kis_random_accessor_ng.h>
#include <KoResourceServerProvider.h>
#include <KoMixColorsOp.h>
#include <kis_wrapped_rect.h>
#include <KisTagFilterResourceProxyModel.h>
#include <KisResourceTypes.h>

#include "kis_tool_utils.h"


namespace
{
// GUI ComboBox index constants
const int SAMPLE_MERGED = 0;
}

KisToolColorSampler::KisToolColorSampler(KoCanvasBase *canvas)
    : KisTool(canvas, KisCursor::samplerCursor()),
      m_config(new KisToolUtils::ColorSamplerConfig)
{
    setObjectName("tool_colorsampler");
}

KisToolColorSampler::~KisToolColorSampler()
{
    if (m_isActivated) {
        m_config->save();
    }
}

void KisToolColorSampler::paint(QPainter &gc, const KoViewConverter &converter)
{
    Q_UNUSED(gc);
    Q_UNUSED(converter);
}

void KisToolColorSampler::activate(const QSet<KoShape*> &shapes)
{
    m_isActivated = true;
    m_config->load();
    updateOptionWidget();

    KisTool::activate(shapes);
}

void KisToolColorSampler::deactivate()
{
    m_config->save();
    m_isActivated = false;
    KisTool::deactivate();
}

bool KisToolColorSampler::sampleColor(const QPointF &pos)
{
    // Timer check.
    if (m_colorSamplerDelayTimer.isActive()) {
        return false;
    }
    else {
        m_colorSamplerDelayTimer.setSingleShot(true);
        m_colorSamplerDelayTimer.start(100);
    }

    QScopedPointer<boost::lock_guard<KisImage>> imageLocker;

    m_sampledColor.setOpacity(0.0);

    // Sample from reference images.
    if (m_optionsWidget->cmbSources->currentIndex() == SAMPLE_MERGED) {
        auto *kisCanvas = dynamic_cast<KisCanvas2 *>(canvas());
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(kisCanvas, false);
        KisSharedPtr<KisReferenceImagesLayer> referenceImageLayer =
            kisCanvas->imageView()->document()->referenceImagesLayer();

        if (referenceImageLayer && kisCanvas->referenceImagesDecoration()->visible()) {
            QColor color = referenceImageLayer->getPixel(pos);
            if (color.isValid()) {
                m_sampledColor.fromQColor(color);
            }
        }
    }

    if (m_sampledColor.opacityU8() == OPACITY_TRANSPARENT_U8) {
        if (!currentImage()->bounds().contains(pos.toPoint()) &&
            !currentImage()->wrapAroundModePermitted()) {
            return false;
        }

        KisPaintDeviceSP dev;

        if (m_optionsWidget->cmbSources->currentIndex() != SAMPLE_MERGED &&
            currentNode() && currentNode()->colorSampleSourceDevice()) {
            dev = currentNode()->colorSampleSourceDevice();
        }
        else {
            imageLocker.reset(new boost::lock_guard<KisImage>(*currentImage()));
            dev = currentImage()->projection();
        }

        KoColor previousColor = canvas()->resourceManager()->foregroundColor();

        KisToolUtils::sampleColor(m_sampledColor, dev, pos.toPoint(), &previousColor, m_config->radius, m_config->blend);
    }

    if (m_config->updateColor &&
        m_sampledColor.opacityU8() != OPACITY_TRANSPARENT_U8) {

        KoColor publicColor = m_sampledColor;
        publicColor.setOpacity(OPACITY_OPAQUE_U8); // Alpha is unwanted for FG and BG colors.

        if (m_config->toForegroundColor) {
            canvas()->resourceManager()->setResource(KoCanvasResource::ForegroundColor, publicColor);
        }
        else {
            canvas()->resourceManager()->setResource(KoCanvasResource::BackgroundColor, publicColor);
        }
    }

    return true;
}

void KisToolColorSampler::beginPrimaryAction(KoPointerEvent *event)
{
    bool sampleMerged = m_optionsWidget->cmbSources->currentIndex() == SAMPLE_MERGED;
    if (!sampleMerged) {
        if (!currentNode()) {
            QMessageBox::information(qApp->activeWindow(), i18nc("@title:window", "Krita"), i18n("Cannot sample a color as no layer is active."));
            event->ignore();
            return;
        }
        if (!currentNode()->visible()) {
            QMessageBox::information(qApp->activeWindow(), i18nc("@title:window", "Krita"), i18n("Cannot sample a color as the active layer is not visible."));
            event->ignore();
            return;
        }
    }

    QPoint pos = convertToImagePixelCoordFloored(event);

    setMode(KisTool::PAINT_MODE);

    bool sampled = sampleColor(pos);
    if (!sampled) {
        // Color sampling has to start in the visible part of the layer
        event->ignore();
        return;
    }

    displaySampledColor();
}

void KisToolColorSampler::continuePrimaryAction(KoPointerEvent *event)
{
    CHECK_MODE_SANITY_OR_RETURN(KisTool::PAINT_MODE);

    QPoint pos = convertToImagePixelCoordFloored(event);
    sampleColor(pos);
    displaySampledColor();
}

#include "kis_display_color_converter.h"

void KisToolColorSampler::endPrimaryAction(KoPointerEvent *event)
{
    Q_UNUSED(event);
    CHECK_MODE_SANITY_OR_RETURN(KisTool::PAINT_MODE);

    if (m_config->addColorToCurrentPalette) {
        KisSwatch swatch;
        swatch.setColor(m_sampledColor);
        // We don't ask for a name, too intrusive here

        QModelIndex idx = m_tagFilterProxyModel->index(m_optionsWidget->cmbPalette->currentIndex(), 0);
        KoColorSetSP palette = qSharedPointerDynamicCast<KoColorSet>(m_tagFilterProxyModel->resourceForIndex(idx));

        if (palette) {
            palette->add(swatch);
            if (!KoResourceServerProvider::instance()->paletteServer()->updateResource(palette)) {
                QMessageBox::critical(qApp->activeWindow(), i18nc("@title:window", "Krita"), i18n("Cannot write to palette file %1. Maybe it is read-only.", palette->filename()));
            }
        }
    }

}

struct SampledChannel {
    QString name;
    QString valueText;
};

void KisToolColorSampler::displaySampledColor()
{
    if (m_sampledColor.data() && m_optionsWidget) {

        QList<KoChannelInfo *> channels = m_sampledColor.colorSpace()->channels();
        m_optionsWidget->listViewChannels->clear();

        QVector<SampledChannel> sampledChannels;
        for (int i = 0; i < channels.count(); ++i) {
            sampledChannels.append(SampledChannel());
        }

        for (int i = 0; i < channels.count(); ++i) {

            SampledChannel pc;
            pc.name = channels[i]->name();

            if (m_config->normaliseValues) {
                pc.valueText = m_sampledColor.colorSpace()->normalisedChannelValueText(m_sampledColor.data(), i);
            } else {
                pc.valueText = m_sampledColor.colorSpace()->channelValueText(m_sampledColor.data(), i);
            }

            sampledChannels[channels[i]->displayPosition()] = pc;

        }

        Q_FOREACH (const SampledChannel &pc, sampledChannels) {
            QTreeWidgetItem *item = new QTreeWidgetItem(m_optionsWidget->listViewChannels);
            item->setText(0, pc.name);
            item->setText(1, pc.valueText);
        }

        KisCanvas2 *kritaCanvas = dynamic_cast<KisCanvas2*>(canvas());
        KoColor newColor = kritaCanvas->displayColorConverter()->applyDisplayFiltering(m_sampledColor, Float32BitsColorDepthID);
        QVector<float> values(4);
        newColor.colorSpace()->normalisedChannelsValue(newColor.data(), values);

        for (int i = 0; i < values.size(); i++) {
            QTreeWidgetItem *item = new QTreeWidgetItem(m_optionsWidget->listViewChannels);
            item->setText(0, QString("DisplayCh%1").arg(i));
            item->setText(1, QString::number(values[i]));
        }
    }
}

QWidget* KisToolColorSampler::createOptionWidget()
{
    m_optionsWidget = new ColorSamplerOptionsWidget(0);
    m_optionsWidget->setObjectName(toolId() + " option widget");
    m_optionsWidget->listViewChannels->setSortingEnabled(false);

    // See https://bugs.kde.org/show_bug.cgi?id=316896
    QWidget *specialSpacer = new QWidget(m_optionsWidget);
    specialSpacer->setObjectName("SpecialSpacer");
    specialSpacer->setFixedSize(0, 0);
    m_optionsWidget->layout()->addWidget(specialSpacer);

    // Initialize blend KisSliderSpinBox
    m_optionsWidget->blend->setRange(0,100);
    m_optionsWidget->blend->setSuffix(i18n("%"));

    updateOptionWidget();

    connect(m_optionsWidget->cbUpdateCurrentColor, SIGNAL(toggled(bool)), SLOT(slotSetUpdateColor(bool)));
    connect(m_optionsWidget->cbNormaliseValues, SIGNAL(toggled(bool)), SLOT(slotSetNormaliseValues(bool)));
    connect(m_optionsWidget->cbPalette, SIGNAL(toggled(bool)),
            SLOT(slotSetAddPalette(bool)));
    connect(m_optionsWidget->radius, SIGNAL(valueChanged(int)),
            SLOT(slotChangeRadius(int)));
    connect(m_optionsWidget->blend, SIGNAL(valueChanged(int)),
            SLOT(slotChangeBlend(int)));
    connect(m_optionsWidget->cmbSources, SIGNAL(currentIndexChanged(int)),
            SLOT(slotSetColorSource(int)));

    m_tagFilterProxyModel = new KisTagFilterResourceProxyModel(ResourceType::Palettes, this);
    m_optionsWidget->cmbPalette->setModel(m_tagFilterProxyModel);
    m_optionsWidget->cmbPalette->setModelColumn(KisAbstractResourceModel::Name);
    m_tagFilterProxyModel->sort(Qt::DisplayRole);

    return m_optionsWidget;
}

void KisToolColorSampler::updateOptionWidget()
{
    if (!m_optionsWidget) return;

    m_optionsWidget->cbNormaliseValues->setChecked(m_config->normaliseValues);
    m_optionsWidget->cbUpdateCurrentColor->setChecked(m_config->updateColor);
    m_optionsWidget->cmbSources->setCurrentIndex(SAMPLE_MERGED + !m_config->sampleMerged);
    m_optionsWidget->cbPalette->setChecked(m_config->addColorToCurrentPalette);
    m_optionsWidget->radius->setValue(m_config->radius);
    m_optionsWidget->blend->setValue(m_config->blend);
}

void KisToolColorSampler::setToForeground(bool newValue)
{
    m_config->toForegroundColor = newValue;
    emit toForegroundChanged();
}

bool KisToolColorSampler::toForeground() const
{
    return m_config->toForegroundColor;
}

void KisToolColorSampler::slotSetUpdateColor(bool state)
{
    m_config->updateColor = state;
}

void KisToolColorSampler::slotSetNormaliseValues(bool state)
{
    m_config->normaliseValues = state;
    displaySampledColor();
}

void KisToolColorSampler::slotSetAddPalette(bool state)
{
    m_config->addColorToCurrentPalette = state;
}

void KisToolColorSampler::slotChangeRadius(int value)
{
    m_config->radius = value;
}

void KisToolColorSampler::slotChangeBlend(int value)
{
    m_config->blend = value;
}

void KisToolColorSampler::slotSetColorSource(int value)
{
    m_config->sampleMerged = value == SAMPLE_MERGED;
}
