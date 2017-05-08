/* This file is part of the KDE project
   Copyright (c) 2007 C. Boemann <cbo@boemann.dk>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/
#ifndef KoSliderCombo_p_h
#define KoSliderCombo_p_h

#include "KoSliderCombo.h"

#include <QTimer>
#include <QApplication>
#include <QSize>
#include <QSlider>
#include <QStyle>
#include <QStylePainter>
#include <QStyleOptionSlider>
#include <QLineEdit>
#include <QValidator>
#include <QHBoxLayout>
#include <QFrame>
#include <QMenu>
#include <QMouseEvent>
#include <QDoubleSpinBox>
#include <QDesktopWidget>

#include <klocalizedstring.h>
#include <WidgetsDebug.h>

class KoSliderComboContainer : public QMenu
{
public:
    KoSliderComboContainer(KoSliderCombo *parent) : QMenu(parent ), m_parent(parent) {}

protected:
    void mousePressEvent(QMouseEvent *e) override;
private:
    KoSliderCombo *m_parent;
};

void KoSliderComboContainer::mousePressEvent(QMouseEvent *e)
{
    QStyleOptionComboBox opt;
    opt.init(m_parent);
    opt.subControls = QStyle::SC_All;
    opt.activeSubControls = QStyle::SC_ComboBoxArrow;
    QStyle::SubControl sc = style()->hitTestComplexControl(QStyle::CC_ComboBox, &opt,
                                                           m_parent->mapFromGlobal(e->globalPos()),
                                                           m_parent);
    if (sc == QStyle::SC_ComboBoxArrow)
        setAttribute(Qt::WA_NoMouseReplay);
    QMenu::mousePressEvent(e);
}

class Q_DECL_HIDDEN KoSliderCombo::KoSliderComboPrivate {
public:
    KoSliderCombo *thePublic;
    QValidator *m_validator;
    QTimer m_timer;
    KoSliderComboContainer *container;
    QSlider *slider;
    QStyle::StateFlag arrowState;
    qreal minimum;
    qreal maximum;
    int decimals;
    bool firstShowOfSlider;

    void showPopup();
    void hidePopup();

    void sliderValueChanged(int value);
    void sliderReleased();
    void lineEditFinished();
};

#endif
