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

#include "Factory.h"
#include "DefaultFactory.h"
#include "EditorView.h"
#include "EditorDataModel.h"
#include "Property.h"
//#include "customproperty.h"
/*
#include "booledit.h"
#include "combobox.h"
#include "coloredit.h"
#include "cursoredit.h"
#include "dateedit.h"
#include "datetimeedit.h"
#include "dummywidget.h"
#include "fontedit.h"
//TODO #include "linestyleedit.h"
#include "pixmapedit.h"
#include "pointedit.h"
#include "rectedit.h"
#include "sizeedit.h"
#include "sizepolicyedit.h"
#include "spinbox.h"
#include "stringlistedit.h"
#include "stringedit.h"
#include "symbolcombo.h"
#include "timeedit.h"
#include "urledit.h"
*/
#include <KDebug>
#include <KIconLoader>

namespace KoProperty
{

Label::Label(QWidget *parent, const KoProperty::ValueDisplayInterface *iface)
    : QLabel(parent)
    , m_iface(iface)
{
  setAutoFillBackground(true);
  setContentsMargins(0,1,0,0);
  setIndent(1);
}

QVariant Label::value() const
{
    return m_value;
}

void Label::setValue(const QVariant& value)
{
    setText( m_iface->displayText(value) );
    m_value = value;
}

void Label::paintEvent( QPaintEvent * event )
{
    QLabel::paintEvent(event);
    KoProperty::Factory::paintTopGridLine(this);
}

//---------------

//! @internal
class FactoryManager::Private
{
public:
    Private()
    {
    }
    ~Private()
    {
        qDeleteAll(factories);
    }

    QSet<Factory*> factories;
    QHash<int, ComposedPropertyCreatorInterface*> composedPropertyCreators;
    QHash<int, EditorCreatorInterface*> editorCreators;
    QHash<int, ValuePainterInterface*> valuePainters;
    QHash<int, ValueDisplayInterface*> valueDisplays;
//    QHash<int, Factory*> factoryForType;
//    QHash<int, CustomPropertyFactory*> registeredCustomProperties;
};

//! @internal
class Factory::Private
{
public:
    Private()
    {
    }
    ~Private()
    {
        qDeleteAll(editorCreatorsSet);
        qDeleteAll(valuePaintersSet);
        qDeleteAll(valueDisplaysSet);
    }

    QHash<int, ComposedPropertyCreatorInterface*> composedPropertyCreators;
    QHash<int, EditorCreatorInterface*> editorCreators;
    QHash<int, ValuePainterInterface*> valuePainters;
    QHash<int, ValueDisplayInterface*> valueDisplays;
    QSet<ComposedPropertyCreatorInterface*> composedPropertyCreatorsSet;
    QSet<EditorCreatorInterface*> editorCreatorsSet;
    QSet<ValuePainterInterface*> valuePaintersSet;
    QSet<ValueDisplayInterface*> valueDisplaysSet;
};

}

using namespace KoProperty;

Factory::Factory()
    : d( new Private )
{
    KIconLoader::global()->addAppDir(KOPROPERTY_APP_DIR);
}

Factory::~Factory()
{
    delete d;
}

QHash<int, ComposedPropertyCreatorInterface*> Factory::composedPropertyCreators() const
{
    return d->composedPropertyCreators;
}

QHash<int, EditorCreatorInterface*> Factory::editorCreators() const
{
    return d->editorCreators;
}

QHash<int, ValuePainterInterface*> Factory::valuePainters() const
{
    return d->valuePainters;
}

QHash<int, ValueDisplayInterface*> Factory::valueDisplays() const
{
    return d->valueDisplays;
}

void Factory::addEditor(int type, EditorCreatorInterface *creator)
{
    addEditorInternal( type, creator, true );
    if (dynamic_cast<ComposedPropertyCreatorInterface*>(creator)) {
        addComposedPropertyCreatorInternal( type, 
            dynamic_cast<ComposedPropertyCreatorInterface*>(creator), false/* !own*/ );
    }
    if (dynamic_cast<ValuePainterInterface*>(creator)) {
        addPainterInternal( type, dynamic_cast<ValuePainterInterface*>(creator), false/* !own*/ );
    }
    if (dynamic_cast<ValueDisplayInterface*>(creator)) {
        addDisplayInternal( type, dynamic_cast<ValueDisplayInterface*>(creator), false/* !own*/ );
    }
}

void Factory::addComposedPropertyCreator( int type, ComposedPropertyCreatorInterface* creator )
{
    addComposedPropertyCreatorInternal( type, creator, true );
    if (dynamic_cast<EditorCreatorInterface*>(creator)) {
        addEditorInternal( type, dynamic_cast<EditorCreatorInterface*>(creator), false/* !own*/ );
    }
    if (dynamic_cast<ValuePainterInterface*>(creator)) {
        addPainterInternal( type, dynamic_cast<ValuePainterInterface*>(creator), false/* !own*/ );
    }
    if (dynamic_cast<ValueDisplayInterface*>(creator)) {
        addDisplayInternal( type, dynamic_cast<ValueDisplayInterface*>(creator), false/* !own*/ );
    }
}

