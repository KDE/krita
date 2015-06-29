/* This file is part of the KDE project
   Copyright (C) 2004 Cedric Pasteur <cedric.pasteur@free.fr>
   Copyright (C) 2004 Alexander Dymo <cloudtemple@mskat.net>
   Copyright (C) 2008-2009 Jarosław Staniek <staniek@kde.org>

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

#include "spinbox.h"
#include "koproperty/Property.h"
#include "koproperty/Property_p.h"
#include "koproperty/EditorDataModel.h"
#include "koproperty/EditorView.h"

#include <climits>

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

#include <QVariant>
#include <QPainter>
#include <QKeyEvent>
#include <QEvent>
#include <QLineEdit>

#include <KoUnit.h>

using namespace KoProperty;

//! @return font size expressed in points (pt)
//! or if points are not available - in pixels (px) for @a font
static QString fontSizeForCSS(const QFont& font)
{
    return font.pointSize() > 0
        ? QString::fromLatin1("%1pt").arg(font.pointSize())
        : QString::fromLatin1("%1px").arg(font.pixelSize());
}

static QString cssForSpinBox(const char *_class, const QFont& font, int itemHeight)
{
        return QString::fromLatin1(
            "%5 { border-left: 0; border-right: 0; font-size: %3; } "
            "%5::down-button { height: %1px; %4 } "
            "%5::up-button { height: %2px; } "
            "QLineEdit { border-width:0px;  } "
        )
        .arg(itemHeight/2 - 1).arg(itemHeight - itemHeight/2 - 1)
        .arg(fontSizeForCSS(font))
        .arg((itemHeight/2 <= 9) ? "bottom: 2px;" : "bottom: 0px;")
        .arg(_class);
}

IntSpinBox::IntSpinBox(const Property* prop, QWidget *parent, int itemHeight)
        : KIntNumInput(parent)
        , m_unsigned( prop->type() == UInt )
{
//    kDebug() << "itemHeight:" << itemHeight;
    QLineEdit* le = spinBox()->findChild<QLineEdit*>();
    spinBox()->setContentsMargins(0,0,0,0);
    if (le) {
//        le->setFixedHeight(itemHeight);
        le->setAlignment(Qt::AlignLeft);
        le->setContentsMargins(0,0,0,0);
    }
//    kDebug() << parent->font().pointSize();
    spinBox()->setFrame(true);
    QString css = cssForSpinBox("QSpinBox", font(), itemHeight);
    Factory::setTopAndBottomBordersUsingStyleSheet(spinBox(), parent, css);
    setStyleSheet(css);

    QVariant minVal(prop->option("min", m_unsigned ? 0 : -INT_MAX));
    QVariant maxVal(prop->option("max", INT_MAX));
    QVariant step(prop->option("step", 1));
    if (!minVal.isNull() && !maxVal.isNull() && !step.isNull()) {
        setRange(minVal.toInt(), maxVal.toInt(), step.toInt());
        setSliderEnabled(prop->option("slider", false).toBool());
    }
    else {
        if (!minVal.isNull())
            setMinimum(minVal.toInt());
        if (!maxVal.isNull())
            setMaximum(maxVal.toInt());
    }

    QString minValueText(prop->option("minValueText").toString());
    if (!minValueText.isEmpty())
        setSpecialValueText(minValueText);
    connect(this, SIGNAL(valueChanged(int)), this, SLOT(slotValueChanged(int)));
}

IntSpinBox::~IntSpinBox()
{
}

QVariant IntSpinBox::value() const
{
    if (m_unsigned)
        return uint( KIntNumInput::value() );
    return KIntNumInput::value();
}

void IntSpinBox::setValue(const QVariant& value)
{
    int v( value.toInt() );
    if (m_unsigned && v<0) {
        kWarning() << "could not assign negative value" << v << "- assigning 0";
        v = 0;
    }
    KIntNumInput::setValue(v);
}

void IntSpinBox::slotValueChanged(int value)
{
    Q_UNUSED(value);
    emit commitData(this);
}

//-----------------------

DoubleSpinBox::DoubleSpinBox(const Property* prop, QWidget *parent, int itemHeight)
        : KDoubleNumInput(parent)
{
    QDoubleSpinBox* sb = findChild<QDoubleSpinBox*>();
    QLineEdit* le = 0;
    if (sb) {
        le = sb->findChild<QLineEdit*>();
        sb->setFrame(false);
    }
    if (le) {
        le->setAlignment(Qt::AlignLeft);
        le->setContentsMargins(0,0,0,0);
        le->setFrame(false);
    }
/*    Factory::setTopAndBottomBordersUsingStyleSheet(sb, parent,
        QString::fromLatin1(
            "QDoubleSpinBox { border-left: 0; border-right: 0; } "
            "QDoubleSpinBox::down-button { height: %1px; } "
            "QDoubleSpinBox::up-button { height: %2px; }"
        ).arg(itemHeight/2).arg(itemHeight - itemHeight/2)
    );*/
    QString css = cssForSpinBox("QDoubleSpinBox", font(), itemHeight);
    Factory::setTopAndBottomBordersUsingStyleSheet(sb, parent, css);
    setStyleSheet(css);

    QVariant minVal(prop->option("min", 0.0));
    QVariant maxVal(prop->option("max", double(INT_MAX / 100)));
    QVariant step(prop->option("step", KOPROPERTY_DEFAULT_DOUBLE_VALUE_STEP));
    if (!minVal.isNull() && !maxVal.isNull() && !step.isNull()) {
        bool slider = prop->option("slider", false).toBool();
        setRange(minVal.toDouble(), maxVal.toDouble(), step.toDouble(), slider);
    }
    else {
        if (!minVal.isNull())
            setMinimum(minVal.toDouble());
        if (!maxVal.isNull())
            setMaximum(maxVal.toDouble());
    }
    QVariant precision(prop->option("precision"));
    if (!precision.isNull())
        setDecimals(precision.toInt());
    QString minValueText(prop->option("minValueText").toString());
    if (!minValueText.isEmpty())
        setSpecialValueText(minValueText);
    m_unit = prop->option("unit").toString();
    connect(this, SIGNAL(valueChanged(double)), this, SLOT(slotValueChanged(double)));
}

