/* This file is part of the KDE project
   Copyright (C) 2004 Cedric Pasteur <cedric.pasteur@free.fr>

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

#include "customproperty.h"
#include "property.h"

#include <QSize>
#include <QRect>
#include <QSizePolicy>
#include <QPoint>
//Added by qt3to4:
#include <Q3ValueList>

#ifdef QT_ONLY
// \todo
#else
#include <klocale.h>
#include <kdebug.h>
#endif

using namespace KoProperty;

CustomProperty::CustomProperty(Property *parent)
 : m_property(parent)
{
}

CustomProperty::~CustomProperty()
{
}

///////////////  SizeCustomProperty /////////////////////

SizeCustomProperty::SizeCustomProperty(Property *property)
: CustomProperty(property)
{
	if(property && (property->type() == Size) ) {
		QSize s = property->value().toSize();
		new Property("width", s.width(), i18n("Width"), i18n("Width"), Size_Width, property);
		new Property("height", s.height(), i18n("Height"), i18n("Height"), Size_Height, property);
	}
}

SizeCustomProperty::~SizeCustomProperty()
{}

bool
SizeCustomProperty::handleValue() const
{
	if(!m_property)
		return false;

	switch(m_property->type()) {
		case Size_Width: case Size_Height:
			return true;
		default:
			return false;
	}
}

void
SizeCustomProperty::setValue(const QVariant &value, bool rememberOldValue)
{
	if(!m_property)
		return;

	if(m_property->parent()) {
		QSize s = m_property->parent()->value().toSize();

		if(m_property->type() == Size_Height)
			s.setHeight(value.toInt());
		else if(m_property->type() == Size_Width)
			s.setWidth(value.toInt());

		m_property->parent()->setValue(s, true, false);
	}
	else{
		QSize s = value.toSize();
		m_property->child("width")->setValue(s.width(), rememberOldValue, false);
		m_property->child("height")->setValue(s.height(), rememberOldValue, false);
	}
}

QVariant
SizeCustomProperty::value() const
{
	if(!m_property || !m_property->parent())
		return QVariant();

	if(m_property->type() == Size_Height)
		return m_property->parent()->value().toSize().height();
	else if(m_property->type() == Size_Width)
		return m_property->parent()->value().toSize().width();

	return QVariant();
}

///////////////  PointCustomProperty /////////////////////

PointCustomProperty::PointCustomProperty(Property *property)
: CustomProperty(property)
{
	if(property && (property->type() == Point) ) {
		QPoint p = property->value().toPoint();
		new Property("x", p.x(), i18n("X"), i18n("X"), Point_X, property);
		new Property("y", p.y(), i18n("Y"), i18n("Y"), Point_Y, property);
	}
}

PointCustomProperty::~PointCustomProperty()
{}

bool
PointCustomProperty::handleValue() const
{
	if(!m_property)
		return false;

	switch(m_property->type()) {
		case Point_X: case Point_Y:
			return true;
		default:
			return false;
	}
}

void
PointCustomProperty::setValue(const QVariant &value, bool rememberOldValue)
{
	if(!m_property)
		return;

	if(m_property->parent()) {
		QPoint p = m_property->parent()->value().toPoint();

		if(m_property->type() == Point_X)
			p.setX(value.toInt());
		else if(m_property->type() == Point_Y)
			p.setY(value.toInt());

		m_property->parent()->setValue(p, true, false);
	}
	else {
		QPoint p = value.toPoint();
		m_property->child("x")->setValue(p.x(), rememberOldValue, false);
		m_property->child("y")->setValue(p.y(), rememberOldValue, false);
	}
}

QVariant
PointCustomProperty::value() const
{
	if(!m_property || !m_property->parent())
		return QVariant();

	if(m_property->type() == Point_X)
		return m_property->parent()->value().toPoint().x();
	else if(m_property->type() == Point_Y)
		return m_property->parent()->value().toPoint().y();

	return QVariant();
}

///////////////  RectCustomProperty /////////////////////

RectCustomProperty::RectCustomProperty(Property *property)
: CustomProperty(property)
{
	if(property && (property->type() == Rect) ) {
		QRect r = property->value().toRect();
		new Property("x", r.x(), i18n("X"), i18n("X"), Rect_X, property);
		new Property("y", r.y(), i18n("Y"), i18n("Y"), Rect_Y, property);
		new Property("width", r.width(), i18n("Width"), i18n("Width"), Rect_Width, property);
		new Property("height", r.height(), i18n("Height"), i18n("Height"), Rect_Height, property);
	}
}

RectCustomProperty::~RectCustomProperty()
{}

bool
RectCustomProperty::handleValue() const
{
	if(!m_property)
		return false;

	switch(m_property->type()) {
		case Rect_X: case Rect_Y: case Rect_Width: case Rect_Height:
			return true;
		default:
			return false;
	}
}

void
RectCustomProperty::setValue(const QVariant &value, bool rememberOldValue)
{
	if(!m_property)
		return;

	if(m_property->parent()) {
		QRect r = m_property->parent()->value().toRect();

		if(m_property->type() == Rect_X) {
			//changing x component of Rect shouldn't change width
			const int delta = value.toInt() - r.x();
			r.setX(value.toInt());
			r.setWidth(r.width()+delta);
		}
		else if(m_property->type() == Rect_Y) {
			//changing y component of Rect shouldn't change height
			const int delta = value.toInt() - r.y();
			r.setY(value.toInt());
			r.setHeight(r.height()+delta);
		}
		else if(m_property->type() == Rect_Width)
			r.setWidth(value.toInt());
		else if(m_property->type() == Rect_Height)
			r.setHeight(value.toInt());

		m_property->parent()->setValue(r, true, false);
	}
	else {
		QRect r = value.toRect();
		m_property->child("x")->setValue(r.x(), rememberOldValue, false);
		m_property->child("y")->setValue(r.y(), rememberOldValue, false);
		m_property->child("width")->setValue(r.width(), rememberOldValue, false);
		m_property->child("height")->setValue(r.height(), rememberOldValue, false);
	}
}

QVariant
RectCustomProperty::value() const
{
	if(!m_property || !m_property->parent())
		return QVariant();

	if(m_property->type() == Rect_X)
		return m_property->parent()->value().toRect().x();
	else if(m_property->type() == Rect_Y)
		return m_property->parent()->value().toRect().y();
	else if(m_property->type() == Rect_Width)
		return m_property->parent()->value().toRect().width();
	else if(m_property->type() == Rect_Height)
		return m_property->parent()->value().toRect().height();

	return QVariant();
}


///////////////  SizePolicyCustomProperty /////////////////////

SizePolicyCustomProperty::SizePolicyCustomProperty(Property *property)
: CustomProperty(property)
{
#warning "kde4: port QVariant::SizePolicy"
#if 0
	if(property && (property->type() == SizePolicy) ) {
//		QMap<QString, QVariant> spValues;
		Q3ValueList<QVariant> keys;
		keys << QSizePolicy::Fixed
			<< QSizePolicy::Minimum
			<< QSizePolicy::Maximum
			<< QSizePolicy::Preferred
			<< QSizePolicy::Expanding
			<< QSizePolicy::MinimumExpanding
			<< QSizePolicy::Ignored;
		QStringList strings;
		strings << i18n("Size Policy", "Fixed")
			<< i18n("Size Policy", "Minimum")
			<< i18n("Size Policy", "Maximum")
			<< i18n("Size Policy", "Preferred")
			<< i18n("Size Policy", "Expanding")
			<< i18n("Size Policy", "Minimum Expanding")
			<< i18n("Size Policy", "Ignored");

		new Property("hSizeType", new Property::ListData(keys, strings),
			(int)property->value().toSizePolicy().horData(), 
			i18n("Horz. Size Type"),i18n("Horizontal Size Type"),
			SizePolicy_HorData, property);
		new Property("vSizeType", new Property::ListData(keys, strings),
			(int)property->value().toSizePolicy().verData(), 
			i18n("Vert. Size Type"), i18n("Vertical Size Type"),
			SizePolicy_VerData, property);
		new Property("hStretch", 
			property->value().toSizePolicy().horStretch(), 
			i18n("Horz. Stretch"), i18n("Horizontal Stretch"),
			SizePolicy_HorStretch, property);
		new Property("vStretch", 
			property->value().toSizePolicy().verStretch(), 
			i18n("Vert. Stretch"), i18n("Vertical Stretch"),
			SizePolicy_VerStretch, property);
	}
#endif	
}

SizePolicyCustomProperty::~SizePolicyCustomProperty()
{
}

bool
SizePolicyCustomProperty::handleValue() const
{
	if(!m_property)
		return false;

	switch(m_property->type()) {
		case SizePolicy_HorData:
		case SizePolicy_VerData:
		case SizePolicy_HorStretch:
		case SizePolicy_VerStretch:
			return true;
		default:
			return false;
	}
}

void
SizePolicyCustomProperty::setValue(const QVariant &value, bool rememberOldValue)
{
	if(!m_property)
		return;

	if(m_property->parent()) {
#warning "kde4: port it"			
#if 0			
		QSizePolicy v = m_property->parent()->value().toSizePolicy();

		if(m_property->type() == SizePolicy_HorData)
			v.setHorData(QSizePolicy::SizeType(value.toInt()));
		else if(m_property->type() == SizePolicy_VerData)
			v.setVerData(QSizePolicy::SizeType(value.toInt()));
		else if(m_property->type() == SizePolicy_HorStretch)
			v.setHorStretch(value.toInt());
		else if(m_property->type() == SizePolicy_VerStretch)
			v.setVerStretch(value.toInt());

		m_property->parent()->setValue(v, true, false);
#endif		
	}
	else {
#warning "kde4: port QVariant::QSizePolicy"			
#if 0
			QSizePolicy v = value.toSizePolicy();
		m_property->child("hSizeType")->setValue(v.horData(), rememberOldValue, false);
		m_property->child("vSizeType")->setValue(v.verData(), rememberOldValue, false);
		m_property->child("hStretch")->setValue(v.horStretch(), rememberOldValue, false);
		m_property->child("vStretch")->setValue(v.verStretch(), rememberOldValue, false);
#endif		
	}
}

QVariant
SizePolicyCustomProperty::value() const
{
	if(!m_property || !m_property->parent())
		return QVariant();
#warning "kde4: port it"
#if 0
	if(m_property->type() == SizePolicy_HorData)
		return m_property->parent()->value().toSizePolicy().horData();
	else if(m_property->type() == SizePolicy_VerData)
		return m_property->parent()->value().toSizePolicy().verData();
	else if(m_property->type() == SizePolicy_HorStretch)
		return m_property->parent()->value().toSizePolicy().horStretch();
	else if(m_property->type() == SizePolicy_VerStretch)
		return m_property->parent()->value().toSizePolicy().verStretch();
#endif
	return QVariant();
}
