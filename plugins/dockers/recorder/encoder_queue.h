/*
 *  Copyright (c) 2019 Shi Yan <billconan@gmail.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef ENCODER_QUEUE_H
#define ENCODER_QUEUE_H

#include "encoder.h"
#include <array>
#include <condition_variable>
#include <kis_canvas2.h>
#include <mutex>
#include <thread>
#include <QString>

class Semaphore
{
public:
    Semaphore(int count_ = 0)
        : count(count_)
    {
    }

    inline void notify()
    {
        std::unique_lock<std::mutex> lock(mtx);
        count++;
        cv.notify_one();
    }
    inline void wait()
    {
        std::unique_lock<std::mutex> lock(mtx);
        while (count == 0) {
            cv.wait(lock);
        }
        count--;
    }

private:
    std::mutex mtx;
    std::condition_variable cv;
    int count;
};

class Payload
{
public:
    enum class Command
    {
        Encode,
        Finish
    };
    uint8_t* m_data = nullptr;
    Command m_command = Command::Encode;
};

class EncoderQueue
{
private:
    bool m_isRecording = false;
    QString m_recordPath;
    int m_recordCounter = 0;
    QPointer<KisCanvas2> m_recordingCanvas;
    Semaphore* m_fullSemaphore = nullptr;
    Semaphore* m_emptySemaphore = nullptr;
    std::array<Payload, 5> m_queue;
    std::thread* m_thread = nullptr;
    bool m_shouldStop = false;
    int m_tail = 0;
    int m_head = 0;

public:
    EncoderQueue()
        : m_isRecording(false)
        , m_recordPath()
        , m_recordCounter(0)
        , m_recordingCanvas()
        , m_fullSemaphore(nullptr)
        , m_emptySemaphore(nullptr)
        , m_queue()
        , m_thread(nullptr)
        , m_shouldStop(false)
        , m_tail(0)
        , m_head(0)
    {
    }

    void setEnable(bool& enabled, const QString& path, const QPointer<KisCanvas2>& m_canvas);
    bool isRecording()
    {
        return m_isRecording;
    }
    QPointer<KisCanvas2> recordingCanvas()
    {
        return m_recordingCanvas;
    }
    void pushFrame(KisPaintDeviceSP dev, int width, int height);
};

#endif
