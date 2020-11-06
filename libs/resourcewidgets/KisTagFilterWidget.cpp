/*
 *    Copyright (C) 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
 *    Copyright (c) 2011 José Luis Vergara <pentalis@gmail.com>
 *    Copyright (c) 2013 Sascha Suelzer <s.suelzer@gmail.com>
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Library General Public
 *    License as published by the Free Software Foundation; either
 *    version 2 of the License, or (at your option) any later version.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Library General Public License for more details.
 *
 *    You should have received a copy of the GNU Library General Public License
 *    along with this library; see the file COPYING.LIB.  If not, write to
 *    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *    Boston, MA 02110-1301, USA.
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
            "<p>Enter search terms here to add resources to, or remove them from, the current tag view.</p>"
            "<p>To filter based on the partial, case insensitive name of a resource:<br/>"
            "<tt>partialname</tt> or <tt>!partialname</tt></p>"
            "<p>To include or exclude other tag sets:<br/>"
            "<tt>[Tagname]</tt> or <tt>![Tagname]</tt></p>"
            "<p>For case sensitive and full name matching in-/exclusion:<br/>"
            "<tt>\"ExactMatch\"</tt> or <tt>!\"ExactMatch\"</tt></p>");

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
