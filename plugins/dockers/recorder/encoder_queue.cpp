#include "encoder_queue.h"
#include <mutex>
#include <QDir>
#include <QRegularExpression>
#include <QUrl>

void EncoderQueue::setEnable(bool& enabled, const QString& path, const QPointer<KisCanvas2>& m_canvas)
{
    m_isRecording = enabled;
    if (m_isRecording) {
        m_recordPath = path;

        QUrl fileUrl(m_recordPath);

        QString filename = fileUrl.fileName();
        QString dirPath = fileUrl.adjusted(QUrl::RemoveFilename).path();

        QDir dir(dirPath);

        if (!dir.exists()) {
            if (!dir.mkpath(dirPath)) {
                enabled = m_isRecording = false;
                return;
            }
        }

        QFileInfoList images = dir.entryInfoList({filename % "_*.webm"});

        QRegularExpression namePattern("^" % filename % "_([0-9]{7}).webm$");
        m_recordCounter = -1;
        Q_FOREACH (auto info, images) {
            QRegularExpressionMatch match = namePattern.match(info.fileName());
            if (match.hasMatch()) {
                QString count = match.captured(1);
                int numCount = count.toInt();

                if (m_recordCounter < numCount) {
                    m_recordCounter = numCount;
                }
            }
        }

        if (m_canvas) {
            m_recordingCanvas = m_canvas;
            m_shouldStop = false;
            m_emptySemaphore = new Semaphore(5);
            m_fullSemaphore = new Semaphore(0);
            m_head = m_tail = 0;

            for (Payload& p : m_queue) {
                p.m_data = new uint8_t[m_canvas->image()->width() * m_canvas->image()->height() * 4];
            }

            m_thread = new std::thread([&]() {
                QString finalFileName = QString(m_recordPath % "_%1.webm").arg(++m_recordCounter, 7, 10, QChar('0'));
                Encoder* m_encoder = new Encoder();
                m_encoder->init(finalFileName.toStdString().c_str(), m_canvas->image()->width(),
                                m_canvas->image()->height());

                while (!m_shouldStop) {
                    m_fullSemaphore->wait();

                    Payload &p = m_queue[m_tail];

                    if (p.m_command == Payload::Command::Finish) {
                        break;
                    }

                    m_encoder->pushFrame(p.m_data);

                    m_tail = (m_tail + 1) % 5;

                    m_emptySemaphore->notify();
                    static int counter = 0;
                    infoPlugins << "Frame pushed" << counter++;
                }

                if (m_encoder) {
                    m_encoder->finish();
                    delete m_encoder;
                    m_encoder = nullptr;
                }
                infoPlugins << "Thread closed";
            });
        } else {
            enabled = m_isRecording = false;
            return;
        }
    } else {
        m_shouldStop = true;
        m_emptySemaphore->wait();
        Payload &p = m_queue[m_head];
        p.m_command = Payload::Command::Finish;
        m_head = (m_head + 1) % 5;
        m_fullSemaphore->notify();

        m_thread->join();
        delete m_fullSemaphore;
        delete m_emptySemaphore;
        delete m_thread;
        for (Payload& p : m_queue) {
            delete[] p.m_data;
        }
        infoPlugins << "File closed";
    }
}

void EncoderQueue::pushFrame(KisPaintDeviceSP dev, int width, int height)
{
    if (!m_shouldStop) {
        m_emptySemaphore->wait();

        Payload p = m_queue[m_head];
        p.m_command = Payload::Command::Encode;
        dev->readBytes((quint8*)p.m_data, 0, 0, width, height);

        m_head = (m_head + 1) % 5;

        m_fullSemaphore->notify();
    }
}
