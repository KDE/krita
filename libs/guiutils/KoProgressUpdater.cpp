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
#include "KoProgressUpdater.h"

#include <QApplication>
#include <QString>
#include <QTimer>
#include <QDebug>

#include "KoUpdaterPrivate.h"
#include "KoUpdater.h"
#include "KoProgressProxy.h"

class KoProgressUpdater::Private {

public:

    Private(KoProgressUpdater* _parent, KoProgressProxy* p)
        : parent( _parent )
        , progressBar(p)
        , totalWeight(0)
        , currentProgress(0)
        , updated( false )
    {
    }

    KoProgressUpdater* parent;
    KoProgressProxy* progressBar;
    int totalWeight;
    int currentProgress;
    bool updated;          // is true whenever the progress needs to be recomputed
    QTimer updateGuiTimer; // fires regulary to update the progress bar widget
    QList<QPointer<KoUpdaterPrivate> > subtasks;

};


KoProgressUpdater::KoProgressUpdater(KoProgressProxy *progressBar)
    : d ( new Private(this, progressBar) )
{
    moveToThread( qApp->thread() );
    qDebug() << "Creating KoProgressUpdater in " << thread() << ", app thread: " << qApp->thread();
    Q_ASSERT(d->progressBar);
    connect( &d->updateGuiTimer, SIGNAL( timeout() ), SLOT( updateUi() ));

}

KoProgressUpdater::~KoProgressUpdater()
{
    qDeleteAll(d->subtasks);
    d->subtasks.clear();
    delete d;
}

void KoProgressUpdater::start(int range, const QString &text)
{
    qDebug() << "KoProgressUpdater::start " << range << ", " << text << " in " << thread();
    qDeleteAll(d->subtasks);
    d->subtasks.clear();

    d->progressBar->setRange(0, range-1);
    d->progressBar->setValue(0);

    if(! text.isEmpty()) {
        d->progressBar->setFormat(text);
    }
    d->totalWeight = 0;
}

QPointer<KoUpdater> KoProgressUpdater::startSubtask(int weight)
{
    qDebug() << "KoProgressUpdater::startSubtask() in " << thread();
    KoUpdaterPrivate *p = new KoUpdaterPrivate(this, weight);
    d->totalWeight += weight;
    d->subtasks.append(p);
    connect( p, SIGNAL( sigUpdated() ), SLOT( update() ) );

    return new KoUpdater(p);
}

void KoProgressUpdater::cancel()
{
    qDebug() << "KoProgressUpdater::cancel in " << thread();
    foreach(KoUpdaterPrivate *updater, d->subtasks) {
        updater->setProgress(100);
        updater->interrupt();
    }
    updateUi();
}

void KoProgressUpdater::update()
{
    d->updated = true;
    qDebug() << "KoProgressUpdater::update(), starting timer in " << thread();
    d->updateGuiTimer.start( 100 ); // 10 updates/second should be enough?

}

void KoProgressUpdater::updateUi() {

    qDebug() << "KoProgressUpdater::updateUi() in " << thread();

    d->updateGuiTimer.stop(); // 10 updates/second should be enough?

    // This function runs in the app main thread. All the progress
    // updates arrive at the KoUpdaterPrivate instances through
    // queued connections, so until we relinguish control to the
    // event loop, the progress values cannot change, and that
    // won't happen until we return from this function (which is
    // triggered by a timer)

    if ( d->updated ) {

        int totalProgress = 0;

        foreach(QPointer<KoUpdaterPrivate> updater, d->subtasks) {

            if(updater->interrupted()) {
                qDebug() << "\tthe updater got interruped, returning";
                d->currentProgress = -1;
                return;
            }

            int progress = updater->progress();

            if(progress > 100 || progress < 0) {
                progress = updater->progress();
            }

            totalProgress += progress * updater->weight();
        }

        d->currentProgress = totalProgress / d->totalWeight;
        d->updated = false;

    }
    qDebug() << "\tupdateUi currentProgress " << d->currentProgress;

    if( d->currentProgress == -1 ) {

        d->progressBar->setValue( d->progressBar->maximum() );
        // should we hide the progressbar after a little while?
        qDebug() << "\t current progress is -1, returning";
        return;
    }

    qDebug() << "\tsetting value!" << d->currentProgress;
    d->progressBar->setValue(d->currentProgress);
}



#include <KoProgressUpdater.moc>
