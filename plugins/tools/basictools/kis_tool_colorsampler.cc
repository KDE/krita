/*
 *  SPDX-FileCopyrightText: 1999 Matthias Elter <me@kde.org>
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  SPDX-FileCopyrightText: 2018 Emmet & Eoin O'Neill <emmetoneill.pdx@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_tool_colorsampler.h"

#include <kis_cursor.h>
#include <kis_canvas2.h>
#include <KoCanvasBase.h>
#include <KoResourceServerProvider.h>
#include <kis_canvas_resource_provider.h>
#include <KisTagFilterResourceProxyModel.h>
#include <KisResourceTypes.h>
#include <KisViewManager.h>
#include "kis_display_color_converter.h"
#include "kis_tool_utils.h"

#include <kis_config.h>
#include <kis_config_notifier.h>
#include <dialogs/kis_dlg_preferences.h>
#include <QSignalBlocker>

namespace
{
// GUI ComboBox index constants
const int SAMPLE_MERGED = 0;
}

KisToolColorSampler::KisToolColorSampler(KoCanvasBase *canvas)
    : KisTool(canvas, KisCursor::samplerCursor()),
      m_config(new KisToolUtils::ColorSamplerConfig),
      m_helper(dynamic_cast<KisCanvas2*>(canvas))
{
    setObjectName("tool_colorsampler");
    connect(&m_helper, SIGNAL(sigRequestCursor(QCursor)), this, SLOT(slotColorPickerRequestedCursor(QCursor)));
    connect(&m_helper, SIGNAL(sigRequestCursorReset()), this, SLOT(slotColorPickerRequestedCursorReset()));
    connect(&m_helper, SIGNAL(sigRequestUpdateOutline()), this, SLOT(slotColorPickerRequestedOutlineUpdate()));
    connect(&m_helper, SIGNAL(sigRawColorSelected(KoColor)), this, SLOT(slotColorPickerSelectedColor(KoColor)));
    connect(&m_helper, SIGNAL(sigFinalColorSelected(KoColor)), this, SLOT(slotColorPickerSelectionFinished(KoColor)));
}

KisToolColorSampler::~KisToolColorSampler()
{
    if (m_isActivated) {
        m_config->save();
    }
}

void KisToolColorSampler::slotColorPickerRequestedCursor(const QCursor &cursor)
{
    useCursor(cursor);
}

void KisToolColorSampler::slotColorPickerRequestedCursorReset()
{
    /// we explicitly avoid resetting the cursor style
    /// to avoid blinking of the cursor
}

void KisToolColorSampler::slotColorPickerRequestedOutlineUpdate()
{
    requestUpdateOutline(m_outlineDocPoint, 0);
}

void KisToolColorSampler::slotColorPickerSelectedColor(const KoColor &color)
{
    /**
     * Please remember that m_sampledColor also have the alpha
     * of the picked color!
     */
    m_sampledColor = color;
    displaySampledColor(m_sampledColor);
}

void KisToolColorSampler::slotColorPickerSelectionFinished(const KoColor &color)
{
    Q_UNUSED(color);

    if (m_config->addColorToCurrentPalette) {
        KisSwatch swatch;
        swatch.setColor(color);
        // We don't ask for a name, too intrusive here

        QModelIndex idx = m_tagFilterProxyModel->index(m_optionsWidget->cmbPalette->currentIndex(), 0);
        KoColorSetSP palette = qSharedPointerDynamicCast<KoColorSet>(m_tagFilterProxyModel->resourceForIndex(idx));

        if (palette) {
            KisSwatchGroup::SwatchInfo info =
                    palette->getClosestSwatchInfo(color);

            if (info.swatch.color() != color) {
                palette->addSwatch(swatch);
                if (!KoResourceServerProvider::instance()->paletteServer()->updateResource(palette)) {
                    KisCanvas2 *canvas = dynamic_cast<KisCanvas2*>(this->canvas());
                    KIS_ASSERT(canvas);
                    canvas->viewManager()->showFloatingMessage(i18n("Cannot write to palette file %1. Maybe it is read-only.", palette->filename()), koIcon("object-locked"));
                }
            }
        }
    }
}

