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

#include "widgets/kis_custom_image_widget.h"

#include <QMimeData>
#include <QPushButton>
#include <QSlider>
#include <QComboBox>
#include <QRect>
#include <QApplication>
#include <QClipboard>
#include <QDesktopWidget>
#include <QDialogButtonBox>
#include <QFile>
#include <QSpacerItem>

#include <QMessageBox>
#include <KoResourcePaths.h>

#include <KFormat>

#include <kis_debug.h>

#include <kis_icon.h>
#include <KoCompositeOp.h>
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

#include "kis_config.h"
#include "KisPart.h"
#include "kis_clipboard.h"
#include "KisDocument.h"
#include "widgets/kis_cmb_idlist.h"
#include <KisSqueezedComboBox.h>


KisCustomImageWidget::KisCustomImageWidget(QWidget* parent, qint32 defWidth, qint32 defHeight, double resolution, const QString& defColorModel, const QString& defColorDepth, const QString& defColorProfile, const QString& imageName)
    : WdgNewImage(parent)
{
    setObjectName("KisCustomImageWidget");
    m_openPane = qobject_cast<KisOpenPane*>(parent);
    Q_ASSERT(m_openPane);

    txtName->setText(imageName);
    m_widthUnit = KoUnit(KoUnit::Pixel, resolution);
    doubleWidth->setValue(defWidth);
    doubleWidth->setDecimals(0);
    m_width = m_widthUnit.fromUserValue(defWidth);
    cmbWidthUnit->addItems(KoUnit::listOfUnitNameForUi(KoUnit::ListAll));
    cmbWidthUnit->setCurrentIndex(m_widthUnit.indexInListForUi(KoUnit::ListAll));

    m_heightUnit = KoUnit(KoUnit::Pixel, resolution);
    doubleHeight->setValue(defHeight);
    doubleHeight->setDecimals(0);
    m_height = m_heightUnit.fromUserValue(defHeight);
    cmbHeightUnit->addItems(KoUnit::listOfUnitNameForUi(KoUnit::ListAll));
    cmbHeightUnit->setCurrentIndex(m_heightUnit.indexInListForUi(KoUnit::ListAll));

    doubleResolution->setValue(72.0 * resolution);
    doubleResolution->setDecimals(0);

    imageGroupSpacer->changeSize(0, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
    grpClipboard->hide();

    sliderOpacity->setRange(0, 100, 0);
    sliderOpacity->setValue(100);
    sliderOpacity->setSuffix("%");

    connect(cmbPredefined, SIGNAL(activated(int)), SLOT(predefinedClicked(int)));
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


    // Create image
    newDialogConfirmationButtonBox->button(QDialogButtonBox::Ok)->setText(i18n("&Create"));
    connect(newDialogConfirmationButtonBox, SIGNAL(accepted()), this, SLOT(createImage()));


    // Cancel Create image button
    connect(newDialogConfirmationButtonBox, SIGNAL(rejected()), this->parentWidget(), SLOT(close()));
    connect(newDialogConfirmationButtonBox, SIGNAL(rejected()), this->parentWidget(), SLOT(deleteLater()));



    bnPortrait->setIcon(KisIconUtils::loadIcon("portrait"));
    connect(bnPortrait, SIGNAL(clicked()), SLOT(setPortrait()));
    connect(bnLandscape, SIGNAL(clicked()), SLOT(setLandscape()));
    bnLandscape->setIcon(KisIconUtils::loadIcon("landscape"));

    connect(doubleWidth, SIGNAL(valueChanged(double)), this, SLOT(switchPortraitLandscape()));
    connect(doubleHeight, SIGNAL(valueChanged(double)), this, SLOT(switchPortraitLandscape()));
    connect(bnSaveAsPredefined, SIGNAL(clicked()), this, SLOT(saveAsPredefined()));

    colorSpaceSelector->setCurrentColorModel(KoID(defColorModel));
    colorSpaceSelector->setCurrentColorDepth(KoID(defColorDepth));
    colorSpaceSelector->setCurrentProfile(defColorProfile);
    connect(colorSpaceSelector, SIGNAL(colorSpaceChanged(const KoColorSpace*)), this, SLOT(changeDocumentInfoLabel()));

    //connect(chkFromClipboard,SIGNAL(stateChanged(int)),this,SLOT(clipboardDataChanged()));
    connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(clipboardDataChanged()));
    connect(QApplication::clipboard(), SIGNAL(selectionChanged()), this, SLOT(clipboardDataChanged()));
    connect(QApplication::clipboard(), SIGNAL(changed(QClipboard::Mode)), this, SLOT(clipboardDataChanged()));

    connect(colorSpaceSelector, SIGNAL(selectionChanged(bool)), newDialogConfirmationButtonBox->button(QDialogButtonBox::Ok), SLOT(setEnabled(bool)));


    KisConfig cfg(true);
    intNumLayers->setValue(cfg.numDefaultLayers());
    KoColor bcol(KoColorSpaceRegistry::instance()->rgb8());
    bcol.fromQColor(cfg.defaultBackgroundColor());
    cmbColor->setColor(bcol);
    setBackgroundOpacity(cfg.defaultBackgroundOpacity());

    KisConfig::BackgroundStyle bgStyle = cfg.defaultBackgroundStyle();

    if (bgStyle == KisConfig::RASTER_LAYER) {
      radioBackgroundAsRaster->setChecked(true);
    } else if (bgStyle == KisConfig::FILL_LAYER) {
      radioBackgroundAsFill->setChecked(true);
    } else {
      radioBackgroundAsProjection->setChecked(true);
    }

    fillPredefined();
    switchPortraitLandscape();

    // this makes the portrait and landscape buttons more
    // obvious what is selected by changing the highlight color
    QPalette p = QApplication::palette();
    QPalette palette_highlight(p );
    QColor c = p.color(QPalette::Highlight);
    palette_highlight.setColor(QPalette::Button, c);
    bnLandscape->setPalette(palette_highlight);
    bnPortrait->setPalette(palette_highlight);
    changeDocumentInfoLabel();
}

