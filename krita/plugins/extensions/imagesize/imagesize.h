/*
 * imagesize.h -- Part of Krita
 *
 * Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
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
#ifndef IMAGESIZE_H
#define IMAGESIZE_H

#include <QVariant>

#include <kparts/plugin.h>

#include "kis_types.h"

class KisView2;
class KisPainter;

class ImageSize : public KParts::Plugin
{
    Q_OBJECT
public:
    ImageSize(QObject *parent, const QVariantList &);
    virtual ~ImageSize();

private slots:

    void slotImageSize();
    void slotCanvasSize();
    void slotLayerSize();
    void slotSelectionScale();
    void slotNodeChanged(const KisNodeSP);

private:

    KisView2 * m_view;
    KisPainter * m_painter;
    KAction* m_scaleLayerAction;

};

#endif // IMAGESIZE_H
