/*
 *  Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#ifndef KIS_CMB_PAINTOP_H_
#define KIS_CMB_PAINTOP_H_

#include <krita_export.h>
#include "kcombobox.h"
#include "kis_paintop_factory.h"

class KisPaintOpsModel;
class KCategorizedSortFilterProxyModel;

/**
 * A combobox filled with the paintops
 */

class KRITAUI_EXPORT KisCmbPaintop : public KComboBox
{

    Q_OBJECT

public:

    KisCmbPaintop(QWidget * parent = 0, const char * name = 0);
    virtual ~KisCmbPaintop();

    const QString& currentItem() const;

    void setPaintOpList(const QList<KisPaintOpFactory*>& list);
    void setCurrent(const KisPaintOpFactory* op);
    void setCurrent(const QString & paintOpId);

signals:

    void activated(const QString&);
    void highlighted(const QString&);

private slots:

    void slotOpActivated(int i);
    void slotOpHighlighted(int i);

private:
    // Prevent deprectated Qt3 method from being called. Use setCurrent instead.
    void setCurrentText(const QString & s);
    const QString& itemAt(int idx) const;

    KisPaintOpsModel* m_lastModel;
    KCategorizedSortFilterProxyModel* m_sortModel;
};

#endif
