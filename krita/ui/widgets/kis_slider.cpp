/*
 *  Copyright (c) 2010 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "kis_slider.h"

#include <QAccessible>
#include <QApplication>
#include <QEvent>
#include <QPainter>
#include <QStyle>
#include <QStyleOption>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QWidget>
#include <QtGlobal>
#include <QBasicTimer>
#include <QTabletEvent>
#include <QTimerEvent>
#include <QWheelEvent>

class KisSliderPrivate
{
public:

    KisSliderPrivate()
        : minimum(0)
        , maximum(99)
        , pageStep(10)
        , value(5)
        , position(5)
        , pressValue(-1)
        , singleStep(1)
        , offsetAccumulated(0)
        , tracking(true)
        , blocktracking(false)
        , pressed(false)
        , repeatAction(KisSlider::SliderNoAction)
    {}

    double minimum;
    double maximum;
    double pageStep;
    double value;
    double position;
    double pressValue;
    double singleStep;
    float offsetAccumulated;

    bool tracking;
    bool blocktracking;
    bool pressed;

    QBasicTimer repeatActionTimer;
    int repeatActionTime;
    KisSlider::SliderAction repeatAction;

};




KisSlider::KisSlider(QWidget *parent)
    : QWidget(parent)
{
    setFocusPolicy(Qt::WheelFocus);
}

KisSlider::~KisSlider() {
}

double KisSlider::minimum() {

}

void KisSlider::setMinimum(double value) {

}

double KisSlider::maximum() {

}

void KisSlider::setMaximum(double value) {

}

void KisSlider::setRange(double minimum, double maximum) {

}

double KisSlider::singleStep() {

}

void KisSlider::setSingleStep(double value) {

}

void KisSlider::setPageStep(double) {

}

double KisSlider::pageStep() const {

}

void KisSlider::setTracking(bool enable) {

}

bool KisSlider::hasTracking() const {

}

void KisSlider::setSliderDown(bool) {

}

bool KisSlider::isSliderDown() const {

}

void KisSlider::setSliderPosition(int) {

}

int KisSlider::sliderPosition() const {

}

int KisSlider::decimals() const {

}

void KisSlider::setDecimals(int prec) {

}

double KisSlider::value() {

}

void KisSlider::triggerAction(SliderAction action) {

}


QSize KisSlider::sizeHint() const {

}

QSize KisSlider::minimumSizeHint() const {

}



bool KisSlider::event(QEvent *event) {

    QWidget::event(event);
}

void KisSlider::keyReleaseEvent(QKeyEvent *event) {

}

void KisSlider::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
}


void KisSlider::paintEvent(QPaintEvent *event) {

    QStyleOptionFrame frameOption;
    frameOption.initFrom(this);

    QStyleOptionSlider sliderOption;
    sliderOption.initFrom(this);

    QStyleOptionSpinBox spinboxOption;
    spinboxOption.initFrom(this);
}

void KisSlider::mousePressEvent(QMouseEvent *event) {
    QWidget::mousePressEvent(event);
}

void KisSlider::mouseReleaseEvent(QMouseEvent *event) {
    QWidget::mouseReleaseEvent(event);
}

void KisSlider::mouseMoveEvent(QMouseEvent *event) {
    QWidget::event(event);
}

void KisSlider::tabletEvent(QTabletEvent* event) {

}

void KisSlider::wheelEvent(QWheelEvent *e) {

}

void KisSlider::timerEvent(QTimerEvent *event) {

}

void KisSlider::changeEvent(QEvent *event) {

}

QValidator::State KisSlider::validate(QString &input, int &pos) const {

}

double KisSlider::valueFromText(const QString &text) const{

}

QString KisSlider::textFromValue(double val) const {

}

void KisSlider::fixup(QString &str) const {

}

void KisSlider::setValue(double value) {

}

