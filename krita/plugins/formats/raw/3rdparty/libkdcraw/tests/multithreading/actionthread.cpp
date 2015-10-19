/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date  : 2014-10-17
 * @brief : a class to manage Raw to Png conversion using threads
 *
 * @author Copyright (C) 2011-2015 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 * @author Copyright (C) 2014 by Veaceslav Munteanu
 *         <a href="mailto:veaceslav dot munteanu90 at gmail dot com">veaceslav dot munteanu90 at gmail dot com</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "actionthread.h"

// Qt includes

#include <QFileInfo>
#include <QImage>
#include <QDebug>

// Local includes

#include "kdcraw.h"
#include "ractionjob.h"

class Task : public RActionJob
{
public:

    Task()
        : RActionJob()
    {
    }

    RawDecodingSettings settings;
    QString             errString;
    QUrl                fileUrl;

protected:

    void run()
    {
        emit signalStarted();

        QImage image;

        if (m_cancel) return;

        emit signalProgress(20);

        KDcraw rawProcessor;

        if (m_cancel) return;

        emit signalProgress(30);

        QFileInfo input(fileUrl.toLocalFile());
        QString   fullFilePath(input.baseName() + QString::fromLatin1(".full.png"));
        QFileInfo fullOutput(fullFilePath);

        if (m_cancel) return;

        emit signalProgress(40);

        if (!rawProcessor.loadFullImage(image, fileUrl.toLocalFile(), settings))
        {
            errString = QString::fromLatin1("raw2png: Loading full RAW image failed. Aborted...");
            return;
        }

        if (m_cancel) return;

        emit signalProgress(60);

        qDebug() << "raw2png: Saving full RAW image to "
                 << fullOutput.fileName() << " size ("
                 << image.width() << "x" << image.height()
                 << ")";

        if (m_cancel) return;

        emit signalProgress(80);

        image.save(fullFilePath, "PNG");

        emit signalDone();
    }
};

// ----------------------------------------------------------------------------------------------------

ActionThread::ActionThread(QObject* const parent)
    : RActionThreadBase(parent)
{
}

ActionThread::~ActionThread()
{
}

void ActionThread::convertRAWtoPNG(const QList<QUrl>& list, const RawDecodingSettings& settings, int priority)
{
    RJobCollection collection;

    foreach (const QUrl& url, list)
    {
        Task* const job = new Task();
        job->fileUrl    = url;
        job->settings   = settings;

        connect(job, SIGNAL(signalStarted()),
                this, SLOT(slotJobStarted()));

        connect(job, SIGNAL(signalProgress(int)),
                this, SLOT(slotJobProgress(int)));

        connect(job, SIGNAL(signalDone()),
                this, SLOT(slotJobDone()));

        collection.insert(job, priority);

        qDebug() << "Appending file to process " << url;
    }

    appendJobs(collection);
}

void ActionThread::slotJobDone()
{
    Task* const task = dynamic_cast<Task*>(sender());
    if (!task) return;

    if (task->errString.isEmpty())
    {
        emit finished(task->fileUrl);
    }
    else
    {
        emit failed(task->fileUrl, task->errString);
    }
}

void ActionThread::slotJobProgress(int p)
{
    Task* const task = dynamic_cast<Task*>(sender());
    if (!task) return;

    emit progress(task->fileUrl, p);
}

void ActionThread::slotJobStarted()
{
    Task* const task = dynamic_cast<Task*>(sender());
    if (!task) return;

    emit starting(task->fileUrl);
}