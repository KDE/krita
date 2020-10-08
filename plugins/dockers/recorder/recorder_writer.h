#ifndef RECORDER_WRITER_H
#define RECORDER_WRITER_H

#include <QThread>
#include <QPointer>

class RecorderConfig;
class KisCanvas2;

struct RecorderWriterSettings
{
    QString outputDirectory;
    int quality;
    int resolution;
    int captureInterval;
};

class RecorderWriter: public QThread
{
    Q_OBJECT
public:
    RecorderWriter();
    ~RecorderWriter();

    void setCanvas(QPointer<KisCanvas2> canvas);
    void setup(const RecorderWriterSettings &settings);

    bool stop();

Q_SIGNALS:
    void pausedChanged(bool paused);
    void prefixChanged(QString prefix);

protected:
    void run() override;
    void timerEvent(QTimerEvent *event) override;

private Q_SLOTS:
    void onImageModified();

private:
    Q_DISABLE_COPY(RecorderWriter)
    class Private;
    Private *const d;
};

#endif // RECORDER_WRITER_H
