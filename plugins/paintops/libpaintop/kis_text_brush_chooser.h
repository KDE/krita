/*
  *  SPDX-FileCopyrightText: 2004 Cyrille Berger <cberger@cberger.net>
  *
  *  SPDX-License-Identifier: GPL-2.0-or-later
  */

#ifndef _KIS_TEXT_BRUSH_CHOOSER_H_
#define _KIS_TEXT_BRUSH_CHOOSER_H_

#include "ui_wdgtextbrush.h"

#include <lager/cursor.hpp>
#include <KisBrushModel.h>

class KisTextBrushModel;

class KisTextBrushChooser : public QWidget, public Ui::KisWdgTextBrush
{
    Q_OBJECT

public:

    KisTextBrushChooser(KisTextBrushModel *model,
                        QWidget *parent);
    ~KisTextBrushChooser();

private Q_SLOTS:
    void updateBrushPreview();
    void getFont();

private:
    struct Private;
    QScopedPointer<Private> m_d;
};

#endif
