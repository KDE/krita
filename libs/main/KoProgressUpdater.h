/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef KOPROGRESSUPDATER_H
#define KOPROGRESSUPDATER_H

#include "komain_export.h"

#include <QString>
#include <QObject>
#include <QPointer>

class KoUpdater;
class KoProgressProxy;
class QTextStream;
class QTime;

/**
 * Allow multiple subtasks to safely update and report progress.
 * This class is able to update a progress bar with the total progress
 * of a project that may be separated into different subtasks.
 * Each subtask will use one KoUpdater which that subtask can then report
 * progress to.  Each KoUpdater.setProgress() call will automatically calculate
 * the total progress over the whole tasks made and update the progress bar
 * with the total progress so far.
 *
 * This class is created specifically with threading in mind so that subtasks
 * can report their progress from their personal subthread and the progress bar
 * will be updated correctly and not more often then repaints can return.
 *
 * Typical usage can be:
 * @code
  KoProgressUpdater *pu = new KoProgressUpdater(myProgressBar);
  pu->start(100);
  // create the subtasks
  KoUpdater smooth = pu->startSubtask(5);
  KoUpdater scale = pu->startSubtask(5);
  KoUpdater cleanup = pu->startSubtask(1);
  @endcode
 * Doing an smooth.setProgress(50) will move the progress bar to 50% of the share
 * of task 'smooth' which is 5 / 11 of the total and thus to 22.
 *
 *
 * KoProgressUpdater should be created in the main thread;
 * KoProgressProxy must be, if it is gui subclass in the QApplication
 * main thread. The other objects can be created in whatever thread
 * one wants.
 */
class KOMAIN_EXPORT KoProgressUpdater : public QObject
{
    Q_OBJECT
public:

    enum Mode {
        Threaded,
        Unthreaded
    };

    /**
     * Constructor.
     * @param progressBar the progress bar to update.
     */
    KoProgressUpdater(KoProgressProxy *progressBar, Mode mode = Threaded,
                      QTextStream *output = 0);

    /// destructor
    virtual ~KoProgressUpdater();

    /**
     * Start a new task.
     *
     * This will invalidate any previously created subtasks and set
     * the range of the progressBar as well as the text in the
     * progressbar.
     *
     * @param range the total range of progress bar makes.
     * @param text The text to show in the progressBar.
     * @see KoProgressProxy::setRange()
     * @see KoProgressProxy::setFormat()
     */
    void start(int range = 100, const QString &text = QLatin1String("%p%"));

    /**
     * After calling start() you can create any number of Updaters,
     * one for each subtask. @param weight use a weight to specify the
     * weight this subtask has compared to the rest of the subtasks.
     *
     * KoProgressUpdater will delete the KoUpdater instances when a
     * start() is called or when it is deleted. The KoUpdater pointers
     * are packed in a QPointer so you can check whether they have
     * been deleted before dereferencing.
     */
    QPointer<KoUpdater> startSubtask(int weight=1,
                                     const QString &name = QString());

    /**
     * Cancelling the action will make each subtask be marked as 'interrupted' and
     * set the total progress to 100%.
     */
    void cancel();

    /**
     * Set the time with respect to which the progress events are logged.
     */
    void setReferenceTime(const QTime &referenceTime);

    /**
     * Get the time with respect to which the progress events are logged.
     */
    QTime referenceTime() const;

private slots:

    void update();
    void updateUi();

private:

    class Private;
    Private *const d;

};




#endif

