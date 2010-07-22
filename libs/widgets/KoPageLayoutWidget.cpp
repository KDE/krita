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

#include "KoPageLayoutWidget.h"

#include <ui_KoPageLayoutWidget.h>

class KoPageLayoutWidget::Private
{
public:
    Ui::KoPageLayoutWidget widget;
    KoPageLayout pageLayout;
    KoUnit unit;

    QButtonGroup *orientationGroup;
    bool marginsEnabled;
    bool allowSignals;
};


KoPageLayoutWidget::KoPageLayoutWidget(QWidget *parent, const KoPageLayout &layout)
    : QWidget(parent)
    , d(new Private)
{
    d->widget.setupUi(this);

    d->pageLayout = layout;
    d->marginsEnabled = true;
    d->allowSignals = true;
    d->orientationGroup = new QButtonGroup(this);
    d->orientationGroup->addButton(d->widget.portrait, KoPageFormat::Portrait);
    d->orientationGroup->addButton(d->widget.landscape, KoPageFormat::Landscape);

    QButtonGroup *group2 = new QButtonGroup(this);
    group2->addButton(d->widget.singleSided);
    group2->addButton(d->widget.facingPages);
    // the two sets of labels we use might have different lengths; make sure this does not create a 'jumping' ui
    d->widget.facingPages->setChecked(true);
    facingPagesChanged();
    int width = qMax(d->widget.leftLabel->width(), d->widget.rightLabel->width());
    d->widget.singleSided->setChecked(true);
    facingPagesChanged();
    width = qMax(width, qMax(d->widget.leftLabel->width(), d->widget.rightLabel->width()));
    d->widget.leftLabel->setMinimumSize(QSize(width, 5));

    d->widget.units->addItems(KoUnit::listOfUnitName());
    d->widget.sizes->addItems(KoPageFormat::allFormats());
    setPageSpread(false);

    connect(d->widget.sizes, SIGNAL(currentIndexChanged(int)), this, SLOT(sizeChanged(int)));
    connect(d->widget.units, SIGNAL(currentIndexChanged(int)), this, SLOT(unitChanged(int)));
    connect(group2, SIGNAL(buttonClicked (int)), this, SLOT(facingPagesChanged()));
    connect(d->orientationGroup, SIGNAL(buttonClicked (int)), this, SLOT(orientationChanged()));
    connect(d->widget.width, SIGNAL(valueChangedPt(qreal)), this, SLOT(optionsChanged()));
    connect(d->widget.height, SIGNAL(valueChangedPt(qreal)), this, SLOT(optionsChanged()));
    connect(d->widget.topMargin, SIGNAL(valueChangedPt(qreal)), this, SLOT(marginsChanged()));
    connect(d->widget.bottomMargin, SIGNAL(valueChangedPt(qreal)), this, SLOT(marginsChanged()));
    connect(d->widget.bindingEdgeMargin, SIGNAL(valueChangedPt(qreal)), this, SLOT(marginsChanged()));
    connect(d->widget.pageEdgeMargin, SIGNAL(valueChangedPt(qreal)), this, SLOT(marginsChanged()));
    connect(d->widget.width, SIGNAL(valueChangedPt(qreal)), this, SLOT(optionsChanged()));
    connect(d->widget.height, SIGNAL(valueChangedPt(qreal)), this, SLOT(optionsChanged()));

    setUnit(KoUnit(KoUnit::Millimeter));
    setPageLayout(layout);
    if (layout.format == 0) // make sure we always call this during startup, even if the A3 (index=0) was chosen
        sizeChanged(layout.format);
    showTextDirection(false);
/* disable page spreads for now */
    d->widget.facingPageLabel->setVisible(false);
    d->widget.facingPages->setVisible(false);
    d->widget.singleSided->setVisible(false);
}

KoPageLayoutWidget::~KoPageLayoutWidget()
{
    delete d;
}

