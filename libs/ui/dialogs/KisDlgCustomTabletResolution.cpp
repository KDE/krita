/*
 *  SPDX-FileCopyrightText: 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisDlgCustomTabletResolution.h"
#include "ui_KisDlgCustomTabletResolution.h"

#include <QSettings>
#include "kis_debug.h"

#include <QGuiApplication>
#include <QScreen>
#include <QStandardPaths>
#include <qpa/qplatformscreen.h>

#include <kstandardguiitem.h>

namespace {
QString rectToString(const QRect &rc) {
    return QString("%1, %2 %3 x %4")
            .arg(rc.x())
            .arg(rc.y())
            .arg(rc.width())
            .arg(rc.height());
}
}

KisDlgCustomTabletResolution::KisDlgCustomTabletResolution(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::KisDlgCustomTabletResolution)
{
    ui->setupUi(this);
    KGuiItem::assign(ui->btnBox->button(QDialogButtonBox::Ok), KStandardGuiItem::ok());
    KGuiItem::assign(ui->btnBox->button(QDialogButtonBox::Cancel), KStandardGuiItem::cancel());
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

    ui->radioMapToEntireScreen->setText(i18nc("@option:radio", "Map to entire virtual screen (%1)", rectToString(virtualScreenRect)));

    QRect nativeScreenRect;
    QPlatformScreen *screen = qGuiApp->primaryScreen()->handle();
    Q_FOREACH (QPlatformScreen *scr, screen->virtualSiblings()) {
        nativeScreenRect |= scr->geometry();
    }

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

    // apply the mode right now
    {
        QRect customTabletRect;
        const Mode tabletMode = getTabletMode(&customTabletRect);
        applyConfiguration(tabletMode, customTabletRect);
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

void KisDlgCustomTabletResolution::applyConfiguration(KisDlgCustomTabletResolution::Mode tabletMode, const QRect &customTabletRect)
{
    if (tabletMode == KisDlgCustomTabletResolution::USE_CUSTOM) {
        qputenv("QT_WINTAB_DESKTOP_RECT",
                QString("%1;%2;%3;%4")
                .arg(customTabletRect.x())
                .arg(customTabletRect.y())
                .arg(customTabletRect.width())
                .arg(customTabletRect.height()).toLatin1());
        qunsetenv("QT_IGNORE_WINTAB_MAPPING");
    } else if (tabletMode == KisDlgCustomTabletResolution::USE_VIRTUAL_SCREEN) {
        qputenv("QT_IGNORE_WINTAB_MAPPING", "1");
        qunsetenv("QT_WINTAB_DESKTOP_RECT");
    } else {
        qunsetenv("QT_WINTAB_DESKTOP_RECT");
        qunsetenv("QT_IGNORE_WINTAB_MAPPING");
    }
}
