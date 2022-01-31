#ifndef KISMLTPLAYER_H
#define KISMLTPLAYER_H

#include <QObject>
#include <QSharedPointer>
#include <mlt++/MltProducer.h>

class KisMediaConsumer : public QObject
{
    Q_OBJECT

public:
    enum Mode {
        PUSH,
        PULL
    };

    KisMediaConsumer(QObject* parent = nullptr);
    ~KisMediaConsumer();

    void seek(int p_frame);
    int playheadPosition();

    void setFrameRate(int fps);
    int getFrameRate();

    void setMode(Mode setting);
    Mode getMode();

    Mlt::Profile* getProfile();

    void setProducer(QSharedPointer<Mlt::Producer> p_producer);

Q_SIGNALS:
    void sigFrameShow(int frameIndex) const;

private:
    QScopedPointer<struct Private> m_d;
};

#endif // KISMLTPLAYER_H