void KoPageLayoutWidget::sizeChanged(int row)
{
    if (row < 0) return;
    if (! d->allowSignals) return;
    d->allowSignals = false;
    d->pageLayout.format = static_cast<KoPageFormat::Format> (row);
    bool custom =  d->pageLayout.format == KoPageFormat::CustomSize;
    d->widget.width->setEnabled( custom );
    d->widget.height->setEnabled( custom );

    if ( !custom ) {
        d->pageLayout.width = MM_TO_POINT( KoPageFormat::width( d->pageLayout.format, d->pageLayout.orientation ) );
        d->pageLayout.height = MM_TO_POINT( KoPageFormat::height( d->pageLayout.format, d->pageLayout.orientation ) );
        if (d->widget.facingPages->isChecked()) // is pagespread
            d->pageLayout.width *= 2;
    }

    d->widget.width->changeValue( d->pageLayout.width );
    d->widget.height->changeValue( d->pageLayout.height );

    emit layoutChanged(d->pageLayout);
    d->allowSignals = true;
}

void KoPageLayoutWidget::unitChanged(int row)
{
    setUnit(KoUnit(static_cast<KoUnit::Unit> (row)));
}

void KoPageLayoutWidget::setUnit(const KoUnit &unit)
{
    d->unit = unit;

    d->widget.width->setUnit(unit);
    d->widget.height->setUnit(unit);
    d->widget.topMargin->setUnit(unit);
    d->widget.bottomMargin->setUnit(unit);
    d->widget.bindingEdgeMargin->setUnit(unit);
    d->widget.pageEdgeMargin->setUnit(unit);
    d->widget.units->setCurrentIndex(unit.indexInList());

    emit unitChanged(d->unit);
}

void KoPageLayoutWidget::setPageLayout(const KoPageLayout &layout)
{
    if (! d->allowSignals) return;
    d->allowSignals = false;
    d->pageLayout = layout;

    Q_ASSERT(d->orientationGroup->button( layout.orientation ));
    d->orientationGroup->button( layout.orientation )->setChecked( true );
    if (layout.bindingSide >= 0 && layout.pageEdge >= 0) {
        d->widget.facingPages->setChecked(true);
        d->widget.bindingEdgeMargin->changeValue(layout.bindingSide);
        d->widget.pageEdgeMargin->changeValue(layout.pageEdge);
        d->pageLayout.leftMargin = -1;
        d->pageLayout.rightMargin = -1;
    }
    else {
        d->widget.singleSided->setChecked(true);
        d->widget.bindingEdgeMargin->changeValue(layout.leftMargin);
        d->widget.pageEdgeMargin->changeValue(layout.rightMargin);
        d->pageLayout.pageEdge = -1;
        d->pageLayout.bindingSide = -1;
    }
    facingPagesChanged();

    d->widget.topMargin->changeValue(layout.topMargin);
    d->widget.bottomMargin->changeValue(layout.bottomMargin);
    d->allowSignals = true;
    d->widget.sizes->setCurrentIndex(layout.format); // calls sizeChanged()
}

void KoPageLayoutWidget::facingPagesChanged()
{
    if (! d->allowSignals) return;
    d->allowSignals = false;
    if (d->widget.singleSided->isChecked()) {
        d->widget.leftLabel->setText(i18n("Left Edge:"));
        d->widget.rightLabel->setText(i18n("Right Edge:"));
    }
    else {
        d->widget.leftLabel->setText(i18n("Binding Edge:"));
        d->widget.rightLabel->setText(i18n("Page Edge:"));
    }
    d->allowSignals = true;
    marginsChanged();
    sizeChanged(d->widget.sizes->currentIndex());
}

