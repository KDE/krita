/*
 *    This file is part of the KDE project
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

#include <QPushButton>
#include <QAction>
#include <QGridLayout>

#include <klineedit.h>
#include <klocale.h>

#include <KoIcon.h>

#include "KoTagFilterWidget.h"

class KoTagFilterWidget::Private
{
public:
    QString tagSearchBarTooltip_saving_disabled;
    QString tagSearchBarTooltip_saving_enabled;
    KLineEdit* tagSearchLineEdit;
    QPushButton* tagSearchSaveButton;
    QGridLayout* filterBarLayout;
};

KoTagFilterWidget::KoTagFilterWidget(QWidget* parent): QWidget(parent)
,d( new Private())
{
    d->tagSearchBarTooltip_saving_disabled = i18nc (
            "@info:tooltip",
            "<qt>Entering search terms here will add to, or remove resources from the current tag view."
            "<para>To filter based on the partial, case insensitive name of a resource:<br>"
            "<icode>partialname</icode> or <icode>!partialname</icode>.</para>"
            "<para>In-/exclusion of other tag sets:<br>"
            "<icode>[Tagname]</icode> or <icode>![Tagname]</icode>.</para>"
            "<para>Case sensitive and full name matching in-/exclusion:<br>"
            "<icode>\"ExactMatch\"</icode> or <icode>!\"ExactMatch\"</icode>.</para>"
            "Filter results cannot be saved for the <interface>All Presets</interface> view.<br>"
            "In this view, pressing <interface>Enter</interface> or clearing the filter box will restore all items.<br>"
            "Create and/or switch to a different tag if you want to save filtered resources into named sets.</qt>"
            );

    d->tagSearchBarTooltip_saving_enabled = i18nc (
            "@info:tooltip",
            "<qt>Entering search terms here will add to, or remove resources from the current tag view."
            "<para>To filter based on the partial, case insensitive name of a resource:<br>"
            "<icode>partialname</icode> or <icode>!partialname</icode>.</para>"
            "<para>In-/exclusion of other tag sets:<br>"
            "<icode>[Tagname]</icode> or <icode>![Tagname]</icode>.</para>"
            "<para>Case sensitive and full name matching in-/exclusion:<br>"
            "<icode>\"ExactMatch\"</icode> or <icode>!\"ExactMatch\"</icode>.</para>"
            "Pressing <interface>Enter</interface> or clicking the <interface>Save</interface> button will save the changes.</qt>"
            );

    QGridLayout* filterBarLayout = new QGridLayout;


    d->tagSearchLineEdit = new KLineEdit(this);
    d->tagSearchLineEdit->setClearButtonShown(true);
    d->tagSearchLineEdit->setClickMessage(i18n("Enter resource filters here"));
    d->tagSearchLineEdit->setToolTip(d->tagSearchBarTooltip_saving_disabled);
    d->tagSearchLineEdit->setEnabled(true);

    filterBarLayout->setSpacing(0);
    filterBarLayout->setMargin(0);
    filterBarLayout->setColumnStretch(0, 1);
    filterBarLayout->addWidget(d->tagSearchLineEdit, 0, 0);

    d->tagSearchSaveButton = new QPushButton(this);
    d->tagSearchSaveButton->setIcon(koIcon("media-floppy"));
    d->tagSearchSaveButton->setToolTip(i18nc("@info:tooltip", "<qt>Save the currently filtered set as the new members of the current tag.</qt>"));
    d->tagSearchSaveButton->setEnabled(false);

    filterBarLayout->addWidget(d->tagSearchSaveButton, 0, 1);

    connect(d->tagSearchSaveButton, SIGNAL(pressed()),
            this, SLOT(onSaveButtonClicked()));
    connect(d->tagSearchLineEdit, SIGNAL(returnPressed()),
            this, SLOT(onSaveButtonClicked()));
    connect(d->tagSearchLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(onTextChanged(QString)));
    allowSave(false);
    this->setLayout(filterBarLayout);

}

KoTagFilterWidget::~KoTagFilterWidget()
{
    delete d;
}
void KoTagFilterWidget::allowSave(bool allow)
{
    if (allow)  {
        d->tagSearchSaveButton->show();
        d->tagSearchLineEdit->setToolTip(d->tagSearchBarTooltip_saving_enabled);
    }
    else {
        d->tagSearchSaveButton->hide();
        d->tagSearchLineEdit->setToolTip(d->tagSearchBarTooltip_saving_disabled);
    }
}

void KoTagFilterWidget::clear()
{
    d->tagSearchLineEdit->clear();
    d->tagSearchSaveButton->setEnabled(false);
}

void KoTagFilterWidget::onTextChanged(const QString& lineEditText)
{
    d->tagSearchSaveButton->setEnabled(!lineEditText.isEmpty());
    emit filterTextChanged(lineEditText);
}

void KoTagFilterWidget::onSaveButtonClicked()
{
    emit saveButtonClicked();
    clear();
}