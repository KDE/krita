/* This file is part of the KDE project
 * Copyright (C) 2007,2010 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoPageLayoutDialog.h"

#include "KoPageLayoutWidget.h"
#include "KoPagePreviewWidget.h"

#include <klocalizedstring.h>
#include <WidgetsDebug.h>

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QTimer>

class Q_DECL_HIDDEN KoPageLayoutDialog::Private
{
public:
    Private() : pageLayoutWidget(0), documentCheckBox(0) {}
    KoPageLayoutWidget *pageLayoutWidget;
    QCheckBox *documentCheckBox;
};


KoPageLayoutDialog::KoPageLayoutDialog(QWidget *parent, const KoPageLayout &layout)
    : KPageDialog(parent)
    , d(new Private)
{
    setWindowTitle(i18n("Page Layout"));
    setFaceType(KPageDialog::Tabbed);

    QWidget *widget = new QWidget(this);
    addPage(widget, i18n("Page"));

    QHBoxLayout *lay = new QHBoxLayout(widget);

    d->pageLayoutWidget = new KoPageLayoutWidget(widget, layout);
    d->pageLayoutWidget->showUnitchooser(false);
    lay->addWidget(d->pageLayoutWidget,1);

    KoPagePreviewWidget *prev = new KoPagePreviewWidget(widget);
    // use not original layout, but "fixed" one (e.g. with 0 values) as now hold by pageLayoutWidget
    prev->setPageLayout(d->pageLayoutWidget->pageLayout());
    lay->addWidget(prev, 1);

    connect (d->pageLayoutWidget, SIGNAL(layoutChanged(const KoPageLayout&)),
            prev, SLOT(setPageLayout(const KoPageLayout&)));
    connect (d->pageLayoutWidget, SIGNAL(layoutChanged(const KoPageLayout&)),
            this, SLOT(setPageLayout(const KoPageLayout&)));
    connect (d->pageLayoutWidget, SIGNAL(unitChanged(const KoUnit&)),
            this, SIGNAL(unitChanged(const KoUnit&)));
}

KoPageLayoutDialog::~KoPageLayoutDialog()
{
    delete d;
}

KoPageLayout KoPageLayoutDialog::pageLayout() const
{
    return d->pageLayoutWidget->pageLayout();
}

void KoPageLayoutDialog::setPageLayout(const KoPageLayout &layout)
{
    d->pageLayoutWidget->setPageLayout(layout);
}

void KoPageLayoutDialog::accept()
{
    KPageDialog::accept();
    deleteLater();
}

void KoPageLayoutDialog::reject()
{
    KPageDialog::reject();
    deleteLater();
}

bool KoPageLayoutDialog::applyToDocument() const
{
    return d->documentCheckBox && d->documentCheckBox->isChecked();
}

void KoPageLayoutDialog::showApplyToDocument(bool on)
{
    if (on && d->documentCheckBox == 0) {
        for (int i = 0; i < children().count(); ++i) {
            if (QDialogButtonBox *buttonBox = qobject_cast<QDialogButtonBox*>(children()[i])) {
                d->documentCheckBox = new QCheckBox(i18n("Apply to document"), buttonBox);
                d->documentCheckBox->setChecked(true);
                buttonBox->addButton(d->documentCheckBox, QDialogButtonBox::ResetRole);
                break;
            }
        }

        Q_ASSERT(d->pageLayoutWidget);
        connect (d->documentCheckBox, SIGNAL(toggled(bool)),
                d->pageLayoutWidget, SLOT(setApplyToDocument(bool)));
    } else if (d->documentCheckBox) {
        d->documentCheckBox->setVisible(on);
    }
}

void KoPageLayoutDialog::showTextDirection(bool on)
{
    d->pageLayoutWidget->showTextDirection(on);
}

void KoPageLayoutDialog::showPageSpread(bool on)
{
    d->pageLayoutWidget->showPageSpread(on);
}

void KoPageLayoutDialog::setPageSpread(bool pageSpread)
{
    d->pageLayoutWidget->setPageSpread(pageSpread);
}

void KoPageLayoutDialog::showUnitchooser(bool on)
{
    d->pageLayoutWidget->showUnitchooser(on);
}

void KoPageLayoutDialog::setUnit(const KoUnit &unit)
{
    d->pageLayoutWidget->setUnit(unit);
}

