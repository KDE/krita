/*
 *  Copyright (c) 2014 Victor Lafon metabolic.ewilan@hotmail.fr
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

#ifndef KORESOURCEMANAGERWIDGET_H
#define KORESOURCEMANAGERWIDGET_H

#include <QtGui/QMainWindow>
#include <QtCore/QModelIndex>
#include <krita_export.h>
#include <QtGui/QLabel>

namespace Ui
{
    class KoResourceManagerWidget;
}

class ClickLabel : public QLabel
{
    Q_OBJECT

public:
    ClickLabel(QWidget * parent = 0)
        :QLabel(parent)
    {

    }

signals:
    void clicked();

private:
    void mousePressEvent ( QMouseEvent * event )
    {
        Q_UNUSED(event);
        emit clicked();
    }
};

class KoResourceManagerControl;

class KoResourceManagerWidget : public QMainWindow
{
    Q_OBJECT

public:
    explicit KoResourceManagerWidget(QWidget *parent = 0);
    ~KoResourceManagerWidget();

    void initializeModel();
    void initializeConnect();
    void connectTables();

private slots:
    void about();
    void createPack();
    void deletePack();
    void endRenaming();
    void filterFieldSelected(bool);
    void filterResourceTypes(int index);
    void installPack();
    void refreshCurrentTable();
    void refreshDetails(QModelIndex newIndex);
    void setMeta();
    void showHide();
    void startRenaming();
    void uninstallPack();

private:
    KoResourceManagerControl *control;
    Ui::KoResourceManagerWidget *ui;
    ClickLabel *resourceNameLabel;
    bool firstRefresh;

    /*QSortFilterProxyModel* m_filter;
    MyTableModel *model2;*/
};

#endif // KORESOURCEMANAGERWIDGET_H
