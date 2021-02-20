/*
  *  SPDX-FileCopyrightText: 2004 Cyrille Berger <cberger@cberger.net>
  *
  *  SPDX-License-Identifier: GPL-2.0-or-later
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

private Q_SLOTS:
    void rebuildTextBrush();
    void getFont();

Q_SIGNALS:
    void sigBrushChanged();

private:
    KisBrushSP m_textBrush;
    QFont m_font;
};

#endif
