/* This file is part of the KDE project
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

#ifndef KPROPERTY_FACTORY_H
#define KPROPERTY_FACTORY_H

#include "koproperty_global.h"
#include "Property.h"
#include <QtCore/QObject>
#include <QtCore/QVariant>
#include <QtCore/QHash>
#include <QtGui/QLabel>
#include <QtGui/QPainter>
#include <QtGui/QStyleOptionViewItem>

class QStyleOptionViewItem;
class QModelIndex;
class QPainter;

namespace KoProperty
{

//! An interface for for composed property handlers
/*! You have to subclass ComposedPropertyInterface to override the behaviour of a property type.\n
  In the constructor, you should create the child properties (if needed).
  Then, you need to implement the functions concerning values.\n

  Example implementation of composed properties can be found in editors/ directory.
*/
class KOPROPERTY_EXPORT ComposedPropertyInterface
{
public:
    explicit ComposedPropertyInterface(Property *parent);
    virtual ~ComposedPropertyInterface();

    /*! This function modifies the child properties for parent value @a value.
     It is called by @ref Property::setValue() when
     the property is composed.
    You don't have to modify the property value, it is done by Property class.
    Note that when calling Property::setValue, you <b>need</b> to set
    useComposedProperty (the third parameter) to false, or there will be infinite recursion. */
    virtual void setValue(Property *property, const QVariant &value, bool rememberOldValue) = 0;

    void childValueChangedInternal(Property *child, const QVariant &value, bool rememberOldValue) {
      if (m_childValueChangedEnabled)
        childValueChanged(child, value, rememberOldValue);
    }

    /*! This function is called by \ref Property::value() when
    a custom property is set and \ref handleValue() is true.
    You should return property's value, taken from parent's value.*/
//    virtual QVariant value() const = 0;

    /*! Tells whether CustomProperty should be used to get the property's value.
    CustomProperty::setValue() will always be called. But if handleValue() == true,
    then the value stored in the Property won't be changed.
    You should return true for child properties, and false for others. */
/*    virtual bool handleValue() const {
        return false;
    }*/
    void setChildValueChangedEnabled(bool set) { m_childValueChangedEnabled = set; }
protected:
    virtual void childValueChanged(Property *child, const QVariant &value, bool rememberOldValue) = 0;

    //    Property  *m_property;

    /*! This method emits the \a Set::propertyChanged() signal for all
    sets our parent-property is registered in. */
    void emitPropertyChanged();
    bool m_childValueChangedEnabled : 1;
};

class KOPROPERTY_EXPORT ComposedPropertyCreatorInterface
{
public:
    ComposedPropertyCreatorInterface();
    
    virtual ~ComposedPropertyCreatorInterface();

    virtual ComposedPropertyInterface* createComposedProperty(Property *parent) const = 0;
};

//! An interface for editor widget creators.
/*! Options can be set in the options attribute in order to customize 
    widget creation process. Do this in the EditorCreatorInterface constructor.
*/
class KOPROPERTY_EXPORT EditorCreatorInterface
{
public:
    EditorCreatorInterface();
    
    virtual ~EditorCreatorInterface();
    
    virtual QWidget * createEditor( int type, QWidget *parent, 
        const QStyleOptionViewItem & option, const QModelIndex & index ) const = 0;

    /*! Options for altering the editor widget creation process, 
        used by FactoryManager::createEditor(). */
    class Options {
    public:
        Options();
        /*! In order to have better look of the widget within the property editor view, 
            we usually remove borders from the widget (see FactoryManager::createEditor()).
            and adding 1 pixel 'gray border' on the top. Default value is true. */
        bool removeBorders : 1;
    };

    //! Options for altering the editor widget creation process
    Options options;
};

class KOPROPERTY_EXPORT ValuePainterInterface
{
public:
    ValuePainterInterface();
    virtual ~ValuePainterInterface();
    virtual void paint( QPainter * painter, 
        const QStyleOptionViewItem & option, const QModelIndex & index ) const = 0;
};

class KOPROPERTY_EXPORT ValueDisplayInterface
{
public:
    ValueDisplayInterface();
    virtual ~ValueDisplayInterface();
    virtual QString displayTextForProperty( const Property* property ) const
        { return displayText(property->value()); }
    virtual QString displayText( const QVariant& value ) const
        { return value.toString(); }
};

