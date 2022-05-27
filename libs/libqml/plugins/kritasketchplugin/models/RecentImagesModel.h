/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Boudewijn Rempt <boud@kogmbh.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef RECENTIMAGESMODEL_H
#define RECENTIMAGESMODEL_H

#include <QAbstractListModel>


class RecentImagesModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QObject* recentFileManager READ recentFileManager WRITE setRecentFileManager NOTIFY recentFileManagerChanged)
public:
    enum PresetRoles {
        ImageRole = Qt::UserRole + 1,
        TextRole,
        UrlRole,
        NameRole,
        DateRole
    };

    explicit RecentImagesModel(QObject *parent = nullptr);
    ~RecentImagesModel() override;
    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role) const override;

    QObject* recentFileManager() const;
    void setRecentFileManager(QObject* recentFileManager);

Q_SIGNALS:
    void recentFileManagerChanged();

public Q_SLOTS:

    void addRecent(const QString &fileName);

private:
    class Private;
    Private* d;

private Q_SLOTS:
    void recentFilesListChanged();
};

#endif // RECENTIMAGESMODEL_H
