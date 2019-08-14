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

#ifndef ACTIONSEARCHWIDGET_H
#define ACTIONSEARCHWIDGET_H

#include <QObject>
#include <QWidget>

#include <kactioncollection.h>

#include "ui_WdgActionSearch.h"

class ActionSearchWidget : public QWidget, public Ui_WdgActionSearch
{
    Q_OBJECT
public:
    explicit ActionSearchWidget(KActionCollection *actionCollection, QWidget *parent = 0);
    virtual ~ActionSearchWidget() override;
Q_SIGNALS:
    void actionTriggered();
private Q_SLOTS:
    void actionSelected(const QModelIndex &idx);
private:
    class Private;
    QScopedPointer<Private> d;
};


#endif
