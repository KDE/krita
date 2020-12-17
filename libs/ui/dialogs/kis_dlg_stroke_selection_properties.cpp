/*
 *  SPDX-FileCopyrightText: 2016 Kapustin Alexey <akapust1n@yandex.ru>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_dlg_stroke_selection_properties.h"

#include <QPushButton>
#include <QRadioButton>
#include <QLayout>
#include <QLabel>
#include <QSpinBox>
#include <QSlider>
#include <QCheckBox>
#include <QPlainTextEdit>
#include <QTextEdit>

#include <klocalizedstring.h>

#include <KoColorSpace.h>
#include "KoColorProfile.h"
#include "KoColorSpaceRegistry.h"
#include "KoColor.h"
#include "KoColorConversionTransformation.h"
#include "KoColorPopupAction.h"
#include "kis_icon_utils.h"
#include "KoID.h"
#include "kis_image.h"
#include "kis_annotation.h"
#include "kis_config.h"
#include "kis_signal_compressor.h"
#include "widgets/kis_cmb_idlist.h"
#include <KisSqueezedComboBox.h>
#include "kis_layer_utils.h"
#include <kis_ls_utils.h>
#include "kis_canvas_resource_provider.h"
#include "KoUnit.h"
#include "kis_display_color_converter.h"

#include <kis_signals_blocker.h>

KisDlgStrokeSelection::KisDlgStrokeSelection(KisImageWSP image, KisViewManager *view, bool isVectorLayer)
    : KoDialog(view->mainWindow())
{
    m_resourceManager = view->mainWindow()->resourceManager();
    KisPropertiesConfigurationSP cfg = KisConfig(true).exportConfiguration("StrokeSelection");

    m_converter = view->canvasBase()->displayColorConverter();
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);
    setCaption(i18nc("@title:window", "Stroke Selection Properties"));
    m_page = new WdgStrokeSelection(this);
    m_page->m_isVectorLayer = isVectorLayer;
    m_page->m_cfg = cfg;

    m_image = image;

    setMainWidget(m_page);

    StrokeSelectionOptions &m_options = m_page->m_options;
    m_options.color = cfg->getColor("color");
    m_options.colorLineSource = static_cast<ColorLineSource>(cfg->getInt("lineColorSource"));
    m_page->lineColorBox->setCurrentIndex(static_cast<int>(m_options.colorLineSource));

    m_page->colorSelector->setColor(getSelectedColor().toQColor());

    m_options.brushSelected = cfg->getBool("useBrush", 0);
    m_page->typeBox->setCurrentIndex(m_options.brushSelected? 0 : 1);

    m_options.colorFillSource = static_cast<ColorFillSource>(cfg->getInt("colorFillSource", 0));
    m_page->fillBox->setCurrentIndex(static_cast<int>(m_options.colorFillSource));
    m_options.customColor = cfg->getColor("customColor");

    if (m_options.colorFillSource == ColorFillSource::CustomColor) {
        m_page->colorFillSelector->setColor(m_options.customColor.toQColor());
    }
    else {
        m_page->colorFillSelector->setColor(getFillSelectedColor().toQColor());
    }

    m_options.lineSize = cfg->getInt("lineSize", 1);
    m_page->lineSize->setValue(m_options.lineSize);

    m_options.lineDimension = cfg->getInt("lineDimension", 0);
    m_page->sizeBox->setCurrentIndex(m_options.lineDimension);

    connect(m_page, SIGNAL(colorSelectorChanged()), SLOT(setColorButton()));
    connect(m_page, SIGNAL(colorFillSelectorChanged()), SLOT(setColorFillButton()));
    connect(m_page->colorFillSelector, SIGNAL(changed(QColor)), SLOT(colorFillChanged(QColor)));
    connect(m_page->colorSelector, SIGNAL(changed(QColor)), SLOT(colorChanged(QColor)));

    m_page->enableControls();

}

KisDlgStrokeSelection::~KisDlgStrokeSelection()
{
    StrokeSelectionOptions &m_options = m_page->m_options;
    m_options.lineSize = m_page->lineSize->value();

    m_options.lineDimension = m_page->sizeBox->currentIndex();
    m_options.colorLineSource = static_cast<ColorLineSource>(m_page->lineColorBox->currentIndex());

    KisPropertiesConfigurationSP cfg(new KisPropertiesConfiguration());
    cfg->setProperty("lineSize", m_options.lineSize);
    cfg->setProperty("colorFillSource", static_cast<int>(m_options.colorFillSource));
    cfg->setProperty("useBrush", m_options.brushSelected);
    cfg->setProperty("lineDimension", m_options.lineDimension);
    cfg->setProperty("lineColorSource", static_cast<int>(m_options.colorLineSource));

    QVariant colorVariant;
    colorVariant.setValue(m_options.customColor);
    cfg->setProperty("customColor", colorVariant);

    colorVariant.setValue(m_options.color);
    cfg->setProperty("color", colorVariant);

    colorVariant.setValue(m_options.fillColor);
    cfg->setProperty("fillColor", colorVariant);

    KisConfig(false).setExportConfiguration("StrokeSelection", cfg);

    delete m_page;
}

KoColor KisDlgStrokeSelection::getSelectedColor() const
{
    KoColor color;

    ColorLineSource currentSource = static_cast<ColorLineSource>(m_page->lineColorBox->currentIndex());
    switch(currentSource) {
    case ColorLineSource::FGColor:
        return m_resourceManager->resource(KoCanvasResource::ForegroundColor).value<KoColor>();
    case ColorLineSource::BGColor:
        return m_resourceManager->resource(KoCanvasResource::BackgroundColor).value<KoColor>();
    default:
        return m_page->m_options.color;
    }
}

KoColor KisDlgStrokeSelection::getFillSelectedColor() const
{
    KoColor color;

    ColorFillSource currentSource = static_cast<ColorFillSource>(m_page->fillBox->currentIndex());
    switch (currentSource) {
    case ColorFillSource::FGColor:
        return m_resourceManager->resource(KoCanvasResource::ForegroundColor).value<KoColor>();
    case ColorFillSource::BGColor:
        return m_resourceManager->resource(KoCanvasResource::BackgroundColor).value<KoColor>();
    case ColorFillSource::PaintColor:
        return m_converter->approximateFromRenderedQColor(m_page->colorSelector->color());
    default:
        return m_page->m_options.customColor;
    }

}


bool KisDlgStrokeSelection::isBrushSelected() const
{
    return (static_cast<DrawType>(m_page->typeBox->currentIndex()) == DrawType::CurrentBrush);
}

StrokeSelectionOptions KisDlgStrokeSelection::getParams() const
{
    StrokeSelectionOptions params;

    params.lineSize = getLineSize();
    params.brushSelected = isBrushSelected();
    params.colorFillSource = m_page->m_options.colorFillSource;
    params.colorLineSource = m_page->m_options.colorLineSource;
    params.lineDimension = m_page->m_options.lineDimension;
    params.color = getSelectedColor();
    params.fillColor = getFillSelectedColor();
    params.customColor = m_page->m_options.customColor;

    return params;

}

void KisDlgStrokeSelection::setColorFillButton()
{
    m_page->colorFillSelector->setColor(getFillSelectedColor().toQColor());
}

void KisDlgStrokeSelection::setColorButton()
{
    m_page->colorSelector->setColor(getSelectedColor().toQColor());
}

int KisDlgStrokeSelection::getLineSize() const
{
    int value = m_page->lineSize->value();

    if (m_page->sizeBox->currentIndex() == 0) {
        return value;
    }
    else if (m_page->sizeBox->currentIndex() == 1) {
        int pixels =  static_cast<int>(KoUnit::convertFromUnitToUnit(value,KoUnit(KoUnit::Millimeter), KoUnit(KoUnit::Pixel)));
        return pixels;
    }
    else {
        int  pixels = static_cast<int>(KoUnit::convertFromUnitToUnit(value, KoUnit(KoUnit::Inch), KoUnit(KoUnit::Pixel)));
        return pixels;
    }
}

LinePosition KisDlgStrokeSelection::getLinePosition() const
{/* TODO
   int index = m_page->linePosition->currentIndex();
   switch(index)
   {
   case(0):
       return linePosition::OUTSIDE;
   case(1):
       return linePosition::INSIDE;
   case(2):
       return linePosition::CENTER;
   default:
       return linePosition::CENTER;
   }*/
    return LinePosition::Center;
}

