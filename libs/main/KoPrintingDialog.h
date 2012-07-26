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
#ifndef KOPRINTINGDIALOG_H
#define KOPRINTINGDIALOG_H

#include <KoPrintJob.h>
#include <QList>
#include "komain_export.h"

class KoShapeManager;
class KoShape;
class KoPrintingDialogPrivate;

/**
 * Dialog that will allow you to print any flake-based document, with full interaction and ability to stop.
 * Using this class allows any application to print a set of shapes that are contained in the shapeManager()
 * in a thread-safe manner, while ensuring that the full content is present before printing.  A requirement
 * for any shape that may use the network or simply use a lot of processing power to prepare its content
 * before it can be painted.
 * This class is of the type of 'create and forget'. Meaning that you create the dialog, initialize it with
 * data and then call show() on it.  It then continues to print and delete itself when ready.
 * @code
    KoPrintingDialog *dia = new KoPrintingDialog(myWidget);
    dia->printer().setOutputFormat(QPrinter::PdfFormat);
    dia->printer().setOutputFileName("output.pdf");
    dia->printer().setResolution(600);
    dia->printer().setFullPage(true); // ignore printer margins
    dia->setShapeManager(m_canvas->shapeManager())
    QList<int> pages;
    pages.append(1);
    dia->setPageRange(pages);
    dia->startPrinting();
    @endcode
 * The dialog works by looping over all pages in the page-range and sequentially calling preparePage(int) and
 * then using the shapeManager() to actually print the shapes.
 *
 * Since preparePage(int) is pure virtual the application wanting to implement printing should inherit from
 * this class and make sure that after the preparePage returns a simple paint can be called on the shapeManager
 * with the painter().
 *
 * XXX: preparePage(int) is no longer pure virtual! Only KSpread reimplements it -- what should be changed
 *      to the docs? (BSAR)
 *
 * This typically means that the preparePage() makes sure the shapeManager is updated and the correct cliprect
 * is set on the painter().
 */
class KOMAIN_EXPORT KoPrintingDialog : public KoPrintJob {
    Q_OBJECT
public:
    /**
     * Create a new dialog.
     * @param parent the widget this dialog will use as a child.
     */
    KoPrintingDialog(QWidget *parent);
    virtual ~KoPrintingDialog();

    /**
     * Set the shape manager that should be used to print.
     * @param sm the shapeManager used for the next page(s)
     */
    void setShapeManager(KoShapeManager *sm);
    /**
     * Set a list of all the pages that should be used to loop over and print.
     * Note that any calls made to this method after printing started are ignored.
     * @param pages a list of page numbers that the preparePage() gets passed.
     */
    void setPageRange(const QList<int> &pages);

    /**
     * Return the printer used to print.
     * @return the printer used to print.
     */
    QPrinter &printer();

public slots:

    /**
     * @see KoPrintJob::startPrinting
     */
    virtual void startPrinting(RemovePolicy removePolicy = DoNotDelete);

protected:
    /**
     * Reimplement this method to setup the shapeManager and painter and maybe the shapes for
     * printing the passed in page number.  The printing itself will not happen in this method.
     * This method will be called in a thread that is not the main-thread. So the processing can take
     * a reasonably long time within posing problems for user interaction.
     * @param pageNumber the number of the page to prepare.
     * @see isStopped() printPage()
     * @returns a cliprect. If the rect is valid then it will be set on the painter right after
     *   newPage is called.
     */
    virtual QRectF preparePage(int pageNumber);

    /**
     * This is a similar method to preparePage(), but is guaranteed to be called in the Ui thread.
     * @param pageNumber the number of the page to prepare.
     * @see isStopped()
     */
    virtual void printPage(int pageNumber, QPainter &painter);

    /**
     * Implement to return the shapes on the requested page.
     */
    virtual QList<KoShape*> shapesOnPage(int pageNumber) = 0;

    /**
     * @returns the shapeManager.
     * Retuns the shapeManager as it has been set on the setShapeManager()
     * @see setShapeManager
     */
    KoShapeManager *shapeManager() const;

    /**
     * Return the painter that will be used to print the shape data.
     */
    QPainter &painter() const;

    /**
     * Return true if the user pressed stop.
     * It is suggested to query this setting in long loops and abort the process as soon at it retuns yes.
     */
    bool isStopped() const;

    /**
     * This virtual hook is called at the end of the printing process, either on success of on failure.
     * The method is empty by default.
     */
    virtual void printingDone() { }

private:
    KoPrintingDialogPrivate * const d;
    friend class KoPrintingDialogPrivate;
    Q_PRIVATE_SLOT(d, void preparePage(const QVariant &page))
    Q_PRIVATE_SLOT(d, void printPage(const QVariant &page))
    Q_PRIVATE_SLOT(d, void stopPressed())
};





#endif

