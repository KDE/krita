#ifndef __TIMELINE_INSERT_KEYFRAME_DIALOG_H
#define __TIMELINE_INSERT_KEYFRAME_DIALOG_H

#include "kritaanimationdocker_export.h"
#include <QDialog>
#include <QSpinBox>

class KRITAANIMATIONDOCKER_EXPORT TimelineInsertKeyframeDialog : QDialog {
    Q_OBJECT
private:
    QSpinBox frameCountSpinbox;
    QSpinBox frameTimingSpinbox;

public:
    TimelineInsertKeyframeDialog(QWidget *parent = 0);

    bool promptUserSettings(int &count, int &timing);
};

#endif // __TIMELINE_INSERT_KEYFRAME_DIALOG_H
