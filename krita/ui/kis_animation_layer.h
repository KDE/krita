/*
 *  Copyright (c) 2014 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or(at you option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KIS_ANIMATION_LAYER_H
    #define KIS_ANIMATION_LAYER_H

#include <QObject>
#include <QMap>

#include <kis_node.h>

class KisAnimationFrame;

#include <krita_export.h>
/**
 * @brief The KisAnimationLayer class represents a layer in an animation. It contains
 * zero or more key frames. Each frame has an instance of a KisNode of the right type.
 */
class KRITAUI_EXPORT KisAnimationLayer : public QObject
{
    Q_OBJECT
public:
    explicit KisAnimationLayer(KisNodeSP templateNode, QObject *parent = 0);

    KisNodeSP createFrameAtPosition(int position);

    /// @return the highest frame number this layer contains
    int maxFramePosition() const;

    QString name() const;

    /// #return the nearest from for the given position
    KisAnimationFrame *frameAt(int position) const;

signals:

public slots:

private:

    KisNodeSP m_template; // template to clone new frames from
    QMap<int, KisAnimationFrame*> m_frames; // The positions at which the frames start.

};

#endif // KIS_ANIMATION_LAYER_H

