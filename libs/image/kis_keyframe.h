/*
 *  SPDX-FileCopyrightText: 2015 Jouni Pentikäinen <joupent@gmail.com>
 *  SPDX-FileCopyrightText: 2020 Emmet O 'Neill <emmetoneill.pdx@gmail.com>
 *  SPDX-FileCopyrightText: 2020 Eoin O 'Neill <eoinoneill1991@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
class KRITAIMAGE_EXPORT KisKeyframe : public QObject {
    Q_OBJECT
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

private:
    struct Private;
    QScopedPointer<Private> m_d;
};

#endif
