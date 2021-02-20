/*
 * This file is part of the KDE project
 * SPDX-FileCopyrightText: 2014 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef COLORDEPTHMODEL_H
#define COLORDEPTHMODEL_H

#include <QAbstractListModel>

class ColorDepthModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString colorModelId READ colorModelId WRITE setColorModelId NOTIFY colorModelIdChanged)

public:
    enum Roles {
        TextRole = Qt::UserRole + 1,
    };

    explicit ColorDepthModel(QObject* parent = 0);
    virtual ~ColorDepthModel();
    QHash<int, QByteArray> roleNames() const;
    virtual QVariant data(const QModelIndex& index, int role) const;
    virtual int rowCount(const QModelIndex& parent) const;

    QString colorModelId() const;

    Q_INVOKABLE QString id(int index);
    Q_INVOKABLE int indexOf(const QString& id);

public Q_SLOTS:
    void setColorModelId(const QString& id);

Q_SIGNALS:
    void colorModelIdChanged();

private:
    class Private;
    Private * const d;
};

#endif // COLORDEPTHMODEL_H
