/*
 *  Copyright (c) 2020 Eoin O'Neill <eoinoneill1991@gmail.com>
 *  Copyright (c) 2020 Emmet O'Neill <emmetoneill.pdx@gmail.com>
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
#include "kis_color_label_button.h"

#include <QStylePainter>
#include <QStyleOption>
#include <QMimeData>

#include "kis_global.h"
#include "kis_debug.h"
#include "krita_container_utils.h"

struct KisColorLabelButton::Private
{
    QColor color;
};

KisColorLabelButton::KisColorLabelButton(QColor color, QWidget *parent) : QAbstractButton(parent), m_d(new Private())
{
    setCheckable(true);
    setChecked(true);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_d->color = color;
}

KisColorLabelButton::~KisColorLabelButton() {}

void KisColorLabelButton::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    QStylePainter painter(this);
    QStyleOption styleOption;
    styleOption.initFrom(this);

    if (isDown() || isChecked()){
        styleOption.state |= QStyle::State_On;
    }

    // Draw fill..
    QRect fillRect = kisGrowRect(rect(), -2);
    fillRect.width();
    if (m_d->color.alpha() > 0) {
        QColor fillColor = m_d->color;

        if (!isChecked()) {
            fillColor.setAlpha(32);
        }

        painter.fillRect(fillRect, fillColor);
    } else {
        // draw an X for no color for the first item
        const int shortestEdge = std::min(fillRect.width(), fillRect.height());
        const int longestEdge = std::max(fillRect.width(), fillRect.height());
        bool horizontalIsShortest = (shortestEdge == fillRect.width());
        QRect srcRect = horizontalIsShortest ?
                    fillRect.adjusted(0, (longestEdge / 2) - (shortestEdge / 2), 0, (shortestEdge / 2) - (longestEdge / 2)) :
                    fillRect.adjusted((longestEdge / 2) - (shortestEdge / 2), 0, (shortestEdge / 2) - (longestEdge / 2), 0);
        QRect crossRect = kisGrowRect(srcRect, -1);

        QColor shade = styleOption.palette.text().color();

        if (!isChecked()) {
            shade.setAlpha(64);
        }

        painter.setPen(QPen(shade, 2));
        painter.drawLine(crossRect.topLeft(), crossRect.bottomRight());
        painter.drawLine(crossRect.bottomLeft(), crossRect.topRight());
    }
}

QSize KisColorLabelButton::sizeHint() const
{
    return QSize(32,32);
}

void KisColorLabelButton::nextCheckState()
{
    KisColorLabelFilterGroup* colorLabelButtonGroup = dynamic_cast<KisColorLabelFilterGroup*>(group());
    if (colorLabelButtonGroup && (colorLabelButtonGroup->countCheckedViableButtons() > 1 || !isChecked())) {
        setChecked(!isChecked());
    } else {
        setChecked(isChecked());
    }
}

KisColorLabelFilterGroup::KisColorLabelFilterGroup(QObject *parent)
    : QButtonGroup(parent)
{
}

KisColorLabelFilterGroup::~KisColorLabelFilterGroup()
{
}

QList<QAbstractButton *> KisColorLabelFilterGroup::viableButtons() const {
    QList<QAbstractButton*> viableButtons;

    Q_FOREACH( int index, viableColorLabels ) {
        viableButtons.append(button(index));
    }

    return viableButtons;
}

void KisColorLabelFilterGroup::setViableLabels(QSet<int> &labels) {
    hideAll();
    QSet<int> removed = viableColorLabels.subtract(labels);

    viableColorLabels = labels;

    if (viableColorLabels.count() > 1) {
        Q_FOREACH( int index, viableColorLabels) {
            if (button(index)) {
                button(index)->setVisible(true);
            }
        }
    }

    Q_FOREACH( int index, removed ) {
        button(index)->setChecked(true);
    }
}

QSet<int> KisColorLabelFilterGroup::getActiveLabels() const {
    QSet<int> checkedLabels = QSet<int>();

    Q_FOREACH( int index, viableColorLabels ) {
        if (button(index)->isChecked()) {
            checkedLabels.insert(index);
        }
    }

    return checkedLabels.count() == viableColorLabels.count() ? QSet<int>() : checkedLabels;
}

QList<QAbstractButton *> KisColorLabelFilterGroup::checkedViableButtons() const {
    QList<QAbstractButton*> checkedButtons = viableButtons();

    KritaUtils::filterContainer(checkedButtons, [](QAbstractButton* btn){
       return (btn->isChecked());
    });

    return checkedButtons;
}

int KisColorLabelFilterGroup::countCheckedViableButtons() const {
    return checkedViableButtons().count();
}

int KisColorLabelFilterGroup::countViableButtons() const {
    return viableColorLabels.count();
}

void KisColorLabelFilterGroup::reset() {
    Q_FOREACH( QAbstractButton* btn, viableButtons() ) {
        btn->setChecked(true);
    }
}

void KisColorLabelFilterGroup::hideAll() {
    Q_FOREACH( QAbstractButton* btn, buttons() ) {
        btn->setVisible(false);
    }
}
