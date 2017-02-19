/*
 *    This file is part of the KDE project
 *    Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *    Copyright (c) 2007 Jan Hambrecht <jaham@gmx.net>
 *    Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
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

#include "KoTagFilterWidget.h"

#include <QPushButton>
#include <QAction>
#include <QGridLayout>

#include <klineedit.h>
#include <klocalizedstring.h>

#include <KoIcon.h>

class KoTagFilterWidget::Private
{
public:
    QString tagSearchBarTooltip_saving_disabled;
    QString tagSearchBarTooltip_saving_enabled;
    QLineEdit* tagSearchLineEdit;
    QPushButton* tagSearchSaveButton;
    QGridLayout* filterBarLayout;
};

KoTagFilterWidget::KoTagFilterWidget(QWidget* parent): QWidget(parent)
,d( new Private())
{
    d->tagSearchBarTooltip_saving_disabled = i18nc (
            "@info:tooltip",
            "Entering search terms here will add to, or remove resources from the current tag view."
            "<p>To filter based on the partial, case insensitive name of a resource:<br/>"
            "<tt>partialname</tt> or <tt>!partialname</tt>.</p>"
            "<p>In-/exclusion of other tag sets:<br/>"
            "<tt>[Tagname]</tt> or <tt>![Tagname]</tt>.</p>"
            "<p>Case sensitive and full name matching in-/exclusion:<br/>"
            "<tt>\"ExactMatch\"</tt> or <tt>!\"ExactMatch\"</tt>.</p>"
            "Filter results cannot be saved for the <b>All Presets</b> view.<br/>"
            "In this view, pressing <b>Enter</b> or clearing the filter box will restore all items.<br/>"
            "Create and/or switch to a different tag if you want to save filtered resources into named sets."
            );

    d->tagSearchBarTooltip_saving_enabled = i18nc (
            "@info:tooltip",
            "Entering search terms here will add to, or remove resources from the current tag view."
            "<p>To filter based on the partial, case insensitive name of a resource:<br/>"
            "<tt>partialname</tt> or <tt>!partialname</tt>.</p>"
            "<p>In-/exclusion of other tag sets:<br/>"
            "<tt>[Tagname]</tt> or <tt>![Tagname]</tt>.</p>"
            "<p>Case sensitive and full name matching in-/exclusion:<br/>"
            "<tt>\"ExactMatch\"</tt> or <tt>!\"ExactMatch\"</tt>.</p>"
            "Pressing <b>Enter</b> or clicking the <b>Save</b> button will save the changes."
            );

    QGridLayout* filterBarLayout = new QGridLayout;


    d->tagSearchLineEdit = new QLineEdit(this);
    d->tagSearchLineEdit->setClearButtonEnabled(true);
    d->tagSearchLineEdit->setPlaceholderText(i18n("Search"));
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
