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

#ifndef KISINPUTCONFIGURATIONPAGE_H
#define KISINPUTCONFIGURATIONPAGE_H

#include <QWidget>
#include <KisKineticScroller.h>

namespace Ui
{
class KisInputConfigurationPage;
}

/**
 * \brief A Configuration Dialog Page to configure the canvas input.
 */
class KisInputConfigurationPage : public QWidget
{
    Q_OBJECT
public:
    KisInputConfigurationPage(QWidget *parent = 0, Qt::WindowFlags f = 0);

public Q_SLOTS:
    void saveChanges();
    void revertChanges();
    void setDefaults();
    void slotScrollerStateChanged(QScroller::State state){KisKineticScroller::updateCursor(this, state);}

private Q_SLOTS:
    void editProfilesButtonClicked();
    void updateSelectedProfile();
    void changeCurrentProfile(const QString &newProfile);

private:
    Ui::KisInputConfigurationPage *ui;

};

#endif // KISINPUTCONFIGURATIONPAGE_H
