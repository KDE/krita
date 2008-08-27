/* This file is part of the KDE project

   Copyright (C) 2007 Emanuele Tamponi <emanuele@valinor.it>

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

class QButtonGroup;
class KisPaintOp;
class KisCanvasResourceProvider;
class KisView2;
class KoColor;
class KoColorSpace;
class MixerCanvas;
class MixerTool;

class KisPainterlyMixer : public QWidget, private Ui::KisPainterlyMixer
{
    Q_OBJECT

public:
    KisPainterlyMixer(QWidget* parent, KisView2 *view);
    ~KisPainterlyMixer();

private:

    void initCanvas();
    void initTool();
    void initSpots();

    void loadColors();

private slots:
    void slotChangeColor(int index);

private:
    KisView2 *m_view;
    KisCanvasResourceProvider *m_resources;
    MixerTool *m_tool;

    QButtonGroup *m_bgColors;
    QList<KoColor> m_vColors;


    const KoColorSpace *m_colorspace;
};

#endif // KIS_PAINTERLY_MIXER_H_
