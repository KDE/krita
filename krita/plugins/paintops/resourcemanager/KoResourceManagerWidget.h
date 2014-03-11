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

#include <QMainWindow>
#include <QString>
#include "KoResourceTableModel.h"
#include "KoResourceModel.h"
#include "KoResourceServerAdapter.h"
#include "KoResourceManagerControl.h"
#include <QSortFilterProxyModel>
#include <krita_export.h>

namespace Ui {
class KoResourceManagerWidget;
}


class KRITAUI_EXPORT KoResourceManagerWidget : public QMainWindow
{
    Q_OBJECT
public:
    explicit KoResourceManagerWidget(QWidget *parent = 0);
    ~KoResourceManagerWidget();

private:
    KoResourceManagerControl *control;
    KoResourceTableModel *model;
    QMenu *buttonMenu;
    Ui::KoResourceManagerWidget *ui;

    /*QSortFilterProxyModel* m_filter;
    MyTableModel *model2;*/

    void initializeModel();
    void initializeConnect();
    void createMiniature(QPixmap);

private slots:
    void showHide();
    void createPack();
    void installPack();
    void uninstallPack();
    void deletePack();
    void setMeta();
    void refreshCurrentTable();
    void filterFieldSelected(bool);
    void rename();
    void about();

signals:

public slots:

};

#endif // KORESOURCEMANAGERWIDGET_H