void KisCustomImageWidget::showEvent(QShowEvent *)
{
    fillPredefined();
    newDialogConfirmationButtonBox->button(QDialogButtonBox::Ok)->setFocus();
    newDialogConfirmationButtonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}

KisCustomImageWidget::~KisCustomImageWidget()
{
    m_predefined.clear();
}

void KisCustomImageWidget::resolutionChanged(double res)
{
    if (m_widthUnit.type() == KoUnit::Pixel) {
        m_widthUnit.setFactor(res / 72.0);
        m_width = m_widthUnit.fromUserValue(doubleWidth->value());
    }

    if (m_heightUnit.type() == KoUnit::Pixel) {
        m_heightUnit.setFactor(res / 72.0);
        m_height = m_heightUnit.fromUserValue(doubleHeight->value());
    }
    changeDocumentInfoLabel();
}


void KisCustomImageWidget::widthUnitChanged(int index)
{
    doubleWidth->blockSignals(true);

    m_widthUnit = KoUnit::fromListForUi(index, KoUnit::ListAll);
    if (m_widthUnit.type() == KoUnit::Pixel) {
        doubleWidth->setDecimals(0);
        m_widthUnit.setFactor(doubleResolution->value() / 72.0);
    } else {
        doubleWidth->setDecimals(2);
    }

    doubleWidth->setValue(m_widthUnit.toUserValuePrecise(m_width));

    doubleWidth->blockSignals(false);
    changeDocumentInfoLabel();
}

void KisCustomImageWidget::widthChanged(double value)
{
    m_width = m_widthUnit.fromUserValue(value);
    changeDocumentInfoLabel();
}

void KisCustomImageWidget::heightUnitChanged(int index)
{
    doubleHeight->blockSignals(true);

    m_heightUnit = KoUnit::fromListForUi(index, KoUnit::ListAll);
    if (m_heightUnit.type() == KoUnit::Pixel) {
        doubleHeight->setDecimals(0);
        m_heightUnit.setFactor(doubleResolution->value() / 72.0);
    } else {
        doubleHeight->setDecimals(2);
    }

    doubleHeight->setValue(m_heightUnit.toUserValuePrecise(m_height));

    doubleHeight->blockSignals(false);
    changeDocumentInfoLabel();
}

void KisCustomImageWidget::heightChanged(double value)
{
    m_height = m_heightUnit.fromUserValue(value);
    changeDocumentInfoLabel();
}

void KisCustomImageWidget::createImage()
{
    newDialogConfirmationButtonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    KisDocument *doc = createNewImage();
    if (doc) {
        doc->setModified(false);
        emit m_openPane->documentSelected(doc);
    }
}

