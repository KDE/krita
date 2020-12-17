/*
 *  SPDX-FileCopyrightText: 2004 Boudewijn Rempt (boud@valdyas.org)
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
