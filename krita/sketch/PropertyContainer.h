/* This file is part of the KDE project
 * Copyright (C) 2012 Dan Leinir Turthra Jensen <admin@leinir.dk>
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

#ifndef PROPERTYCONTAINER_H
#define PROPERTYCONTAINER_H

#include <QObject>
#include <QVariant>

/**
 * The only purpose of this class is to expose the dynamic property
 * system of Qt to QML, so we can set and get properties on a generic
 * object. It is a little bit of a hack, but QML deliberately does
 * not have access to this (according to the developers).
 */
class PropertyContainer : public QObject
{
    Q_OBJECT
public:
    PropertyContainer(QString name, QObject* parent = 0);
    virtual ~PropertyContainer();

    // As QObject already as setProperty and property() functions, we must
    // name ours differently
    Q_INVOKABLE void writeProperty(QString name, QVariant value);
    Q_INVOKABLE QVariant readProperty(QString name);

    Q_INVOKABLE QString name();
private:
    QString m_name;
};

#endif // PROPERTYCONTAINER_H
