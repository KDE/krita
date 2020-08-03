/*
 *  Copyright (c) 2015 Jouni Pentikäinen <joupent@gmail.com>
 *  Copyright (c) 2020 Emmet O'Neill <emmetoneill.pdx@gmail.com>
 *  Copyright (c) 2020 Eoin O'Neill <eoinoneill1991@gmail.com>
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

#ifndef KIS_KEYFRAME_H
#define KIS_KEYFRAME_H

#include <qglobal.h>
#include <qmetatype.h>
#include <QScopedPointer>

#include "kritaimage_export.h"
#include "kis_types.h"

class KisKeyframeChannel;

/** @brief Krita's base keyframe class.
 * Mainly contained by KisKeyframeChannels.
 * A core part of Krita's animation bankend.
 */
class KRITAIMAGE_EXPORT KisKeyframe {
public:
    KisKeyframe();
    virtual ~KisKeyframe();

    int colorLabel() const;
    void setColorLabel(int colorIndex);

    /** Creates a copy of this keyframe.
     * @param  newChannel  (Optional) The channel that will hold this duplicate.
     * This is used when some action must be taken to insert a frame into a new channel,
     * for example, the registration of a KisRasterKeyframe with the new channel's paint device. */
    virtual KisKeyframeSP duplicate(class KisKeyframeChannel* newChannel = nullptr) = 0;

protected:
    KisKeyframe(const KisKeyframe &rhs);

private:
    struct Private;
    QScopedPointer<Private> m_d;
};

Q_DECLARE_METATYPE(KisKeyframe*)
Q_DECLARE_METATYPE(KisKeyframeSP)
#endif
