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

#ifndef PANELCONFIGURATION_H
#define PANELCONFIGURATION_H

#include <QObject>
#include <QQmlParserStatus>
#include <QQmlListProperty>

class QQuickItem;

class PanelConfiguration : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<QQuickItem> panels READ panels)
    Q_PROPERTY(QQmlListProperty<QQuickItem> panelAreas READ panelAreas)
    Q_INTERFACES(QQmlParserStatus)

public:
    explicit PanelConfiguration(QObject* parent = 0);
    virtual ~PanelConfiguration();

    virtual void componentComplete();
    virtual void classBegin();

    QQmlListProperty<QQuickItem> panels();
    QQmlListProperty<QQuickItem> panelAreas();

public Q_SLOTS:
    void restore();
    void save();

private:
    class Private;
    Private * const d;
};

#endif // PANELCONFIGURATION_H
