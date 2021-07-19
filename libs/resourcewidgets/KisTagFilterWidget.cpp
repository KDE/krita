/*
 *    SPDX-FileCopyrightText: 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
 *    SPDX-FileCopyrightText: 2011 José Luis Vergara <pentalis@gmail.com>
 *    SPDX-FileCopyrightText: 2013 Sascha Suelzer <s.suelzer@gmail.com>
 *
 *    SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KisTagFilterWidget.h"

#include <QPushButton>
#include <QAction>
#include <QGridLayout>
#include <QLineEdit>
#include <QCompleter>
#include <QCheckBox>

#include <klocalizedstring.h>

#include <KoIcon.h>

#include <kis_debug.h>
#include <kconfig.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

class KisTagFilterWidget::Private
{
public:
    QString tagSearchBarTooltip_saving_disabled;
    QString tagSearchBarTooltip_saving_enabled;
    QLineEdit *tagSearchLineEdit;
    QGridLayout *filterBarLayout;
    QCompleter *completer;
    QCheckBox *filterByTagCheckbox;

    QString configGroup {"resources"};
    QString configName {"filterByTagChecked"};

};

KisTagFilterWidget::KisTagFilterWidget(KisTagModel* model, QWidget* parent)
    : QWidget(parent)
    , d(new Private())
{
    QString searchTooltipMaintext = i18nc(
            "@info:tooltip",
            "<p>Enter search terms to filter by name</p>");

    d->tagSearchBarTooltip_saving_disabled = searchTooltipMaintext + i18nc(
            "@info:tooltip",
            "<p>Filter results cannot be saved for the <b>All Presets</b> view. "
            "In this view, pressing <b>Enter</b> or clearing the filter box will restore all items. "
            "Create and/or switch to a different tag if you want to save filtered resources into named sets.</p>");

    d->tagSearchBarTooltip_saving_enabled = searchTooltipMaintext + i18nc(
            "@info:tooltip",
            "<p>Pressing <b>Enter</b> or clicking the <b>Save</b> button will save the changes.</p>");

    QGridLayout* filterBarLayout = new QGridLayout(this);


    d->tagSearchLineEdit = new QLineEdit(this);
    d->tagSearchLineEdit->setClearButtonEnabled(true);
    d->tagSearchLineEdit->setPlaceholderText(i18n("Search"));
    d->tagSearchLineEdit->setToolTip(d->tagSearchBarTooltip_saving_disabled);
    d->tagSearchLineEdit->setEnabled(true);

    d->completer = new QCompleter(model, this);
    d->completer->setCompletionRole(Qt::DisplayRole);
    d->completer->setCaseSensitivity(Qt::CaseInsensitive);
    d->tagSearchLineEdit->setCompleter(d->completer);

    filterBarLayout->setMargin(0);
    filterBarLayout->setColumnStretch(0, 1);
    filterBarLayout->addWidget(d->tagSearchLineEdit, 0, 0);

    d->filterByTagCheckbox = new QCheckBox(this);
    d->filterByTagCheckbox->setText(i18nc("It appears in the checkbox next to the filter box "
                                          "in resources dockers; must be short.", "Filter in Tag"));

    KConfigGroup cfg = KSharedConfig::openConfig()->group(d->configGroup);
    bool filterByTagCheckboxChecked = cfg.readEntry(d->configName, true);
    d->filterByTagCheckbox->setChecked(filterByTagCheckboxChecked);


    filterBarLayout->addWidget(d->filterByTagCheckbox, 0, 1);
    connect(d->tagSearchLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(onTextChanged(QString)));
    connect(d->filterByTagCheckbox, SIGNAL(stateChanged(int)), this, SLOT(slotFilterByTagChanged(int)));
}

KisTagFilterWidget::~KisTagFilterWidget()
{
    delete d;
}

void KisTagFilterWidget::clear()
{
    d->tagSearchLineEdit->clear();
}

bool KisTagFilterWidget::isFilterByTagChecked()
{
    return d->filterByTagCheckbox->isChecked();
}


void KisTagFilterWidget::onTextChanged(const QString& lineEditText)
{
    emit filterTextChanged(lineEditText);
}

void KisTagFilterWidget::slotFilterByTagChanged(int filterByTag)
{
    emit filterByTagChanged(filterByTag == Qt::Checked);
    KConfigGroup cfg = KSharedConfig::openConfig()->group(d->configGroup);
    cfg.writeEntry(d->configName, filterByTag == Qt::Checked);
}
