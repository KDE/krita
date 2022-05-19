/*
 * This file is part of the KDE Libraries
 * SPDX-FileCopyrightText: 2007 Krzysztof Lichota (lichota@mimuw.edu.pl)
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 *
 */

#ifndef _KSWITCHLANGUAGEDIALOG_H_
#define _KSWITCHLANGUAGEDIALOG_H_

#include <QDialog>

namespace KDEPrivate
{

class KisKSwitchLanguageDialogPrivate;

/**
 * @short Standard "switch application language" dialog box.
 *
 * This class provides "switch application language" dialog box that is used
 * in KisKHelpMenu
 *
 * @author Krzysztof Lichota (lichota@mimuw.edu.pl)
 * @internal
 */

class KisKSwitchLanguageDialog : public QDialog
{
    Q_OBJECT

public:
    /**
        * Constructor. Creates a fully featured "Switch application language" dialog box.
        * Note that this dialog is made modeless in the KisKHelpMenu class so
        * the users may expect a modeless dialog.
        *
        * @param parent The parent of the dialog box. You should use the
        *        toplevel window so that the dialog becomes centered.
        */
    KisKSwitchLanguageDialog(QWidget *parent = nullptr);

    ~KisKSwitchLanguageDialog() override;

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
    KisKSwitchLanguageDialogPrivate *const d;

    friend class KisKSwitchLanguageDialogPrivate;
};

}

#endif
