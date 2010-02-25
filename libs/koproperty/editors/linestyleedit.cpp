/* This file is part of the KDE project
   Copyright (C) 2010 Jarosław Staniek <staniek@kde.org>

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

#include <QApplication>
#include <QPainter>
#include <QPen>

#include <KColorScheme>
#include <KDebug>

using namespace KoProperty;

LineStyleCombo::LineStyleCombo(QWidget *parent)
    : KoLineStyleSelector(parent)
{
    setFrame(false);
    connect(this, SIGNAL(activated(int)), this, SLOT(slotValueChanged(int)));

    QString styleSheet;
    KColorScheme cs(QPalette::Active);
    QColor focus = cs.decoration(KColorScheme::FocusColor).color();

    styleSheet = QString("KoLineStyleSelector { \
    border: 1px solid %1; \
    border-radius: 0px; \
    padding: 0px 18px; }").arg(focus.name());

    setStyleSheet(styleSheet);
}

LineStyleCombo::~LineStyleCombo()
{
}

QVariant LineStyleCombo::value() const
{
    return lineStyle();
}

static bool hasVisibleStyle(const QVariant &value)
{
    return !value.isNull() && value.canConvert(QVariant::Int) && value.toInt() < Qt::CustomDashLine;
}

void LineStyleCombo::setValue(const QVariant &value)
{
    if (!hasVisibleStyle(value)) {
        setLineStyle(Qt::NoPen);
        return;
    }
    setLineStyle(static_cast<Qt::PenStyle>(value.toInt()));
}

void LineStyleCombo::slotValueChanged(int)
{
    emit commitData(this);
}

QWidget * LineStyleComboDelegate::createEditor( int type, QWidget *parent,
    const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    Q_UNUSED(type)
    Q_UNUSED(option)
    Q_UNUSED(index)
    return new LineStyleCombo(parent);
}

void LineStyleComboDelegate::paint( QPainter * painter,
    const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    painter->save();
    QPen pen(Qt::NoPen);
    if (hasVisibleStyle(index.data(Qt::EditRole))) {
        pen.setBrush(option.palette.text());
        pen.setWidth(3);
        pen.setStyle(static_cast<Qt::PenStyle>(index.data(Qt::EditRole).toInt()));
    }
    painter->setPen(pen);
    const QWidget *paintedWidget = dynamic_cast<QWidget*>(painter->device());
    const QStyle *style = paintedWidget ? paintedWidget->style() : qApp->style();
    QStyleOptionComboBox cbOption;
    cbOption.rect = option.rect;
    QRect r = style->subControlRect(QStyle::CC_ComboBox, &cbOption, QStyle::SC_ComboBoxEditField, 0);
    r.setRight(option.rect.right() - (r.left() - option.rect.left()));
    painter->drawLine(r.left(), r.center().y(), r.right(), r.center().y());
    painter->restore();
}

#include "linestyleedit.moc"
