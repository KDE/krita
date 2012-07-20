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

#ifndef KOPRINTJOB_H
#define KOPRINTJOB_H

#include <QObject>
#include <QList>
#include <QAbstractPrintDialog>
#include <QPrinter>

#include "komain_export.h"

class QWidget;

/**
 * A print job is an interface that the KoView uses to create an application-specific
 * class that can take care of printing.
 * The printjob should be able to print again after a print job has been completed,
 * using the same QPrinter to allow the user to alter settings on the QPrinter and
 * call print again.
 * The printjob can thus see startPrinting() called more than once, and the implementation
 * of that signal should honor the removePolicy passed to it.
 */
class KOMAIN_EXPORT KoPrintJob : public QObject
{
    Q_OBJECT
public:
    /**
     * Constructor.
     * @param parent the parent qobject that is passed for memory management purposes.
     */
    KoPrintJob(QObject *parent = 0);
    virtual ~KoPrintJob();

    /// A policy to allow the printjob to delete itself after its done printing.
    enum RemovePolicy {
        DeleteWhenDone, ///< Delete the job when its done with printing.
        DoNotDelete     ///< Keep the job around so it can be started again.
    };

    /// Returns the printer that is used for this print job so others can alter the details of the print-job.
    virtual QPrinter &printer() = 0;
    /// If this print job is used in combination with a printdialog the option widgets this method
    /// retuns will be shown in the print dialog.
    virtual QList<QWidget*> createOptionWidgets() const = 0;

    virtual int documentFirstPage() const {
        return 1;
    }
    virtual int documentLastPage() const {
        return 1;
    }

    virtual QAbstractPrintDialog::PrintDialogOptions printDialogOptions() const;

    /**
     *@brief Check if the painter can print to the printer
     *@returns true if the print job can print to the given printer
     */
    virtual bool canPrint();

public slots:
    /**
     * This is called every time the job should be executed.
     * When called the document should be printed a new painter using the printer
     * of this printJob in order to honor the settings the user made on the printer.
     * canPrint() should be called before startPrinting to check if the painter can print
     * to the printer
     * @param removePolicy a policy that should be honored so the caller can make sure
     *   this job doesn't leak memory after being used.
     */
    virtual void startPrinting(RemovePolicy removePolicy = DoNotDelete);
};

#endif
