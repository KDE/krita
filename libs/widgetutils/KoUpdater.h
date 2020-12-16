/* This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2006-2007 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KO_UPDATER_H
#define KO_UPDATER_H

#include "KoProgressProxy.h"
#include <QObject>
#include <QPointer>

class KoProgressUpdater;
class KoUpdaterPrivate;


/**
 * An KoUpdater is a helper for keeping the progress of each subtask up to speed.
 * This class is not thread safe, and it should only be used from one thread.
 * The thread it is used in can be different from any other subtask or the
 * KoProgressUpdater, though.
 *
 * It is possible to create a KoProgressUpdater on a KoUpdater for when you
 * need to recursively split up progress reporting. (For instance, when your
 * progress reporting routine can be called by other progress reporting
 * routines.)
 *
 * KoUpdater implements KoProgressProxy because it is possible to recursively
 * create another KoProgressUpdater with an updater as progress proxy.
 *
 * @see KoProgressUpdater::startSubtask()
 */
class KRITAWIDGETUTILS_EXPORT KoUpdater : public QObject, public KoProgressProxy {

    Q_OBJECT

public:
    virtual ~KoUpdater();

    /**
     * Call this when this subtask wants to abort all the actions.
     */
    void cancel();

public Q_SLOTS:
    /**
     * Update your progress. Progress is always from 0 to 100.
     * The global progress shown to the user is determined by the total
     * amount of subtasks there are. This means that each subtasks can just
     * report its own private progress in the range from 0 to 100.
     */
    void setProgress(int percent);

public:
    /**
     * return true when this task should stop processing immediately.
     * When the task has been cancelled all the subtasks will get interrupted
     * and should stop working.  It is therefore important to have repeated calls to
     * this method at regular (time) intervals and as soon as the method returns true
     * cancel the subtask.
     * @return true when this task should stop processing immediately.
     */
    bool interrupted() const;

    /**
     * return the progress this subtask has made.
     */
    int progress() const;

public: // KoProgressProxy implementation

    int maximum() const override;
    void setValue( int value ) override;
    void setRange( int minimum, int maximum ) override;
    void setFormat( const QString & format ) override;
    void setAutoNestedName(const QString &name) override;

Q_SIGNALS:

    /// emitted whenever the subtask has called cancel on us
    void sigCancel();

    /// emitted whenever the subtask has called setProgress on us
    void sigProgress( int percent );

    void sigNestedNameChanged(const QString &value);
    void sigHasValidRangeChanged(bool value);

protected:

    friend class KoUpdaterPrivate;
    KoUpdater(KoUpdaterPrivate *_d);

public:

    QPointer<KoUpdaterPrivate> d;
    int range;
    int min;
    int max;

private Q_SLOTS:

    void setInterrupted(bool value);

private:

    bool m_interrupted;
    int  m_progressPercent;
};

/// An updater that does nothing
class KRITAWIDGETUTILS_EXPORT KoDummyUpdater : public KoUpdater
{
public:
    KoDummyUpdater();
};

#endif

