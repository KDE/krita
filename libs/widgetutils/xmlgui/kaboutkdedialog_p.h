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
 * in KHelpMenu. Normally you should not use this class directly, but
 * rather the KHelpMenu class or even better just subclass your
 * toplevel window from KMainWindow. If you do the latter, the help
 * menu and thereby this dialog box is available through the
 * KMainWindow::helpMenu() function.
 *
 * @author Urs Wolfer uwolfer @ kde.org
 * @internal
 */

class KAboutKdeDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * Constructor. Creates a fully featured "About KDE" dialog box.
     * Note that this dialog is made modeless in the KHelpMenu class so
     * the users may expect a modeless dialog.
     *
     * @param parent The parent of the dialog box. You should use the
     *        toplevel window so that the dialog becomes centered.
     */
    explicit KAboutKdeDialog(QWidget *parent = 0);

private:
    class Private;
    Private *const d;
    Q_DISABLE_COPY(KAboutKdeDialog)
};

}

#endif
