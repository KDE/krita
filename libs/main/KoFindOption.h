/* This file is part of the KDE project
 *
 * Copyright (c) 2010 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KOFINDOPTION_H
#define KOFINDOPTION_H

#include <QtCore/QObject>
#include <QtCore/QSharedDataPointer>


class KoFindOption : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int id READ id)
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
    Q_PROPERTY(QVariant value READ value WRITE setValue NOTIFY valueChanged)

public:
    explicit KoFindOption(int id, QObject* parent = 0);
    KoFindOption(const KoFindOption &other);
    virtual ~KoFindOption();

    int id() const;
    QString title() const;
    QString description() const;
    QVariant value() const;

public Q_SLOTS:
    void setTitle(const QString &newTitle);
    void setDescription(const QString &newDescription);
    void setValue(const QVariant &newValue);

Q_SIGNALS:
    void titleChanged();
    void descriptionChanged();
    void valueChanged(const QVariant &newValue);

private:
    class Private;
    QSharedDataPointer<Private> d;
};

#endif // KOFINDOPTION_H
