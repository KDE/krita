/*
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
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
#ifndef KIS_BRUSH_CHOOSER_H_
#define KIS_BRUSH_CHOOSER_H_

#include "kis_itemchooser.h"
#include <QLabel>

class QLabel;
class QCheckBox;

class KisDoubleWidget;

class KisBrushChooser : public KisItemChooser {
    typedef KisItemChooser super;
    Q_OBJECT

public:
    KisBrushChooser(QWidget *parent = 0, const char *name = 0);
    virtual ~KisBrushChooser();

protected:
    virtual void update(QTableWidgetItem *item);

private slots:
    void slotSetItemSpacing(double spacing);
    void slotSetItemUseColorAsMask(bool);

private:
    QLabel *m_lbName;
    QLabel *m_lbSpacing;
    KisDoubleWidget *m_slSpacing;
    QCheckBox *m_chkColorMask;
};

#endif // KIS_BRUSH_CHOOSER_H_

