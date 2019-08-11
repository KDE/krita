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

#ifndef __TIMELINE_INSERT_KEYFRAME_DIALOG_H
#define __TIMELINE_INSERT_KEYFRAME_DIALOG_H

#include "kritaanimationdocker_export.h"
#include <QDialog>
#include <QSpinBox>
#include <QRadioButton>

enum TimelineDirection : short;

class KRITAANIMATIONDOCKER_EXPORT TimelineInsertKeyframeDialog : QDialog {
    Q_OBJECT
private:
    QSpinBox frameCountSpinbox;
    QSpinBox frameTimingSpinbox;

    QRadioButton *leftBefore;
    QRadioButton *rightAfter;

public:
    TimelineInsertKeyframeDialog(QWidget *parent = 0);

    bool promptUserSettings(int &count, int &timing, TimelineDirection &out_direction);

    int defaultTimingOfAddedFrames() const;
    void setDefaultTimingOfAddedFrames(int value);

    int defaultNumberOfHoldFramesToRemove() const;
    void setDefaultNumberOfHoldFramesToRemove(int value);

};

#endif // __TIMELINE_INSERT_KEYFRAME_DIALOG_H
