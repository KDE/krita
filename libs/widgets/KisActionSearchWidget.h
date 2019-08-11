/*
 *  Copyright (c) 2019 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KISACTIONSEARCHWIDGET_H
#define KISACTIONSEARCHWIDGET_H

#include <QObject>
#include <QWidget>
#include <QLineEdit>
#include <QMultiHash>
#include <QAbstractListModel>
#include <QSortFilterProxyModel>

#include <kactioncollection.h>

#include "ui_WdgActionSearch.h"

#include <kritawidgets_export.h>

class KisActionModel : public QAbstractListModel {
    Q_OBJECT
public:
    explicit KisActionModel(KActionCollection *actionCollection, QObject *parent = 0);

    // reimp from QAbstractListModel
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
private:
    KActionCollection *m_actionCollection {0};
};

class KisActionSearchModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:

    explicit KisActionSearchModel(QObject *parent = 0);
    void setFilterText(const QString &filter);
protected:
    bool filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const override;
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;
private:
    QString m_filter;
};


class KisActionSearchWidget : public QWidget, public Ui_WdgActionSearch
{
    Q_OBJECT
public:
    explicit KisActionSearchWidget(KActionCollection *actionCollection, QWidget *parent = 0);
    virtual ~KisActionSearchWidget() override;
Q_SIGNALS:
    void actionTriggered();
private Q_SLOTS:
    void actionSelected(const QModelIndex &idx);
private:
    class Private;
    QScopedPointer<Private> d;
};


class KRITAWIDGETS_EXPORT KisActionSearchLine : public QLineEdit
{
    Q_OBJECT
public:
    explicit KisActionSearchLine(KActionCollection *actionCollection, QWidget *parent = 0);
    virtual ~KisActionSearchLine() override;

private Q_SLOTS:
    void showPopup();
    void hidePopup();
protected:
    void focusInEvent(QFocusEvent *) override;
private:
    void adjustPosition();

    class Private;
    QScopedPointer<Private> d;
};


#endif // KISACTIONSEARCHWIDGET_H