void KisDlgStrokeSelection::colorChanged(const QColor &newColor)
{
    KisSignalsBlocker blocker(m_page->fillBox, m_page->lineColorBox);

    if (m_page->fillBox->currentIndex() == static_cast<int>(ColorFillSource::PaintColor)) {
        m_page->colorFillSelector->setColor(newColor);
    }

    KoColor FGColor = m_resourceManager->resource(KoCanvasResource::ForegroundColor).value<KoColor>();
    KoColor BGColor = m_resourceManager->resource(KoCanvasResource::BackgroundColor).value<KoColor>();
    KoColor tempColor = m_converter->approximateFromRenderedQColor(newColor);

    if (tempColor.toQColor() == FGColor.toQColor()) {
        m_page->lineColorBox->setCurrentIndex(static_cast<int>(ColorLineSource::FGColor));
    }
    else if (tempColor.toQColor() == BGColor.toQColor()) {
        m_page->lineColorBox->setCurrentIndex(static_cast<int>(ColorLineSource::BGColor));
    }
    else {
        m_page->lineColorBox->setCurrentIndex((int)ColorLineSource::CustomColor);
    }
    m_page->m_options.color = tempColor;
}

void KisDlgStrokeSelection::colorFillChanged(const QColor &newColor)
{
    KisSignalsBlocker blocker(m_page->fillBox);

    KoColor PaintColor = m_converter->approximateFromRenderedQColor(m_page->colorSelector->color());
    KoColor FGColor = m_resourceManager->resource(KoCanvasResource::ForegroundColor).value<KoColor>();
    KoColor BGColor = m_resourceManager->resource(KoCanvasResource::BackgroundColor).value<KoColor>();
    KoColor tempColor= m_converter->approximateFromRenderedQColor(newColor);

    if (tempColor.toQColor() == FGColor.toQColor()) {
        m_page->fillBox->setCurrentIndex(static_cast<int>(ColorFillSource::FGColor));

    }
    else if (tempColor.toQColor() == BGColor.toQColor()) {
        m_page->fillBox->setCurrentIndex(static_cast<int>(ColorFillSource::BGColor));
    }
    else if (PaintColor.toQColor() != tempColor.toQColor() ) {
        m_page->fillBox->setCurrentIndex((int)ColorFillSource::CustomColor);
        m_page->m_options.customColor = tempColor;
    }

    m_page->m_options.fillColor = tempColor;
}



