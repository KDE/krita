/*
 * This file is part of the KDE project
 * SPDX-FileCopyrightText: 2014 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef COLORPROFILEMODEL_H
#define COLORPROFILEMODEL_H

#include <QAbstractListModel>

class ColorProfileModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString colorModelId READ colorModelId WRITE setColorModelId NOTIFY colorModelIdChanged)
    Q_PROPERTY(QString colorDepthId READ colorDepthId WRITE setColorDepthId NOTIFY colorDepthIdChanged)
    Q_PROPERTY(int defaultProfile READ defaultProfile NOTIFY defaultProfileChanged)

public:
    enum Roles {
        TextRole = Qt::UserRole + 1,
    };

    explicit ColorProfileModel(QObject* parent = 0);
    virtual ~ColorProfileModel();
    QHash<int, QByteArray> roleNames() const;
    virtual QVariant data(const QModelIndex& index, int role) const;
    virtual int rowCount(const QModelIndex& parent) const;

    QString colorModelId() const;
    QString colorDepthId() const;

    int defaultProfile() const;

    Q_INVOKABLE QString id(int index);

public Q_SLOTS:
    void setColorModelId(const QString& id);
    void setColorDepthId(const QString& id);

Q_SIGNALS:
    void colorModelIdChanged();
    void colorDepthIdChanged();
    void defaultProfileChanged();

private:
    class Private;
    Private * const d;
};

#endif // COLORDEPTHMODEL_H
