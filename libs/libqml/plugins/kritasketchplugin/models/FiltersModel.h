/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Dan Leinir Turthra Jensen <admin@leinir.dk>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef FILTERSMODEL_H
#define FILTERSMODEL_H

#include <QModelIndex>
#include <kis_types.h>

class FiltersModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QObject* view READ view WRITE setView NOTIFY viewChanged)
public:
    enum FiltersModelRoles {
        TextRole = Qt::UserRole + 1
    };
    explicit FiltersModel(QObject* parent = 0);
    virtual ~FiltersModel();
    QHash<int, QByteArray> roleNames() const;
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    Q_INVOKABLE bool filterRequiresConfiguration(int index);
    Q_INVOKABLE QString filterID(int index);
    Q_INVOKABLE void activateFilter(int index);
    KisFilter* filter(int index);

    QString categoryId;
    QString categoryName;

    void addFilter(KisFilterSP filter);

    QObject* view() const;
    void setView(QObject* newView);

    Q_INVOKABLE QObject* configuration(int index);
    Q_INVOKABLE void setConfiguration(int index, QObject* configuration);

Q_SIGNALS:
    void viewChanged();
    void configurationChanged(int index);
    void filterActivated(int index);

private:
    class Private;
    Private* d;
};

#endif // FILTERSMODEL_H
