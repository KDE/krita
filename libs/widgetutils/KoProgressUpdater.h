/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006-2007 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KOPROGRESSUPDATER_H
#define KOPROGRESSUPDATER_H

#include "kritawidgetutils_export.h"

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
 * will be updated correctly and not more often than repaints can occur.
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
 * Doing a smooth.setProgress(50) will move the progress bar to 50% of the share
 * of task 'smooth' which is 5 / 11 of the total and thus to 22.
 *
 * KoProgressUpdater should be created in the main thread;
 * KoProgressProxy must be, if it is gui subclass in the QApplication
 * main thread. The other objects can be created in whatever thread
 * one wants.
 *
 * Also to prevent jumps in the progress-calculation and -display it is recommend
 * to first create all the subtasks and then start to use setProgress on them.
 */
class KRITAWIDGETUTILS_EXPORT KoProgressUpdater : public QObject
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
    explicit KoProgressUpdater(KoProgressProxy *progressProxy, Mode mode = Threaded);

    /**
     * @brief a special constructor for connecting the progress updater to a self-destructable
     * KoUpdater object.
     *
     * HACK ALERT: KoUpdater inherits KoProgressProxy, so be careful when constructing
     *             the updater and check which override is actually used.
     */
    explicit KoProgressUpdater(QPointer<KoUpdater> updater);

    /// destructor
    ~KoProgressUpdater() override;

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
    void start(int range = 100, const QString &text = "");

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
                                     const QString &name = QString(), bool isPersistent = false);

    void removePersistentSubtask(QPointer<KoUpdater> updater);

    /**
     * Cancelling the action will make each subtask be marked as 'interrupted' and
     * set the total progress to 100%.
     */
    void cancel();

    /**
     * @return true when the processing is interrupted
     */
    bool interrupted() const;

    void setUpdateInterval(int ms);
    int updateInterval() const;

    void setAutoNestNames(bool value);
    bool autoNestNames() const;

private Q_SLOTS:

    void update();
    void updateUi();

private:

    class Private;
    Private *const d;

};




#endif

