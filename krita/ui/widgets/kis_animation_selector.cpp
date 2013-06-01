/*
 *  Copyright (c) 2013 Somsubhra Bairi <somsubhra.bairi@gmail.com>
 *  Copyright (C) 2005 Thomas Zander <zander@kde.org>
 *  Copyright (C) 2005 C. Boemann <cbo@boemann.dk>
 *  Copyright (C) 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
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
#include <kcolorcombo.h>
#include <kcomponentdata.h>
#include <kfiledialog.h>
#include <kstandarddirs.h>
#include <kglobal.h>

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

#include <kis_fill_painter.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_group_layer.h>
#include <kis_paint_layer.h>
#include <kis_paint_device.h>
#include <kis_painter.h>

#include "kis_clipboard.h"
#include "kis_doc2.h"
#include "kis_part2.h"
#include "widgets/kis_cmb_idlist.h"
#include "widgets/squeezedcombobox.h"
#include "kis_animation.h"
#include <stdlib.h>

KisAnimationSelector::KisAnimationSelector(QWidget *parent, KisDoc2 *document, qint32 defWidth, qint32 defHeight, double resolution, const QString &defColorModel, const QString &defColorDepth, const QString &defColorProfile, const QString &animationName) : WdgAnimationSelector(parent){
    setObjectName("KisAnimationSelector");
    m_document = document;
    txtAnimationName->setText(animationName);

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
    inputFps->setValue(12);

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
}

KisAnimationSelector::~KisAnimationSelector(){
    qDeleteAll(m_predefined);
    m_predefined.clear();
}

void KisAnimationSelector::createAnimation(){
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
    static_cast<KisPart2*>(m_document->documentPart())->setAnimation(animation);

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

void KisAnimationSelector::resolutionChanged(double value){
    if(m_widthUnit.type() == KoUnit::Pixel){
        m_widthUnit.setFactor(value / 72.0);
        m_width = m_widthUnit.fromUserValue(inputWidth->value());
    }

    if(m_heightUnit.type() == KoUnit::Pixel){
        m_heightUnit.setFactor(value / 72.0);
        m_height = m_heightUnit.fromUserValue(inputHeight->value());
    }
}

void KisAnimationSelector::widthUnitChanged(int index){
    inputWidth->blockSignals(true);
    m_widthUnit = KoUnit::fromListForUi(index, KoUnit::ListAll);
    if(m_widthUnit.type() == KoUnit::Pixel){
        inputWidth->setDecimals(0);
        m_widthUnit.setFactor(inputResolution->value() / 72.0);
    }
    else{
        inputWidth->setDecimals(2);
    }

    inputWidth->setValue(KoUnit::ptToUnit(m_width, m_widthUnit));
    inputWidth->blockSignals(false);
}

void KisAnimationSelector::heightUnitChanged(int index){
    inputHeight->blockSignals(true);

    m_heightUnit = KoUnit::fromListForUi(index, KoUnit::ListAll);
    if(m_heightUnit.type() == KoUnit::Pixel){
        inputHeight->setDecimals(0);
        m_heightUnit.setFactor(inputResolution->value() / 72.0);
    }
    else{
        inputHeight->setDecimals(2);
    }

    inputHeight->setValue(KoUnit::ptToUnit(m_height, m_heightUnit));
    inputHeight->blockSignals(false);
}

void KisAnimationSelector::heightChanged(double value){
    m_height = m_heightUnit.fromUserValue(value);
}

void KisAnimationSelector::widthChanged(double value){
    m_width = m_widthUnit.fromUserValue(value);
}

quint8 KisAnimationSelector::backgroundOpacity(){
    qint32 opacity = inputOpacity->value();

    if(!opacity){
        return 0;
    }

    return (opacity * 255) / 100;
}
