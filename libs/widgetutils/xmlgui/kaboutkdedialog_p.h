/* This file is part of the KDE libraries
   SPDX-FileCopyrightText: 2007 Urs Wolfer <uwolfer at kde.org>

   Parts of this class have been take from the KAboutKDE class, which was
   SPDX-FileCopyrightText: 2000 Espen Sand <espen@kde.org>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KABOUT_KDE_DIALOG_H
#define KABOUT_KDE_DIALOG_H

#include <QDialog>

namespace KDEPrivate
{

/**
 * @short Standard "About KDE" dialog box
 *
 * This class provides the standard "About KDE" dialog box that is used
 * in KisKHelpMenu. Normally you should not use this class directly, but
 * rather the KisKHelpMenu class or even better just subclass your
 * toplevel window from KisKMainWindow. If you do the latter, the help
 * menu and thereby this dialog box is available through the
 * KisKMainWindow::helpMenu() function.
 *
 * @author Urs Wolfer uwolfer @ kde.org
 * @internal
 */

class KisKAboutKdeDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * Constructor. Creates a fully featured "About KDE" dialog box.
     * Note that this dialog is made modeless in the KisKHelpMenu class so
     * the users may expect a modeless dialog.
     *
     * @param parent The parent of the dialog box. You should use the
     *        toplevel window so that the dialog becomes centered.
     */
    explicit KisKAboutKdeDialog(QWidget *parent = 0);

private:
    class Private;
    Private *const d;
    Q_DISABLE_COPY(KisKAboutKdeDialog)
};

}

#endif
