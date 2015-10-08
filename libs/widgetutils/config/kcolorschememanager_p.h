/* This file is part of the KDE project
 * Copyright (C) 2013 Martin Gräßlin <mgraesslin@kde.org>
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
    virtual ~KColorSchemeModel();
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

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
