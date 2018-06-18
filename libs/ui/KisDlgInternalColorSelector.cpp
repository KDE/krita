/*
 * Copyright (C) Wolthera van Hovell tot Westerflier <griffinvalley@gmail.com>, (C) 2016
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

#include <QHBoxLayout>
#include <QDebug>

#include "KisDlgInternalColorSelector.h"
#include "KisScreenColorPicker.h"

KisDlgInternalColorSelector::KisDlgInternalColorSelector(QWidget* parent, KoColor color, Config config, const QString &caption, const KoColorDisplayRendererInterface *displayRenderer)
    : KisBasicInternalColorSelector(parent, color, config, caption, displayRenderer)
{
    m_screenColorPicker = new KisScreenColorPicker(this);
    m_ui->screenColorPickerWidget->setLayout(new QHBoxLayout(m_ui->screenColorPickerWidget));
    m_ui->screenColorPickerWidget->layout()->addWidget(m_screenColorPicker);
    qDebug() << "dlg setup";
    if (config.screenColorPicker) {
        connect(m_screenColorPicker, SIGNAL(sigNewColorPicked(KoColor)),this, SLOT(slotColorUpdated(KoColor)));
        qDebug() << "dlg setup connected";
    } else {
        m_ui->screenColorPickerWidget->hide();
    }
}


void KisDlgInternalColorSelector::updateAllElements(QObject *source)
{
    KisBasicInternalColorSelector::updateAllElements(source);
    m_screenColorPicker->updateIcons();
}

KoColor KisDlgInternalColorSelector::getModalColorDialog(const KoColor color, QWidget* parent, QString caption)
{
    Config config = Config();
    KisDlgInternalColorSelector dialog(parent, color, config, caption);
    dialog.setPreviousColor(color);
    dialog.exec();
    return dialog.getCurrentColor();
}