DoubleSpinBox::~DoubleSpinBox()
{
}

void DoubleSpinBox::resizeEvent( QResizeEvent * event )
{
    QDoubleSpinBox* sb = findChild<QDoubleSpinBox*>();
    sb->setFixedHeight(height()+1);
    KDoubleNumInput::resizeEvent(event);
}

void DoubleSpinBox::setValue(double v)
{
    if (!m_unit.isEmpty()) {
        KDoubleNumInput::setValue(KoUnit::fromSymbol(m_unit).toUserValue(v));
        return;
    }
    KDoubleNumInput::setValue(v);
}

double DoubleSpinBox::value() const
{
    if (!m_unit.isEmpty()) {
        return KoUnit::fromSymbol(m_unit).fromUserValue(KDoubleNumInput::value());
    }
    return KDoubleNumInput::value();
}

void DoubleSpinBox::slotValueChanged(double value)
{
    Q_UNUSED(value);
    emit commitData(this);
}

//-----------------------

IntSpinBoxDelegate::IntSpinBoxDelegate()
{
}

QString IntSpinBoxDelegate::displayTextForProperty( const Property* prop ) const
{
    if (prop->hasOptions()) {
        //replace min value with minValueText if defined
        QVariant minValue(prop->option("min"));
        QString minValueText(prop->option("minValueText").toString());
        if (!minValue.isNull() && !minValueText.isEmpty()
            && minValue.toInt() == prop->value().toInt())
        {
            return minValueText;
        }
    }
    return QString::number(prop->value().toInt());
}

QWidget* IntSpinBoxDelegate::createEditor( int type, QWidget *parent,
    const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    Q_UNUSED(type);

    const EditorDataModel *editorModel
        = dynamic_cast<const EditorDataModel*>(index.model());
    Property *prop = editorModel->propertyForItem(index);
    return new IntSpinBox(prop, parent, option.rect.height() - 2);
}

//-----------------------

DoubleSpinBoxDelegate::DoubleSpinBoxDelegate()
{
}

QString DoubleSpinBoxDelegate::displayTextForProperty( const Property* prop ) const
{
    QString valueText;
    const QString unit(prop->option("unit").toString());
    if (prop->hasOptions()) {
        //replace min value with minValueText if defined
        QVariant minValue(prop->option("min"));
        QString minValueText(prop->option("minValueText").toString());
        if (!minValue.isNull() && !minValueText.isEmpty()
            && minValue.toDouble() == prop->value().toDouble())
        {
            if (unit.isEmpty())
                return minValueText;
            else
                return minValueText + ' ' + unit;
        }
    }
//! @todo precision?
//! @todo rounding using KLocale::formatNumber(const QString &numStr, bool round = true,int precision = 2)?
    QString display;
    if (!unit.isEmpty()) {
        return KGlobal::locale()->formatNumber(KoUnit::fromSymbol(unit).toUserValue(prop->value().toDouble())) +
               QLatin1Char(' ') + unit;
    }
    return KGlobal::locale()->formatNumber(prop->value().toDouble());
}

QWidget* DoubleSpinBoxDelegate::createEditor( int type, QWidget *parent,
    const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    Q_UNUSED(type);
    const EditorDataModel *editorModel
        = dynamic_cast<const EditorDataModel*>(index.model());
    Property *prop = editorModel->propertyForItem(index);
    return new DoubleSpinBox(prop, parent, option.rect.height() - 2 - 1);
}

#include "spinbox.moc"
