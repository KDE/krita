/*
 *  Copyright (c) 2013 Somsubhra Bairi <somsubhra.bairi@gmail.com>
 *  Copyright (C) 2005 Thomas Zander <zander@kde.org>
 *  Copyright (C) 2005 C. Boemann <cbo@boemann.dk>
 *  Copyright (C) 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License (or at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_animation_selector.h"

#include <QMimeData>
#include <QPushButton>
#include <QSlider>
#include <QComboBox>
#include <QRect>
#include <QApplication>
#include <QClipboard>
#include <QDesktopWidget>
#include <kundo2command.h>
#include <QFile>
#include <QFileDialog>
#include <kcomponentdata.h>

#include <kis_debug.h>

#include <KoIcon.h>
#include <KoCompositeOp.h>
#include <KoUnitDoubleSpinBox.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorProfile.h>
#include <KoColorSpace.h>
#include <KoID.h>
#include <KoColor.h>
#include <KoUnit.h>
#include <KoColorModelStandardIds.h>
#include <KMessageBox>

#include <kis_fill_painter.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_group_layer.h>
#include <kis_paint_layer.h>
#include <kis_paint_device.h>
#include <kis_painter.h>

#include "kis_clipboard.h"
#include "kis_animation_doc.h"
#include "kis_animation_part.h"
#include "widgets/kis_cmb_idlist.h"
#include "widgets/squeezedcombobox.h"
#include "kis_animation.h"
#include <stdlib.h>
#include <QDesktopServices>
#include <kis_config.h>

KisAnimationSelector::KisAnimationSelector(QWidget *parent, KisAnimationDoc *document, qint32 defWidth, qint32 defHeight, double resolution, const QString &defColorModel, const QString &defColorDepth, const QString &defColorProfile, const QString &animationName)
    : WdgAnimationSelector(parent)
{
    setObjectName("KisAnimationSelector");
    m_document = document;
    txtAnimationName->setText(animationName);

    txtLocation->setText(QDesktopServices::storageLocation(QDesktopServices::PicturesLocation));

    m_widthUnit = KoUnit(KoUnit::Pixel, resolution);
    inputWidth->setValue(defWidth);
    inputWidth->setDecimals(0);
    m_width = m_widthUnit.fromUserValue(defWidth);
    inputWidthUnit->addItems(KoUnit::listOfUnitNameForUi(KoUnit::ListAll));
    inputWidthUnit->setCurrentIndex(m_widthUnit.indexInListForUi(KoUnit::ListAll));

    m_heightUnit = KoUnit(KoUnit::Pixel, resolution);
    inputHeight->setValue(defHeight);
    inputHeight->setDecimals(0);
    m_height = m_heightUnit.fromUserValue(defHeight);
    inputHeightUnit->addItems(KoUnit::listOfUnitNameForUi(KoUnit::ListAll));
    inputHeightUnit->setCurrentIndex(m_heightUnit.indexInListForUi(KoUnit::ListAll));

    inputResolution->setValue(72.0 * resolution);
    inputResolution->setDecimals(0);

    txtAuthor->setText(getenv("USER"));

    KisConfig cfg;
    inputFps->setValue(cfg.defFps());

    inputTime->setRange(0,999999);
    inputTime->setValue(100);

    colorSpaceSelector->setCurrentColorModel(KoID(defColorModel));
    colorSpaceSelector->setCurrentColorDepth(KoID(defColorDepth));
    colorSpaceSelector->setCurrentProfile(defColorProfile);

    QColor color("White");
    inputBackground->setColor(color);

    connect(inputResolution, SIGNAL(valueChanged(double)), this, SLOT(resolutionChanged(double)));
    connect(inputHeightUnit, SIGNAL(activated(int)), this, SLOT(heightUnitChanged(int)));
    connect(inputWidthUnit, SIGNAL(activated(int)), this, SLOT(widthUnitChanged(int)));
    connect(inputHeight, SIGNAL(valueChanged(double)), this, SLOT(heightChanged(double)));
    connect(inputWidth, SIGNAL(valueChanged(double)), this, SLOT(widthChanged(double)));
    connect(bnCreateAnimation, SIGNAL(clicked()), this, SLOT(createAnimation()));
    connect(locationButton, SIGNAL(clicked()), this, SLOT(changeLocation()));
    connect(btnSelectFile, SIGNAL(clicked()), this, SLOT(selectFile()));
    connect(btnOpen, SIGNAL(clicked()), this, SLOT(openAnimation()));
}

KisAnimationSelector::~KisAnimationSelector()
{
    qDeleteAll(m_predefined);
    m_predefined.clear();
}

void KisAnimationSelector::changeLocation()
{
    QFileDialog folderSelector;
    folderSelector.setFileMode(QFileDialog::Directory);
    folderSelector.setOption(QFileDialog::ShowDirsOnly);
    folderSelector.setDirectory(txtLocation->text());

    QString location = folderSelector.getExistingDirectory();

    txtLocation->setText(location);
}

void KisAnimationSelector::selectFile()
{
    QFileDialog fileSelector;
    QString location = fileSelector.getExistingDirectory();
    txtOpenFile->setText(location);
}

void KisAnimationSelector::openAnimation()
{
    if(!txtOpenFile->text().length()) {
        KMessageBox::error(0, "No file name specified.", "No file name specified.");
        return;
    }

    KisAnimationStore* store = new KisAnimationStore(txtOpenFile->text());

    if(!store->hasFile("maindoc.xml")) {
        KMessageBox::error(0, "This file does not appear to be a valid animation file", "Not a valid animation file");
        return;
    }

    store->openFileReading("maindoc.xml");
    QByteArray xmlData = store->getDevice("maindoc.xml")->readAll();

    QDomDocument doc;
    doc.setContent(xmlData);

    QDomNode metaData = doc.elementsByTagName("metadata").at(0);
    QDomNamedNodeMap attributes = metaData.attributes();

    KisAnimation* animation = new KisAnimation();
    animation->setName(attributes.namedItem("name").nodeValue());
    animation->setAuthor(attributes.namedItem("author").nodeValue());
    animation->setDescription(attributes.namedItem("des").nodeValue());
    animation->setFps(attributes.namedItem("fps").nodeValue().toInt());
    animation->setColorSpace(colorSpaceSelector->currentColorSpace());
    animation->setWidth(attributes.namedItem("width").nodeValue().toInt());
    animation->setHeight(attributes.namedItem("height").nodeValue().toInt());
    animation->setResolution(attributes.namedItem("res").nodeValue().toDouble());
    animation->setLocation(txtOpenFile->text());
    animation->setBgColor(KoColor(inputBackground->color(), colorSpaceSelector->currentColorSpace()));

    static_cast<KisAnimationPart*>(m_document->documentPart())->setAnimation(animation);

    //Load a temporary image before opening the animation file
    m_document->newImage(animation->name(), animation->width(), animation->height(), animation->colorSpace(),
                         animation->bgColor(), animation->description(), animation->resolution());

    KisImageWSP image = m_document->image();

    if(image && image->root() && image->root()->firstChild()) {
        KisLayer* layer = dynamic_cast<KisLayer*>(image->root()->firstChild().data());

        if(layer) {
            layer->setOpacity(OPACITY_OPAQUE_U8);
        }

        if(layer && backgroundOpacity() < OPACITY_OPAQUE_U8) {
            KisFillPainter painter;
            painter.begin(layer->paintDevice());
            painter.fillRect(0, 0, animation->width(), animation->height(), animation->bgColor(), backgroundOpacity());
        }
        layer->setDirty(QRect(0, 0, animation->width(), animation->height()));
    }
    emit documentSelected();

    dynamic_cast<KisAnimationDoc*>(m_document)->loadAnimationFile(animation, store, doc);
}

void KisAnimationSelector::createAnimation()
{
    const KoColorSpace* cs = colorSpaceSelector->currentColorSpace();
    QColor qc = inputBackground->color();
    qint32 width, height;
    double resolution;
    resolution = inputResolution->value() / 72.0;
    width = static_cast<qint32>(0.5  + KoUnit::ptToUnit(m_width, KoUnit(KoUnit::Pixel, resolution)));
    height = static_cast<qint32>(0.5 + KoUnit::ptToUnit(m_height, KoUnit(KoUnit::Pixel, resolution)));
    qc.setAlpha(backgroundOpacity());
    KoColor bgColor(qc, cs);

    KisAnimation* animation = new KisAnimation();

    animation->setName(txtAnimationName->text());
    animation->setAuthor(txtAuthor->text());
    animation->setDescription(txtDescription->text());
    animation->setFps(inputFps->value());
    animation->setTime(inputTime->value());
    animation->setColorSpace(colorSpaceSelector->currentColorSpace());
    animation->setWidth(width);
    animation->setHeight(height);
    animation->setResolution(resolution);
    animation->setBgColor(bgColor);

    QString animationLocation = txtLocation->text();
    QDir* directory = new QDir(animationLocation);

    if(!directory->exists()) {
        directory->mkdir(animationLocation);
    }

    animation->setLocation(animationLocation);

    static_cast<KisAnimationPart*>(m_document->documentPart())->setAnimation(animation); 

    m_document->newImage(txtAnimationName->text(), width, height, cs, bgColor, txtDescription->text(), resolution);

    KisImageWSP image = m_document->image();
    if (image && image->root() && image->root()->firstChild()) {
        KisLayer * layer = dynamic_cast<KisLayer*>(image->root()->firstChild().data());
        if (layer) {
            layer->setOpacity(OPACITY_OPAQUE_U8);
        }
        // Hack: with a semi-transparent background color, the projection isn't composited right if we just set the default pixel
        if (layer && backgroundOpacity() < OPACITY_OPAQUE_U8) {
            KisFillPainter painter;
            painter.begin(layer->paintDevice());
            painter.fillRect(0, 0, width, height, bgColor, backgroundOpacity());

        }

        layer->setDirty(QRect(0, 0, width, height));
    }

    emit documentSelected();
}

void KisAnimationSelector::resolutionChanged(double value)
{
    if(m_widthUnit.type() == KoUnit::Pixel) {
        m_widthUnit.setFactor(value / 72.0);
        m_width = m_widthUnit.fromUserValue(inputWidth->value());
    }

    if(m_heightUnit.type() == KoUnit::Pixel) {
        m_heightUnit.setFactor(value / 72.0);
        m_height = m_heightUnit.fromUserValue(inputHeight->value());
    }
}

void KisAnimationSelector::widthUnitChanged(int index)
{
    inputWidth->blockSignals(true);
    m_widthUnit = KoUnit::fromListForUi(index, KoUnit::ListAll);
    if(m_widthUnit.type() == KoUnit::Pixel) {
        inputWidth->setDecimals(0);
        m_widthUnit.setFactor(inputResolution->value() / 72.0);
    }
    else {
        inputWidth->setDecimals(2);
    }

    inputWidth->setValue(KoUnit::ptToUnit(m_width, m_widthUnit));
    inputWidth->blockSignals(false);
}

void KisAnimationSelector::heightUnitChanged(int index)
{
    inputHeight->blockSignals(true);

    m_heightUnit = KoUnit::fromListForUi(index, KoUnit::ListAll);
    if(m_heightUnit.type() == KoUnit::Pixel) {
        inputHeight->setDecimals(0);
        m_heightUnit.setFactor(inputResolution->value() / 72.0);
    }
    else {
        inputHeight->setDecimals(2);
    }

    inputHeight->setValue(KoUnit::ptToUnit(m_height, m_heightUnit));
    inputHeight->blockSignals(false);
}

void KisAnimationSelector::heightChanged(double value)
{
    m_height = m_heightUnit.fromUserValue(value);
}

void KisAnimationSelector::widthChanged(double value)
{
    m_width = m_widthUnit.fromUserValue(value);
}

quint8 KisAnimationSelector::backgroundOpacity()
{
    qint32 opacity = inputOpacity->value();

    if(!opacity) {
        return 0;
    }

    return (opacity * 255) / 100;
}
