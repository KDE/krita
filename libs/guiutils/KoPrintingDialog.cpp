/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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
#include "KoPrintingDialog.h"
#include "KoProgressUpdater.h"

#include <KoAction.h>
#include <KoZoomHandler.h>
#include <KoShapeManager.h>

// #include <KDebug>
#include <KLocale>
#include <QPainter>
#include <QPrinter>
#include <QGridLayout>
#include <QLabel>
#include <QProgressBar>

class KoPrintingDialog::Private {
public:
    Private(KoPrintingDialog *dia)
        : parent(dia),
        stop(false),
        shapeManager(0),
        painter(0),
        printer(new QPrinter()),
        index(0),
        progress(0)
    {
        action = new KoAction(parent);
        QObject::connect(action, SIGNAL(triggered (const QVariant&)),
                parent, SLOT(preparePage(const QVariant&)), Qt::DirectConnection);
        QObject::connect(action, SIGNAL(updateUi (const QVariant&)),
                parent, SLOT(printPage(const QVariant&)), Qt::DirectConnection);
    }

    ~Private() {
        delete printer;
    }

    void preparePage(const QVariant &page) {
        const int pageNumber = page.toInt();
        KoUpdater updater = updaters.at(index-1);
        painter->save();
        if(! stop)
            parent->preparePage(pageNumber);
        updater.setProgress(50);

        // here we should create a set of progress objects and give one
        // to each of the KoSHape::waitUntilReady() objects; calling the 'finish()' when done.
        QList<KoShape*> shapes = parent->shapesOnPage(pageNumber);
        const int progressPart = 50 / shapes.count();
        foreach(KoShape *shape, shapes) {
            ; // shape->waitUntilReady();
            updater.setProgress(updater.progress() + progressPart);
        }
        updater.setProgress(100);
    }

    void printPage(const QVariant &page) {
        const int pageNumber = page.toInt();
        if(! stop)
            shapeManager->paint( *painter, zoomer, true );
        painter->restore(); // matching the one in preparePage above

        if(!stop && index < pages.count()) {
            printer->newPage();
            action->execute(pages[index++]);
            return;
        }

        // printing done!
        painter->end();
        progress->cancel();
        parent->printingDone();
        parent->done(0);
    }

    void cancelPressed() {
        stop = true;
        progress->cancel();
    }

    KoZoomHandler zoomer;
    KoAction *action;
    KoPrintingDialog *parent;
    bool stop;
    KoShapeManager *shapeManager;
    QPainter *painter;
    QPrinter *printer;
    int index; // index in the pages list.
    KoProgressUpdater *progress;
    QLabel *pageNumber;
    QList<int> pages;
    QList<KoUpdater> updaters;
};

KoPrintingDialog::KoPrintingDialog(QWidget *parent)
    : QDialog(parent),
    d(new Private(this))
{
    setModal(true);
    setAttribute(Qt::WA_DeleteOnClose, true);
    QGridLayout *grid = new QGridLayout(this);
    setLayout(grid);
    QLabel *label = new QLabel(i18n("Printing"), this);
    grid->addWidget(label, 0, 0);
    d->pageNumber = new QLabel(this);
    grid->addWidget(label, 1, 0);
    QProgressBar *bar = new QProgressBar(this);
    grid->addWidget(bar, 0, 1, 2, 1);
    d->progress = new KoProgressUpdater(bar);
}

KoPrintingDialog::~KoPrintingDialog()
{
    delete d;
}

void KoPrintingDialog::setShapeManager(KoShapeManager *sm) {
    d->shapeManager = sm;
}

void KoPrintingDialog::setPageRange(const QList<int> &pages) {
    if(d->index == 0) // can't change after we started
        d->pages = pages;
}

KoShapeManager *KoPrintingDialog::shapeManager() const {
    return d->shapeManager;
}

QPainter & KoPrintingDialog::painter() const {
    Q_ASSERT(d->painter);
    return *d->painter;
}

bool KoPrintingDialog::isCancelled() const {
    return d->stop;
}

void KoPrintingDialog::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);
    if(d->index == 0 && d->pages.count() > 0 && d->printer) {
        d->painter = new QPainter(d->printer);
        d->zoomer.setZoomAndResolution(100, d->printer->resolution(), d->printer->resolution());
        d->progress->start();
        for(int i=0; i < d->pages.count(); i++)
            d->updaters.append(d->progress->startSubtask()); // one per page
        d->action->execute(d->pages[d->index++]);
    }
}

QPrinter &KoPrintingDialog::printer() {
    return *d->printer;
}

#include <KoPrintingDialog.moc>
