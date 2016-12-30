#ifndef KISSYNCEDAUDIOPLAYBACK_H
#define KISSYNCEDAUDIOPLAYBACK_H

#include <QScopedPointer>
#include <QObject>

class KisSyncedAudioPlayback : public QObject
{
    Q_OBJECT
public:
    KisSyncedAudioPlayback(const QString &fileName);
    virtual ~KisSyncedAudioPlayback();

    void setSoundOffsetTolerance(qint64 value);
    void syncWithVideo(qint64 position);

    bool isPlaying() const;

    void setVolume(qreal value);

public Q_SLOTS:
    void setSpeed(qreal value);
    void play(qint64 startPosition);
    void stop();

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISSYNCEDAUDIOPLAYBACK_H
