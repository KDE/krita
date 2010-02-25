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

#include <KGlobal>
#include <KLocale>
#include <KDebug>

#include <QVariant>
#include <QPainter>
#include <QKeyEvent>
#include <QEvent>
#include <QLineEdit>

#ifdef KOPROPERTY_USE_KOLIBS
# include <KoUnit.h>
#endif

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
    setMinimum(minVal.toInt());
    setMaximum(maxVal.toInt());
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

/*void IntSpinBox::setValue(const QVariant &value)
{
    if (dynamic_cast<IntEdit*>(parentWidget()) && dynamic_cast<IntEdit*>(parentWidget())->isReadOnly())
        return;
    if (value.isNull())
        lineEdit()->clear();
    else
        KIntSpinBox::setValue(value.toInt());
}*/

/* TODO?
bool
IntSpinBox::eventFilter(QObject *o, QEvent *e)
{
    if (o == lineEdit()) {
        if (e->type() == QEvent::KeyPress) {
            QKeyEvent* ev = static_cast<QKeyEvent*>(e);
            if ((ev->key() == Qt::Key_Up || ev->key() == Qt::Key_Down) && ev->modifiers() != Qt::ControlModifier) {
                parentWidget()->eventFilter(o, e);
                return true;
            }
        }
    }
    if ((o == lineEdit() || o == this || o->parent() == this)
            && e->type() == QEvent::Wheel && static_cast<IntEdit*>(parentWidget())->isReadOnly()) {
        return true; //avoid value changes for read-only widget
    }

    return KIntSpinBox::eventFilter(o, e);
}
*/

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
IntEdit::IntEdit(Property *property, QWidget *parent)
        : Widget(property, parent)
{
    QVariant minVal(property ? property->option("min") : 0);
    QVariant maxVal(property ? property->option("max") : QVariant());
    QVariant minValueText(property ? property->option("minValueText") : QVariant());
    if (minVal.isNull())
        minVal = 0;
    if (maxVal.isNull())
        maxVal = INT_MAX;

    m_edit = new IntSpinBox(minVal.toInt(), maxVal.toInt(), 1, 0, 10, this);
    if (!minValueText.isNull())
        m_edit->setSpecialValueText(minValueText.toString());
    m_edit->setMinimumHeight(5);
    setPlainWidgetStyle(m_edit);

    setEditor(m_edit);

    setLeavesTheSpaceForRevertButton(true);
    setFocusWidget(m_edit);
    connect(m_edit, SIGNAL(valueChanged(int)), this, SLOT(slotValueChanged(int)));
}

IntEdit::~IntEdit()
{}

QVariant
IntEdit::value() const
{
    if (m_edit->cleanText().isEmpty())
        return QVariant();
    return m_edit->value();
}

void
IntEdit::setValue(const QVariant &value, bool emitChange)
{
    m_edit->blockSignals(true);
    m_edit->setValue(value);
    updateSpinWidgets();
    m_edit->blockSignals(false);
    if (emitChange)
        emit valueChanged(this);
}

void
IntEdit::drawViewer(QPainter *p, const QColorGroup &cg, const QRect &r, const QVariant &value)
{
    QString valueText = value.toString();
    if (property() && property()->hasOptions()) {
        //replace min value with minValueText if defined
        QVariant minValue(property()->option("min"));
        QVariant minValueText(property()->option("minValueText"));
        if (!minValue.isNull() && !minValueText.isNull() && minValue.toInt() == value.toInt()) {
            valueText = minValueText.toString();
        }
    }

    Widget::drawViewer(p, cg, r, valueText);
// p->eraseRect(r);
// p->drawText(r, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine, valueText);
}

void
IntEdit::slotValueChanged(int)
{
    emit valueChanged(this);
}

