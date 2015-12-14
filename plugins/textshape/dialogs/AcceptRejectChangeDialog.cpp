/* This file is part of the KDE project
 * Copyright (C) 2011 Ganesh Paramasivam <ganesh@crystalfab.com>
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
 */

#include "AcceptRejectChangeDialog.h"
#include <KoChangeTracker.h>
#include <KoChangeTrackerElement.h>

AcceptRejectChangeDialog::AcceptRejectChangeDialog(KoChangeTracker *changeTracker, int changeId)
{
    ui.setupUi(this);
    ui.authorNameLineEdit->setText(changeTracker->elementById(changeId)->getCreator());
    ui.dateLineEdit->setText(changeTracker->elementById(changeId)->getDate());
    KoGenChange::Type changeType = changeTracker->elementById(changeId)->getChangeType();

    if (changeType == KoGenChange::InsertChange) {
        ui.changeTypeLineEdit->setText(QString("Insertion"));
    } else if (changeType == KoGenChange::FormatChange) {
        ui.changeTypeLineEdit->setText(QString("Formatting"));
    } else {
        ui.changeTypeLineEdit->setText(QString("Deletion"));
    }

    connect(ui.acceptButton, SIGNAL(released()), this, SLOT(changeAccepted()));
    connect(ui.rejectButton, SIGNAL(released()), this, SLOT(changeRejected()));
    connect(ui.cancelButton, SIGNAL(released()), this, SLOT(dialogCancelled()));

}

AcceptRejectChangeDialog::~AcceptRejectChangeDialog()
{
}

void AcceptRejectChangeDialog::changeAccepted()
{
    this->done(AcceptRejectChangeDialog::eChangeAccepted);
}

void AcceptRejectChangeDialog::changeRejected()
{
    this->done(AcceptRejectChangeDialog::eChangeRejected);
}

void AcceptRejectChangeDialog::dialogCancelled()
{
    this->done(AcceptRejectChangeDialog::eDialogCancelled);
}
