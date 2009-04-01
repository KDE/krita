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

#include "KoChangeTracker.h"

#include "styles/KoCharacterStyle.h"

#include <KDebug>
#include <KDateTime>

#include <QList>

class KoChangeTracker::Private
{
public:
  Private() { }
  ~Private() { }
  
  QHash<int, int> m_childs;
  QHash<int, int> m_parents;
  QHash<int, KoChangeTrackerElement *> m_changes;
  QList<int> m_saveChanges;
  int m_changeId;
};

KoChangeTracker::KoChangeTracker(QObject *parent)
        : QObject(parent),
	d(new Private())
{
  d->m_changeId = 1;
}

KoChangeTracker::~KoChangeTracker()
{
  delete d;
}

int KoChangeTracker::getFormatChangeId(QString title, QTextFormat &format, QTextFormat &prevFormat, int existingChangeId)
{
  if ( existingChangeId ) {
    d->m_childs.insert(existingChangeId, d->m_changeId);
    d->m_parents.insert(d->m_changeId, existingChangeId);
  }
  
  KoChangeTrackerElement *changeElement = new KoChangeTrackerElement(title, KoGenChange::formatChange);
  changeElement->setChangeFormat(format);
  changeElement->setPrevFormat(format);
  
  changeElement->setDate(KDateTime::currentLocalDateTime().toString(KDateTime::ISODate));
  changeElement->setCreator(QString("essai format"));

  d->m_changes.insert(d->m_changeId, changeElement);
  changeElement=d->m_changes.value(d->m_changeId);
    
  return d->m_changeId++;
}

int KoChangeTracker::getInsertChangeId(QString title, int existingChangeId)
{
  if ( existingChangeId ) {
    d->m_childs.insert(existingChangeId, d->m_changeId);
    d->m_parents.insert(d->m_changeId, existingChangeId);
  }
  
  KoChangeTrackerElement *changeElement = new KoChangeTrackerElement(title, KoGenChange::insertChange);

  changeElement->setDate(KDateTime::currentLocalDateTime().toString(KDateTime::ISODate));
  changeElement->setCreator(QString("essai insert"));

  d->m_changes.insert(d->m_changeId, changeElement);
  
  return d->m_changeId++;
}

bool KoChangeTracker::containsInlineChanges(const QTextFormat &format)
{
  if (format.property(KoCharacterStyle::ChangeTrackerId).toInt())
    return true;
  else
 
 return false;
}

bool KoChangeTracker::saveInlineChange(int changeId, KoGenChange &change)
{
  if (!d->m_changes.contains(changeId))
    return false;
  
  change.setType(d->m_changes.value(changeId)->getChangeType());
  if (d->m_changes.value(changeId)->hasCreator())
    change.addChangeMetaData("dc-creator", d->m_changes.value(changeId)->getCreator());
  if (d->m_changes.value(changeId)->hasDate())
    change.addChangeMetaData("dc-date", d->m_changes.value(changeId)->getDate());
  if (d->m_changes.value(changeId)->hasExtraMetaData())
    change.addChildElement("changeMetaData", d->m_changes.value(changeId)->getExtraMetaData());
  
  if (d->m_changes.value(changeId)->hasDeleteData())
    change.addChildElement("deleteData", d->m_changes.value(changeId)->getDeleteData());
    
  return true;
}



#include "KoChangeTracker.moc"
