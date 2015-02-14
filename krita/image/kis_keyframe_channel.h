#ifndef KIS_KEYFRAME_CHANNEL_H
#define KIS_KEYFRAME_CHANNEL_H

#include <QVariant>
#include <krita_export.h>

class KRITAIMAGE_EXPORT KisKeyframeChannel
{

public:
    KisKeyframeChannel(const QString& name, const QString& displayName);

    void setKeyframe(int time, const QVariant& value);
    void deleteKeyframe(int time);
    QVariant getValueAt(int time);

    QList<int> times();

private:

    struct Private;
    Private * const m_d;
};

#endif // KIS_KEYFRAME_CHANNEL_H
