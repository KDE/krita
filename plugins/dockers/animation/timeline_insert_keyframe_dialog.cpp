#include "timeline_insert_keyframe_dialog.h"

#include <QLabel>
#include <QSpinBox>
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

    frameCountSpinbox.setMinimum(1);
    frameCountSpinbox.setValue(1);

    frameTimingSpinbox.setMinimum(1);
    frameTimingSpinbox.setValue(1);

    QWidget *formsWidget = new QWidget();
    QFormLayout *formLayout = new QFormLayout();
    formsWidget->setLayout( formLayout );
    layout()->addWidget(formsWidget);

    QDialogButtonBox *buttonbox = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
    layout()->addWidget(buttonbox);

    formLayout->addRow(QString(i18nc("@label:spinbox", "Number of frames:")), &frameCountSpinbox);
    formLayout->addRow(QString(i18nc("@label:spinbox", "Frame timing:")), &frameTimingSpinbox);

    connect(buttonbox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonbox, SIGNAL(rejected()), this, SLOT(reject()));
}

bool TimelineInsertKeyframeDialog::promptUserSettings(int &out_count, int &out_timing){
    if(exec() == QDialog::Accepted){
        out_count = frameCountSpinbox.value();
        out_timing = frameTimingSpinbox.value();
        return true;
    }
    return false;
}