void Factory::addPainter(int type, ValuePainterInterface *painter)
{
    addPainterInternal(type, painter, true);
    if (dynamic_cast<ComposedPropertyCreatorInterface*>(painter)) {
        addComposedPropertyCreatorInternal( type, 
        dynamic_cast<ComposedPropertyCreatorInterface*>(painter), false/* !own*/ );
    }
    if (dynamic_cast<EditorCreatorInterface*>(painter)) {
        addEditorInternal( type, dynamic_cast<EditorCreatorInterface*>(painter), false/* !own*/ );
    }
    if (dynamic_cast<ValueDisplayInterface*>(painter)) {
        addDisplayInternal( type, dynamic_cast<ValueDisplayInterface*>(painter), false/* !own*/ );
    }
}

void Factory::addDisplay(int type, ValueDisplayInterface *display)
{
    addDisplayInternal(type, display, true);
    if (dynamic_cast<ComposedPropertyCreatorInterface*>(display)) {
        addComposedPropertyCreatorInternal( type, 
        dynamic_cast<ComposedPropertyCreatorInterface*>(display), false/* !own*/ );
    }
    if (dynamic_cast<EditorCreatorInterface*>(display)) {
        addEditorInternal( type, dynamic_cast<EditorCreatorInterface*>(display), false/* !own*/ );
    }
    if (dynamic_cast<ValueDisplayInterface*>(display)) {
        addDisplayInternal( type, dynamic_cast<ValueDisplayInterface*>(display), false/* !own*/ );
    }
}

void Factory::addEditorInternal(int type, EditorCreatorInterface *editor, bool own)
{
    if (own)
        d->editorCreatorsSet.insert(editor);
    d->editorCreators.insert(type, editor);
}

void Factory::addComposedPropertyCreatorInternal(int type, ComposedPropertyCreatorInterface* creator, bool own)
{
    if (own)
        d->composedPropertyCreatorsSet.insert(creator);
    d->composedPropertyCreators.insert(type, creator);
}

void Factory::addPainterInternal(int type, ValuePainterInterface *painter, bool own)
{
    if (own)
        d->valuePaintersSet.insert(painter);
    d->valuePainters.insert(type, painter);
}

void Factory::addDisplayInternal(int type, ValueDisplayInterface *display, bool own)
{
    if (own)
        d->valueDisplaysSet.insert(display);
    d->valueDisplays.insert(type, display);
}

//static
void Factory::paintTopGridLine(QWidget *widget)
{
    // paint top grid line
    QPainter p(widget);
    QColor gridLineColor( dynamic_cast<EditorView*>(widget->parentWidget()) ? 
        dynamic_cast<EditorView*>(widget->parentWidget())->gridLineColor()
        : EditorView::defaultGridLineColor() );
    p.setPen(QPen( QBrush(gridLineColor), 1));
    p.drawLine(0, 0, widget->width()-1, 0);
}

//static
void Factory::setTopAndBottomBordersUsingStyleSheet(QWidget *widget, QWidget* parent, const QString& extraStyleSheet)
{
    QColor gridLineColor( dynamic_cast<KoProperty::EditorView*>(parent) ? 
        dynamic_cast<KoProperty::EditorView*>(parent)->gridLineColor()
        : KoProperty::EditorView::defaultGridLineColor() );
    widget->setStyleSheet(
        QString::fromLatin1("%1 { border-top: 1px solid %2;border-bottom: 1px solid %2; } %3")
        .arg(widget->metaObject()->className()).arg(gridLineColor.name()).arg(extraStyleSheet));
}

//------------

FactoryManager::FactoryManager()
        : QObject(0)
        , d(new Private)
{
    setObjectName("KoProperty::FactoryManager");
    registerFactory(new DefaultFactory);
}

FactoryManager::~FactoryManager()
{
    delete d;
}

FactoryManager* FactoryManager::self()
{
    K_GLOBAL_STATIC(KoProperty::FactoryManager, _self);
    return _self;
}

void FactoryManager::registerFactory(Factory *factory)
{
    d->factories.insert(factory);
    QHash<int, ComposedPropertyCreatorInterface*>::ConstIterator composedPropertyCreatorsItEnd
        = factory->composedPropertyCreators().constEnd();
    for (QHash<int, ComposedPropertyCreatorInterface*>::ConstIterator it( factory->composedPropertyCreators().constBegin() );
        it != composedPropertyCreatorsItEnd; ++it)
    {
        d->composedPropertyCreators.insert(it.key(), it.value());
    }
    QHash<int, EditorCreatorInterface*>::ConstIterator editorCreatorsItEnd 
        = factory->editorCreators().constEnd();
    for (QHash<int, EditorCreatorInterface*>::ConstIterator it( factory->editorCreators().constBegin() );
        it != editorCreatorsItEnd; ++it)
    {
        d->editorCreators.insert(it.key(), it.value());
    }
    QHash<int, ValuePainterInterface*>::ConstIterator valuePaintersItEnd
        = factory->valuePainters().constEnd();
    for (QHash<int, ValuePainterInterface*>::ConstIterator it( factory->valuePainters().constBegin() );
        it != valuePaintersItEnd; ++it)
    {
        d->valuePainters.insert(it.key(), it.value());
    }
    QHash<int, ValueDisplayInterface*>::ConstIterator valueDisplaysItEnd
        = factory->valueDisplays().constEnd();
    for (QHash<int, ValueDisplayInterface*>::ConstIterator it( factory->valueDisplays().constBegin() );
        it != valueDisplaysItEnd; ++it)
    {
        d->valueDisplays.insert(it.key(), it.value());
    }
}

