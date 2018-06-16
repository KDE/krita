/*
 *  Copyright (c) 2018 Emmet O'Neill <emmetoneill.pdx@gmail.com>
 *  Copyright (c) 2018 Eoin O'Neill <eoinoneill1991@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "timeline_insert_keyframe_dialog.h"
#include "timeline_frames_view.h"

#include <QLabel>
#include <QGroupBox>
#include <QSpinBox>
#include <QRadioButton>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QFormLayout>
#include <klocalizedstring.h>

TimelineInsertKeyframeDialog::TimelineInsertKeyframeDialog(QWidget *parent) :
    QDialog(parent)
{
    setWindowTitle(i18nc("@title:window","Insert Keyframes"));
    setModal(true);
    setLayout(new QVBoxLayout(this));
    {   // Count and Spacing Forms.
        QWidget *forms = new QWidget(this);
        layout()->addWidget(forms);

        frameCountSpinbox.setMinimum(1);
        frameCountSpinbox.setValue(1);

        frameTimingSpinbox.setMinimum(1);
        frameTimingSpinbox.setValue(1);

        QFormLayout *LO = new QFormLayout(forms);


        LO->addRow(QString(i18nc("@label:spinbox", "Number of frames:")), &frameCountSpinbox);
        LO->addRow(QString(i18nc("@label:spinbox", "Frame timing:")), &frameTimingSpinbox);
    }
    {   // Side Buttons.
        QGroupBox *sideRadioButtons = new QGroupBox(i18nc("@label:group","Side:"), this);
        layout()->addWidget(sideRadioButtons);

        leftBefore = new QRadioButton(i18nc("@label:radio", "Left / Before"), sideRadioButtons);
        rightAfter = new QRadioButton(i18nc("@label:radio", "Right / After"), sideRadioButtons);
        leftBefore->setChecked(true);

        QVBoxLayout *LO = new QVBoxLayout(sideRadioButtons);

        LO->addWidget(leftBefore);
        LO->addWidget(rightAfter);
    }
    {   // Cancel / OK Buttons.
        QDialogButtonBox *buttonbox = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
        layout()->addWidget(buttonbox);

        connect(buttonbox, SIGNAL(accepted()), this, SLOT(accept()));
        connect(buttonbox, SIGNAL(rejected()), this, SLOT(reject()));
    }
}

bool TimelineInsertKeyframeDialog::promptUserSettings(int &out_count, int &out_timing, TimelineDirection &out_direction)
{
    if (exec() == QDialog::Accepted) {
        out_count = frameCountSpinbox.value();
        out_timing = frameTimingSpinbox.value();

        out_direction = TimelineDirection::LEFT; // Default
        if (rightAfter && rightAfter->isChecked()) {
            out_direction = TimelineDirection::RIGHT;
        }

        return true;
    }
    return false;
}
