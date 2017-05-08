/*
 *  Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#ifndef KIS_PAINTOP_LIST_WIDGET_H_
#define KIS_PAINTOP_LIST_WIDGET_H_

#include <kritaui_export.h>
#include "kis_categorized_list_view.h"

class KisPaintOpFactory;
class KisSortedPaintOpListModel;

/**
 * A ListBox filled with the paintops
 */
//*
class KRITAUI_EXPORT KisPaintOpListWidget: public KisCategorizedListView
{
    Q_OBJECT
public:
     KisPaintOpListWidget(QWidget* parent=0, const char* name=0);
    ~KisPaintOpListWidget() override;
    
    QString currentItem() const;
    
    void setPaintOpList(const QList<KisPaintOpFactory*>& list);
    void setCurrent(const KisPaintOpFactory* op);
    void setCurrent(const QString & paintOpId);
    
Q_SIGNALS:
    void activated(const QString&);
    
private Q_SLOTS:
    void slotOpActivated(const QModelIndex& index);
    
protected:
    QString itemAt(int idx) const;

private:
    KisSortedPaintOpListModel *m_model;
};

#endif // KIS_PAINTOP_LIST_WIDGET_H_
