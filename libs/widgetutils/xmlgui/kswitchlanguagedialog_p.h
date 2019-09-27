/*
 * This file is part of the KDE Libraries
 * Copyright (C) 2007 Krzysztof Lichota (lichota@mimuw.edu.pl)
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
 *
 */

#ifndef _KSWITCHLANGUAGEDIALOG_H_
#define _KSWITCHLANGUAGEDIALOG_H_

#include <QDialog>

namespace KDEPrivate
{

class KSwitchLanguageDialogPrivate;

/**
 * @short Standard "switch application language" dialog box.
 *
 * This class provides "switch application language" dialog box that is used
 * in KHelpMenu
 *
 * @author Krzysztof Lichota (lichota@mimuw.edu.pl)
 * @internal
 */

class KSwitchLanguageDialog : public QDialog
{
    Q_OBJECT

public:
    /**
        * Constructor. Creates a fully featured "Switch application language" dialog box.
        * Note that this dialog is made modeless in the KHelpMenu class so
        * the users may expect a modeless dialog.
        *
        * @param parent The parent of the dialog box. You should use the
        *        toplevel window so that the dialog becomes centered.
        */
    KSwitchLanguageDialog(QWidget *parent = 0);

    ~KSwitchLanguageDialog() override;

protected Q_SLOTS:
    /**
    * Activated when the Ok button has been clicked.
    */
    virtual void slotOk();
    void slotDefault();

    /**
        Called when one of language buttons changes state.
    */
    virtual void languageOnButtonChanged(const QString &);

    /**
        Called to add one language button to dialog.
    */
    virtual void slotAddLanguageButton();

    /**
        Called when "Remove" language button is clicked.
    */
    virtual void removeButtonClicked();

private:
    KSwitchLanguageDialogPrivate *const d;

    friend class KSwitchLanguageDialogPrivate;
};

}

#endif
