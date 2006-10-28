/*
 *  Copyright (c) 2006 Thomas Zander <zander@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef PROGRESSUPDATER_H
#define PROGRESSUPDATER_H

#include <QList>
#include <QString>
#include <QMutex>
#include <QPointer>

class QProgressBar;
class KisUpdater;
class KisUpdaterPrivate;
class KoAction;

/**
 * Allow multiple subtasks to safely update and report progress.
 * This class is able to update a progress bar with the total progress
 * of a project that may be separated into different subtasks.
 * Each subtask will use one KisUpdater which that subtask can then report
 * progress to.  Each KisUpdater.setProgress() call will automatically calculate
 * the total progress over the whole tasks made and update the progress bar
 * with the total progress so far.
 *
 * This class is created specifically with threading in mind so that subtasks
 * can report their progress from their personal subthread and the progress bar
 * will be updated correctly and not more often then repaints can return.
 *
 * Typical usage can be:
 * @code
  KisProgressUpdater *pu = new KisProgressUpdater(myProgressBar);
  pu->start(100);
  // create the subtasks
  KisUpdater smooth = pu->startSubtask(5);
  KisUpdater scale = pu->startSubtask(5);
  KisUpdater cleanup = pu->startSubtask(1);
  @endcode
 * Doing an smooth.setProgress(50) will move the progress bar to 50% of the share
 * of task 'smooth' which is 5 / 11 of the total and thus to 22.
 */
class KisProgressUpdater : public QObject {
    Q_OBJECT
public:
    /**
     * Constructor.
     * @param progressBar the progress bar to update.
     */
    KisProgressUpdater(QProgressBar *progressBar);
    /// destructor
    virtual ~KisProgressUpdater();

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
    KisUpdater startSubtask(int weight=1);

    /**
     * Cancelling the action will make each subtask be marked as 'interrupted' and
     * set the total progress to 100%.
     */
    void cancel();

protected:
    //friend class KisUpdater;
    friend class KisUpdaterPrivate;
    void scheduleUpdate();

private slots:
    /// called by the action based on a scheduleUpdate
    void update();
    /// called by the action based on a scheduleUpdate
    void updateUi();

private:
    QProgressBar *m_progressBar;
    QList<KisUpdaterPrivate*> m_subtasks;
    int m_totalWeight;
    int m_currentProgress; // used for the update and updateUi methods. Don't use elsewhere
    QMutex m_lock; // protects access to m_subtasks
    KoAction *m_action;
};

/**
 * An Kisupdater is a helper for keeping the progress of each subtask up to speed.
 * This class is not thread safe, and it should only be used from one thread.
 * The thread it is used in can be different from any other subtask or the
 * KisProgressUpdater, though.
 * @see KisProgressUpdater::startSubtask()
 */
class KisUpdater {
public:
    /// copy constructor.
    KisUpdater(const KisUpdater &other);
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
    friend class KisProgressUpdater;
    KisUpdater(KisUpdaterPrivate *p);

private:
    QPointer<KisUpdaterPrivate> d;
};


#endif
