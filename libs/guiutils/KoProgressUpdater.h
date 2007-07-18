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

#include "koguiutils_export.h"

#include <QList>
#include <QString>
#include <QMutex>
#include <QPointer>

class QProgressBar;
class KoUpdater;
class KoProgressUpdaterPrivate;
class KoAction;

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
 */
class KOGUIUTILS_EXPORT KoProgressUpdater : public QObject {
    Q_OBJECT
public:
    /**
     * Constructor.
     * @param progressBar the progress bar to update.
     */
    KoProgressUpdater(QProgressBar *progressBar);
    /// destructor
    virtual ~KoProgressUpdater();

    /**
     * Start a new task.
     * This will invalidate any previously created subtasks and set the range of
     * the progressBar as well as the text in the progressbar.
     * @param range the total range of progress bar makes.
     * @param text The text to show in the progressBar.
     * @see QProgressBar::setRange()
     * @see QProgressBar::setFormat()
     */
    void start(int range = 100, const QString &text = "%p%");

    /**
     * After calling start() you can create any number of Updaters, one for each subtask.
     * @param weight use a weight to specify the weight this subtask has compared
     * to the rest of the subtasks.
     */
    KoUpdater startSubtask(int weight=1);

    /**
     * Cancelling the action will make each subtask be marked as 'interrupted' and
     * set the total progress to 100%.
     */
    void cancel();

protected:
    friend class KoProgressUpdaterPrivate;
    void scheduleUpdate();

private:
    class Private;
    Private * const d;
    Q_PRIVATE_SLOT(d, void update())
    Q_PRIVATE_SLOT(d, void updateUi())
};

/**
 * An KoUpdater is a helper for keeping the progress of each subtask up to speed.
 * This class is not thread safe, and it should only be used from one thread.
 * The thread it is used in can be different from any other subtask or the
 * KoProgressUpdater, though.
 * @see KoProgressUpdater::startSubtask()
 */
class KOGUIUTILS_EXPORT KoUpdater {
public:
    /// copy constructor.
    KoUpdater(const KoUpdater &other);
    /**
     * Call this when this subtask wants to abort all the actions.
     */
    void cancel();
    /**
     * Update your progress. Progress is always from 0 to 100.
     * The global progress shown to the user is determined by the total
     * amount of subtasks there are. This means that each subtasks can just
     * report its own private progress in the range from 0 to 100.
     */
    void setProgress(int percent);

    /**
     * return true when this task should stop processing immediately.
     * When the task has been cancelled all the subtasks will get interrupted
     * and should stop working.  It is therefor important to have repeated calls to
     * this method at regular (time) intervals and as soon as the method returns true
     * cancel the subtask.
     * @return true when this task should stop processing immediately.
     */
    bool interrupted() const;
    /**
     * return the progress this subtask has made.
     */
    int progress() const;

protected:
    friend class KoProgressUpdater;
    KoUpdater(KoProgressUpdaterPrivate *p);

private:
    QPointer<KoProgressUpdaterPrivate> d;
};

#endif

