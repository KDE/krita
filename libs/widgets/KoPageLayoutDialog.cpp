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

#include <klocale.h>
#include <kdebug.h>

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QTimer>

class KoPageLayoutDialog::Private
{
public:
    Private() : pageLayoutWidget(0), documentCheckBox(0) {}
    KoPageLayout layout;
    KoPageLayoutWidget *pageLayoutWidget;
    QCheckBox *documentCheckBox;
};


KoPageLayoutDialog::KoPageLayoutDialog(QWidget *parent, const KoPageLayout &layout)
    : KPageDialog(parent)
    , d(new Private)
{
    d->layout = layout;

    setWindowTitle(i18n("Page Layout"));
    setFaceType(KPageDialog::Tabbed);

    QWidget *widget = new QWidget(this);
    addPage(widget, i18n("Page"));

    QHBoxLayout *lay = new QHBoxLayout(widget);
    lay->setMargin(0);
    widget->setLayout(lay);

    d->pageLayoutWidget = new KoPageLayoutWidget(widget, d->layout);
    d->pageLayoutWidget->showUnitchooser(false);
    d->pageLayoutWidget->layout()->setMargin(0);
    lay->addWidget(d->pageLayoutWidget);

    KoPagePreviewWidget *prev = new KoPagePreviewWidget(widget);
    prev->setPageLayout(d->layout);
    lay->addWidget(prev);

    connect (d->pageLayoutWidget, SIGNAL(layoutChanged(const KoPageLayout&)),
            prev, SLOT(setPageLayout(const KoPageLayout&)));
    connect (d->pageLayoutWidget, SIGNAL(layoutChanged(const KoPageLayout&)),
            this, SLOT(setPageLayout(const KoPageLayout&)));
}

KoPageLayoutDialog::~KoPageLayoutDialog()
{
    delete d;
}

KoPageLayout KoPageLayoutDialog::pageLayout() const
{
    return d->layout;
}

void KoPageLayoutDialog::setPageLayout(const KoPageLayout &layout)
{
    d->layout = layout;
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

KoText::Direction KoPageLayoutDialog::textDirection() const
{
    return d->pageLayoutWidget->textDirection();
}

void KoPageLayoutDialog::setTextDirection(KoText::Direction direction)
{
    d->pageLayoutWidget->setTextDirection(direction);
}

void KoPageLayoutDialog::showPageSpread(bool on)
{
    d->pageLayoutWidget->showPageSpread(on);
}

void KoPageLayoutDialog::setPageSpread(bool pageSpread)
{
    d->pageLayoutWidget->setPageSpread(pageSpread);
}


