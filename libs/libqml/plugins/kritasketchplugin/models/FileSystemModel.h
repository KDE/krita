/* This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */



#ifndef CALLIGRAMOBILE_FILESYSTEMMODEL_H
#define CALLIGRAMOBILE_FILESYSTEMMODEL_H

#include <QQmlParserStatus>

#include <QAbstractListModel>

class FileSystemModel : public QAbstractListModel, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged)
    Q_PROPERTY(QString parentFolder READ parentFolder)
    Q_PROPERTY(QString filter READ filter WRITE setFilter)

public:
    enum FileRoles {
        FileNameRole = Qt::UserRole,
        FilePathRole,
        FileIconRole,
        FileDateRole
    };

    explicit FileSystemModel(QObject* parent = 0);
    virtual ~FileSystemModel();

    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;

    virtual void classBegin() override;
    virtual void componentComplete() override;

    virtual QString path();
    virtual void setPath(const QString& path);

    virtual QString parentFolder();

    virtual QString filter();
    virtual void setFilter(const QString& filter);

    QHash<int, QByteArray> roleNames() const override;

Q_SIGNALS:
    void pathChanged();

private:
    class Private;
    Private * const d;
};

#endif // CALLIGRAMOBILE_FILESYSTEMMODEL_H
