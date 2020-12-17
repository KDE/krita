/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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
