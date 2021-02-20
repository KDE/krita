/*
 * This file is part of the KDE project
 * SPDX-FileCopyrightText: 2013 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISEDITPROFILESDIALOG_H
#define KISEDITPROFILESDIALOG_H

#include <KoDialog.h>

/**
 * \brief A dialog that provides facilities to edit all the available profiles.
 *
 */
class KisEditProfilesDialog : public KoDialog
{
    Q_OBJECT
public:
    KisEditProfilesDialog(QWidget *parent = 0, Qt::WindowFlags f = Qt::WindowFlags());
    ~KisEditProfilesDialog() override;

private Q_SLOTS:
    void removeButtonClicked();
    void duplicateButtonClicked();
    void renameButtonClicked();
    void resetButtonClicked();

private:
    class Private;
    Private *const d;
};

#endif // KISEDITPROFILESDIALOG_H
