/* This file is part of the KDE project
 *
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2009 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KO_UPDATER_H
#define KO_UPDATER_H

#include "KoProgressProxy.h"
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
class KOWIDGETS_EXPORT KoUpdater : public QObject, public KoProgressProxy {

    Q_OBJECT

public:

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

public: // KoProgressProxy implementation

    int maximum() const;
    void setValue( int value );
    void setRange( int minimum, int maximum );
    void setFormat( const QString & format );

signals:

    /// emitted whenever the subtask has called cancel on us
    void sigCancel();

    /// emitted whenever the subtask has called setProgress on us
    void sigProgress( int percent );

protected:

    friend class KoProgressUpdater;
    KoUpdater(KoUpdaterPrivate *p);

public:

    QPointer<KoUpdaterPrivate> d;
    int range;
    int min;
    int max;

private slots:

    void interrupt();

private:

    bool m_interrupted;
    int  m_progressPercent;
};


typedef QPointer<KoUpdater> KoUpdaterPtr;

#endif