KisDocument* KisCustomImageWidget::createNewImage()
{
    const KoColorSpace * cs = colorSpaceSelector->currentColorSpace();

    if (cs->colorModelId() == RGBAColorModelID &&
        cs->colorDepthId() == Integer8BitsColorDepthID) {

        const KoColorProfile *profile = cs->profile();

        if (profile->name().contains("linear") ||
            profile->name().contains("scRGB") ||
            profile->info().contains("linear") ||
            profile->info().contains("scRGB")) {

            int result =
                QMessageBox::warning(this,
                                     i18nc("@title:window", "Krita"),
                                     i18n("Linear gamma RGB color spaces are not supposed to be used "
                                          "in 8-bit integer modes. It is suggested to use 16-bit integer "
                                          "or any floating point colorspace for linear profiles.\n\n"
                                          "Press \"Ok\" to create a 8-bit integer linear RGB color space "
                                          "or \"Cancel\" to return to the settings dialog."),
                                     QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);

            if (result == QMessageBox::Cancel) {
                dbgKrita << "Model RGB8" << "NOT SUPPORTED";
                dbgKrita << ppVar(cs->name());
                dbgKrita << ppVar(cs->profile()->name());
                dbgKrita << ppVar(cs->profile()->info());
                return 0;
            }
        }
    }
    KisDocument *doc = static_cast<KisDocument*>(KisPart::instance()->createDocument());

    qint32 width, height;
    double resolution;
    resolution = doubleResolution->value() / 72.0;  // internal resolution is in pixels per pt

    width = static_cast<qint32>(0.5  + KoUnit(KoUnit::Pixel, resolution).toUserValuePrecise(m_width));
    height = static_cast<qint32>(0.5 + KoUnit(KoUnit::Pixel, resolution).toUserValuePrecise(m_height));

    QColor qc = cmbColor->color().toQColor();
    qc.setAlpha(backgroundOpacity());
    KoColor bgColor(qc, cs);

    KisConfig::BackgroundStyle bgStyle = KisConfig::CANVAS_COLOR;
    if( radioBackgroundAsRaster->isChecked() ){
        bgStyle = KisConfig::RASTER_LAYER;
    } else if( radioBackgroundAsFill->isChecked() ){
        bgStyle = KisConfig::FILL_LAYER;
    }

    doc->newImage(txtName->text(), width, height, cs, bgColor, bgStyle, intNumLayers->value(), txtDescription->toPlainText(), resolution);

    KisConfig cfg(true);
    cfg.setNumDefaultLayers(intNumLayers->value());
    cfg.setDefaultBackgroundOpacity(backgroundOpacity());
    cfg.setDefaultBackgroundColor(cmbColor->color().toQColor());
    cfg.setDefaultBackgroundStyle(bgStyle);

    return doc;
}

void KisCustomImageWidget::setNumberOfLayers(int layers)
{
    intNumLayers->setValue(layers);
}

quint8 KisCustomImageWidget::backgroundOpacity() const
{
    qint32 opacity = sliderOpacity->value();

    if (!opacity)
        return 0;

    return (opacity * 255) / 100;
}

void KisCustomImageWidget::setBackgroundOpacity(quint8 value) {
  sliderOpacity->setValue((value * 100) / 255);
}

void KisCustomImageWidget::clipboardDataChanged()
{
}

void KisCustomImageWidget::fillPredefined()
{
    cmbPredefined->clear();
    m_predefined.clear();

    cmbPredefined->addItem("");

    QStringList definitions = KoResourcePaths::findAllResources("data", "predefined_image_sizes/*.predefinedimage", KoResourcePaths::Recursive);
    definitions.sort();

    if (!definitions.empty()) {

        Q_FOREACH (const QString &definition, definitions) {
            QFile f(definition);
            f.open(QIODevice::ReadOnly);
            if (f.exists()) {
                QString xml = QString::fromUtf8(f.readAll());
                KisPropertiesConfigurationSP predefined = new KisPropertiesConfiguration;
                predefined->fromXML(xml);
                if (predefined->hasProperty("name")
                        && predefined->hasProperty("width")
                        && predefined->hasProperty("height")
                        && predefined->hasProperty("resolution")
                        && predefined->hasProperty("x-unit")
                        && predefined->hasProperty("y-unit")) {
                    m_predefined << predefined;
                    cmbPredefined->addItem(predefined->getString("name"));
                }
            }
        }
    }

    cmbPredefined->setCurrentIndex(0);

}


void KisCustomImageWidget::predefinedClicked(int index)
{
    if (index < 1 || index > m_predefined.size()) return;

    KisPropertiesConfigurationSP predefined = m_predefined[index - 1];
    txtPredefinedName->setText(predefined->getString("name"));
    doubleResolution->setValue(predefined->getDouble("resolution"));
    cmbWidthUnit->setCurrentIndex(predefined->getInt("x-unit"));
    cmbHeightUnit->setCurrentIndex(predefined->getInt("y-unit"));

    widthUnitChanged(cmbWidthUnit->currentIndex());
    heightUnitChanged(cmbHeightUnit->currentIndex());

    doubleWidth->setValue(predefined->getDouble("width"));
    doubleHeight->setValue(predefined->getDouble("height"));
    changeDocumentInfoLabel();

}

