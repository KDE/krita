/* This file is part of the KDE project

   Copyright (C)  2011 Mojtaba Shahi Senobari <mojtaba.shahi3000@gmail.com>
   Copyright (C)  2012 Gopalakrishna Bhat A <gopalakbhat@gmail.com>

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

#include "ParagraphDropCaps.h"

#include "KoParagraphStyle.h"
#include <QMessageBox>

ParagraphDropCaps::ParagraphDropCaps(QWidget *parent)
    : QWidget(parent)
{
    widget.setupUi(this);

    widget.distance->changeValue(0);
    widget.characters->setSpecialValueText(i18n("Whole Word"));
    widget.characters->setValue(0);
    widget.lines->setValue(2);

    connect(widget.capsState, SIGNAL(stateChanged(int)), this, SLOT(dropCapsStateChanged()));
    connect(widget.distance, SIGNAL(valueChangedPt(qreal)), this, SLOT(paragraphDistanceChanged(qreal)));
    connect(widget.characters, SIGNAL(valueChanged(int)), this, SLOT(dropedCharacterCountChanged(int)));
    connect(widget.lines, SIGNAL(valueChanged(int)), this, SLOT(dropsLineSpanChanged(int)));
}

void ParagraphDropCaps::dropCapsStateChanged()
{
    if (widget.capsState->isChecked()) {
        widget.setting->setEnabled(true);
        m_dropCapsInherited = false;
    } else {
        widget.setting->setEnabled(false);
    }
    emit parStyleChanged();
}

void ParagraphDropCaps::setDisplay(KoParagraphStyle *style)
{
    if (!style) {
        return;
    }
    if (!style->dropCaps()) {
        widget.setting->setEnabled(false);
        return;
    }

    widget.capsState->setChecked(true);
    widget.distance->changeValue(style->dropCapsDistance());
    widget.characters->setValue(style->dropCapsLength());
    widget.lines->setValue(style->dropCapsLines());

    m_dropCapsInherited = !style->hasProperty(KoParagraphStyle::DropCaps);
    m_capsDistanceInherited = !style->hasProperty(KoParagraphStyle::DropCapsDistance);
    m_capsLengthInherited = !style->hasProperty(KoParagraphStyle::DropCapsLength);
    m_capsLinesInherited = !style->hasProperty(KoParagraphStyle::DropCapsLines);
}

void ParagraphDropCaps::save(KoParagraphStyle *style)
{
    if (!style) {
        return;
    }

    if (!m_dropCapsInherited) {
        style->setDropCaps(widget.capsState->isChecked());
    }

    if (!m_capsDistanceInherited) {
        style->setDropCapsDistance(widget.distance->value());
    }

    if (!m_capsLengthInherited) {
        style->setDropCapsLength(widget.characters->value());
    }

    if (!m_capsLinesInherited) {
        style->setDropCapsLines(widget.lines->value());
    }
}

void ParagraphDropCaps::setUnit(const KoUnit &unit)
{
    widget.distance->setUnit(unit);
}

void ParagraphDropCaps::paragraphDistanceChanged(qreal distance)
{
    Q_UNUSED(distance);
    m_capsDistanceInherited = false;
    emit parStyleChanged();
}

void ParagraphDropCaps::dropsLineSpanChanged(int lineSpan)
{
    Q_UNUSED(lineSpan);
    m_capsLinesInherited = false;
    emit parStyleChanged();
}

void ParagraphDropCaps::dropedCharacterCountChanged(int count)
{
    Q_UNUSED(count);
    m_capsLengthInherited = false;
    emit parStyleChanged();
}
