/*
 *  SPDX-FileCopyrightText: 2018 Emmet O 'Neill <emmetoneill.pdx@gmail.com>
 *  SPDX-FileCopyrightText: 2018 Eoin O 'Neill <eoinoneill1991@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
