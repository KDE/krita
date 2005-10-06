/*
 *  kis_cmb_composite.h - part of KImageShop/Krayon/Krita
 *
 *  Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
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

#ifndef KIS_CMB_COMPOSITE_H_
#define KIS_CMB_COMPOSITE_H_

#include <koffice_export.h>
#include "qcombobox.h"
#include "kis_composite_op.h"

/**
 * A combobox filled with the various composition strategies defined in kis_global.
 *
 * XXX: devise some kind of capabilities database for the various colour strategies
 *
 * enum constant       Description CMYK  RGBA  GRAYA
 * 1    COMPOSITE_OVER Over        X     -     X
 *
 * But that's for later...
 */

class KRITAUI_EXPORT KisCmbComposite : public QComboBox
{
    typedef QComboBox super;

    Q_OBJECT

 public:

    KisCmbComposite(QWidget * parent = 0, const char * name = 0 );
    virtual ~KisCmbComposite();

    KisCompositeOp currentItem() const;

    void setCompositeOpList(const KisCompositeOpList& list);
    void setCurrentItem(const KisCompositeOp& op);
    void setCurrentText(const QString & s);

signals:

    void activated(const KisCompositeOp &);
    void highlighted(const KisCompositeOp &);

private slots:

    void slotOpActivated(int i);
    void slotOpHighlighted(int i);

private:
    KisCompositeOpList m_list;
};

#endif
