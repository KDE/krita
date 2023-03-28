/*
 * SPDX-FileCopyrightText: 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "DlgConfigureHistoryDock.h"

#include <QFormLayout>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QLabel>
#include <kis_double_parse_spin_box.h>
#include <kis_int_parse_spin_box.h>

DlgConfigureHistoryDock::DlgConfigureHistoryDock(KisUndoView *view, KUndo2QStack *stack, QWidget *parent)
    : KoDialog(parent)
    , m_stack(stack)
{
    setButtons(KoDialog::Close);

    QWidget *page = new QWidget(this);

    QVBoxLayout *vboxLayout = new QVBoxLayout(page);

    QFormLayout *form = new QFormLayout();
    vboxLayout->addLayout(form);

    QCheckBox *chkCumulative = new QCheckBox(i18n("Enable Cumulative Undo"), page);
    chkCumulative->setChecked(stack->useCumulativeUndoRedo());
    connect(chkCumulative, SIGNAL(toggled(bool)), view, SLOT(toggleCumulativeUndoRedo()));
    form->addRow(chkCumulative, new QWidget(page));

    QLabel *l = new QLabel(i18n("Wait before merging strokes:"), page);
    QDoubleSpinBox *s = new KisDoubleParseSpinBox(page);
    s->setToolTip(i18nc("@info:tooltip", "The amount of time during which the strokes will be kept unmerged. When a stroke becomes old enough, Krita will try to merge it"));
    s->setRange(3,60);
    s->setValue(m_stack->timeT1());
    s->setSuffix(i18n(" sec"));
    form->addRow(l, s);
    s->setEnabled(chkCumulative->isChecked());
    connect(chkCumulative, SIGNAL(toggled(bool)), s, SLOT(setEnabled(bool)));
    connect(s, SIGNAL(valueChanged(double)), view, SLOT(setStackT1(double)));

    QLabel *l2 = new QLabel(i18n("Exclude last strokes from merge:"));
    QSpinBox *s2 = new KisIntParseSpinBox();
    s2->setToolTip(i18nc("@info:tooltip", "The number of last strokes that Krita will not merge (even if they are old enough)"));
    s2->setRange(1, m_stack->undoLimit() > 0 ? m_stack->undoLimit() : 100);
    s2->setValue(m_stack->strokesN());
    form->addRow(l2, s2);
    s2->setEnabled(chkCumulative->isChecked());
    connect(chkCumulative, SIGNAL(toggled(bool)), s2, SLOT(setEnabled(bool)));
    connect(s2, SIGNAL(valueChanged(int)), view, SLOT(setStackN(int)));

    QLabel *l1 = new QLabel(i18n("Max group strokes delay:"));
    QDoubleSpinBox *s1 = new KisDoubleParseSpinBox();
    s1->setToolTip(i18nc("@info:tooltip", "If two strokes have short time interval (delay) between each other, then they will be considered as belonging to the same group and merged"));
    s1->setRange(0.3,s->value());
    s1->setValue(m_stack->timeT2());
    s1->setSuffix(i18n(" sec"));
    form->addRow(l1, s1);
    s1->setEnabled(chkCumulative->isChecked());
    connect(chkCumulative, SIGNAL(toggled(bool)), s1, SLOT(setEnabled(bool)));
    connect(s1, SIGNAL(valueChanged(double)), view, SLOT(setStackT2(double)));


    QLabel *l3 = new QLabel(i18n("Max group duration:"));
    QDoubleSpinBox *s3 = new KisDoubleParseSpinBox();
    s3->setToolTip(i18nc("@info:tooltip",
                         "Maximum allowed duration of the group after the merge. "
                         "If the group is going to be longer than this limit in "
                         "the result of the merge, this merge will not happen."));
    s3->setRange(0.3, 10000.0);
    s3->setValue(m_stack->maxGroupDuration() / 1000.0);
    s3->setSuffix(i18n(" sec"));
    form->addRow(l3, s3);
    s3->setEnabled(chkCumulative->isChecked());
    connect(chkCumulative, SIGNAL(toggled(bool)), s3, SLOT(setEnabled(bool)));
    connect(s3, SIGNAL(valueChanged(double)), view, SLOT(setStackSetMaxGroupSeparation(double)));


    vboxLayout->addItem(new QSpacerItem(20, 20));

    QLabel *help = new QLabel(i18n("Cumulative Undo allows Krita to merge undo actions "
                                   "and make undo history cleaner. Krita will still keep "
                                   "a few latest actions unmerged acoording to "
                                   "\"Wait before merging strokes\" and \"Exclude last strokes from merge\" "
                                   "options. Whenever an action gets outdated using "
                                   "the time limit and this action is not excuded using "
                                   "\"Exclude last strokes from merge\", Krita will try "
                                   " to merge this action into a group. The groups are "
                                   "fomed using \"Max group strokes delay\" and "
                                   "\"Max group duration\" options."),
                              page);
    help->setWordWrap(true);
    help->setAlignment(Qt::AlignJustify);
    vboxLayout->addWidget(help);

    vboxLayout->addItem(new QSpacerItem(20, 20));

    setMainWidget(page);
}
