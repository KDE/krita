/* This file is part of the KDE project
 * Copyright (C) 2008 Pierre Stirnweiss \pierre.stirnweiss_koffice@gadz.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoChangeTrackerElement.h"

#include <KDebug>

KoChangeTrackerElement::KoChangeTrackerElement(QString title, KoGenChange::Type type):
//  QObject(),
  m_title(title),
  m_type(type)
{
}

KoChangeTrackerElement::KoChangeTrackerElement()
{
}

KoChangeTrackerElement::~KoChangeTrackerElement()
{
}

void KoChangeTrackerElement::setChangeType(KoGenChange::Type type)
{
  m_type = type;
}

KoGenChange::Type KoChangeTrackerElement::getChangeType()
{
  return m_type;
}
    
void KoChangeTrackerElement::setChangeTitle(QString title)
{
  m_title = title;
}

QString KoChangeTrackerElement::getChangeTitle()
{
  return m_title;
}
    
void KoChangeTrackerElement::setChangeFormat(QTextFormat &format)
{
  m_changeFormat = format;
}

QTextFormat KoChangeTrackerElement::getChangeFormat()
{
  return m_changeFormat;
}

void KoChangeTrackerElement::setPrevFormat(QTextFormat &format)
{
  m_prevFormat = format;
}

QTextFormat KoChangeTrackerElement::getPrevFormat()
{
  return m_prevFormat;
}

bool KoChangeTrackerElement::hasCreator()
{
  return !m_creator.isEmpty();
}

void KoChangeTrackerElement::setCreator(QString creator)
{
  m_creator = creator;
}

QString KoChangeTrackerElement::getCreator()
{
  return m_creator;
}
    
bool KoChangeTrackerElement::hasDate()
{
  return !m_date.isEmpty();
}

void KoChangeTrackerElement::setDate(QString date)
{
  m_date = date;
}

QString KoChangeTrackerElement::getDate()
{
  return m_date;
}
    
bool KoChangeTrackerElement::hasExtraMetaData()
{
  return !m_extraMetaData.isEmpty();
}

void KoChangeTrackerElement::setExtraMetaData(QString metaData)
{
  m_extraMetaData = metaData;
}

QString KoChangeTrackerElement::getExtraMetaData()
{
  return m_extraMetaData;
}
    
bool KoChangeTrackerElement::hasDeleteData()
{
  return !m_deleteData.isEmpty();
}

void KoChangeTrackerElement::setDeleteData(QString data)
{
  m_deleteData = data;
}

QString KoChangeTrackerElement::getDeleteData()
{
  return m_deleteData;
}
