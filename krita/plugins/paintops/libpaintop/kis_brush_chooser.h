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

#include <QLabel>
#include <krita_export.h>
#include <kis_brush.h>

class QLabel;
class QCheckBox;

class KisDoubleWidget;
class KoResourceItemChooser;
class KoResource;

class PAINTOP_EXPORT KisBrushChooser : public QWidget
{

    Q_OBJECT

public:
    KisBrushChooser(QWidget *parent = 0, const char *name = 0);
    virtual ~KisBrushChooser();

    KisBrushSP brush() {
        return m_brush;
    };

    void setBrush(KisBrushSP);

private slots:

    void slotSetItemSpacing(double);
    void slotSetItemUseColorAsMask(bool);
    void slotActivatedBrush(KoResource *);
    void update(KoResource *);

signals:

    void sigBrushChanged();

private:
    QLabel* m_lbName;
    QLabel* m_lbSpacing;
    KisDoubleWidget* m_slSpacing;
    QCheckBox* m_chkColorMask;
    KisBrushSP m_brush;
    KoResourceItemChooser* m_itemChooser;

};

#endif // KIS_BRUSH_CHOOSER_H_
