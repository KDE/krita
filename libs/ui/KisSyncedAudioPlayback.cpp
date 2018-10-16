#include "KisSyncedAudioPlayback.h"

#include "config-qtmultimedia.h"

#ifdef HAVE_QT_MULTIMEDIA
#include <QtMultimedia/QMediaPlayer>
#else
class QIODevice;

#include <QUrl>

namespace {

    class QMediaPlayer : public QObject {
        Q_OBJECT
    public:

    enum Error
    {
        NoError,
        ResourceError,
        FormatError,
        NetworkError,
        AccessDeniedError,
        ServiceMissingError,
        MediaIsPlaylist
    };

    enum State
    {
        StoppedState,
        PlayingState,
        PausedState
    };

    State state() const { return StoppedState; }

    void play() {}
    void stop() {}

    qint64 position() const { return 0; }
    qreal playbackRate() const { return 1.0; }
    void setPosition(qint64) {}
    void setPlaybackRate(qreal) {}
    void setVolume(int) {}
    void setMedia(const QUrl&, QIODevice * device = 0) { Q_UNUSED(device);}
    QString errorString() const { return QString(); }

    Q_SIGNALS:
    void error(Error value);
    };
}
#endif



#include <QFileInfo>



struct KisSyncedAudioPlayback::Private
{
    QMediaPlayer player;
    qint64 tolerance = 200;
};


KisSyncedAudioPlayback::KisSyncedAudioPlayback(const QString &fileName)
    : QObject(0),
      m_d(new Private)
{
    QFileInfo fileInfo(fileName);
    Q_ASSERT(fileInfo.exists());

    m_d->player.setMedia(QUrl::fromLocalFile(fileInfo.absoluteFilePath()));
    m_d->player.setVolume(50);

    connect(&m_d->player, SIGNAL(error(QMediaPlayer::Error)), SLOT(slotOnError()));
}

KisSyncedAudioPlayback::~KisSyncedAudioPlayback()
{
}

void KisSyncedAudioPlayback::setSoundOffsetTolerance(qint64 value)
{
    m_d->tolerance = value;
}

void KisSyncedAudioPlayback::syncWithVideo(qint64 position)
{
    if (qAbs(position - m_d->player.position()) > m_d->tolerance) {
        m_d->player.setPosition(position);
    }
}

bool KisSyncedAudioPlayback::isPlaying() const
{
    return m_d->player.state() == QMediaPlayer::PlayingState;
}

qint64 KisSyncedAudioPlayback::position() const
{
    return m_d->player.position();
}

void KisSyncedAudioPlayback::setVolume(qreal value)
{
    m_d->player.setVolume(qRound(100.0 * value));
}

void KisSyncedAudioPlayback::setSpeed(qreal value)
{
    if (qFuzzyCompare(value, m_d->player.playbackRate())) return;

    if (m_d->player.state() == QMediaPlayer::PlayingState) {
        const qint64 oldPosition = m_d->player.position();

        m_d->player.stop();
        m_d->player.setPlaybackRate(value);
        m_d->player.setPosition(oldPosition);
        m_d->player.play();
    } else {
        m_d->player.setPlaybackRate(value);
    }
}

void KisSyncedAudioPlayback::play(qint64 startPosition)
{
    m_d->player.setPosition(startPosition);
    m_d->player.play();
}

void KisSyncedAudioPlayback::stop()
{
    m_d->player.stop();
}

void KisSyncedAudioPlayback::slotOnError()
{
#ifdef HAVE_QT_MULTIMEDIA
    emit error(m_d->player.media().canonicalUrl().toLocalFile(), m_d->player.errorString());
#endif
}

#ifndef HAVE_QT_MULTIMEDIA
#include "KisSyncedAudioPlayback.moc"
#endif
