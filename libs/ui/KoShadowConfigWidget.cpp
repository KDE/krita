/* This file is part of the KDE project
 * Copyright (C) 2012 C. Boemann <cbo@boemann.dk>
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
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

#include "KoShadowConfigWidget.h"
#include "ui_KoShadowConfigWidget.h"

#include <KoIcon.h>
#include <KoUnit.h>
#include <KoColorPopupAction.h>
#include <KoCanvasBase.h>
#include <KoCanvasResourceManager.h>
#include <KoSelection.h>
#include <KoShapeShadow.h>
#include <KoShapeShadowCommand.h>
#include <KoSelectedShapesProxy.h>

#include <klocalizedstring.h>

#include <QCheckBox>

#include <math.h>

class Q_DECL_HIDDEN KoShadowConfigWidget::Private
{
public:
    Private()
    {
    }
    Ui_KoShadowConfigWidget widget;
    KoColorPopupAction *actionShadowColor;
    KoCanvasBase *canvas;
};

KoShadowConfigWidget::KoShadowConfigWidget(QWidget *parent)
    : QWidget(parent)
    , d(new Private())
{
    d->widget.setupUi(this);
    d->widget.shadowOffset->setValue(8.0);
    d->widget.shadowBlur->setValue(8.0);
    d->widget.shadowBlur->setMinimum(0.0);
    d->widget.shadowAngle->setValue(315.0);
    d->widget.shadowAngle->setMinimum(0.0);
    d->widget.shadowAngle->setMaximum(360.0);
    d->widget.shadowVisible->setChecked(false);
    visibilityChanged();

    d->actionShadowColor = new KoColorPopupAction(this);
    d->actionShadowColor->setCurrentColor(QColor(0, 0, 0, 192)); // some reasonable default for shadow
    d->actionShadowColor->setIcon(koIcon("format-stroke-color"));
    d->actionShadowColor->setToolTip(i18n("Change the color of the shadow"));
    d->widget.shadowColor->setDefaultAction(d->actionShadowColor);

    connect(d->widget.shadowVisible, SIGNAL(toggled(bool)), this, SLOT(applyChanges()));
    connect(d->widget.shadowVisible, SIGNAL(toggled(bool)), this, SLOT(visibilityChanged()));
    connect(d->actionShadowColor, SIGNAL(colorChanged(const KoColor&)), this, SLOT(applyChanges()));
    connect(d->widget.shadowAngle, SIGNAL(valueChanged(int)), this, SLOT(applyChanges()));
    connect(d->widget.shadowOffset, SIGNAL(valueChangedPt(qreal)), this, SLOT(applyChanges()));
    connect(d->widget.shadowBlur, SIGNAL(valueChangedPt(qreal)), this, SLOT(applyChanges()));
}

KoShadowConfigWidget::~KoShadowConfigWidget()
{
    delete d;
}

void KoShadowConfigWidget::setShadowColor(const QColor &color)
{
    d->widget.shadowColor->blockSignals(true);
    d->actionShadowColor->blockSignals(true);

    d->actionShadowColor->setCurrentColor( color );

    d->actionShadowColor->blockSignals(false);
    d->widget.shadowColor->blockSignals(false);
}

QColor KoShadowConfigWidget::shadowColor() const
{
    return d->actionShadowColor->currentColor();
}

void KoShadowConfigWidget::setShadowOffset(const QPointF &offset)
{
    qreal length = sqrt(offset.x()*offset.x() + offset.y()*offset.y());
    qreal angle = atan2(-offset.y(), offset.x());
    if (angle < 0.0) {
        angle += 2*M_PI;
    }

    d->widget.shadowAngle->blockSignals(true);
    d->widget.shadowAngle->setValue(-90 - angle * 180.0 / M_PI);
    d->widget.shadowAngle->blockSignals(false);

    d->widget.shadowOffset->blockSignals(true);
    d->widget.shadowOffset->changeValue(length);
    d->widget.shadowOffset->blockSignals(false);
}

QPointF KoShadowConfigWidget::shadowOffset() const
{
    QPointF offset(d->widget.shadowOffset->value(), 0);
    QTransform m;
    m.rotate(d->widget.shadowAngle->value() + 90);
    return m.map(offset);
}

void KoShadowConfigWidget::setShadowBlur(const qreal &blur)
{
    d->widget.shadowBlur->blockSignals(true);
    d->widget.shadowBlur->changeValue(blur);
    d->widget.shadowBlur->blockSignals(false);
}

qreal KoShadowConfigWidget::shadowBlur() const
{
    return d->widget.shadowBlur->value();
}

void KoShadowConfigWidget::setShadowVisible(bool visible)
{
    d->widget.shadowVisible->blockSignals(true);
    d->widget.shadowVisible->setChecked(visible);
    d->widget.shadowVisible->blockSignals(false);
    visibilityChanged();
}

bool KoShadowConfigWidget::shadowVisible() const
{
    return d->widget.shadowVisible->isChecked();
}

void KoShadowConfigWidget::visibilityChanged()
{
    d->widget.shadowAngle->setEnabled( d->widget.shadowVisible->isChecked() );
    d->widget.shadowBlur->setEnabled( d->widget.shadowVisible->isChecked() );
    d->widget.shadowColor->setEnabled( d->widget.shadowVisible->isChecked() );
    d->widget.shadowOffset->setEnabled( d->widget.shadowVisible->isChecked() );
}

void KoShadowConfigWidget::applyChanges()
{
    if (d->canvas) {
        KoSelection *selection = d->canvas->selectedShapesProxy()->selection();
        KoShape * shape = selection->firstSelectedShape();
        if (! shape) {
            return;
        }

        KoShapeShadow *newShadow = new KoShapeShadow();
        newShadow->setVisible(shadowVisible());
        newShadow->setColor(shadowColor());
        newShadow->setOffset(shadowOffset());
        newShadow->setBlur(shadowBlur());
        d->canvas->addCommand(new KoShapeShadowCommand(selection->selectedShapes(), newShadow));
    }
}

void KoShadowConfigWidget::selectionChanged()
{
    if (! d->canvas) {
        return;
    }

    KoSelection *selection = d->canvas->selectedShapesProxy()->selection();
    KoShape * shape = selection->firstSelectedShape();

    setEnabled(shape != 0);

    if (! shape) {
        setShadowVisible(false);
        return;
    }

    KoShapeShadow * shadow = shape->shadow();
    if (! shadow) {
        setShadowVisible(false);
        return;
    }

    setShadowVisible(shadow->isVisible());
    setShadowOffset(shadow->offset());
    setShadowColor(shadow->color());
    setShadowBlur(shadow->blur());
}

void KoShadowConfigWidget::setCanvas(KoCanvasBase *canvas)
{
    d->canvas = canvas;
    connect(canvas->selectedShapesProxy(), SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));
    connect(canvas->selectedShapesProxy(), SIGNAL(selectionContentChanged()), this, SLOT(selectionChanged()));

    setUnit(canvas->unit());

    connect( d->canvas->resourceManager(), SIGNAL( canvasResourceChanged( int, const QVariant& ) ),
             this, SLOT( resourceChanged( int, const QVariant& ) ) );
}

void KoShadowConfigWidget::setUnitManagers(KisSpinBoxUnitManager* managerBlur, KisSpinBoxUnitManager *managerOffset)
{
    d->widget.shadowOffset->blockSignals(true);
    d->widget.shadowBlur->blockSignals(true);
    d->widget.shadowOffset->setUnitManager(managerOffset);
    d->widget.shadowBlur->setUnitManager(managerBlur);
    d->widget.shadowOffset->blockSignals(false);
    d->widget.shadowBlur->blockSignals(false);
}

void KoShadowConfigWidget::setUnit(const KoUnit &unit)
{
    d->widget.shadowOffset->blockSignals(true);
    d->widget.shadowBlur->blockSignals(true);
    d->widget.shadowOffset->setUnit(unit);
    d->widget.shadowBlur->setUnit(unit);
    d->widget.shadowOffset->blockSignals(false);
    d->widget.shadowBlur->blockSignals(false);
}

void KoShadowConfigWidget::resourceChanged( int key, const QVariant & res )
{
    if( key == KoCanvasResourceManager::Unit ) {
        setUnit(res.value<KoUnit>());
    }
}
