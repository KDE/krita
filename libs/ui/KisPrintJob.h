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

#ifndef KISPRINTJOB_H
#define KISPRINTJOB_H

#include <QObject>
#include <QList>
#include <QAbstractPrintDialog>
#include <QPrinter>

#include "kritaui_export.h"

#include <kis_types.h>

/**
 * A print job is an interface that the KisView uses to create an application-specific
 * class that can take care of printing.
 * The printjob should be able to print again after a print job has been completed,
 * using the same QPrinter to allow the user to alter settings on the QPrinter and
 * call print again.
 * The printjob can thus see startPrinting() called more than once, and the implementation
 * of that signal should honor the removePolicy passed to it.
 */
class KRITAUI_EXPORT KisPrintJob : public QObject
{
    Q_OBJECT
public:
    /**
     * Constructor.
     * @param image the image that is passed for management purposes.
     */
    explicit KisPrintJob(KisImageWSP image);
    ~KisPrintJob() override;

    /// A policy to allow the printjob to delete itself after its done printing.
    enum RemovePolicy {
        DeleteWhenDone, ///< Delete the job when its done with printing.
        DoNotDelete     ///< Keep the job around so it can be started again.
    };

    /// Returns the printer that is used for this print job so others can alter the details of the print-job.
     QPrinter &printer() { return m_printer; }

     int documentFirstPage() const {
        return 1;
    }
     int documentLastPage() const {
        return 1;
    }
     int documentCurrentPage() const {
        return 1;
    }

     QAbstractPrintDialog::PrintDialogOptions printDialogOptions() const;

    /**
     *@brief Check if the painter can print to the printer
     *@returns true if the print job can print to the given printer
     */
     bool canPrint();

public Q_SLOTS:
    /**
     * This is called every time the job should be executed.
     * When called the document should be printed a new painter using the printer
     * of this printJob in order to honor the settings the user made on the printer.
     * canPrint() should be called before startPrinting to check if the painter can print
     * to the printer
     * @param removePolicy a policy that should be honored so the caller can make sure
     *   this job doesn't leak memory after being used.
     */
     void startPrinting(RemovePolicy removePolicy = DoNotDelete);
private:
    KisImageWSP m_image;
    QPrinter m_printer;
};

#endif
