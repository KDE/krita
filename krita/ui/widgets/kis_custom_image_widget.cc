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
#include <QFile>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QSpacerItem>

#include <QMessageBox>
#include <kcomponentdata.h>
#include <kstandarddirs.h>
#include <kglobal.h>

#include <kis_debug.h>

#include <KoIcon.h>
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
#include "widgets/squeezedcombobox.h"


KisCustomImageWidget::KisCustomImageWidget(QWidget* parent, qint32 defWidth, qint32 defHeight, double resolution, const QString& defColorModel, const QString& defColorDepth, const QString& defColorProfile, const QString& imageName)
    : WdgNewImage(parent)
{
    setObjectName("KisCustomImageWidget");

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
    connect(createButton, SIGNAL(clicked()), this, SLOT(createImage()));
    createButton->setDefault(true);

    bnPortrait->setIcon(koIcon("portrait"));
    connect(bnPortrait, SIGNAL(clicked()), SLOT(setPortrait()));
    connect(bnLandscape, SIGNAL(clicked()), SLOT(setLandscape()));
    bnLandscape->setIcon(koIcon("landscape"));

    connect(doubleWidth, SIGNAL(valueChanged(double)), this, SLOT(switchPortraitLandscape()));
    connect(doubleHeight, SIGNAL(valueChanged(double)), this, SLOT(switchPortraitLandscape()));
    connect(bnSaveAsPredefined, SIGNAL(clicked()), this, SLOT(saveAsPredefined()));

    colorSpaceSelector->setCurrentColorModel(KoID(defColorModel));
    colorSpaceSelector->setCurrentColorDepth(KoID(defColorDepth));
    colorSpaceSelector->setCurrentProfile(defColorProfile);

    //connect(chkFromClipboard,SIGNAL(stateChanged(int)),this,SLOT(clipboardDataChanged()));
    connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(clipboardDataChanged()));
    connect(QApplication::clipboard(), SIGNAL(selectionChanged()), this, SLOT(clipboardDataChanged()));
    connect(QApplication::clipboard(), SIGNAL(changed(QClipboard::Mode)), this, SLOT(clipboardDataChanged()));

    connect(bnScreenSize, SIGNAL(clicked()), this, SLOT(screenSizeClicked()));
    connect(colorSpaceSelector, SIGNAL(selectionChanged(bool)), createButton, SLOT(setEnabled(bool)));

    KisConfig cfg;
    intNumLayers->setValue(cfg.numDefaultLayers());
    cmbColor->setColor(cfg.defaultBackgroundColor());
    setBackgroundOpacity(cfg.defaultBackgroundOpacity());
    
    KisConfig::BackgroundStyle bgStyle = cfg.defaultBackgroundStyle();
    
    if (bgStyle == KisConfig::LAYER) {
      radioBackgroundAsLayer->setChecked(true);
    } else {
      radioBackgroundAsProjection->setChecked(true);
    }
    
    fillPredefined();
    switchPortraitLandscape();
}

void KisCustomImageWidget::showEvent(QShowEvent *)
{
    fillPredefined();
    this->createButton->setFocus();
}

KisCustomImageWidget::~KisCustomImageWidget()
{
    qDeleteAll(m_predefined);
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

    m_heightUnit = KoUnit::fromListForUi(index, KoUnit::ListAll);
    if (m_heightUnit.type() == KoUnit::Pixel) {
        doubleHeight->setDecimals(0);
        m_heightUnit.setFactor(doubleResolution->value() / 72.0);
    } else {
        doubleHeight->setDecimals(2);
    }

    doubleHeight->setValue(KoUnit::ptToUnit(m_height, m_heightUnit));

    doubleHeight->blockSignals(false);
}

void KisCustomImageWidget::heightChanged(double value)
{
    m_height = m_heightUnit.fromUserValue(value);
}