void KoPageLayoutWidget::marginsChanged()
{
    if (! d->allowSignals) return;
    d->allowSignals = false;
    d->pageLayout.leftMargin = -1;
    d->pageLayout.rightMargin = -1;
    d->pageLayout.bindingSide = -1;
    d->pageLayout.pageEdge = -1;
    d->pageLayout.topMargin = d->marginsEnabled?d->widget.topMargin->value():0;
    d->pageLayout.bottomMargin = d->marginsEnabled?d->widget.bottomMargin->value():0;
    qreal left = d->marginsEnabled?d->widget.bindingEdgeMargin->value():0;
    qreal right = d->marginsEnabled?d->widget.pageEdgeMargin->value():0;
    if (left + right > d->pageLayout.width - 10) {
        // make sure the actual text area is never smaller than 10 points.
        qreal diff = d->pageLayout.width - 10 - left - right;
        left = qMin(d->pageLayout.width - 10, qMax(qreal(0.0), left - diff / qreal(2.0)));
        right = qMax(qreal(0.0), right - d->pageLayout.width - 10 - left);
    }

    if (d->widget.singleSided->isChecked()) {
        d->pageLayout.leftMargin  = left;
        d->pageLayout.rightMargin = right;
    }
    else {
        d->pageLayout.bindingSide = left;
        d->pageLayout.pageEdge = right;
    }
    // kDebug() << "  " << d->pageLayout.left <<"|"<< d->pageLayout.bindingSide << "," <<
    //    d->pageLayout.right << "|"<< d->pageLayout.pageEdge;
    emit layoutChanged(d->pageLayout);
    d->allowSignals = true;
}

void KoPageLayoutWidget::setTextAreaAvailable(bool available)
{
    d->marginsEnabled = available;
    d->widget.margins->setEnabled(available);
    marginsChanged();
}

void KoPageLayoutWidget::optionsChanged()
{
    if (! d->allowSignals) return;
    if (d->widget.sizes->currentIndex() == KoPageFormat::CustomSize) {
        d->pageLayout.width = d->widget.width->value();
        d->pageLayout.height = d->widget.height->value();
    } else
        sizeChanged(d->widget.sizes->currentIndex());

    marginsChanged();
}

void KoPageLayoutWidget::orientationChanged()
{
    if (! d->allowSignals) return;
    d->allowSignals = false;
    d->pageLayout.orientation = d->widget.landscape->isChecked() ? KoPageFormat::Landscape : KoPageFormat::Portrait;

    qreal x = d->widget.height->value();
    d->widget.height->changeValue( d->widget.width->value() );
    d->widget.width->changeValue( x );

    d->allowSignals = true;
    optionsChanged();
}

void KoPageLayoutWidget::showUnitchooser(bool on) {
    d->widget.units->setVisible(on);
    d->widget.unitsLabel->setVisible(on);
}

void KoPageLayoutWidget::showPageSpread(bool on)
{
    d->widget.facingPageLabel->setVisible(on);
    d->widget.singleSided->setVisible(on);
    d->widget.facingPages->setVisible(on);
}

void KoPageLayoutWidget::setPageSpread(bool pageSpread)
{
    if (pageSpread)
        d->widget.facingPages->setChecked(true);
    else
        d->widget.singleSided->setChecked(true);
}

void KoPageLayoutWidget::setApplyToDocument(bool apply)
{
    if (apply) {
        d->widget.facingPageLabel->setText(i18n("Facing Pages:"));
        d->widget.facingPages->setText(i18n("Facing pages"));
    }
    else {
        d->widget.facingPageLabel->setText(i18n("Page Layout:"));
        d->widget.facingPages->setText(i18n("Page spread"));
    }
}

void KoPageLayoutWidget::showTextDirection(bool on)
{
    d->widget.directionLabel->setVisible(on);
    d->widget.textDirection->setVisible(on);
}

void KoPageLayoutWidget::setTextDirection(KoText::Direction direction )
{
    int index = 0;
    switch(direction) {
    case KoText::LeftRightTopBottom:
    case KoText::PerhapsLeftRightTopBottom:
        index = 1;
        break;
    case KoText::RightLeftTopBottom:
    case KoText::PerhapsRightLeftTopBottom:
        index = 2;
        break;
    case KoText::TopBottomRightLeft: // unused for now.
    case KoText::InheritDirection:
    case KoText::AutoDirection:
        index = 0;
    }
    d->widget.textDirection->setCurrentIndex(index);
}

KoText::Direction KoPageLayoutWidget::textDirection() const
{
    switch(d->widget.textDirection->currentIndex()) {
    case 1: return KoText::LeftRightTopBottom;
    case 2: return KoText::RightLeftTopBottom;
    default:
    case 0: return KoText::AutoDirection;
    }
}
