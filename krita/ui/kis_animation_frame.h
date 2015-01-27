/*
 *  Copyright (c) 2014 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License, or (at your option)
 *  any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KIS_ANIMATION_FRAME_H
#define KIS_ANIMATION_FRAME_H

#include <QObject>

#include <kis_node.h>
#include <krita_export.h>

/**
 * A frame contains an instance of a Krita layer or mask.
 */
class KRITAUI_EXPORT KisAnimationFrame : public QObject
{
    Q_OBJECT
public:
    explicit KisAnimationFrame(KisNodeSP node, QObject *parent = 0);

signals:

public slots:

private:
    KisNodeSP m_frameNode;
};

#endif // KIS_ANIMATION_FRAME_H