//! Label widget that can be used for displaying text-based read-only items
//! Used in LabelCreator.
class KOPROPERTY_EXPORT Label : public QLabel
{
    Q_OBJECT
    Q_PROPERTY(QVariant value READ value WRITE setValue USER true)
public:
    Label(QWidget *parent, const ValueDisplayInterface *iface);
    QVariant value() const;
signals:
    void commitData( QWidget * editor );
public slots:
    void setValue(const QVariant& value);

protected:
    virtual void paintEvent( QPaintEvent * event );

private:
    const ValueDisplayInterface *m_iface;
    QVariant m_value;
};

//! Creator returning label
template<class Widget>
class KOPROPERTY_EXPORT EditorCreator : public EditorCreatorInterface, 
                                        public ValueDisplayInterface,
                                        public ValuePainterInterface
{
public:
    EditorCreator() : EditorCreatorInterface() {}

    virtual ~EditorCreator() {}

    virtual QWidget * createEditor( int type, QWidget *parent, 
        const QStyleOptionViewItem & option, const QModelIndex & index ) const
    {
        Q_UNUSED(type);
        Q_UNUSED(option);
        Q_UNUSED(index);
        return new Widget(parent, this);
    }

    virtual void paint( QPainter * painter, 
        const QStyleOptionViewItem & option, const QModelIndex & index ) const
    {
        painter->save();
        QRect r(option.rect);
        r.setLeft(r.left()+1);
        painter->drawText( r, Qt::AlignLeft | Qt::AlignVCenter, 
            displayText( index.data(Qt::EditRole) ) );
        painter->restore();
    }
};

typedef EditorCreator<Label> LabelCreator;

//! Creator returning composed property object
template<class ComposedProperty>
class KOPROPERTY_EXPORT ComposedPropertyCreator : public ComposedPropertyCreatorInterface
{
public:
    ComposedPropertyCreator() : ComposedPropertyCreatorInterface() {}
    
    virtual ~ComposedPropertyCreator() {}

    virtual ComposedProperty* createComposedProperty(Property *parent) const {
        return new ComposedProperty(parent);
    }
};

class KOPROPERTY_EXPORT Factory
{
public:
    Factory();
    virtual ~Factory();
    QHash<int, ComposedPropertyCreatorInterface*> composedPropertyCreators() const;
    QHash<int, EditorCreatorInterface*> editorCreators() const;
    QHash<int, ValuePainterInterface*> valuePainters() const;
    QHash<int, ValueDisplayInterface*> valueDisplays() const;

    //! Adds editor creator @a creator for type @a type.
    //! The creator becomes owned by the factory.
    void addEditor(int type, EditorCreatorInterface *creator);

    void addComposedPropertyCreator( int type, ComposedPropertyCreatorInterface* creator );

    void addPainter(int type, ValuePainterInterface *painter);

    void addDisplay(int type, ValueDisplayInterface *display);

    static void paintTopGridLine(QWidget *widget);
    static void setTopAndBottomBordersUsingStyleSheet(QWidget *widget, QWidget* parent,
        const QString& extraStyleSheet = QString());

protected:
    void addEditorInternal(int type, EditorCreatorInterface *editor, bool own = true);

    void addComposedPropertyCreatorInternal(int type, 
        ComposedPropertyCreatorInterface* creator, bool own = true);

    //! Adds value painter @a painter for type @a type.
    //! The painter becomes owned by the factory.
    void addPainterInternal(int type, ValuePainterInterface *painter, bool own = true);

    //! Adds value-to-text converted @a painter for type @a type.
    //! The converter becomes owned by the factory.
    void addDisplayInternal(int type, ValueDisplayInterface *display, bool own = true);

    class Private;
    Private * const d;
};

class Property;
class CustomProperty;

class KOPROPERTY_EXPORT FactoryManager : public QObject
{
    Q_OBJECT
public:
    bool isEditorForTypeAvailable( int type ) const;

    QWidget * createEditor(
        int type,
        QWidget *parent,
        const QStyleOptionViewItem & option,
        const QModelIndex & index ) const;

    bool paint( int type,
        QPainter * painter,
        const QStyleOptionViewItem & option, 
        const QModelIndex & index ) const;

    ComposedPropertyInterface* createComposedProperty(Property *parent);

    bool canConvertValueToText( int type ) const;

    bool canConvertValueToText( const Property* property ) const;

    QString convertValueToText( const Property* property ) const;

    //! Registers factory @a factory. It becomes owned by the manager.
    void registerFactory(Factory *factory);

    CustomProperty* createCustomProperty( Property *parent );

    /*! \return a pointer to a factory manager instance.*/
    static FactoryManager* self();
private:
//    Factory* factoryForType(int type) const;

    FactoryManager();
    ~FactoryManager();

    class Private;
    Private * const d;
};

}

#endif
