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

#ifndef MIXERTOOL_H_
#define MIXERTOOL_H_

#include <QPointF>
#include <QString>

#include "kis_tool_freehand.h"

class QPainter;
class KisResourceProvider;
class KoViewConverter;
class KisPaintDevice;
class KisPainterlyOverlay;
class KisPaintLayer;
class MixerCanvas;

class MixerTool : public KisToolFreehand {
    Q_OBJECT

public:
    MixerTool(MixerCanvas *mixer, KisResourceProvider *rp);
    ~MixerTool();

public:
    void setDirty(const QRegion& region);

protected:
    void initPaint(KoPointerEvent *e);
    void endPaint();

    MixerCanvas *m_mixer;
    KisResourceProvider *m_resources;
};


#endif // MIXERTOOL_H_
