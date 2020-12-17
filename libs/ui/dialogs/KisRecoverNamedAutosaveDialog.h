/*
 *  SPDX-FileCopyrightText: 2020 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_RECOVER_NAMED_AUTOSAVE_DIALOG_H
#define KIS_RECOVER_NAMED_AUTOSAVE_DIALOG_H

#include <QDialog>
#include <QScopedPointer>
#include "kis_types.h"

namespace Ui {
class KisRecoverNamedAutosaveDialog;
}

/**
 * @brief The KisRecoverNamedAutosaveDialog class is a dialog to recover already existing files from autosave
 *
 * When the user saves a file, then works on it a bit more, and then Krita crashes or something else
 * unexpected happens that makes it impossible to close Krita correctly, often there is an autosave left behind
 * in the directory of the file. This dialog allows choosing whether to open the autosaved file or the original file.
 */
class KisRecoverNamedAutosaveDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief The ResultType enum represents three possible decisions for the user
     *
     * OpenAutosave = open the autosaved file
     * OpenMainFile = discard the autosave and open the original file
     * Cancel = Do nothing
     */
    enum ResultType {
        OpenAutosave,
        OpenMainFile,
        Cancel
    };

public:
    /**
     * @brief KisRecoverNamedAutosaveDialog basic constructor
     * @param parent parent widget
     * @param mainFile path to the main file (used to create a thumbnail)
     * @param autosaveFile path to the autosaved file (used to create a thumbnail)
     */
    explicit KisRecoverNamedAutosaveDialog(QWidget *parent = 0, QString mainFile = "", QString autosaveFile = "");
    /**
     * @brief ~KisRecoverNamedAutosaveDialog basic destructor
     */
    ~KisRecoverNamedAutosaveDialog() override;

private Q_SLOTS:
    /**
     * @brief slotOkRequested sets the correct result of the dialog in case the user pressed OK button and closes the dialog
     *
     * This is a slot for button pressed signal for the OK button.
     * It sets the result to either OpenMainFile or OpenAutosave, depending on which radio button is checked.
     * Then it closes the dialog.
     */
    void slotOkRequested();
    /**
     * @brief slotCancelRequested sets the correct result of the dialog in case the user pressed Cancel button and closes the dialog
     *
     * This is a slot for button pressed signal for the Cancel button.
     * It sets the result to Cancel.
     * Then it closes the dialog.
     */
    void slotCancelRequested();


private:
    Ui::KisRecoverNamedAutosaveDialog *ui;

};

#endif // KIS_RECOVER_NAMED_AUTOSAVE_DIALOG_H
