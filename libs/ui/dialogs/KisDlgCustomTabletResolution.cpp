/*
 *  Copyright (c) 2019 Dmitry Kazakov <dimula73@gmail.com>
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

#include "KisDlgCustomTabletResolution.h"
#include "ui_KisDlgCustomTabletResolution.h"

#include <QSettings>
#include "kis_debug.h"

#include <QGuiApplication>
#include <QScreen>
#include <QStandardPaths>
#include <qpa/qplatformscreen.h>


KisDlgCustomTabletResolution::KisDlgCustomTabletResolution(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::KisDlgCustomTabletResolution)
{
    ui->setupUi(this);
    connect(ui->btnBox, SIGNAL(rejected()), this, SLOT(reject()));
    connect(ui->btnBox, SIGNAL(accepted()), this, SLOT(accept()));

    connect(ui->radioCustomMapping, SIGNAL(toggled(bool)), ui->lblXOffset, SLOT(setEnabled(bool)));
    connect(ui->radioCustomMapping, SIGNAL(toggled(bool)), ui->lblYOffset, SLOT(setEnabled(bool)));
    connect(ui->radioCustomMapping, SIGNAL(toggled(bool)), ui->lblWidth, SLOT(setEnabled(bool)));
    connect(ui->radioCustomMapping, SIGNAL(toggled(bool)), ui->lblHeight, SLOT(setEnabled(bool)));

    connect(ui->radioCustomMapping, SIGNAL(toggled(bool)), ui->intXOffset, SLOT(setEnabled(bool)));
    connect(ui->radioCustomMapping, SIGNAL(toggled(bool)), ui->intYOffset, SLOT(setEnabled(bool)));
    connect(ui->radioCustomMapping, SIGNAL(toggled(bool)), ui->intWidth, SLOT(setEnabled(bool)));
    connect(ui->radioCustomMapping, SIGNAL(toggled(bool)), ui->intHeight, SLOT(setEnabled(bool)));

    const QRect virtualScreenRect = calcNativeScreenRect();

    const QString rectToString =
        QString("%1, %2 %3 x %4")
            .arg(virtualScreenRect.x())
            .arg(virtualScreenRect.y())
            .arg(virtualScreenRect.width())
            .arg(virtualScreenRect.height());

    ui->radioMapToEntireScreen->setText(i18nc("@option:radio", "Map to entire virtual screen (%1)", rectToString));

    QRect customScreenRect = virtualScreenRect;
    Mode mode = getTabletMode(&customScreenRect);

    if (mode == USE_CUSTOM) {
        ui->radioCustomMapping->setChecked(true);
    } else if (mode == USE_VIRTUAL_SCREEN) {
        ui->radioMapToEntireScreen->setChecked(true);
    } else {
        ui->radioMapAsWintab->setChecked(true);
    }

    ui->intXOffset->setValue(customScreenRect.x());
    ui->intYOffset->setValue(customScreenRect.y());
    ui->intWidth->setValue(customScreenRect.width());
    ui->intHeight->setValue(customScreenRect.height());
}

void KisDlgCustomTabletResolution::accept()
{
    const QString configPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    QSettings cfg(configPath + QStringLiteral("/kritadisplayrc"), QSettings::IniFormat);

    if (ui->radioMapAsWintab->isChecked()) {
        cfg.setValue("wintabResolutionMode", "wintab");
    } else if (ui->radioMapToEntireScreen->isChecked()) {
        cfg.setValue("wintabResolutionMode", "virtual-screen");
    } else if (ui->radioCustomMapping->isChecked()) {
        cfg.setValue("wintabResolutionMode", "custom");

        cfg.setValue("wintabCustomResolutionX", ui->intXOffset->value());
        cfg.setValue("wintabCustomResolutionY", ui->intYOffset->value());
        cfg.setValue("wintabCustomResolutionWidth", ui->intWidth->value());
        cfg.setValue("wintabCustomResolutionHeight", ui->intHeight->value());
    }

    QDialog::accept();
}

KisDlgCustomTabletResolution::~KisDlgCustomTabletResolution()
{
    delete ui;
}

QRect KisDlgCustomTabletResolution::calcNativeScreenRect()
{
    QRect nativeScreenRect;
    QPlatformScreen *screen = qGuiApp->primaryScreen()->handle();
    Q_FOREACH (QPlatformScreen *scr, screen->virtualSiblings()) {
        nativeScreenRect |= scr->geometry();
    }
    return nativeScreenRect;
}

KisDlgCustomTabletResolution::Mode KisDlgCustomTabletResolution::getTabletMode(QRect *customRect)
{
    const QString configPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    QSettings cfg(configPath + QStringLiteral("/kritadisplayrc"), QSettings::IniFormat);

    const QString mode = cfg.value("wintabResolutionMode", QString("wintab")).toString();
    Mode modeValue = USE_WINTAB;

    if (mode == "custom") {
        modeValue = USE_CUSTOM;
    } else if (mode == "virtual-screen") {
        modeValue = USE_VIRTUAL_SCREEN;
    } else {
        modeValue = USE_WINTAB;
    }

    if (mode == "custom") {
        customRect->setX(cfg.value("wintabCustomResolutionX", customRect->x()).toInt());
        customRect->setY(cfg.value("wintabCustomResolutionY", customRect->y()).toInt());
        customRect->setWidth(cfg.value("wintabCustomResolutionWidth", customRect->width()).toInt());
        customRect->setHeight(cfg.value("wintabCustomResolutionHeight", customRect->height()).toInt());
    }

    return modeValue;
}
