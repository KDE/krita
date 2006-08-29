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

#include "cursoredit.h"

#include <QMap>
#include <QVariant>
#include <QCursor>

#include <klocale.h>
#include <kdebug.h>

#include "property.h"

using namespace KoProperty;

//QMap<QString, QVariant> *CursorEdit::m_spValues = 0;
Property::ListData *m_cursorListData = 0;


CursorEdit::CursorEdit(Property *property, QWidget *parent)
: ComboBox(property, parent)
{
	/*
	if(!m_spValues) {
		m_spValues = new QMap<QString, QVariant>();
		(*m_spValues)[i18n("Arrow")] = Qt::ArrowCursor;
		(*m_spValues)[i18n("Up Arrow")] = Qt::UpArrowCursor;
		(*m_spValues)[i18n("Cross")] = Qt::CrossCursor;
		(*m_spValues)[i18n("Waiting")] = Qt::WaitCursor;
		(*m_spValues)[i18n("iBeam")] = Qt::IbeamCursor;
		(*m_spValues)[i18n("Size Vertical")] = Qt::SizeVerCursor;
		(*m_spValues)[i18n("Size Horizontal")] = Qt::SizeHorCursor;
		(*m_spValues)[i18n("Size Slash")] = Qt::SizeBDiagCursor;
		(*m_spValues)[i18n("Size Backslash")] = Qt::SizeFDiagCursor;
		(*m_spValues)[i18n("Size All")] = Qt::SizeAllCursor;
		(*m_spValues)[i18n("Blank")] = Qt::BlankCursor;
		(*m_spValues)[i18n("Split Vertical")] = Qt::SplitVCursor;
		(*m_spValues)[i18n("Split Horizontal")] = Qt::SplitHCursor;
		(*m_spValues)[i18n("Pointing Hand")] = Qt::PointingHandCursor;
		(*m_spValues)[i18n("Forbidden")] = Qt::ForbiddenCursor;
		(*m_spValues)[i18n("What's this")] = Qt::WhatsThisCursor;
	}*/

//! @todo NOT THREAD-SAFE
	if (!m_cursorListData) {
		QList<QVariant> keys;
		keys 
			<< Qt::BlankCursor
			<< Qt::ArrowCursor
			<< Qt::UpArrowCursor
			<< Qt::CrossCursor
			<< Qt::WaitCursor
			<< Qt::IBeamCursor
			<< Qt::SizeVerCursor
			<< Qt::SizeHorCursor
			<< Qt::SizeBDiagCursor
			<< Qt::SizeFDiagCursor
			<< Qt::SizeAllCursor
			<< Qt::SplitVCursor
			<< Qt::SplitHCursor
			<< Qt::PointingHandCursor
			<< Qt::ForbiddenCursor
			<< Qt::WhatsThisCursor;
		QStringList strings;
		strings << i18nc("Mouse Cursor Shape", "No Cursor")
			<< i18nc("Mouse Cursor Shape", "Arrow")
			<< i18nc("Mouse Cursor Shape", "Up Arrow")
			<< i18nc("Mouse Cursor Shape", "Cross")
			<< i18nc("Mouse Cursor Shape", "Waiting")
			<< i18nc("Mouse Cursor Shape", "I")
			<< i18nc("Mouse Cursor Shape", "Size Vertical")
			<< i18nc("Mouse Cursor Shape", "Size Horizontal")
			<< i18nc("Mouse Cursor Shape", "Size Slash")
			<< i18nc("Mouse Cursor Shape", "Size Backslash")
			<< i18nc("Mouse Cursor Shape", "Size All")
			<< i18nc("Mouse Cursor Shape", "Split Vertical")
			<< i18nc("Mouse Cursor Shape", "Split Horizontal")
			<< i18nc("Mouse Cursor Shape", "Pointing Hand")
			<< i18nc("Mouse Cursor Shape", "Forbidden")
			<< i18nc("Mouse Cursor Shape", "What's This?");
		m_cursorListData = new Property::ListData(keys, strings);
	}

	if(property)
		property->setListData(new Property::ListData(*m_cursorListData));
}

CursorEdit::~CursorEdit()
{
	delete m_cursorListData;
	m_cursorListData = 0;
}

QVariant
CursorEdit::value() const
{
	return QCursor((Qt::CursorShape)ComboBox::value().toInt());
}

void
CursorEdit::setValue(const QVariant &value, bool emitChange)
{
	ComboBox::setValue(value.value<QCursor>().shape(), emitChange);
}

void
CursorEdit::drawViewer(QPainter *p, const QColorGroup &cg, const QRect &r, const QVariant &value)
{
	ComboBox::drawViewer(p, cg, r, value.value<QCursor>().shape());
}

void
CursorEdit::setProperty(Property *prop)
{
	if(prop && prop != property())
		prop->setListData(new Property::ListData(*m_cursorListData));
	ComboBox::setProperty(prop);
}

#include "cursoredit.moc"
