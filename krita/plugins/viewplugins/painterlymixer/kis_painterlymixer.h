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

#include <QColor>

#include "ui_kis_painterlymixer.h"

class QButtonGroup;
class QToolButton;
class QWidget;

class KisView2;

class MixerTool;

class KisPainterlyMixer : public QWidget, private Ui::KisPainterlyMixer {
    Q_OBJECT

public:
    KisPainterlyMixer(QWidget* parent, KisView2 *view);
    ~KisPainterlyMixer();

private:

    void initTool();
    void initSpots();

    void setupButton(QToolButton *button, QRgb color);

private slots:
    void changeColor(int index);

private:
    KisView2 *m_view;
    MixerTool *m_tool;

    QButtonGroup *m_bgColors;

    QToolButton *m_bWet;
    QToolButton *m_bDry;

    KoCanvasResourceProvider *m_resources;

};

#endif // KIS_PAINTERLY_MIXER_H_