bool FactoryManager::isEditorForTypeAvailable( int type ) const
{
    return d->editorCreators.value(type);
}

QWidget * FactoryManager::createEditor( 
    int type, QWidget *parent,
    const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    const EditorCreatorInterface *creator = d->editorCreators.value(type);
    if (!creator)
        return 0;
    QWidget *w = creator->createEditor(type, parent, option, index);
    if (w) {
       const EditorDataModel *editorModel
           = dynamic_cast<const EditorDataModel*>(index.model());
       Property *property = editorModel->propertyForItem(index);
       w->setObjectName( property->name() );
       if (creator->options.removeBorders) {
//! @todo get real border color from the palette
            QColor gridLineColor( dynamic_cast<EditorView*>(parent) ? 
                dynamic_cast<EditorView*>(parent)->gridLineColor()
                : EditorView::defaultGridLineColor() );
            QString css =
//                w->styleSheet() + " " + 
                QString::fromLatin1("%1 { border-top: 1px solid %2; } ")
                .arg(QString::fromLatin1(w->metaObject()->className()).replace("KoProperty::", QString()))
                .arg(gridLineColor.name());
//            kDebug() << css;
            w->setStyleSheet(css);
        }
    }
    return w;
}

bool FactoryManager::paint( int type, QPainter * painter, 
    const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    const ValuePainterInterface *_painter = d->valuePainters.value(type);
    if (!_painter)
        return false;
    QStyleOptionViewItem realOption(option);
    if (option.state & QStyle::State_Selected) {
        // paint background because there may be editor widget with no autoFillBackground set
        realOption.palette.setBrush(QPalette::Text, realOption.palette.highlightedText());
        painter->fillRect(realOption.rect, realOption.palette.highlight());
    }
    painter->setPen(realOption.palette.text().color());
    _painter->paint(painter, realOption, index);
    return true;
}

bool FactoryManager::canConvertValueToText( int type ) const
{
    return d->valueDisplays.value(type) != 0;
}

bool FactoryManager::canConvertValueToText( const Property* property ) const
{
    return d->valueDisplays.value( property->type() ) != 0;
}

QString FactoryManager::convertValueToText( const Property* property ) const
{
    const ValueDisplayInterface *display = d->valueDisplays.value( property->type() );
    return display ? display->displayTextForProperty( property ) : property->value().toString();
}

ComposedPropertyInterface* FactoryManager::createComposedProperty(Property *parent)
{
    const ComposedPropertyCreatorInterface *creator = d->composedPropertyCreators.value( parent->type() );
    return creator ? creator->createComposedProperty(parent) : 0;
}

#if 0
    const int type = parent->type();
/* TODO
    CustomPropertyFactory *factory = d->registeredWidgets[type];
    if (factory)
        return factory->createCustomProperty(parent);
*/
    switch (type) {
    case Size:
    case Size_Width:
    case Size_Height:
        return new SizeCustomProperty(parent);
    case Point:
    case Point_X:
    case Point_Y:
        return new PointCustomProperty(parent);
    case Rect:
    case Rect_X:
    case Rect_Y:
    case Rect_Width:
    case Rect_Height:
        return new RectCustomProperty(parent);
    case SizePolicy:
/*    case SizePolicy_HorizontalStretch:
    case SizePolicy_VerticalStretch:
    case SizePolicy_HorizontalPolicy:
    case SizePolicy_VerticalPolicy:*/
        return new SizePolicyCustomProperty(parent);
    default:;
    }
    return 0;
#endif

ComposedPropertyInterface::ComposedPropertyInterface(Property *parent)
 : m_childValueChangedEnabled(true)
{
    Q_UNUSED(parent)
}

ComposedPropertyInterface::~ComposedPropertyInterface()
{
}

ComposedPropertyCreatorInterface::ComposedPropertyCreatorInterface()
{
}

ComposedPropertyCreatorInterface::~ComposedPropertyCreatorInterface()
{
}

EditorCreatorInterface::EditorCreatorInterface()
{
}

EditorCreatorInterface::~EditorCreatorInterface()
{
}

EditorCreatorInterface::Options::Options()
 : removeBorders(true)
{
}

ValuePainterInterface::ValuePainterInterface()
{
}

ValuePainterInterface::~ValuePainterInterface()
{
}

ValueDisplayInterface::ValueDisplayInterface()
{
}

ValueDisplayInterface::~ValueDisplayInterface()
{
}
