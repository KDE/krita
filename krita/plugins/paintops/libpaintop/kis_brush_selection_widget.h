/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_BRUSH_SELECTION_WIDGET_H
#define KIS_BRUSH_SELECTION_WIDGET_H

#include <QWidget>
#include "kis_brush.h"

class QTabWidget;
class KisAutoBrushWidget;
class KisBrushChooser;
class KisTextBrushChooser;
class KisCustomBrushWidget;
class KisBrush;

class KisView2;

/**
 * Compound widget that collects all the various brush selection widgets.
 */
class PAINTOP_EXPORT KisBrushSelectionWidget : public QWidget
{
    Q_OBJECT

public:
    KisBrushSelectionWidget(QWidget * parent = 0);

    ~KisBrushSelectionWidget();

    KisBrushSP brush();

    void setAutoBrush(bool on);
    void setPredefinedBrushes(bool on);
    void setCustomBrush(bool on);
    void setTextBrush(bool on);

    void setImage(KisImageWSP image);

    void setCurrentBrush(KisBrushSP brush);
    
    void setAutoBrushDiameter(qreal diameter);
    qreal autoBrushDiameter();

signals:

    void sigBrushChanged();

private:

    QTabWidget * m_brushesTab;
    KisAutoBrushWidget * m_autoBrushWidget;
    KisBrushChooser * m_brushChooser;
    KisTextBrushChooser * m_textBrushWidget;
    KisCustomBrushWidget * m_customBrushWidget;

};

#endif