WdgStrokeSelection::WdgStrokeSelection(QWidget *parent) : QWidget(parent)
{
    setupUi(this);
}

void WdgStrokeSelection::enableControls()
{
    m_options.fillColor = m_cfg->getColor("fillColor");
    if (m_options.colorFillSource == ColorFillSource::None) {
        colorFillSelector->setEnabled(false);
    }
    else {
        colorFillSelector->setEnabled(true);
    }

    if (m_isVectorLayer) {
        typeBox->setCurrentIndex(1);
        typeBox->setEnabled(false);
    }
    else {
        typeBox->setEnabled(true);
    }

    on_typeBox_currentIndexChanged(typeBox->currentIndex());
}

void WdgStrokeSelection::on_fillBox_currentIndexChanged(int index)
{
    if (index == static_cast<int>(ColorFillSource::None)) {
        colorFillSelector->setDisabled(true);
    }
    else {
        colorFillSelector->setDisabled(false);
        emit colorFillSelectorChanged();
    }
    m_options.colorFillSource = static_cast<ColorFillSource>(index);
}

void WdgStrokeSelection::on_typeBox_currentIndexChanged(int arg1)
{
    if (arg1 == 0) {
        m_options.brushSelected = true;
        lineSize->setEnabled(false);
        fillBox->setEnabled(false);
        colorFillSelector->setEnabled(false);
        sizeBox->setEnabled(false);
    }
    else {
        m_options.brushSelected = false;
        lineSize->setEnabled(true);
        fillBox->setEnabled(true);
        colorFillSelector->setEnabled(true);
        sizeBox->setEnabled(true);
    }
}

void WdgStrokeSelection::on_lineColorBox_currentIndexChanged(int/*arg1*/)
{
    emit colorSelectorChanged();
}


StrokeSelectionOptions ::StrokeSelectionOptions()
{
    color.fromQColor(Qt::black);
    fillColor.fromQColor(Qt::black);
    customColor.fromQColor(Qt::black);
}

KisToolShapeUtils::FillStyle StrokeSelectionOptions::fillStyle() const
{
    using namespace KisToolShapeUtils;

    ColorFillSource tempColor = static_cast<ColorFillSource>(colorFillSource);
    FillStyle style = FillStyleNone;

    switch (tempColor) {
    case ColorFillSource::PaintColor:
        style = FillStyleForegroundColor;
        break;
    case ColorFillSource::BGColor:
        style = FillStyleBackgroundColor;
        break;
    case ColorFillSource::CustomColor:
        style = FillStyleBackgroundColor;
        break;
    case ColorFillSource::None:
        style = FillStyleNone;
        break;
    case ColorFillSource::FGColor:
        style = FillStyleBackgroundColor;
        break;
    }
    return style;
}


