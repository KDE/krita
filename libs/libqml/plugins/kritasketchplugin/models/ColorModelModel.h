/*
 * This file is part of the KDE project
 * SPDX-FileCopyrightText: 2014 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef COLORMODELMODEL_H
#define COLORMODELMODEL_H

#include <QAbstractListModel>

class ColorModelModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        TextRole = Qt::UserRole + 1,
    };

    ColorModelModel(QObject *parent = nullptr);
    ~ColorModelModel() override;
    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    Q_INVOKABLE QString id(int index);
    Q_INVOKABLE int indexOf(const QString& id);

private:
    class Private;
    Private * const d;
};

#endif // COLORMODELMODEL_H
