#ifndef KIS_KEYFRAME_SEQUENCE_H
#define KIS_KEYFRAME_SEQUENCE_H

#include <krita_export.h>
#include "kis_keyframe_channel.h"

class KRITAIMAGE_EXPORT KisKeyframeSequence
{

public:

    KisKeyframeSequence();
    ~KisKeyframeSequence();

    KisKeyframeChannel* createChannel(const QString& name, const QString& displayName);
    KisKeyframeChannel* getChannel(const QString& name);


private:

    struct Private;
    Private * const m_d;

};

#endif // KIS_KEYFRAME_SEQUENCE_H
