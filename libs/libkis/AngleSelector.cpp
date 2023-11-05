/*
 *  SPDX-FileCopyrightText: 2020 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "AngleSelector.h"

#include "kis_debug.h"


const QList<QString> FlipOptionsMode = {
    "NoFlipOptions",    // 0 = KisAngleSelector::FlipOptionsMode_NoFlipOptions
    "MenuButton",       //     KisAngleSelector::FlipOptionsMode_MenuButton
    "Buttons",          //     KisAngleSelector::FlipOptionsMode_Buttons
    "ContextMenu"       //     KisAngleSelector::FlipOptionsMode_ContextMenu
};

const QList<QString> IncreasingDirection = {
    "CounterClockwise", // 0 = KisAngleGauge::IncreasingDirection_CounterClockwise
    "Clockwise"         //     KisAngleGauge::IncreasingDirection_Clockwise
};

struct AngleSelector::Private {
    Private() {}

    KisAngleSelector *widget;
};

AngleSelector::AngleSelector()
    : QObject()
    , d(new Private)
{
    d->widget = new KisAngleSelector();

    // Forward KisAngleSelector::angleChanged to AngleSelector::angleChanged
    connect(d->widget, SIGNAL(angleChanged(qreal)), this, SIGNAL(angleChanged(qreal)));
}

AngleSelector::~AngleSelector()
{
    delete d;
}

QWidget* AngleSelector::widget() const
{
    return d->widget;
}

qreal AngleSelector::angle() const
{
    return d->widget->angle();
}

qreal AngleSelector::snapAngle() const
{
    return d->widget->snapAngle();
}

qreal AngleSelector::resetAngle() const
{
    return d->widget->resetAngle();
}

int	AngleSelector::decimals() const
{
    return d->widget->decimals();
}

qreal AngleSelector::maximum() const
{
    return d->widget->maximum();
}

qreal AngleSelector::minimum() const
{
    return d->widget->minimum();
}

QString AngleSelector::prefix() const
{
    return d->widget->prefix();
}

bool AngleSelector::wrapping() const
{
    return d->widget->wrapping();
}

QString AngleSelector::flipOptionsMode() const
{
    KisAngleSelector::FlipOptionsMode mode = d->widget->flipOptionsMode();
    if (!(0 <= mode && mode <= FlipOptionsMode.size())) {
        warnScript << "AngleSelector::flipOptionsMode() doesn't handle mode '" << mode << "'!";
        return "";
    }
    return FlipOptionsMode[mode];
}

int AngleSelector::widgetsHeight() const
{
    return d->widget->widgetsHeight();
}

QString AngleSelector::increasingDirection() const
{
    KisAngleGauge::IncreasingDirection increasingDirection = d->widget->increasingDirection();
    if (!(0 <= increasingDirection && increasingDirection <= IncreasingDirection.size())) {
        warnScript << "AngleSelector::increasingDirection() doesn't handle mode '" << increasingDirection << "'!";
        return "";
    }
    return IncreasingDirection[increasingDirection];
}

bool AngleSelector::isUsingFlatSpinBox() const
{
    return d->widget->isUsingFlatSpinBox();
}

void AngleSelector::setAngle(qreal newAngle)
{
    d->widget->setAngle(newAngle);
}

void AngleSelector::setSnapAngle(qreal newSnapAngle)
{
    d->widget->setSnapAngle(newSnapAngle);
}

void AngleSelector::setResetAngle(qreal newResetAngle)
{
    d->widget->setResetAngle(newResetAngle);
}

void AngleSelector::setDecimals(int newNumberOfDecimals)
{
    d->widget->setDecimals(newNumberOfDecimals);
}

void AngleSelector::setMaximum(qreal newMaximum)
{
    d->widget->setMaximum(newMaximum);
}

void AngleSelector::setMinimum(qreal newMinimum)
{
    d->widget->setMinimum(newMinimum);
}

void AngleSelector::setRange(qreal newMinimum, qreal newMaximum)
{
    d->widget->setRange(newMinimum, newMaximum);
}

void AngleSelector::setPrefix(const QString &newPrefix)
{
    d->widget->setPrefix(newPrefix);
}

void AngleSelector::setWrapping(bool newWrapping)
{
    d->widget->setWrapping(newWrapping);
}

void AngleSelector::setFlipOptionsMode(QString newMode)
{
    int index = FlipOptionsMode.indexOf(newMode);
    if (index == -1) {
        dbgScript << "Script using AngleSelector.setFlipOptionsMode() passed invalid mode '" << newMode << "', ignoring.";
        return;
    }
    d->widget->setFlipOptionsMode((KisAngleSelector::FlipOptionsMode) index);
}

void AngleSelector::setWidgetsHeight(int newHeight)
{
    d->widget->setWidgetsHeight(newHeight);
}

void AngleSelector::setIncreasingDirection(QString newIncreasingDirection)
{
    int index = IncreasingDirection.indexOf(newIncreasingDirection);
    if (index == -1) {
        dbgScript << "Script using AngleSelector.setIncreasingDirection() passed invalid mode '" << newIncreasingDirection << "', ignoring.";
        return;
    }
    d->widget->setIncreasingDirection((KisAngleGauge::IncreasingDirection) index);
}

void AngleSelector::useFlatSpinBox(bool newUseFlatSpinBox)
{
    d->widget->useFlatSpinBox(newUseFlatSpinBox);
}

void AngleSelector::reset()
{
    d->widget->reset();
}

qreal AngleSelector::closestCoterminalAngleInRange(qreal angle, qreal minimum, qreal maximum, bool *ok)
{
    return KisAngleSelector::closestCoterminalAngleInRange(angle, minimum, maximum, ok);
}

qreal AngleSelector::closestCoterminalAngleInRange(qreal angle, bool *ok) const
{
    return d->widget->closestCoterminalAngleInRange(angle, ok);
}

qreal AngleSelector::flipAngle(qreal angle, Qt::Orientations orientations)
{
    return KisAngleSelector::flipAngle(angle, orientations);
}

qreal AngleSelector::flipAngle(qreal angle, qreal minimum, qreal maximum, Qt::Orientations orientations, bool *ok)
{
    return KisAngleSelector::flipAngle(angle, minimum, maximum, orientations, ok);
}

void AngleSelector::flip(Qt::Orientations orientations)
{
    bool ok = false;
    qreal flippedAngle = flipAngle(angle(), minimum(), maximum(), orientations, &ok);
    if (ok) {
        setAngle(flippedAngle);
    }
}
