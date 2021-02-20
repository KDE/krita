/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef KISSYNCEDAUDIOPLAYBACK_H
#define KISSYNCEDAUDIOPLAYBACK_H

#include <QScopedPointer>
#include <QObject>

class KisSyncedAudioPlayback : public QObject
{
    Q_OBJECT
public:
    KisSyncedAudioPlayback(const QString &fileName);
    ~KisSyncedAudioPlayback() override;

    void setSoundOffsetTolerance(qint64 value);
    void syncWithVideo(qint64 position);

    bool isPlaying() const;
    qint64 position() const;

    void setVolume(qreal value);

public Q_SLOTS:
    void setSpeed(qreal value);
    void play(qint64 startPosition);
    void stop();

Q_SIGNALS:
    void error(const QString &filename, const QString &message);

private Q_SLOTS:
    void slotOnError();

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISSYNCEDAUDIOPLAYBACK_H
