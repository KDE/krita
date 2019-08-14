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

#ifndef SearchWidget_H
#define SearchWidget_H

#include <QObject>
#include <QWidget>
#include <QLineEdit>
#include <QMultiHash>
#include <QAbstractListModel>
#include <QSortFilterProxyModel>

#include <kactioncollection.h>

#include "ui_WdgActionSearch.h"

class SearchWidget : public QLineEdit
{
    Q_OBJECT
public:
    explicit SearchWidget(KActionCollection *actionCollection, QWidget *parent = 0);
    virtual ~SearchWidget() override;

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


#endif
