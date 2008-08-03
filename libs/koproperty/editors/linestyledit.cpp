/* This file is part of the KDE project
   Copyright (C) 2004 Cedric Pasteur <cedric.pasteur@free.fr>
   Copyright (C) 2004  Alexander Dymo <cloudtemple@mskat.net>

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

#include "linestyleedit.h"
#include "editoritem.h"

#include <QPainter>
#include <QPixmap>
#include <QComboBox>
#include <QLayout>
#include <QVariant>
#include <QHBoxLayout>
#include <QPen>

#include <libs/guiutils/KoLineStyleSelector.h>

using namespace KoProperty;

LineStyleEdit::LineStyleEdit(Property *property, QWidget *parent)
        : Widget(property, parent)
{
    QHBoxLayout *l = new QHBoxLayout(this);
    l->setMargin(0);
    l->setSpacing(0);

    m_edit = new KoLineStyleSelector(this);
    m_edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_edit->setMinimumHeight(5);
    setPlainWidgetStyle(m_edit);

    l->addWidget(m_edit);

    setLeavesTheSpaceForRevertButton(true);
    setFocusWidget(m_edit);
    connect(m_edit, SIGNAL(activated(int)), this, SLOT(slotValueChanged(int)));
}

LineStyleEdit::~LineStyleEdit()
{}

QVariant
LineStyleEdit::value() const
{
    return m_edit->lineStyle();
}

void
LineStyleEdit::setValue(const QVariant &value, bool emitChange)
{
    if (!value.canConvert(QVariant::Int))
        return;
    if ((value.toInt() > 5) || (value.toInt() < 0))
        return;

    m_edit->blockSignals(true);
    m_edit->setCurrentIndex(value.toInt());
    m_edit->blockSignals(false);
    if (emitChange)
        emit valueChanged(this);
}

void
LineStyleEdit::drawViewer(QPainter *p, const QColorGroup &, const QRect &r, const QVariant &v)
{
    p->eraseRect(r);

    if (!value().canConvert(QVariant::Int))
        return;

    QPen pen(Qt::black);
    pen.setStyle((Qt::PenStyle)(value().toInt()));
    p->setPen(pen);
    p->drawLine(r.left() + KPROPEDITOR_ITEM_MARGIN, r.center().y(), r.right(), r.center().y());
}

void
LineStyleEdit::slotValueChanged(int)
{
    emit valueChanged(this);
}

void
LineStyleEdit::setReadOnlyInternal(bool readOnly)
{
    setVisibleFlag(!readOnly);
}

#include "linestyleedit.moc"
