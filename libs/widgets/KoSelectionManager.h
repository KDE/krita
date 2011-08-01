/* This file is part of the KDE project
*
* Copyright (C) 2008 Peter Penz <peter.penz19@gmail.com>
* Copyright (C) 2011 Paul Mendez <paulestebanms@gmail.com>
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

#ifndef KOSELECTIONMANAGER_H
#define KOSELECTIONMANAGER_H

#include <QObject>
#include "kowidgets_export.h"

class QAbstractItemView;
class QModelIndex;
class QItemSelection;
class KoSelectionToggle;

/**
 * @brief Allows to select and deselect items for item views.
 *
 * Whenever an item is hovered by the mouse, a toggle button is shown
 * which allows to select/deselect the current item.
 */
class KOWIDGETS_EXPORT KoSelectionManager : public QObject
{
    Q_OBJECT

public:
    KoSelectionManager(QAbstractItemView *parent);
    virtual ~KoSelectionManager();
    virtual bool eventFilter(QObject *watched, QEvent *event);

public slots:
    /**
     * Resets the selection manager so that the toggle button gets
     * invisible.
     */
    void reset();

signals:
    /** Is emitted if the selection has been changed by the toggle button. */
    void selectionChanged();

private slots:
    void slotEntered(const QModelIndex &index);
    void slotViewportEntered();
    void setItemSelected(bool selected);
    void slotRowsRemoved(const QModelIndex &parent, int start, int end);
    void slotSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

private:
    void applyPointingHandCursor();
    void restoreCursor();
    QAbstractItemView *m_view;
    KoSelectionToggle *m_toggle;
    bool m_connected;
    bool m_appliedPointingHandCursor;
};
#endif // KOSELECTIONMANAGER_H
