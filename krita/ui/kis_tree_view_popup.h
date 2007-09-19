/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_TREE_VIEW_POPUP_H_
#define _KIS_TREE_VIEW_POPUP_H_

#include "kis_popup_button.h"

class QAbstractItemModel;
class QModelIndex;

/**
 * This class is a workaround some limitation of QComboBox which doesn't
 * allow to embed a QTreeView
 * (see http://trolltech.com/developer/task-tracker/index_html?method=entry&id=87744
 * and http://trolltech.com/developer/task-tracker/index_html?method=entry&id=109685).
 * Once (if) those bugs are fixed, this class should be removed and should not be 
 * part of Krita's official API.
 */
class KisTreeViewPopup : public KisPopupButton {
    Q_OBJECT
    public:
        KisTreeViewPopup(QWidget* parent);
        void setModel(QAbstractItemModel* model);
    protected slots:
        void setCurrentIndex(const QModelIndex &);
    private:
        struct Private;
        Private* const d;
};


#endif
