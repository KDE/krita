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

#include "KoIconUtils.h"

#include <QIcon>
#include <QApplication>
#include <QPalette>
#include <QDebug>
#include <QFile>

#include <kiconloader.h>
#include <kiconengine.h>

void initWidgetIcons()
{
    Q_INIT_RESOURCE(kritawidgets);
}

namespace KoIconUtils
{

QIcon themedIcon(const QString &name) {

    initWidgetIcons();

    QString realName;

    // try load themed icon
    QColor background = qApp->palette().background().color();
    bool useDarkIcons = background.value() > 100;
    const char * const prefix = useDarkIcons ? "dark_" : "light_";

    realName = QLatin1String(prefix) + name;


    QStringList names = QStringList() << ":/pics/" + realName + ".svg"
                                      << ":/pics/" + realName + ".png"
                                      << ":/" + name
                                      << ":/" + name + ".svg"
                                      << ":/" + name + ".png"
                                      << ":/pics/" + name + ".svg"
                                      << ":/pics/" + name + ".png";

    foreach(const QString &resname, names) {
        if (QFile(resname).exists()) {
            QIcon icon(resname);
            return icon;
        }
    }


//    qDebug() << ">>>>>>>" << realName << KIconLoader::global()->iconPath(realName, KIconLoader::User, true) << "\n\t"
//             << name  << KIconLoader::global()->iconPath(name, KIconLoader::User, true);

    if (KIconLoader::global()->iconPath(realName, KIconLoader::User, true).isEmpty()) {
        realName = name;
    }

    QIcon icon = QIcon(new KIconEngine(realName, KIconLoader::global()));

    // fallback
    if (icon.isNull())
        icon = QIcon::fromTheme(name);

    return icon;

}

}
