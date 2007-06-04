/* This file is part of the KDE project
   Made by Emanuele Tamponi (emanuele@valinor.it)
   Copyright (C) 2007 Emanuele Tamponi

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KIS_PAINTERLY_MIXER_H_
#define KIS_PAINTERLY_MIXER_H_

#include "ui_kis_painterlymixer.h"

class QWidget;

class KisView2;

class KisPainterlyMixer : public QWidget, private Ui::KisPainterlyMixer {
    Q_OBJECT

// protected:
//     bool eventFilter(QObject *obj, QEvent *event);

public:
    KisPainterlyMixer(QWidget* parent, KisView2 *view);
    ~KisPainterlyMixer();

private:
    KisView2 *m_view;

};

#endif // KIS_PAINTERLY_MIXER_H_
