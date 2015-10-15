/* This file is part of the KDE project
   Copyright (C) 1999 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef KBUGREPORT_H
#define KBUGREPORT_H

#include <QDialog>
#include <kritawidgetutils_export.h>

class KAboutData;
class KBugReportPrivate;

/**
 * @short A dialog box for sending bug reports.
 *
 * All the information needed by the dialog box
 * (program name, version, bug-report address, etc.)
 * comes from the KAboutData class.
 * Make sure you create an instance of KAboutData and pass it
 * to KCmdLineArgs.
 *
 * \image html kbugreport.png "KDE Bug Report Dialog"
 *
 * @author David Faure <faure@kde.org>
 */
class KRITAWIDGETUTILS_EXPORT KBugReport : public QDialog
{
    Q_OBJECT

public:
    /**
     * Creates a bug-report dialog.
     * Note that you shouldn't have to do this manually,
     * since KHelpMenu takes care of the menu item
     * for "Report Bug..." and of creating a KBugReport dialog.
     */
    explicit KBugReport(const KAboutData &aboutData, QWidget *parent = 0L);

    /**
     * Destructor
     */
    virtual ~KBugReport();


    /**
      * OK has been clicked
     */
    void accept() Q_DECL_OVERRIDE;

private:
    /**
     * Update the url to match the current os, compiler, selected app, etc
     */
    Q_PRIVATE_SLOT(d, void _k_updateUrl())


private:
    friend class KBugReportPrivate;
    KBugReportPrivate *const d;

    Q_DISABLE_COPY(KBugReport)
};

#endif

