/*
 * This file is part of the KDE project
 * Copyright (C) 2013 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
    KisEditProfilesDialog(QWidget *parent = 0, Qt::WindowFlags flags = 0);
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