void KisCustomImageWidget::saveAsPredefined()
{
    QString fileName = txtPredefinedName->text();
    if (fileName.isEmpty()) {
        return;
    }
    QString saveLocation = KoResourcePaths::saveLocation("data", "predefined_image_sizes/", true);
    QFile f(saveLocation + '/' + fileName.replace(' ', '_').replace('(', '_').replace(')', '_').replace(':', '_') + ".predefinedimage");

    f.open(QIODevice::WriteOnly | QIODevice::Truncate);
    KisPropertiesConfigurationSP predefined = new KisPropertiesConfiguration();
    predefined->setProperty("name", txtPredefinedName->text());
    predefined->setProperty("width", doubleWidth->value());
    predefined->setProperty("height", doubleHeight->value());
    predefined->setProperty("resolution", doubleResolution->value());
    predefined->setProperty("x-unit", cmbWidthUnit->currentIndex());
    predefined->setProperty("y-unit", cmbHeightUnit->currentIndex());

    QString xml = predefined->toXML();

    f.write(xml.toUtf8());
    f.flush();
    f.close();

    int i = 0;
    bool found = false;
    Q_FOREACH (KisPropertiesConfigurationSP pr, m_predefined) {
        if (pr->getString("name") == txtPredefinedName->text()) {
            found = true;
            break;
        }
        ++i;
    }
    if (found) {
        m_predefined[i] = predefined;
    }
    else {
        m_predefined.append(predefined);
        cmbPredefined->addItem(txtPredefinedName->text());
    }

}

void KisCustomImageWidget::setLandscape()
{
    if (doubleWidth->value() < doubleHeight->value()) {
        switchWidthHeight();
    }
}

void KisCustomImageWidget::setPortrait()
{
    if (doubleWidth->value() > doubleHeight->value()) {
        switchWidthHeight();
    }
}

void KisCustomImageWidget::switchWidthHeight()
{
    double width = doubleWidth->value();
    double height = doubleHeight->value();

    doubleHeight->blockSignals(true);
    doubleWidth->blockSignals(true);
    cmbWidthUnit->blockSignals(true);
    cmbHeightUnit->blockSignals(true);

    doubleWidth->setValue(height);
    doubleHeight->setValue(width);
    cmbWidthUnit->setCurrentIndex(m_heightUnit.indexInListForUi(KoUnit::ListAll));
    cmbHeightUnit->setCurrentIndex(m_widthUnit.indexInListForUi(KoUnit::ListAll));

    doubleHeight->blockSignals(false);
    doubleWidth->blockSignals(false);
    cmbWidthUnit->blockSignals(false);
    cmbHeightUnit->blockSignals(false);
    switchPortraitLandscape();

    widthChanged(doubleWidth->value());
    heightChanged(doubleHeight->value());
    changeDocumentInfoLabel();
}

void KisCustomImageWidget::switchPortraitLandscape()
{
    if(doubleWidth->value() > doubleHeight->value())
        bnLandscape->setChecked(true);
    else
        bnPortrait->setChecked(true);
}

void KisCustomImageWidget::changeDocumentInfoLabel()
{

    qint64 width, height;
    double resolution;
    resolution = doubleResolution->value() / 72.0;  // internal resolution is in pixels per pt

    width = static_cast<qint64>(0.5  + KoUnit(KoUnit::Pixel, resolution).toUserValuePrecise(m_width));
    height = static_cast<qint64>(0.5 + KoUnit(KoUnit::Pixel, resolution).toUserValuePrecise(m_height));

    qint64 layerSize = width * height;
    const KoColorSpace *cs = colorSpaceSelector->currentColorSpace();
    int bitSize = 8 * cs->pixelSize(); //pixelsize is in bytes.
    layerSize = layerSize * cs->pixelSize();
    QString text = i18nc("arg1: width. arg2: height. arg3: colorspace name. arg4: size of a channel in bits. arg5: image size",
                         "This document will be %1 pixels by %2 pixels in %3, which means the pixel size is %4 bit. A single paint layer will thus take up %5 of RAM.",
                         width,
                         height,
                         cs->name(),
                         bitSize,
                         KFormat().formatByteSize(layerSize));
    lblDocumentInfo->setText(text);
}

