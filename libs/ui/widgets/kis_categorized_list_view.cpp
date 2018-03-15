/*
 *  Copyright (c) 2011 Silvio Heinrich <plassy@web.de>
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

#include "kis_categorized_list_view.h"
#include "kis_categorized_list_model.h"
#include <QMouseEvent>
#include <QMenu>
#include <QAction>
#include <QShowEvent>
#include <kconfig.h>
#include <klocalizedstring.h>
#include <kis_icon.h>
#include "kis_debug.h"

KisCategorizedListView::KisCategorizedListView(QWidget* parent):
    QListView(parent)
{
    connect(this, SIGNAL(clicked(QModelIndex)), this, SLOT(slotIndexChanged(QModelIndex)));
}

KisCategorizedListView::~KisCategorizedListView()
{
}

void KisCategorizedListView::setModel(QAbstractItemModel* model)
{
    QListView::setModel(model);
    updateRows(0, model->rowCount());
    model->sort(0);
}

QSize KisCategorizedListView::sizeHint() const
{
    const QSize sh = QListView::sizeHint();
    const int width = sizeHintForColumn(0);

    return QSize(width, sh.height());
}

void KisCategorizedListView::setCompositeBoxControl(bool value)
{
    isCompositeBoxControl = value;
}

void KisCategorizedListView::updateRows(int begin, int end)
{
    for(; begin!=end; ++begin) {
        QModelIndex index    = model()->index(begin, 0);
        bool        isHeader = model()->data(index, __CategorizedListModelBase::IsHeaderRole).toBool();
        bool        expanded = model()->data(index, __CategorizedListModelBase::ExpandCategoryRole).toBool();
        setRowHidden(begin, !expanded && !isHeader);
    }
}

void KisCategorizedListView::slotIndexChanged(const QModelIndex& index)
{
    if(model()->data(index, __CategorizedListModelBase::IsHeaderRole).toBool()) {
        bool expanded = model()->data(index, __CategorizedListModelBase::ExpandCategoryRole).toBool();
        model()->setData(index, !expanded, __CategorizedListModelBase::ExpandCategoryRole);
        emit sigCategoryToggled(index, !expanded);
    }
}

void KisCategorizedListView::dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int> &roles)
{
    QListView::dataChanged(topLeft, bottomRight);
    updateRows(topLeft.row(), bottomRight.row()+1);

    // check to see if the data changed was a check box
    // if it is a checkbox tell the brush edtor that the preset is now "dirty"
    int i = 0;
    for (QVector<int>::const_iterator iterator = roles.begin(); iterator != roles.end(); ++iterator) {

        if (Qt::CheckStateRole == roles.at(i) ) {
            int row = topLeft.row();
            int column = topLeft.column();

            emit sigEntryChecked(model()->index(row, column));
        } else if (__CategorizedListModelBase::ExpandCategoryRole == roles.at(i)) {
           // logic to target the expand/contract menus if needed
        }

        i++;
    }
}

void KisCategorizedListView::rowsInserted(const QModelIndex& parent, int start, int end)
{
    QListView::rowsInserted(parent, start, end);
    updateRows(0, model()->rowCount());
    model()->sort(0);
}

void KisCategorizedListView::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    QListView::rowsAboutToBeRemoved(parent, start, end);
    model()->sort(0);
}

void KisCategorizedListView::mousePressEvent(QMouseEvent* event)
{
    QListView::mousePressEvent(event);

    QModelIndex index = QListView::indexAt(event->pos());


    // hack: the custom compositeop combo box has issues with events being sent
    // those widgets will run this extra code to help them know if a checkbox is clicked
    if (isCompositeBoxControl) {

        if (index.isValid() && (event->pos().x() < 25) && (model()->flags(index) & Qt::ItemIsUserCheckable)) {
            QListView::mousePressEvent(event);
            QMouseEvent releaseEvent(QEvent::MouseButtonRelease,
                            event->pos(),
                            event->globalPos(),
                            event->button(),
                            event->button() | event->buttons(),
                            event->modifiers());

            QListView::mouseReleaseEvent(&releaseEvent);
            emit sigEntryChecked(index);

            return; // don't worry about running the 'right' click logic below. that is not relevant with composite ops
        }
    }



    if(event->button() == Qt::RightButton){
        QMenu menu(this);
        if(index.data(__CategorizedListModelBase::isLockableRole).toBool() && index.isValid()) {

            bool locked = index.data(__CategorizedListModelBase::isLockedRole).toBool();

            QIcon icon = locked ? KisIconUtils::loadIcon("locked") : KisIconUtils::loadIcon("unlocked");

            QAction* action1 = menu.addAction(icon, locked ? i18n("Unlock (restore settings from preset)") : i18n("Lock"));

            connect(action1, SIGNAL(triggered()), this, SIGNAL(rightClickedMenuDropSettingsTriggered()));

            if (locked){
                QAction* action2 = menu.addAction(icon, i18n("Unlock (keep current settings)"));
                connect(action2, SIGNAL(triggered()), this, SIGNAL(rightClickedMenuSaveSettingsTriggered()));
            }
            menu.exec(event->globalPos());
        }
    }
}

void KisCategorizedListView::mouseReleaseEvent(QMouseEvent* event)
{
    QListView::mouseReleaseEvent(event);
}



