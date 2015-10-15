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

#include "kis_icon_utils.h"

#include "kis_debug.h"

#include <QApplication>
#include <QAction>
#include <QAbstractButton>
#include <QComboBox>
#include <QIcon>
#include <QFile>

#include <KoIconUtils.h>
#include <KoIcon.h>

namespace KisIconUtils
{

QIcon loadIcon(const QString &name)
{
    return KoIconUtils::themedIcon(name);
}

bool adjustIcon(QIcon *icon)
{
    bool result = false;

    QString iconName = icon->name();
    if (iconName.isNull()) return result;

    QString realIconName = iconName;

    if (iconName.startsWith("dark_")) {
        realIconName = iconName.mid(5);
    }

    if (iconName.startsWith("light_")) {
        realIconName = iconName.mid(6);
    }

    if (!realIconName.isNull()) {
        *icon = loadIcon(realIconName);
        result = !icon->isNull();
    }

    return result;
}

void updateIconCommon(QObject *object)
{
    QAbstractButton* button = dynamic_cast<QAbstractButton*>(object);
    if (button) {
        updateIcon(button);
    }

    QComboBox* comboBox = dynamic_cast<QComboBox*>(object);
    if (comboBox) {
        updateIcon(comboBox);
    }

    QAction* action = dynamic_cast<QAction*>(object);
    if (action) {
        updateIcon(action);
    }
}

void updateIcon(QAbstractButton *button)
{
    QIcon icon = button->icon();

    if (adjustIcon(&icon)) {
        button->setIcon(icon);
    }
}

void updateIcon(QComboBox *comboBox)
{
    for (int i = 0; i < comboBox->count(); i++) {
        QIcon icon = comboBox->itemIcon(i);
        if (adjustIcon(&icon)) {
            comboBox->setItemIcon(i, icon);
        }
    }
}

void updateIcon(QAction *action)
{
    QIcon icon = action->icon();

    if (adjustIcon(&icon)) {
        action->setIcon(icon);
    }
}
}
