/*
 *  Copyright (c) 2015 Jouni Pentikäinen <mctyyppi42@gmail.com>
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

#ifndef KIS_KEYFRAME_SEQUENCE_H
#define KIS_KEYFRAME_SEQUENCE_H

#include <krita_export.h>
#include "kis_keyframe_channel.h"
#include "kis_node.h"

class KRITAIMAGE_EXPORT KisKeyframeSequence
{

public:

    KisKeyframeSequence(KisNodeWSP node=0);
    ~KisKeyframeSequence();

    KisKeyframeChannel *createChannel(const QString& name, const QString& displayName);
    KisKeyframeChannel *getChannel(const QString& name);

    QList<KisKeyframeChannel*> channels() const;

    KisNodeWSP node();
private:

    struct Private;
    Private * const m_d;

};

#endif // KIS_KEYFRAME_SEQUENCE_H
