/* This file is part of the KDE project
 * Copyright (C) 2007, 2009 Thomas Zander <zander@kde.org>
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
#include <KoShape.h>
#include <KoProgressBar.h>
#include <KoUpdater.h>

#include <KDebug>
#include <KLocale>
#include <QPainter>
#include <QPrinter>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QDialog>

class KoPrintingDialogPrivate {
public:
    KoPrintingDialogPrivate(KoPrintingDialog *dia)
        : parent(dia),
        stop(true),
        shapeManager(0),
        painter(0),
        printer(new QPrinter()),
        index(0),
        progress(0),
        dialog(0),
        removePolicy(KoPrintJob::DoNotDelete)
    {
        action = new KoAction(parent);
        QObject::connect(action, SIGNAL(triggered (const QVariant&)),
                parent, SLOT(preparePage(const QVariant&)), Qt::DirectConnection);
        QObject::connect(action, SIGNAL(updateUi (const QVariant&)),
                parent, SLOT(printPage(const QVariant&)), Qt::DirectConnection);
    }

    ~KoPrintingDialogPrivate() {
        stop = true;
        delete progress;
        if (painter && painter->isActive()) {
            painter->end();
        }

        updaters.clear();

        delete printer;
        delete dialog;
    }

    void preparePage(const QVariant &page) {
        const int pageNumber = page.toInt();

        QPointer<KoUpdater> updater = updaters.at(index-1);

        if (painter) {
            painter->save(); // state before page preparation
        }

        QRectF clipRect;

        if (! stop) {
            clipRect = parent->preparePage(pageNumber);
        }

        updater->setProgress(45);

        if (!painter) {
            // force the painter to be created *after* the preparePage since the page
            // size may have been updated there and that doesn't work with the first page
            painter = new QPainter(printer);
            painter->save(); // state before page preparation (2)
        }
        if (index > 1)
            printer->newPage();
        if (clipRect.isValid()) // make sure the clipRect is done *after* the newPage. Required for printPreview
            painter->setClipRect(clipRect);
        updater->setProgress(55);
        painter->save(); // state after page preparation

        QList<KoShape*> shapes = parent->shapesOnPage(pageNumber);
        if (shapes.isEmpty()) {
            kDebug(30004) << "Printing page" << pageNumber << "I notice there are no shapes on this page";
        } else {
            const int progressPart = 45 / shapes.count();
            foreach(KoShape *shape, shapes) {
                kDebug(30004) << "Calling waitUntilReady on shape;" << shape;
                if(! stop)
                    shape->waitUntilReady(zoomer);
                kDebug(30004) << "done";
                updater->setProgress(updater->progress() + progressPart);
            }
        }
        updater->setProgress(100);
    }

    void resetValues() {
        index = 0;
        updaters.clear();
        if (painter && painter->isActive())
            painter->end();
        delete painter;
        painter = 0;
        stop = false;
    }

    void printPage(const QVariant &page) {
        painter->restore(); // state after page preparation
        painter->save();
        parent->printPage(page.toInt(), *painter);
        painter->restore();
        if (!stop && shapeManager)
            shapeManager->paint(*painter, zoomer, true);
        painter->restore(); // state before page preparation

        if (parent->property("blocking").toBool())
            return;
        if (!stop && index < pages.count()) {
            pageNumber->setText(i18n("Printing page %1", QString::number(pages[index])));
            action->execute(pages[index++]);
            return;
        }

        // printing done!
        painter->end();
        progress->cancel();
        parent->printingDone();
        pageNumber->setText(i18n("Printing done"));
        button->setText(i18n("Close"));
        stop = true;
        QTimer::singleShot(1200, dialog, SLOT(accept()));
        if (removePolicy == KoPrintJob::DeleteWhenDone)
            parent->deleteLater();
        else
            resetValues();
    }

    void stopPressed() {
        if (stop) { // pressed a second time.
            dialog->done(0);
            return;
        }
        stop = true;
        progress->cancel();
        parent->printingDone();
        pageNumber->setText(i18n("Stopped"));
        QTimer::singleShot(1200, dialog, SLOT(accept()));
        if (removePolicy == KoPrintJob::DeleteWhenDone)
            parent->deleteLater();
        else
            resetValues();
    }

    KoZoomHandler zoomer;
    KoAction *action;
    KoPrintingDialog *parent;
    volatile bool stop;
    KoShapeManager *shapeManager;
    QPainter *painter;
    QPrinter *printer;
    int index; // index in the pages list.
    KoProgressUpdater *progress;
    QLabel *pageNumber;
    QPushButton *button;
    QList<int> pageRange; ///< user requested list of pages
    QList<int> pages; ///< effecive list of pages
    QList< QPointer<KoUpdater> > updaters;
    QDialog *dialog;
    KoPrintJob::RemovePolicy removePolicy;
};

class PrintDialog : public QDialog {
public:
    PrintDialog(KoPrintingDialogPrivate *d, QWidget *parent)
        : QDialog(parent)
    {
        setModal(true);
        QGridLayout *grid = new QGridLayout(this);
        setLayout(grid);

        d->pageNumber = new QLabel(this);
        d->pageNumber->setMinimumWidth(200);
        grid->addWidget(d->pageNumber, 0, 0, 1, 2);
        KoProgressBar *bar = new KoProgressBar(this);
        d->progress = new KoProgressUpdater(bar);
        grid->addWidget(bar, 1, 0, 1, 2);
        d->button = new QPushButton(i18n("Stop"), this);
        grid->addWidget(d->button, 2, 1);
        grid->setColumnStretch(0, 1);
    }
};


KoPrintingDialog::KoPrintingDialog(QWidget *parent)
    : KoPrintJob(parent),
    d(new KoPrintingDialogPrivate(this))
{
    d->dialog = new PrintDialog(d, parent);

    connect(d->button, SIGNAL(released()), this, SLOT(stopPressed()));
}

KoPrintingDialog::~KoPrintingDialog()
{
    d->stopPressed();
    delete d;
}

void KoPrintingDialog::setShapeManager(KoShapeManager *sm)
{
    d->shapeManager = sm;
}

KoShapeManager *KoPrintingDialog::shapeManager() const
{
    return d->shapeManager;
}

void KoPrintingDialog::setPageRange(const QList<int> &pages)
{
    if (d->index == 0) // can't change after we started
        d->pageRange = pages;
}

QPainter & KoPrintingDialog::painter() const
{
    if (d->painter == 0) {
        d->painter = new QPainter(d->printer);
        d->painter->save(); // state before page preparation (3)
    }
    return *d->painter;
}

bool KoPrintingDialog::isStopped() const
{
    return d->stop;
}

void KoPrintingDialog::startPrinting(RemovePolicy removePolicy)
{
    d->removePolicy = removePolicy;
    d->pages = d->pageRange;
    if (d->pages.isEmpty()) { // auto-fill from min/max
        if (d->printer->fromPage() == 0) { // all pages, no range.
            for (int i=documentFirstPage(); i <= documentLastPage(); i++)
                d->pages.append(i);
        } else {
            for (int i=d->printer->fromPage(); i <= d->printer->toPage(); i++)
                d->pages.append(i);
        }
    }
    if (d->pages.isEmpty()) {
        kWarning(30004) << "KoPrintingDialog::startPrinting: No pages to print, did you forget to call setPageRange()?";
        return;
    }

    const bool blocking = property("blocking").toBool();
    if (d->index == 0 && d->pages.count() > 0 && d->printer) {
        if (!blocking)
            d->dialog->show();
        d->stop = false;
        delete d->painter;
        d->painter = 0;
        d->zoomer.setZoomAndResolution(100, d->printer->resolution(), d->printer->resolution());
        d->progress->start();

        if (d->printer->numCopies() > 1) {
            QList<int> oldPages = d->pages;
            if (d->printer->collateCopies()) { // means we print whole doc at once
                for (int count = 1; count < d->printer->numCopies(); ++count)
                    d->pages.append(oldPages);
            } else {
                d->pages.clear();
                foreach (int page, oldPages) {
                    for (int count = 1; count < d->printer->numCopies(); ++count)
                        d->pages.append(page);
                }
            }
        }
        if (d->printer->pageOrder() == QPrinter::LastPageFirst) {
            QList<int> pages = d->pages;
            d->pages.clear();
            QList<int>::Iterator iter = pages.end();
            do {
                --iter;
                d->pages << *iter;
            } while (iter != pages.begin());
        }

        if (blocking) {
            d->resetValues();
            foreach (int page, d->pages) {
                d->index++;
                d->updaters.append(d->progress->startSubtask()); // one per page
                d->preparePage(page);
                d->printPage(page);
            }
            d->painter->end();
            printingDone();
            d->stop = true;
            d->resetValues();
        } else {
            for (int i=0; i < d->pages.count(); i++)
                d->updaters.append(d->progress->startSubtask()); // one per page
            d->pageNumber->setText(i18n("Printing page %1", QString::number(d->pages[d->index])));
            d->action->execute(d->pages[d->index++]);
        }
    }
}

QPrinter &KoPrintingDialog::printer()
{
    return *d->printer;
}

void KoPrintingDialog::printPage(int, QPainter &)
{
}

QRectF KoPrintingDialog::preparePage(int)
{
    return QRectF();
}

#include <KoPrintingDialog.moc>
