/*
 *  Copyright (c) 2016 Kapustin Alexey <akapust1n@yandex.ru>
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

KisDlgStrokeSelection::KisDlgStrokeSelection(KisImageWSP image, KisViewManager *view, bool isVectorLayer)
    : KoDialog(view->mainWindow())
{
    m_resourceManager = view->mainWindow()->resourceManager();

    converter = view->canvasBase()->displayColorConverter();
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);
    setCaption(i18nc("@title:window", "Stroke Selection Properties"));
    m_page = new WdgStrokeSelection(this);

    m_image = image;

    setMainWidget(m_page);
    resize(m_page->sizeHint());

    KisPropertiesConfigurationSP cfg = KisConfig(true).exportConfiguration("StrokeSelection");

    auto &m_options = m_page->m_options;
    m_options.color = cfg->getColor("color");
    m_options.lineColorSource = cfg->getInt("lineColorSource");
    m_page->lineColorBox->setCurrentIndex(m_options.lineColorSource);

    m_page->colorSelector->setColor(getSelectedColor().toQColor());

    m_options.brushSelected = cfg->getBool("useBrush", 0);
    m_page->typeBox->setCurrentIndex(m_options.brushSelected? 0 : 1);

    m_options._colorFillSource = cfg->getInt("colorFillSource", 0);
    m_page->fillBox->setCurrentIndex(m_options._colorFillSource);
    m_options.customColor = cfg->getColor("customColor");
    if (m_options._colorFillSource == static_cast<int>(colorFillSource::CustomColor)) {
        m_page->colorFillSelector->setColor(m_options.customColor.toQColor());
    }
    else {
        m_page->colorFillSelector->setColor(getFillSelectedColor().toQColor());
    }

    m_options.fillColor = cfg->getColor("fillColor");
    if (m_options._colorFillSource == static_cast<int>(colorFillSource::None)) {
        m_page->colorFillSelector->setDisabled(true);
    }
    else {
        m_page->colorFillSelector->setDisabled(false);    }

    m_options.lineSize = cfg->getInt("lineSize", 1);
    m_page->lineSize->setValue(m_options.lineSize);
    if (m_options.brushSelected) {
        m_page->lineSize->setDisabled(true);
        m_page->fillBox->setDisabled(true);
        m_page->colorFillSelector->setDisabled(true);
        m_page->sizeBox->setDisabled(true);
    }

    m_options.lineDimension = cfg->getInt("lineDimension", 0);
    m_page->sizeBox->setCurrentIndex(m_options.lineDimension);

    connect(m_page, SIGNAL(colorSelectorChanged()), SLOT(setColorButton()));
    connect(m_page, SIGNAL(colorFillSelectorChanged()), SLOT(setColorFillButton()));
    connect(m_page->colorFillSelector, SIGNAL(changed(QColor)), SLOT(colorFillChanged(QColor)));
    connect(m_page->colorSelector, SIGNAL(changed(QColor)), SLOT(colorChanged(QColor)));

    if (isVectorLayer) {
        lockVectorLayerFunctions();
    }
}

KisDlgStrokeSelection::~KisDlgStrokeSelection()
{
    auto &m_options = m_page->m_options;
    m_options.lineSize = m_page->lineSize->value();

    m_options.lineDimension = m_page->sizeBox->currentIndex();
    m_options.lineColorSource = m_page->lineColorBox->currentIndex();

    KisPropertiesConfigurationSP cfg(new KisPropertiesConfiguration());
    cfg->setProperty("lineSize", m_options.lineSize);
    cfg->setProperty("colorFillSource", m_options._colorFillSource);
    cfg->setProperty("useBrush", m_options.brushSelected);
    cfg->setProperty("lineDimension", m_options.lineDimension);
    cfg->setProperty("lineColorSource", m_options.lineColorSource);

    QVariant colorVariant("KoColor");
    colorVariant.setValue(m_options.customColor);
    cfg->setProperty("customColor", colorVariant);

    QVariant  _colorVariant("KoColor");
    _colorVariant.setValue(m_options.color);
    cfg->setProperty("color", _colorVariant);

    QVariant _cVariant("KoColor");
    _cVariant.setValue(m_options.fillColor);
    cfg->setProperty("fillColor", _cVariant);

    KisConfig(false).setExportConfiguration("StrokeSelection", cfg);

    delete m_page;
}

KoColor KisDlgStrokeSelection::getSelectedColor() const
{
    KoColor color;

    QString currentSource = m_page->lineColorBox->currentText();

    if (currentSource == "Foreground color") {
        color = m_resourceManager->resource(KoCanvasResourceProvider::ForegroundColor).value<KoColor>();
    }
    else  if (currentSource == "Background color") {
              color = m_resourceManager->resource(KoCanvasResourceProvider::BackgroundColor).value<KoColor>();
          }
          else  {
              color = m_page->m_options.color;
           }

    return color;
}

KoColor KisDlgStrokeSelection::getFillSelectedColor() const
{
    KoColor color;

    colorFillSource currentSource = static_cast<colorFillSource>(m_page->fillBox->currentIndex());

    if (currentSource == colorFillSource::FGColor) {
        color = m_resourceManager->resource(KoCanvasResourceProvider::ForegroundColor).value<KoColor>();
    }
    else  if (currentSource == colorFillSource::BGColor) {
              color = m_resourceManager->resource(KoCanvasResourceProvider::BackgroundColor).value<KoColor>();
          }
          else  if (currentSource == colorFillSource::PaintColor) {
                    color = converter->approximateFromRenderedQColor(m_page->colorSelector->color());
                }
                else  {
                    color = m_page->m_options.customColor;
                }

    return color;
}


bool KisDlgStrokeSelection::isBrushSelected() const
{
     int index = m_page->typeBox->currentIndex();
     drawType type = static_cast<drawType>(index);

     if (type == drawType::brushDraw){
         return true;
     }
     else {
         return false;
     }
}

StrokeSelectionOptions KisDlgStrokeSelection::getParams() const
 {
     StrokeSelectionOptions params;

     params.lineSize = getLineSize();
     params.color = getSelectedColor();
     params.brushSelected = isBrushSelected();
     params.fillColor = getFillSelectedColor();
     params._colorFillSource = m_page->m_options._colorFillSource;
     return params;

}

void KisDlgStrokeSelection::lockVectorLayerFunctions()
{
    m_page->colorFillSelector->setEnabled(false);
    m_page->lineSize->setEnabled(false);
    m_page->sizeBox->setEnabled(false);
    m_page->fillBox->setEnabled(false);
    m_page->typeBox->setEnabled(false);
}

void KisDlgStrokeSelection::unlockVectorLayerFunctions()
{
    m_page->colorFillSelector->setEnabled(true);
    m_page->lineSize->setEnabled(true);
    m_page->sizeBox->setEnabled(true);
    m_page->fillBox->setEnabled(true);
    m_page->typeBox->setEnabled(true);
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

    if (m_page->sizeBox->currentText() == "px") {
        return value;
    }
    else if (m_page->sizeBox->currentText() == "mm"){
             int pixels =  static_cast<int>(KoUnit::convertFromUnitToUnit(value,KoUnit(KoUnit::Millimeter), KoUnit(KoUnit::Pixel)));
             return pixels;
    }
         else {
             int  pixels = static_cast<int>(KoUnit::convertFromUnitToUnit(value, KoUnit(KoUnit::Inch), KoUnit(KoUnit::Pixel)));
             return pixels;
    }
}

linePosition KisDlgStrokeSelection::getLinePosition() const
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
    return linePosition::CENTER;
}

void KisDlgStrokeSelection::colorChanged(const QColor &newColor)
{
    if (m_page->fillBox->currentText() == "Paint color") {
        m_page->colorFillSelector->setColor(newColor);
    }
    QColor BGColor = m_resourceManager->resource(KoCanvasResourceProvider::BackgroundColor).value<KoColor>().toQColor();
    QColor FGColor = m_resourceManager->resource(KoCanvasResourceProvider::ForegroundColor).value<KoColor>().toQColor();
    KoColor tempColor= converter->approximateFromRenderedQColor(newColor);


     if (!(newColor == BGColor) && !(newColor == FGColor)) {
        m_page->m_options.color = tempColor;
        m_page->lineColorBox->setCurrentIndex(2);  //custom color
     }
}

void KisDlgStrokeSelection::colorFillChanged(const QColor &newColor)
{
    QColor PaintColor = m_page->colorSelector->color();
    QColor BGcolor = m_resourceManager->resource(KoCanvasResourceProvider::BackgroundColor).value<KoColor>().toQColor();
    QColor FGColor = m_resourceManager->resource(KoCanvasResourceProvider::ForegroundColor).value<KoColor>().toQColor();
    KoColor tempColor= converter->approximateFromRenderedQColor(newColor);

    if (!(newColor == FGColor) && !(newColor == BGcolor) && !(newColor == PaintColor)) {
        m_page->m_options.customColor = tempColor;
        m_page->fillBox->setCurrentIndex(static_cast<int>(colorFillSource::CustomColor));
    }
    m_page->m_options.fillColor = tempColor;
}



WdgStrokeSelection::WdgStrokeSelection(QWidget *parent) : QWidget(parent)
{
    setupUi(this);
}

void WdgStrokeSelection::on_fillBox_currentIndexChanged(int index)
{
    if (index == static_cast<int>(colorFillSource::None)) {
        colorFillSelector->setDisabled(true);
    }
    else {
        colorFillSelector->setDisabled(false);
        emit colorFillSelectorChanged();
    }
    m_options._colorFillSource = index;
}

void WdgStrokeSelection::on_typeBox_currentIndexChanged(const QString &arg1)
{
  if (arg1 == "Current Brush") {
      m_options.brushSelected = true;
      lineSize->setDisabled(true);
      fillBox->setDisabled(true);
      colorFillSelector->setDisabled(true);
      sizeBox->setDisabled(true);
  }
  else {
      m_options.brushSelected = false;
      lineSize->setDisabled(false);
      fillBox->setDisabled(false);
      colorFillSelector->setDisabled(false);
      sizeBox->setDisabled(false);
      }
}

void WdgStrokeSelection::on_lineColorBox_currentIndexChanged(const QString &/*arg1*/)
{
    emit colorSelectorChanged();
}


StrokeSelectionOptions ::StrokeSelectionOptions():
    lineSize(1),
    brushSelected(false),
    _colorFillSource(0),
    lineColorSource(0),
    lineDimension(0)
{
    color.fromQColor(Qt::black);
    fillColor.fromQColor(Qt::black);
    customColor.fromQColor(Qt::black);
}

KisPainter::FillStyle StrokeSelectionOptions::fillStyle() const
{
    colorFillSource tempColor = static_cast<colorFillSource>(_colorFillSource);
    KisPainter::FillStyle style;

    switch (tempColor) {
    case colorFillSource::PaintColor:
        style = KisPainter::FillStyleForegroundColor;
        break;
    case colorFillSource::BGColor:
        style = KisPainter::FillStyleBackgroundColor;
        break;
    case colorFillSource::CustomColor:
        style = KisPainter::FillStyleBackgroundColor;
        break;
    case colorFillSource::None:
        style = KisPainter::FillStyleNone;
        break;
    case colorFillSource::FGColor:
        style = KisPainter::FillStyleBackgroundColor;
        break;
    default:
        style = KisPainter::FillStyleBackgroundColor;
    }
    return style;
}


