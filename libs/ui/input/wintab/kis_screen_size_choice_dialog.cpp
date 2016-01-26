/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_screen_size_choice_dialog.h"
#include "ui_kis_screen_size_choice_dialog.h"

#include <QButtonGroup>

#include <kconfig.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>


#include "kis_debug.h"


QString rectToString(const QRect &rc) {
    return i18nc("Screen rect when choosing Wintab/Qt settings", "%1x%2 offset: %3,%4", rc.width(), rc.height(), rc.x(), rc.y());
}

KisScreenSizeChoiceDialog::KisScreenSizeChoiceDialog(QWidget *parent,
                                                     const QRect &wintabRect,
                                                     const QRect &qtRect)
    : QDialog(parent),
      ui(new Ui::KisScreenSizeChoiceDialog),
      m_wintabRect(wintabRect),
      m_qtRect(qtRect)
{
    ui->setupUi(this);

    connect(ui->radioManual, SIGNAL(toggled(bool)), ui->lblXOffset, SLOT(setEnabled(bool)));
    connect(ui->radioManual, SIGNAL(toggled(bool)), ui->lblYOffset, SLOT(setEnabled(bool)));
    connect(ui->radioManual, SIGNAL(toggled(bool)), ui->lblWidth, SLOT(setEnabled(bool)));
    connect(ui->radioManual, SIGNAL(toggled(bool)), ui->lblHeight, SLOT(setEnabled(bool)));

    connect(ui->radioManual, SIGNAL(toggled(bool)), ui->intXOffset, SLOT(setEnabled(bool)));
    connect(ui->radioManual, SIGNAL(toggled(bool)), ui->intYOffset, SLOT(setEnabled(bool)));
    connect(ui->radioManual, SIGNAL(toggled(bool)), ui->intWidth, SLOT(setEnabled(bool)));
    connect(ui->radioManual, SIGNAL(toggled(bool)), ui->intHeight, SLOT(setEnabled(bool)));

    // cold-init the state of the manua controls
    ui->radioManual->setChecked(true);

    ui->radioWintab->setText(i18nc("@option:radio", "%1 (Wintab)", rectToString(wintabRect)));
    ui->radioQt->setText(i18nc("@option:radio", "%1 (Qt)", rectToString(qtRect)));

    m_dataSource = new QButtonGroup(this);
    m_dataSource->addButton(ui->radioWintab, 0);
    m_dataSource->addButton(ui->radioQt, 1);
    m_dataSource->addButton(ui->radioManual, 2);

    loadSettings(wintabRect);

    connect(this, SIGNAL(finished(int)), SLOT(slotFinished()));
}

void KisScreenSizeChoiceDialog::slotFinished()
{
    saveSettings();
}

bool KisScreenSizeChoiceDialog::canUseDefaultSettings() const
{
    return ui->chkRememberSetting->isChecked();
}

QRect KisScreenSizeChoiceDialog::screenRect() const
{
    QRect rect;

    switch (m_dataSource->checkedId()) {
    case 0:
        rect = m_wintabRect;
        break;
    case 1:
        rect = m_qtRect;
        break;
    case 2:
        rect.setRect(ui->intXOffset->value(),
                     ui->intYOffset->value(),
                     ui->intWidth->value(),
                     ui->intHeight->value());
        break;

    }

    return rect;
}

void KisScreenSizeChoiceDialog::loadSettings(const QRect &defaultRect)
{
    KConfigGroup cfg = KSharedConfig::openConfig()->group("ScreenSizeWorkaroundDialog");
    int index = qBound(0, cfg.readEntry("choiceIndex", 0), 2);

    m_dataSource->button(index)->setChecked(true);

    ui->intXOffset->setValue(cfg.readEntry("xOffset", defaultRect.x()));
    ui->intYOffset->setValue(cfg.readEntry("yOffset", defaultRect.y()));
    ui->intWidth->setValue(cfg.readEntry("Width", defaultRect.width()));
    ui->intHeight->setValue(cfg.readEntry("Height", defaultRect.height()));

    ui->chkRememberSetting->setChecked(cfg.readEntry("RememberSetting", false));
}

void KisScreenSizeChoiceDialog::saveSettings()
{
    KConfigGroup cfg = KSharedConfig::openConfig()->group("ScreenSizeWorkaroundDialog");

    cfg.writeEntry("choiceIndex", m_dataSource->checkedId());

    cfg.writeEntry("xOffset", ui->intXOffset->value());
    cfg.writeEntry("yOffset", ui->intYOffset->value());
    cfg.writeEntry("Width", ui->intWidth->value());
    cfg.writeEntry("Height", ui->intHeight->value());

    cfg.writeEntry("RememberSetting", ui->chkRememberSetting->isChecked());
}

KisScreenSizeChoiceDialog::~KisScreenSizeChoiceDialog()
{
    delete ui;
}
