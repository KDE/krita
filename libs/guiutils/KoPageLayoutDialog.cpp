/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#include <KoLayoutVisitor.h>

#include <klocale.h>

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QTimer>

class KoPageLayoutDialog::Private
{
public:
    KoPageLayout layout;
    KoPageLayoutWidget *pageLayoutWidget;
    bool visited;
    QCheckBox* documentCheckBox;
};


KoPageLayoutDialog::KoPageLayoutDialog(QWidget *parent, const KoPageLayout& layout)
    : KPageDialog(parent)
    , d(new Private)
{
    d->layout = layout;
    d->visited = false;

    setWindowTitle(i18n("Page Layout"));
    setFaceType(KPageDialog::Auto);

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

    for (int i = 0; i < children().count(); ++i) {
        if (QDialogButtonBox* buttonBox = qobject_cast<QDialogButtonBox*>(children()[i])) {
            d->documentCheckBox = new QCheckBox(i18n("Apply to document"), buttonBox);
            buttonBox->addButton(d->documentCheckBox, QDialogButtonBox::ResetRole);
            break;
        }
    }

    connect (d->documentCheckBox, SIGNAL(toggled(bool)),
            d->pageLayoutWidget, SLOT(setApplyToDocument(bool)));
    connect (d->pageLayoutWidget, SIGNAL(layoutChanged(const KoPageLayout&)),
            prev, SLOT(setPageLayout(const KoPageLayout&)));
    connect (d->pageLayoutWidget, SIGNAL(layoutChanged(const KoPageLayout&)),
            this, SLOT(setPageLayout(const KoPageLayout&)));
}

KoPageLayoutDialog::~KoPageLayoutDialog()
{
    delete d;
}

const KoPageLayout& KoPageLayoutDialog::pageLayout() const
{
    return d->layout;
}

void KoPageLayoutDialog::setPageLayout(const KoPageLayout &layout) {
    d->layout = layout;
}

void KoPageLayoutDialog::accept() {
    KPageDialog::accept();
    deleteLater();
}

void KoPageLayoutDialog::reject() {
    KPageDialog::reject();
    deleteLater();
}

void KoPageLayoutDialog::showEvent (QShowEvent *e) {
    KDialog::showEvent(e);
    if(d->visited) return;
    d->visited = true;
    QTimer::singleShot(0, this, SLOT(visit()));
}

void KoPageLayoutDialog::visit() {
    KoLayoutVisitor visitor;
    visitor.visit(d->pageLayoutWidget);
    visitor.relayout();
}

bool KoPageLayoutDialog::applyToDocument() const
{
    return d->documentCheckBox->isChecked();
}

void KoPageLayoutDialog::showTextDirection(bool on) {
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

void KoPageLayoutDialog::showPageSpread(bool on) {
    d->pageLayoutWidget->showPageSpread(on);
}

void KoPageLayoutDialog::setPageSpread(bool pageSpread)
{
    d->pageLayoutWidget->setPageSpread(pageSpread);
}

int KoPageLayoutDialog::startPageNumber() const
{
    return d->pageLayoutWidget->startPageNumber();
}

void KoPageLayoutDialog::setStartPageNumber(int pageNumber)
{
    d->pageLayoutWidget->setStartPageNumber(pageNumber);
}
