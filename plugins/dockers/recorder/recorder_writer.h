#ifndef RECORDER_WRITER_H
#define RECORDER_WRITER_H

#include <QThread>
#include <QPointer>

class RecorderConfig;
class KisCanvas2;

class RecorderWriter: public QThread
{
    Q_OBJECT
public:
    RecorderWriter();
    ~RecorderWriter();

    void setCanvas(QPointer<KisCanvas2> canvas);

    bool stop();

Q_SIGNALS:
    void pausedChanged(bool paused);

protected:
    void run() override;
    void timerEvent(QTimerEvent *event) override;

private Q_SLOTS:
    void onImageModified();

private:
    class Private;
    Private *const d;
};

#endif // RECORDER_WRITER_H
