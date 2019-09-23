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

#include <QApplication>
#include <QAction>
#include <QAbstractButton>
#include <QComboBox>
#include <QIcon>
#include <QFile>
#include <QPair>
#include <QDebug>

#include <KoIcon.h>
#if QT_VERSION >= 0x050900
#define CACHE_ICONS
#endif

namespace KisIconUtils
{

#if defined CACHE_ICONS
static QMap<QString, QIcon> s_cache;
#endif
static QMap<qint64, QString> s_icons;

QIcon loadIcon(const QString &name)
{
#if defined CACHE_ICONS
    QMap<QString, QIcon>::const_iterator cached = s_cache.constFind(name);
    if (cached != s_cache.constEnd()) {
        return cached.value();
    }
#endif
    // try load themed icon


    const char * const prefix = useDarkIcons() ? "dark_" : "light_";

    QString  realName = QLatin1String(prefix) + name;

    // Dark and light, no size specified
    const QStringList names = { ":/pics/" + realName + ".png",
                                ":/pics/" + realName + ".svg",
                                ":/pics/" + realName + ".svgz",
                                ":/pics/" + name + ".png",
                                ":/pics/" + name + ".svg",
                                ":/pics/" + name + ".svz",
                                ":/" + realName + ".png",
                                ":/" + realName + ".svg",
                                ":/" + realName + ".svz",
                                ":/" + name,
                                ":/" + name + ".png",
                                ":/" + name + ".svg",
                                ":/" + name + ".svgz"};

    for (const QString &resname : names) {
        if (QFile(resname).exists()) {
            QIcon icon(resname);
            s_icons.insert(icon.cacheKey(), name);
#if defined CACHE_ICONS
            s_cache.insert(name, icon);
#endif
            return icon;
        }
    }

    // Now check for icons with sizes
    QStringList sizes = QStringList() << "16_" << "22_" << "32_" << "48_" << "64_" << "128_" << "256_" << "512_" << "1048_";
    QVector<QPair<QString, QString> > icons;
    Q_FOREACH (const QString &size, sizes) {
        const QStringList names = { ":/pics/" + size + realName + ".png",
                                    ":/pics/" + size + realName + ".svg",
                                    ":/pics/" + size + realName + ".svgz",
                                    ":/pics/" + size + name + ".png",
                                    ":/pics/" + size + name + ".svg",
                                    ":/pics/" + size + name + ".svz",
                                    ":/" + size + realName + ".png",
                                    ":/" + size + realName + ".svg",
                                    ":/" + size + realName + ".svz",
                                    ":/" + size + name,
                                    ":/" + size + name + ".png",
                                    ":/" + size + name + ".svg",
                                    ":/" + size + name + ".svgz"};

        for (const QString &resname : names) {
            if (QFile(resname).exists()) {
                icons << qMakePair(size, resname);
            }
        }
    }

    if (!icons.isEmpty()) {
        QIcon icon;
        Q_FOREACH (auto p, icons) {
            QString sz = p.first;
            sz.chop(1);
            int size = sz.toInt();
            icon.addFile(p.second, QSize(size, size));
        }
        s_icons.insert(icon.cacheKey(), name);
#if defined CACHE_ICONS
        s_cache.insert(name, icon);
#endif
        return icon;
    }

    QIcon icon = QIcon::fromTheme(name);
    //qDebug() << "falling back on QIcon::FromTheme:" << name;
    s_icons.insert(icon.cacheKey(), name);
#if defined CACHE_ICONS
    s_cache.insert(name, icon);
#endif
    return icon;
}




bool useDarkIcons() {
    QColor background = qApp->palette().window().color();
    return  background.value() > 100;
}

bool adjustIcon(QIcon *icon)
{
    bool result = false;

    QString iconName = icon->name();
    if (iconName.isNull()) {
        if (s_icons.contains(icon->cacheKey())) {
            iconName = s_icons.take(icon->cacheKey());
        }
    }

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
        s_icons.insert(icon->cacheKey(), iconName);
    }

    return result;
}

void updateIconCommon(QObject *object)
{
    QAbstractButton* button = qobject_cast<QAbstractButton*>(object);
    if (button) {
        updateIcon(button);
    }

    QComboBox* comboBox = qobject_cast<QComboBox*>(object);
    if (comboBox) {
        updateIcon(comboBox);
    }

    QAction* action = qobject_cast<QAction*>(object);
    if (action) {
        updateIcon(action);
    }
}

void clearIconCache() {
    #if defined CACHE_ICONS
        s_cache.clear();
    #endif
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
