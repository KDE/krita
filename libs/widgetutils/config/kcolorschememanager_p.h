/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2013 Martin Gräßlin <mgraesslin@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KCOLORSCHEMEMANAGER_P_H
#define KCOLORSCHEMEMANAGER_P_H

#include <QAbstractListModel>
#include <QIcon>

struct KColorSchemeModelData {
    QString name;
    QString path;
    QIcon preview;
};

class KColorSchemeModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit KColorSchemeModel(QObject *parent = 0);
    ~KColorSchemeModel() override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

private:
    void init();
    QIcon createPreview(const QString &path);
    QVector<KColorSchemeModelData> m_data;
};

class KColorSchemeManagerPrivate
{
public:
    KColorSchemeManagerPrivate();

    QScopedPointer<KColorSchemeModel> model;
};

#endif