void KisToolColorSampler::slotSetPreviewStyleComboBoxFromConfig()
{
    if (m_optionsWidget) {
        QSignalBlocker blocker(m_optionsWidget->cmbPreviewStyle);
        GeneralTab::setColorSamplerPreviewStyleIndexByValue(m_optionsWidget->cmbPreviewStyle,
                                                            KisConfig(true).colorSamplerPreviewStyle());
    }
}

void KisToolColorSampler::slotSetPreviewStyleConfigFromComboBox()
{
    if (m_optionsWidget) {
        KisConfig::ColorSamplerPreviewStyle style =
            GeneralTab::getColorSamplerPreviewStyleValue(m_optionsWidget->cmbPreviewStyle);
        KisConfig(false).setColorSamplerPreviewStyle(style);
    }
}

void KisToolColorSampler::paint(QPainter &gc, const KoViewConverter &converter)
{
    m_helper.paint(gc, converter);
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

void KisToolColorSampler::beginPrimaryAction(KoPointerEvent *event)
{
    m_helper.setUpdateGlobalColor(m_config->updateColor);
    m_helper.activate(!m_config->sampleMerged, m_config->toForegroundColor);
    m_helper.startAction(event->point, m_config->radius, m_config->blend);
    requestUpdateOutline(event->point, event);

    setMode(KisTool::PAINT_MODE);
}

void KisToolColorSampler::mouseMoveEvent(KoPointerEvent *event){
    KisTool::mouseMoveEvent(event);
}

void KisToolColorSampler::continuePrimaryAction(KoPointerEvent *event)
{
    CHECK_MODE_SANITY_OR_RETURN(KisTool::PAINT_MODE);

    m_helper.continueAction(event->point);
    requestUpdateOutline(event->point, event);
}

void KisToolColorSampler::endPrimaryAction(KoPointerEvent *event)
{
    CHECK_MODE_SANITY_OR_RETURN(KisTool::PAINT_MODE);

    m_helper.endAction();
    m_helper.deactivate();
    requestUpdateOutline(event->point, event);

}
void KisToolColorSampler::activatePrimaryAction()
{
    /**
     * We explicitly avoid calling KisTool::activatePrimaryAction()
     * here, because it resets the cursor, causing cursor blinking
     */
    m_helper.updateCursor(!m_config->sampleMerged, m_config->toForegroundColor);
}

void KisToolColorSampler::deactivatePrimaryAction()
{
    /**
     * We explicitly avoid calling KisTool::endPrimaryAction()
     * here, because it resets the cursor, causing cursor blinking
     */
}

void KisToolColorSampler::requestUpdateOutline(const QPointF &outlineDocPoint, const KoPointerEvent *event)
{
    Q_UNUSED(event);

    KisConfig cfg(true);

    QRectF colorPreviewDocUpdateRect;

    qreal zoomX;
    qreal zoomY;
    canvas()->viewConverter()->zoom(&zoomX, &zoomY);
    qreal xoffset = 2.0/zoomX;
    qreal yoffset = 2.0/zoomY;

    m_outlineDocPoint = outlineDocPoint;

    colorPreviewDocUpdateRect = m_helper.colorPreviewDocRect(m_outlineDocPoint);

    if (!colorPreviewDocUpdateRect.isEmpty()) {
        colorPreviewDocUpdateRect = colorPreviewDocUpdateRect.adjusted(-xoffset,-yoffset,xoffset,yoffset);
    }

    if (!m_oldColorPreviewUpdateRect.isEmpty()){
        canvas()->updateCanvas(m_oldColorPreviewUpdateRect);
    }

    if (!colorPreviewDocUpdateRect.isEmpty()){
        canvas()->updateCanvas(colorPreviewDocUpdateRect);
    }

    m_oldColorPreviewUpdateRect = colorPreviewDocUpdateRect;
}


struct SampledChannel {
    QString name;
    QString valueText;
};

void KisToolColorSampler::displaySampledColor(const KoColor &color)
{
    if (color.data() && m_optionsWidget) {

        const QList<KoChannelInfo *> channels = color.colorSpace()->channels();
        m_optionsWidget->listViewChannels->clear();

        QVector<SampledChannel> sampledChannels;
        for (int i = 0; i < channels.count(); ++i) {
            sampledChannels.append(SampledChannel());
        }

        for (int i = 0; i < channels.count(); ++i) {

            SampledChannel pc;
            pc.name = channels[i]->name();

            if (m_config->normaliseValues) {
                pc.valueText = color.colorSpace()->normalisedChannelValueText(color.data(), i);
            } else {
                pc.valueText = color.colorSpace()->channelValueText(color.data(), i);
            }

            sampledChannels[channels[i]->displayPosition()] = pc;

        }

        Q_FOREACH (const SampledChannel &pc, sampledChannels) {
            QTreeWidgetItem *item = new QTreeWidgetItem(m_optionsWidget->listViewChannels);
            item->setText(0, pc.name);
            item->setText(1, pc.valueText);
        }


        if (qEnvironmentVariableIsSet("KRITA_DEBUG_DISPLAY_COLOR")) {
            KisCanvas2 *kritaCanvas = dynamic_cast<KisCanvas2*>(canvas());
            KIS_ASSERT(kritaCanvas);
            KoColor newColor = kritaCanvas->displayColorConverter()->applyDisplayFiltering(color, Float32BitsColorDepthID);
            KIS_SAFE_ASSERT_RECOVER_RETURN(newColor.colorSpace()->colorModelId() == RGBAColorModelID);

            QVector<float> values(4);
            newColor.colorSpace()->normalisedChannelsValue(newColor.data(), values);

            for (int i = 0; i < values.size(); i++) {
                QTreeWidgetItem *item = new QTreeWidgetItem(m_optionsWidget->listViewChannels);
                item->setText(0, QString("DisplayCh%1").arg(i));
                item->setText(1, QString::number(values[i]));
            }
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

    GeneralTab::setColorSamplerPreviewStyleItems(m_optionsWidget->cmbPreviewStyle);
    connect(KisConfigNotifier::instance(),
            &KisConfigNotifier::sigColorSamplerPreviewStyleChanged,
            this,
            &KisToolColorSampler::slotSetPreviewStyleComboBoxFromConfig);
    connect(m_optionsWidget->cmbPreviewStyle,
            QOverload<int>::of(&QComboBox::activated),
            this,
            &KisToolColorSampler::slotSetPreviewStyleConfigFromComboBox);

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


    KConfigGroup config =  KSharedConfig::openConfig()->group(toolId());
    QString paletteName = config.readEntry("ColorSamplerPalette", "");
    if (!paletteName.isEmpty()) {
        for (int i = 0; i < m_tagFilterProxyModel->rowCount(); i++) {
            QModelIndex idx = m_tagFilterProxyModel->index(i, 0);
            QString name = m_tagFilterProxyModel->data(idx, Qt::UserRole + KisAbstractResourceModel::Name).toString();
            if (name == paletteName) {
                m_optionsWidget->cmbPalette->setCurrentIndex(i);
                break;
            }
        }
    }

    connect(m_optionsWidget->cmbPalette, SIGNAL(currentIndexChanged(int)), SLOT(slotChangePalette(int)));

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

    slotSetPreviewStyleComboBoxFromConfig();
}

void KisToolColorSampler::slotSetUpdateColor(bool state)
{
    m_config->updateColor = state;
}

void KisToolColorSampler::slotSetNormaliseValues(bool state)
{
    m_config->normaliseValues = state;
    displaySampledColor(m_sampledColor);
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

void KisToolColorSampler::slotChangePalette(int)
{
    QString paletteName = m_optionsWidget->cmbPalette->currentData(Qt::UserRole + KisAbstractResourceModel::Name).toString();
    KConfigGroup config =  KSharedConfig::openConfig()->group(toolId());
    config.writeEntry("ColorSamplerPalette", paletteName);
}
