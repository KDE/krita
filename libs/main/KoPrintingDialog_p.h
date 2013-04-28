/* This file is part of the KDE project
 * Copyright (C) 2007, 2009 Thomas Zander <zander@kde.org>
 * Copyright (C) 2011 Boudewijn Rempt <boud@kde.org>
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
#ifndef KoPrintingDialog_p_h
#define KoPrintingDialog_p_h

#include "KoPrintingDialog.h"
#include "KoProgressUpdater.h"

#include <KoZoomHandler.h>
#include <KoShapeManager.h>
#include <KoShape.h>
#include <KoProgressBar.h>
#include <KoUpdater.h>

#include <QCoreApplication>
#include <kdebug.h>
#include <klocale.h>
#include <QPainter>
#include <QPrinter>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QDialog>
#include <QThread>


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

        QPointer<KoUpdater> updater = updaters.at(index - 1);

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
        if (!stop && shapeManager) {
            shapeManager->paint(*painter, zoomer, true);
        }
        painter->restore(); // state before page preparation

        if (parent->property("blocking").toBool()) {
            return;
        }
    }

    void printingDone() {

        // printing done!
        painter->end();
        progress->cancel();
        parent->printingDone();
        pageNumber->setText(i18n("Printing done"));
        button->setText(i18n("Close"));
        stop = true;
        QTimer::singleShot(1200, dialog, SLOT(accept()));
        if (removePolicy == KoPrintJob::DeleteWhenDone) {
            parent->deleteLater();
        }
        else {
            resetValues();
        }

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

    KoPrintingDialog *parent;
    KoZoomHandler zoomer;

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

#endif