/* This file is part of the KDE project
   Copyright (C) 2004 Cedric Pasteur <cedric.pasteur@free.fr>
   Copyright (C) 2004 Alexander Dymo <cloudtemple@mskat.net>
   Copyright (C) 2008 Jarosław Staniek <staniek@kde.org>

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

#ifndef KPROPERTY_SPINBOX_H
#define KPROPERTY_SPINBOX_H

#include "koproperty/Factory.h"

#include <KNumInput>

namespace KoProperty {

//! A delegate supporting Int and UInt types
/*! Note that due to KIntNumInput limitations, for UInt the maximum value 
    is INT_MAX, not UINT_MAX.
*/
class KOPROPERTY_EXPORT IntSpinBox : public KIntNumInput
{
    Q_OBJECT
    Q_PROPERTY(QVariant value READ value WRITE setValue USER true)

public:
    IntSpinBox(const Property* prop, QWidget *parent, int itemHeight);
    virtual ~IntSpinBox();

    QVariant value() const;

//    virtual void setProperty(const Property *prop);
    
//    virtual void drawViewer(QPainter *p, const QColorGroup &cg, const QRect &r, const QVariant &value);

signals:
    void commitData(QWidget* editor);

public slots:
    void setValue(const QVariant& value);

//todo?    virtual bool eventFilter(QObject *o, QEvent *e);
/*    QLineEdit * lineEdit() const {
        return KIntSpinBox::lineEdit();
    }*/
protected slots:
    void slotValueChanged(int value);

private:
    bool m_unsigned;
};

/*class KOPROPERTY_EXPORT IntEdit : public Widget
{
    Q_OBJECT

public:
    IntEdit(Property *property, QWidget *parent = 0);
    virtual ~IntEdit();

    virtual QVariant value() const;
    virtual void setValue(const QVariant &value, bool emitChange = true);
    virtual void drawViewer(QPainter *p, const QColorGroup &cg, const QRect &r, const QVariant &value);

protected:
    virtual void setReadOnlyInternal(bool readOnly);
    void updateSpinWidgets();

protected slots:
    void slotValueChanged(int value);

private:
    IntSpinBox  *m_edit;
};*/

// Double editor

class KOPROPERTY_EXPORT DoubleSpinBox : public KDoubleNumInput
{
    Q_OBJECT
    Q_PROPERTY(double value READ value WRITE setValue USER true)

public:
//! @todo Support setting precision limits, step, etc.
    DoubleSpinBox(const Property* prop, QWidget *parent, int itemHeight);
    virtual ~DoubleSpinBox();

//    virtual bool eventFilter(QObject *o, QEvent *e);
/*    QLineEdit * lineEdit() const {
        return QDoubleSpinBox::lineEdit();
    }*/

    double value() const;

signals:
    void commitData(QWidget* editor);

public slots:
    void setValue(double value);

protected slots:
    void slotValueChanged(double value);

protected:
    //! Used to fix height of the internal spin box
    virtual void resizeEvent( QResizeEvent * event );

    QString m_unit;
};

/*
class KOPROPERTY_EXPORT DoubleEdit : public Widget
{
    Q_OBJECT

public:
    DoubleEdit(Property *property, QWidget *parent = 0);
    virtual ~DoubleEdit();

    virtual QVariant value() const;
    virtual void setValue(const QVariant &value, bool emitChange = true);
    virtual void drawViewer(QPainter *p, const QColorGroup &cg, const QRect &r, const QVariant &value);

protected:
    virtual void setReadOnlyInternal(bool readOnly);
    void updateSpinWidgets();

protected slots:
    void slotValueChanged(double value);

private:
    DoubleSpinBox  *m_edit;
};*/

//! A delegate supporting Int, UInt, LongLong and ULongLong types
class KOPROPERTY_EXPORT IntSpinBoxDelegate : public EditorCreatorInterface, 
                                             public ValueDisplayInterface
{
public:
    IntSpinBoxDelegate();
    
    virtual QString displayTextForProperty( const Property* prop ) const;

    virtual QWidget * createEditor( int type, QWidget *parent, 
        const QStyleOptionViewItem & option, const QModelIndex & index ) const;
};

class KOPROPERTY_EXPORT DoubleSpinBoxDelegate : public EditorCreatorInterface, 
                              public ValueDisplayInterface
{
public:
    DoubleSpinBoxDelegate();
    
    virtual QString displayTextForProperty( const Property* prop ) const;

    virtual QWidget * createEditor( int type, QWidget *parent, 
        const QStyleOptionViewItem & option, const QModelIndex & index ) const;
};

}

#endif
