/*
 * This file is part of the KDE project
 * SPDX-FileCopyrightText: 2014 Dan Leinir Turthra Jensen <admin@leinir.dk>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */

#ifndef TEMPLATESMODEL_H
#define TEMPLATESMODEL_H

#include <QAbstractListModel>

class TemplatesModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum TemplateRoles {
        NameRole = Qt::UserRole + 1,
        DescriptionRole,
        FileRole,
        IconRole,
        GroupName,
        GroupFolded
    };
    explicit TemplatesModel(QObject* parent = 0);
    virtual ~TemplatesModel();
    QHash<int, QByteArray> roleNames() const;
    virtual QVariant data(const QModelIndex& index, int role) const;
    virtual int rowCount(const QModelIndex& parent) const;
    Q_INVOKABLE QString groupNameOf(int index) const;
    Q_INVOKABLE void toggleGroup(const QString& name);
    Q_SLOT void populate();

private:
    struct ItemData;
    class Private;
    Private* d;
};

#endif // TEMPLATESMODEL_H