void
IntEdit::updateSpinWidgets()
{
    // NOTE: If this code must be compiled with MSVC 6, replace findchildren with qFindChildren
    // An empty string matches all object names.
    QList<QAbstractSpinBox*> spinwidgets = findChildren<QAbstractSpinBox*>("");
#ifndef Q_WS_WIN
#ifdef __GNUC__
#warning TODO: fix for Qt4
#endif
#endif
    QAbstractSpinBox* spin = spinwidgets.isEmpty() ? 0 : spinwidgets.first();
    if (spin) {
        spin->setReadOnly(isReadOnly());
    }
}

void
IntEdit::setReadOnlyInternal(bool readOnly)
{
    //disable editor and spin widget
    m_edit->lineEdit()->setReadOnly(readOnly);
    updateSpinWidgets();
    if (readOnly)
        setLeavesTheSpaceForRevertButton(false);
}
*/

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DoubleSpinBox::DoubleSpinBox(const Property* prop, QWidget *parent, int itemHeight)
        : KDoubleNumInput(parent)
{
    QDoubleSpinBox* sb = findChild<QDoubleSpinBox*>();
    QLineEdit* le = 0;
    if (sb) {
        le = sb->findChild<QLineEdit*>();
    }
    if (le) {
        le->setAlignment(Qt::AlignLeft);
        le->setContentsMargins(0,0,0,0);
    }
    sb->setFrame(false);
    le->setFrame(false);
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
    bool slider(prop->option("slider", false).toBool());
    if (!minVal.isNull() && !maxVal.isNull() && !step.isNull()) {
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
#ifdef KOPROPERTY_USE_KOLIBS
    if (!m_unit.isEmpty()) {
        KDoubleNumInput::setValue(KoUnit::unit(m_unit).toUserValue(v));
        return;
    }
#endif
    KDoubleNumInput::setValue(v);
}

double DoubleSpinBox::value() const
{
#ifdef KOPROPERTY_USE_KOLIBS
    if (!m_unit.isEmpty()) {
        return KoUnit::unit(m_unit).fromUserValue(KDoubleNumInput::value());
    }
#endif
    return KDoubleNumInput::value();
}

void DoubleSpinBox::slotValueChanged(double value)
{
    Q_UNUSED(value);
    emit commitData(this);
}

/*
bool
DoubleSpinBox::eventFilter(QObject *o, QEvent *e)
{
    if (o == lineEdit()) {
        if (e->type() == QEvent::KeyPress) {
            QKeyEvent* ev = static_cast<QKeyEvent*>(e);
            if ((ev->key() == Qt::Key_Up || ev->key() == Qt::Key_Down) && ev->modifiers() != Qt::ControlModifier) {
                parentWidget()->eventFilter(o, e);
                return true;
            }
        }
    }
    if ((o == lineEdit() || o == this || o->parent() == this)
            && e->type() == QEvent::Wheel && static_cast<IntEdit*>(parentWidget())->isReadOnly()) {
        return true; //avoid value changes for read-only widget
    }

    return QDoubleSpinBox::eventFilter(o, e);
}

void DoubleSpinBox::setValue(const QVariant& value)
{
    if (dynamic_cast<DoubleEdit*>(parentWidget()) && dynamic_cast<DoubleEdit*>(parentWidget())->isReadOnly())
        return;
    if (value.isNull())
        lineEdit()->clear();
    else
        QDoubleSpinBox::setValue(value.toDouble());
}*/

/*
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DoubleEdit::DoubleEdit(Property *property, QWidget *parent)
        : Widget(property, parent)
{
    QVariant minVal(property ? property->option("min") : 0);
    QVariant maxVal(property ? property->option("max") : QVariant());
    QVariant step(property ? property->option("step") : QVariant());
    QVariant precision(property ? property->option("precision") : QVariant());
    QVariant minValueText(property ? property->option("minValueText") : QVariant());
    if (minVal.isNull())
        minVal = 0;
    if (maxVal.isNull())
        maxVal = (double)(INT_MAX / 100);
    if (step.isNull())
        step = 0.1;
    if (precision.isNull())
        precision = 2;

    m_edit = new DoubleSpinBox(minVal.toDouble(), maxVal.toDouble(), step.toDouble(),
                               0, precision.toInt(), this);
    if (!minValueText.isNull())
        m_edit->setSpecialValueText(minValueText.toString());
    m_edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_edit->setMinimumHeight(5);
    setPlainWidgetStyle(m_edit);

    setEditor(m_edit);

    setLeavesTheSpaceForRevertButton(true);
    setFocusWidget(m_edit);
    connect(m_edit, SIGNAL(valueChanged(double)), this, SLOT(slotValueChanged(double)));
}

DoubleEdit::~DoubleEdit()
{}

QVariant
DoubleEdit::value() const
{
    if (m_edit->cleanText().isEmpty())
        return QVariant();
    return m_edit->value();
}

void
DoubleEdit::setValue(const QVariant &value, bool emitChange)
{
    m_edit->blockSignals(true);
    m_edit->setValue(value);
    updateSpinWidgets();
    m_edit->blockSignals(false);
    if (emitChange)
        emit valueChanged(this);
}

void
DoubleEdit::drawViewer(QPainter *p, const QColorGroup &cg, const QRect &r, const QVariant &value)
{
    QString valueText;
    if (property() && property()->hasOptions()) {
        //replace min value with minValueText if defined
        QVariant minValue(property()->option("min"));
        QVariant minValueText(property()->option("minValueText"));
        if (!minValue.isNull() && !minValueText.isNull() && minValue.toString().toDouble() == value.toString().toDouble()) {
            valueText = minValueText.toString();
        }
    }
    if (valueText.isEmpty())
        valueText = QString(value.toString()).replace('.', KGlobal::locale()->decimalSymbol());

    Widget::drawViewer(p, cg, r, valueText);
// p->eraseRect(r);
// p->drawText(r, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine, valueText);
}

void
DoubleEdit::slotValueChanged(double)
{
    emit valueChanged(this);
}

void
DoubleEdit::updateSpinWidgets()
{
    // NOTE: If this code must be compiled with MSVC 6, replace findchildren with qFindChildren
    // An empty string matches all object names.
    QList<QAbstractSpinBox*> spinwidgets = findChildren<QAbstractSpinBox*>("");
#ifndef Q_WS_WIN
#ifdef __GNUC__
#warning TODO: fix for Qt4
#endif
#endif
    QAbstractSpinBox* spin = spinwidgets.isEmpty() ? 0 : static_cast<QAbstractSpinBox*>(spinwidgets.first());
    if (spin) {
        spin->setReadOnly(isReadOnly());
    }
}

void
DoubleEdit::setReadOnlyInternal(bool readOnly)
{
    //disable editor and spin widget
    m_edit->lineEdit()->setReadOnly(readOnly);
    updateSpinWidgets();
    if (readOnly)
        setLeavesTheSpaceForRevertButton(false);
}
*/

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
                return minValueText + " " + unit;
        }
    }
//! @todo precision? 
//! @todo rounding using KLocale::formatNumber(const QString &numStr, bool round = true,int precision = 2)?
    QString display;
#ifdef KOPROPERTY_USE_KOLIBS
    if (!unit.isEmpty()) {
        return KGlobal::locale()->formatNumber(KoUnit::unit(unit).toUserValue(prop->value().toDouble())) + " " + unit;
    }
#endif
    return KGlobal::locale()->formatNumber(prop->value().toDouble());
}

QWidget* DoubleSpinBoxDelegate::createEditor( int type, QWidget *parent, 
    const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    const EditorDataModel *editorModel
        = dynamic_cast<const EditorDataModel*>(index.model());
    Property *prop = editorModel->propertyForItem(index);
    return new DoubleSpinBox(prop, parent, option.rect.height() - 2 - 1);
}

#include "spinbox.moc"
