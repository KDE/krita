/*
 * This file is part of the KDE project
 * SPDX-FileCopyrightText: 2013 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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
    KisInputConfigurationPage(QWidget *parent = 0, Qt::WindowFlags f = Qt::WindowFlags());

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
