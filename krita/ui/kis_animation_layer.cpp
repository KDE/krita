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
#include "kis_animation_layer.h"

#include <KoColorSpaceConstants.h>

#include <kis_group_layer.h>
#include <kis_paint_layer.h>
#include <kis_clone_layer.h>
#include "kis_shape_layer.h"
#include <kis_adjustment_layer.h>
#include <generator/kis_generator_layer.h>
#include <kis_transparency_mask.h>
#include <kis_filter_mask.h>
#include <kis_selection_mask.h>

#include "kis_animation_frame.h"

#include "klocale.h"

KisAnimationLayer::KisAnimationLayer(KisNodeSP templateNode, QObject *parent)
    : QObject(parent)
    , m_template(templateNode)
{
}

KisNodeSP KisAnimationLayer::createFrameAtPosition(int position)
{
    KisNodeSP node = m_template->clone();

    KisAnimationFrame *frame = new KisAnimationFrame(node, this);
    if (m_frames.contains(position)) {
        delete m_frames.take(position);
    }
    m_frames.insert(position, frame);

    return node;
}

int KisAnimationLayer::maxFramePosition() const
{
    int frameNumber = 0;
    foreach(int i, m_frames.keys()) {
        frameNumber = qMax(frameNumber, i);
    }
    return frameNumber;
}

QString KisAnimationLayer::name() const
{
    return m_template->name();
}

KisAnimationFrame *KisAnimationLayer::frameAt(int position) const
{
    KisAnimationFrame *frame = 0;
    foreach(int pos, m_frames.keys()) {
        if (pos < position) {
            frame = m_frames[pos];
        }
        else {
            break;
        }
    }
    return frame;
}
