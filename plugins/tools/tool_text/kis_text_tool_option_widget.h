/*
 *
 *  Copyright (c) 2011 Sven Langkamp <sven.langkamp@gmail.com>
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


#ifndef KIS_TEXT_TOOL_OPTION_WIDGET_H
#define KIS_TEXT_TOOL_OPTION_WIDGET_H

#include "ui_wdgtextoptions.h"
#include <kis_painter.h>
#include <QButtonGroup>

class KisTextToolOptionWidget : public QWidget, public Ui_WdgTextOptions
{
    Q_OBJECT
public:
    KisTextToolOptionWidget(QWidget* parent = 0);
    QButtonGroup* m_buttonGroup;

    enum TextMode{
        MODE_ARTISTIC,
        MODE_MULTILINE
    };
    
    TextMode mode();
    
    KisPainter::FillStyle style();
private Q_SLOTS:
    void modeChanged(int mode);

};

#endif // KIS_TEXT_TOOL_OPTION_WIDGET_H
