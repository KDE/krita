/*
 * SPDX-FileCopyrightText: 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "DlgConfigureHistoryDock.h"

#include <QFormLayout>
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

    QFormLayout *form = new QFormLayout(page);
    QCheckBox *chkCumulative = new QCheckBox(i18n("Enable Cumulative Undo"), page);
    chkCumulative->setChecked(stack->useCumulativeUndoRedo());
    connect(chkCumulative, SIGNAL(toggled(bool)), view, SLOT(toggleCumulativeUndoRedo()));

    form->addRow(chkCumulative, new QWidget(page));

    QLabel *l = new QLabel(i18n("Start merging time"), page);
    QDoubleSpinBox *s = new KisDoubleParseSpinBox(page);
    s->setToolTip(i18nc("@info:tooltip", "The amount of time after a merged stroke before merging again"));
    s->setRange(3,10);
    s->setValue(m_stack->timeT1());
    form->addRow(l, s);
    s->setEnabled(chkCumulative->isChecked());
    connect(chkCumulative, SIGNAL(toggled(bool)), s, SLOT(setEnabled(bool)));
    connect(s, SIGNAL(valueChanged(double)), view, SLOT(setStackT1(double)));

    QLabel *l1 = new QLabel(i18n("Group time"));
    QDoubleSpinBox *s1 = new KisDoubleParseSpinBox();
    s1->setToolTip(i18nc("@info:tooltip", "The amount of time every stroke should be apart from its previous stroke to be classified in one group"));
    s1->setRange(0.3,s->value());
    s1->setValue(m_stack->timeT2());
    form->addRow(l1, s1);
    s1->setEnabled(chkCumulative->isChecked());
    connect(chkCumulative, SIGNAL(toggled(bool)), s1, SLOT(setEnabled(bool)));
    connect(s1, SIGNAL(valueChanged(double)), view, SLOT(setStackT2(double)));

    QLabel *l2 = new QLabel(i18n("Split Strokes"));
    QSpinBox *s2 = new KisIntParseSpinBox();
    s2->setToolTip(i18nc("@info:tooltip", "The number of last strokes which Krita should store separately"));
    s2->setRange(1,m_stack->undoLimit());
    s2->setValue(m_stack->strokesN());
    form->addRow(l2, s2);
    s2->setEnabled(chkCumulative->isChecked());
    connect(chkCumulative, SIGNAL(toggled(bool)), s2, SLOT(setEnabled(bool)));
    connect(s2,SIGNAL(valueChanged(int)),SLOT(view(int)));

    setMainWidget(page);
}
