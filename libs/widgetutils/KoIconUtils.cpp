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

namespace KoIconUtils
{

QIcon themedIcon(const QString &name) {

    // try load themed icon
    QColor background = qApp->palette().background().color();
    bool useDarkIcons = background.value() > 100;
    const char * const prefix = useDarkIcons ? "dark_" : "light_";

    QString  realName = QLatin1String(prefix) + name;


    QStringList names = QStringList() << ":/pics/" + realName + ".png"
                                      << ":/pics/" + realName + ".svg"
                                      << ":/pics/" + realName + ".svgz"
                                      << ":/pics/" + name + ".png"
                                      << ":/pics/" + name + ".svg"
                                      << ":/pics/" + name + ".svz"
                                      << ":/" + realName + ".png"
                                      << ":/" + realName + ".svg"
                                      << ":/" + realName + ".svz"
                                      << ":/" + name
                                      << ":/" + name + ".png"
                                      << ":/" + name + ".svg"
                                      << ":/" + name + ".svgz"
                                         ;

    foreach(const QString &resname, names) {
        if (QFile(resname).exists()) {
            QIcon icon(resname);
            return icon;
        }
    }


    QIcon icon = QIcon::fromTheme(name);
    qDebug() << "\tfalling back on QIcon::FromTheme:" << name;
    return icon;
}

}