void KisCustomImageWidget::createImage()
{
    KisDocument *doc = createNewImage();
    if (doc) {
        emit documentSelected(doc);
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
                                          "Press \"Continue\" to create a 8-bit integer linear RGB color space "
                                          "or \"Cancel\" to return to the settings dialog."),
                                     QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);

            if (result == QMessageBox::Cancel) {
                qDebug() << "Model RGB8" << "NOT SUPPORTED";
                qDebug() << ppVar(cs->name());
                qDebug() << ppVar(cs->profile()->name());
                qDebug() << ppVar(cs->profile()->info());
                return 0;
            }
        }
    }
    KisDocument *doc = static_cast<KisDocument*>(KisPart::instance()->createDocument());

    qint32 width, height;
    double resolution;
    resolution = doubleResolution->value() / 72.0;  // internal resolution is in pixels per pt

    width = static_cast<qint32>(0.5  + KoUnit::ptToUnit(m_width, KoUnit(KoUnit::Pixel, resolution)));
    height = static_cast<qint32>(0.5 + KoUnit::ptToUnit(m_height, KoUnit(KoUnit::Pixel, resolution)));

    QColor qc = cmbColor->color();
    qc.setAlpha(backgroundOpacity());
    KoColor bgColor(qc, cs);

    bool backgroundAsLayer = radioBackgroundAsLayer->isChecked();

    doc->newImage(txtName->text(), width, height, cs, bgColor, backgroundAsLayer, intNumLayers->value(), txtDescription->toPlainText(), resolution);

    KisConfig cfg;
    cfg.setNumDefaultLayers(intNumLayers->value());
    cfg.setDefaultBackgroundOpacity(backgroundOpacity());
    cfg.setDefaultBackgroundColor(cmbColor->color());
    cfg.setDefaultBackgroundStyle(backgroundAsLayer ? KisConfig::LAYER : KisConfig::PROJECTION);

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

void KisCustomImageWidget::screenSizeClicked()
{
    QSize sz = QApplication::desktop()->screenGeometry(this).size();

    const int index = KoUnit(KoUnit::Pixel).indexInListForUi(KoUnit::ListAll);
    cmbWidthUnit->setCurrentIndex(index);
    cmbHeightUnit->setCurrentIndex(index);
    widthUnitChanged(cmbWidthUnit->currentIndex());
    heightUnitChanged(cmbHeightUnit->currentIndex());

    doubleWidth->setValue(sz.width());
    doubleHeight->setValue(sz.height());
}

void KisCustomImageWidget::fillPredefined()
{
    cmbPredefined->clear();

    cmbPredefined->addItem("");

    QString appName = KGlobal::mainComponent().componentName();
    QStringList definitions = KGlobal::dirs()->findAllResources("data", appName + "/predefined_image_sizes/*", KStandardDirs::Recursive);
    definitions.sort();

    if (!definitions.empty()) {

        foreach(const QString &definition, definitions) {
            QFile f(definition);
            f.open(QIODevice::ReadOnly);
            if (f.exists()) {
                QString xml = QString::fromUtf8(f.readAll());
                KisPropertiesConfiguration *predefined = new KisPropertiesConfiguration;
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

    KisPropertiesConfiguration *predefined = m_predefined[index - 1];
    txtPredefinedName->setText(predefined->getString("name"));
    doubleResolution->setValue(predefined->getDouble("resolution"));
    cmbWidthUnit->setCurrentIndex(predefined->getInt("x-unit"));
    cmbHeightUnit->setCurrentIndex(predefined->getInt("y-unit"));

    widthUnitChanged(cmbWidthUnit->currentIndex());
    heightUnitChanged(cmbHeightUnit->currentIndex());

    doubleWidth->setValue(predefined->getDouble("width"));
    doubleHeight->setValue(predefined->getDouble("height"));

}

void KisCustomImageWidget::saveAsPredefined()
{
    QString fileName = txtPredefinedName->text();
    if (fileName.isEmpty()) {
        return;
    }
    QString saveLocation = KGlobal::mainComponent().dirs()->saveLocation("data");
    QString appName = KGlobal::mainComponent().componentName();

    QDir d;
    d.mkpath(saveLocation + appName + "/predefined_image_sizes/");

    QFile f(saveLocation + appName + "/predefined_image_sizes/" + fileName.replace(' ', '_').replace('(', '_').replace(')', '_') + ".predefinedimage");

    f.open(QIODevice::WriteOnly | QIODevice::Truncate);
    KisPropertiesConfiguration *predefined = new KisPropertiesConfiguration();
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
    foreach(KisPropertiesConfiguration *pr, m_predefined) {
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
}

void KisCustomImageWidget::switchPortraitLandscape()
{
    if(doubleWidth->value() > doubleHeight->value())
        bnLandscape->setChecked(true);
    else
        bnPortrait->setChecked(true);
}

#include "kis_custom_image_widget.moc"
