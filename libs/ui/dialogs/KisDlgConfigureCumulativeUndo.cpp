/*
 * SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KisDlgConfigureCumulativeUndo.h"

#include <klocalizedstring.h>
#include <lager/state.hpp>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QLabel>
#include <kis_double_parse_spin_box.h>
#include <kis_int_parse_spin_box.h>

#include "KisCumulativeUndoModel.h"
#include "KisWidgetConnectionUtils.h"

#include "kis_config.h"

struct KisDlgConfigureCumulativeUndo::Private
{
    Private(const KisCumulativeUndoData &_data)
        : data(_data)
        , model(data)
    {
    }

    lager::state<KisCumulativeUndoData, lager::automatic_tag> data;
    KisCumulativeUndoModel model;
};

KisDlgConfigureCumulativeUndo::KisDlgConfigureCumulativeUndo(const KisCumulativeUndoData &data,
                                                 int undoLimit,
                                                 QWidget *parent)
    : KoDialog(parent)
    , m_d(new Private(data))
{
    using namespace KisWidgetConnectionUtils;

    setButtons(KoDialog::Ok | KoDialog::Cancel | KoDialog::Default);

    QWidget *page = new QWidget(this);

    QVBoxLayout *vboxLayout = new QVBoxLayout(page);

    QFormLayout *form = new QFormLayout();
    vboxLayout->addLayout(form);

    QDoubleSpinBox *dblMergeTimeout = new KisDoubleParseSpinBox(page);
    dblMergeTimeout->setToolTip(
        i18nc("@info:tooltip",
              "The amount of time during which the strokes will "
              "be kept unmerged. When a stroke becomes old enough, "
              "Krita will try to merge it"));
    dblMergeTimeout->setRange(3, 600);
    dblMergeTimeout->setSuffix(i18nc("suffix for \"seconds\"", " sec"));
    form->addRow(i18n("Wait before merging strokes:"), dblMergeTimeout);

    connectControl(dblMergeTimeout, &m_d->model, "mergeTimeout");

    QSpinBox *intExcludeFromMerge = new KisIntParseSpinBox();
    intExcludeFromMerge->setToolTip(
        i18nc("@info:tooltip",
              "The number of last strokes that Krita will not merge "
              "(even if they are old enough)"));
    intExcludeFromMerge->setRange(1, undoLimit > 0 ? undoLimit : 1000);
    form->addRow(i18n("Exclude last strokes from merge:"), intExcludeFromMerge);

    connectControl(intExcludeFromMerge, &m_d->model, "excludeFromMerge");

    QDoubleSpinBox *dblMaxGroupSeparation = new KisDoubleParseSpinBox();
    dblMaxGroupSeparation->setToolTip(
        i18nc("@info:tooltip",
              "If two strokes have short time interval (delay) between "
              "each other, then they will be considered as belonging to "
              "the same group and merged"));
    dblMaxGroupSeparation->setRange(0.3, 60);
    dblMaxGroupSeparation->setSuffix(i18nc("suffix for \"seconds\"", " sec"));
    form->addRow(i18n("Max interval of grouped strokes:"), dblMaxGroupSeparation);

    connectControl(dblMaxGroupSeparation, &m_d->model, "maxGroupSeparation");

    QDoubleSpinBox *dblMaxGroupDuration = new KisDoubleParseSpinBox();
    dblMaxGroupDuration->setToolTip(
        i18nc("@info:tooltip",
              "Maximum allowed duration of the group after the merge. "
              "If the group is going to be longer than this limit in "
              "the result of the merge, this merge will not happen."));
    dblMaxGroupDuration->setRange(0.3, 600.0);
    dblMaxGroupDuration->setSuffix(i18nc("suffix for \"seconds\"", " sec"));
    form->addRow(i18n("Max group duration:"), dblMaxGroupDuration);

    connectControl(dblMaxGroupDuration, &m_d->model, "maxGroupDuration");

    vboxLayout->addItem(new QSpacerItem(20, 20));

    QLabel *help = new QLabel(
        i18n("Cumulative Undo allows Krita to merge undo actions "
             "and make undo history cleaner. Krita will still keep "
             "a few latest actions unmerged according to "
             "\"Wait before merging strokes\" and \"Exclude last strokes from merge\" "
             "options. Whenever an action gets outdated using "
             "the time limit and this action is not excluded using "
             "\"Exclude last strokes from merge\", Krita will try "
             " to merge this action into a group. The groups are "
             "formed using \"Max group strokes delay\" and "
             "\"Max group duration\" options."),
        page);
    help->setWordWrap(true);
    help->setAlignment(Qt::AlignJustify);
    vboxLayout->addWidget(help);

    vboxLayout->addItem(new QSpacerItem(20, 20));

    setMainWidget(page);

    connect(this, SIGNAL(defaultClicked()), SLOT(slotDefaultClicked()));
}

KisDlgConfigureCumulativeUndo::~KisDlgConfigureCumulativeUndo()
{
}

KisCumulativeUndoData KisDlgConfigureCumulativeUndo::cumulativeUndoData() const
{
    return m_d->data.get();
}

void KisDlgConfigureCumulativeUndo::slotDefaultClicked()
{
    const KisConfig cfg(true);
    m_d->data.set(KisCumulativeUndoData());
}
