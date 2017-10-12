/* This file is part of the KDE project
 * Copyright (C) 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
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

#include "PanelConfiguration.h"

#include <QQuickItem>
#include <QSettings>
#include <QCoreApplication>
#include <KoResourcePaths.h>

#include <kis_config.h>

class PanelConfiguration::Private
{
public:
    QList<QQuickItem*> panels;
    QList<QQuickItem*> panelAreas;

    QHash<QString, QString> panelAreaMap;
};

PanelConfiguration::PanelConfiguration(QObject* parent)
    : QObject(parent), d(new Private)
{
    connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()), SLOT(save()));
}

PanelConfiguration::~PanelConfiguration()
{
    delete d;
}

void PanelConfiguration::componentComplete()
{
    QString configFile = KoResourcePaths::locate("config", "kritasketchpanelsrc");
    QSettings panelConfig(configFile, QSettings::IniFormat);

    int count = panelConfig.beginReadArray("Panels");
    for(int i = 0; i < count; ++i) {
        panelConfig.setArrayIndex(i);

        QString panel = panelConfig.value("panel").toString();
        QString area = panelConfig.value("area").toString();
        d->panelAreaMap.insert(panel, area);
    }
    panelConfig.endArray();
}

void PanelConfiguration::classBegin()
{

}

QQmlListProperty< QQuickItem > PanelConfiguration::panels()
{
    return QQmlListProperty<QQuickItem>(this, d->panels);
}

QQmlListProperty< QQuickItem > PanelConfiguration::panelAreas()
{
    return QQmlListProperty<QQuickItem>(this, d->panelAreas);
}

void PanelConfiguration::restore()
{
    if (d->panelAreaMap.count() == d->panels.count()) {
        Q_FOREACH (QQuickItem* panel, d->panels) {
            QString panelName = panel->objectName();
            QString area = d->panelAreaMap.value(panelName);

            Q_FOREACH (QQuickItem* panelArea, d->panelAreas) {
                if (panelArea->objectName() == area) {
                    panel->setParentItem(panelArea);
                    break;
                }
            }
        }
    } else if (d->panels.count() <= d->panelAreas.count()) {
        for(int i = 0; i < d->panels.count(); ++i) {
            d->panels.at(i)->setParentItem(d->panelAreas.at(i));
        }
    }
}

void PanelConfiguration::save()
{
    QString configFile = KoResourcePaths::locateLocal("config", "kritasketchpanelsrc");
    QSettings panelConfig(configFile, QSettings::IniFormat);

    panelConfig.beginWriteArray("Panels");
    int index = 0;
    Q_FOREACH (QQuickItem* panel, d->panels) {
        panelConfig.setArrayIndex(index++);

        panelConfig.setValue("panel", panel->objectName());
        panelConfig.setValue("area", panel->parentItem()->objectName());
    }
    panelConfig.endArray();
}



