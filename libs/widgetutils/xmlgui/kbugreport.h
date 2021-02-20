/* This file is part of the KDE project
   SPDX-FileCopyrightText: 1999 David Faure <faure@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
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
    ~KBugReport() override;


    /**
      * OK has been clicked
     */
    void accept() override;

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

