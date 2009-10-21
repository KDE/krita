/*
  *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_TEXT_BRUSH_CHOOSER_H_
#define _KIS_TEXT_BRUSH_CHOOSER_H_

#include "ui_wdgtextbrush.h"

#include <kis_text_brush.h>

class KisTextBrushChooser : public QWidget, public Ui::KisWdgTextBrush
{
    Q_OBJECT

public:

    KisTextBrushChooser(QWidget *parent, const char* name, const QString& caption);

    KisBrushSP brush() {
        return m_textBrush;
    }

    void setBrush(KisBrushSP brush);

private slots:

    void rebuildTextBrush();

    void getFont();

signals:

    void sigBrushChanged();

private:
    KisBrushSP m_textBrush;
    QFont m_font;
};

#endif
