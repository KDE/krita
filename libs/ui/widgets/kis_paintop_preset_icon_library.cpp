/* This file is part of the KDE project
 * Copyright (C) 2017 Wolthera van Hövell tot Westerflier
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "kis_paintop_preset_icon_library.h"
#include <QImage>
#include <QStandardItem>
#include <QSlider>
#include <QPainter>
#include <QLabel>
#include <QListView>
#include <kis_icon.h>
#include <QDebug>
#include <KoResourcePaths.h>

KisPaintopPresetIconLibrary::KisPaintopPresetIconLibrary(QWidget *parent): QWidget(parent), ui(new Ui_wdgpreseticonlibrary)
{
    ui->setupUi(this);

    ui->sldHue->setRange(0.0,360.0,1);
    ui->sldHue->setSingleStep(1.0);
    ui->sldHue->setPrefix(i18n("Hue:"));

    ui->sldSat->setRange(-50.0,50.0,1);
    ui->sldSat->setSingleStep(1.0);
    ui->sldSat->setPrefix(i18n("Saturation:"));

    ui->sldLevels->setRange(-50.0,50.0,1);
    ui->sldLevels->setSingleStep(1.0);
    ui->sldLevels->setPrefix(i18n("Mid-gray level:"));


    m_baseModel = new QStandardItemModel();
    ui->vwBase->setModel(m_baseModel);

    m_optionalModel = new QStandardItemModel();
    ui->vwOptional->setModel(m_optionalModel);

    QStringList background_paths = KoResourcePaths::findAllResources("data", "preset_icons/*.png");
    if (background_paths.size()>0) {
        m_background.load(background_paths.at(0));
        m_background = m_background.scaled(200, 200);
    }
    ui->lblIconPreview->setPixmap(QPixmap::fromImage(m_background));


    //empty default:

    QImage empty = QImage(200, 200, QImage::Format_ARGB32);
    empty.fill(Qt::transparent);
    m_baseModel->appendRow(new QStandardItem(QIcon(QPixmap::fromImage(empty)), NULL));

    QStringList toolIcon_paths = KoResourcePaths::findAllResources("data", "preset_icons/tool_icons/*.png");
    for (int i=0; i<toolIcon_paths.size(); i++) {
        QImage pix;
        pix.load(toolIcon_paths.at(i));
        QStandardItem *image = new QStandardItem(QIcon(QPixmap::fromImage(pix)), NULL);
        m_baseModel->appendRow(image);
    }


    empty = QImage(40, 40, QImage::Format_ARGB32);
    empty.fill(Qt::transparent);
    m_optionalModel->appendRow(new QStandardItem(QIcon(QPixmap::fromImage(empty)), NULL));

    QStringList emblemIcon_paths = KoResourcePaths::findAllResources("data", "preset_icons/emblem_icons/*.png");
    for (int i=0; i<emblemIcon_paths.size(); i++) {
        QImage pix;
        pix.load(emblemIcon_paths.at(i));
        QStandardItem *image = new QStandardItem(QIcon(QPixmap::fromImage(pix)), NULL);
        m_optionalModel->appendRow(image);
    }

    connect(ui->vwBase->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(updateIcon()));
    connect(ui->vwOptional->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(updateIcon()));
    connect(ui->sldHue, SIGNAL(valueChanged(qreal)), this, SLOT(updateIcon()));
    connect(ui->sldSat, SIGNAL(valueChanged(qreal)), this, SLOT(updateIcon()));
    connect(ui->sldLevels, SIGNAL(valueChanged(qreal)), this, SLOT(updateIcon()));
}

KisPaintopPresetIconLibrary::~KisPaintopPresetIconLibrary()
{
    delete ui;
    m_optionalModel->clear();
    delete m_optionalModel;
    m_baseModel->clear();
    delete m_baseModel;
}

QImage KisPaintopPresetIconLibrary::getImage()
{
    QImage preset = m_background;
    QImage base = QImage(200, 200, QImage::Format_ARGB32);
    base.fill(Qt::transparent);
    if (ui->vwBase->currentIndex().isValid()) {
        base = m_baseModel->itemFromIndex(ui->vwBase->currentIndex())->icon().pixmap(QSize(200, 200)).toImage();
    }
    if (ui->sldHue->value()>0 || ui->sldSat->value()!=0.0 || ui->sldLevels->value()!=0.0) {
        base = hueTransform(base);
    }
    QImage optional = QImage(40, 40, QImage::Format_ARGB32);
    optional.fill(Qt::transparent);
    if (ui->vwOptional->currentIndex().isValid()) {
         optional = m_optionalModel->itemFromIndex(ui->vwOptional->currentIndex())->icon().pixmap(QSize(40, 40)).toImage();
    }

    QPainter p(&preset);
    p.drawImage(0, 0, base);
    int x = 35;
    int y = 31;
    p.drawImage(x, y, optional);
    return preset;
}

QImage KisPaintopPresetIconLibrary::hueTransform(QImage img)
{
    //This is a super simple levels operation instead of a regular lightness adjustment.
    //This is so that we can keep the contrasts in the icon instead of making it
    //just darker or lighter.
    QVector<int> values(256);
    values.fill(255);
    int level = qMax(qMin( 28 + ( (int(ui->sldLevels->value()) + 50) * 2), 255), 0);
    for (int v = 0; v < level; v++) {
        values[v] = int((128.0 / qreal(level))*v);
    }
    for (int v = level; v < 255; v++) {
        values[v] = qMax(qMin(int( (128.0 / qreal(255 - level)) *(v - level)+128.0), 255), 0);
    }

    //This is very slow by Krita standards, but we cannot get hsv transforms, ad the image is only 200x200.
    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            QColor c = img.pixelColor(x, y);
            int hue = c.hslHue()+(int)ui->sldHue->value();
            if (hue > 360) {
                hue -= 360;
            }
            int sat = qMax(qMin(c.hslSaturation() + int(ui->sldSat->value() * (255.0 / 100.0)), 255), 0);
            c.setHsl(hue, sat, values.at(c.lightness()), c.alpha());
            img.setPixelColor(x, y, c);
        }
    }
    return img;
}

void KisPaintopPresetIconLibrary::updateIcon()
{
    ui->lblIconPreview->setPixmap(QPixmap::fromImage(getImage()));
}


