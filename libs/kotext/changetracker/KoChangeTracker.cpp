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

//KOffice includes
#include "styles/KoCharacterStyle.h"
#include "KoChangeTrackerElement.h"
#include <KoXmlReader.h>
#include <KoXmlNS.h>

//KDE includes
#include <KDebug>
#include <KDateTime>
#include <KGlobal>
#include <KLocale>
#include <KUser>

//Qt includes
#include <QList>
#include <QString>
#include <QHash>
#include <QMultiHash>
#include <QTextCursor>
#include <QTextFormat>
#include <QTextCharFormat>
#include <QTextDocumentFragment>

class KoChangeTracker::Private
{
public:
    Private()
      : m_changeId(1),
        m_enabled(false),
        m_displayDeleted(false)
    {
    }
    ~Private() { }

    // TODO remove the m_ prefix
    QMultiHash<int, int> children;
    QHash<int, int> m_parents;
    QHash<int, KoChangeTrackerElement *> m_changes;
    QHash<QString, int> m_loadedChanges;
    QList<int> m_saveChanges;
    int m_changeId;
    bool m_enabled;
    bool m_displayDeleted;
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

void KoChangeTracker::setEnabled(bool enabled)
{
    d->m_enabled = enabled;
}

bool KoChangeTracker::isEnabled()
{
    return d->m_enabled;
}

void KoChangeTracker::setDisplayDeleted(bool enabled)
{
    d->m_displayDeleted = enabled;
}

bool KoChangeTracker::displayDeleted()
{
    return d->m_displayDeleted;
}

int KoChangeTracker::getChangeId(QString &title, KoGenChange::Type type, QTextCursor &selection, QTextFormat& newFormat, int prevCharChangeId, int nextCharChangeId)
{
    Q_UNUSED(title)
    Q_UNUSED(type)
    Q_UNUSED(selection)
    Q_UNUSED(newFormat)
    Q_UNUSED(prevCharChangeId)
    Q_UNUSED(nextCharChangeId)
    return 0;
}

int KoChangeTracker::getFormatChangeId(QString title, QTextFormat &format, QTextFormat &prevFormat, int existingChangeId)
{
    if ( existingChangeId ) {
        d->children.insert(existingChangeId, d->m_changeId);
        d->m_parents.insert(d->m_changeId, existingChangeId);
    }

    KoChangeTrackerElement *changeElement = new KoChangeTrackerElement(title, KoGenChange::formatChange);
    changeElement->setChangeFormat(format);
    changeElement->setPrevFormat(prevFormat);

    changeElement->setDate(KDateTime::currentLocalDateTime().toString(KDateTime::ISODate).replace(KGlobal::locale()->decimalSymbol(), QString(".")));

    KUser user(KUser::UseRealUserID);
    changeElement->setCreator(user.property(KUser::FullName).toString());

    changeElement->setEnabled(d->m_enabled);

    d->m_changes.insert(d->m_changeId, changeElement);

    return d->m_changeId++;
}

int KoChangeTracker::getInsertChangeId(QString title, int existingChangeId)
{
    if ( existingChangeId ) {
        d->children.insert(existingChangeId, d->m_changeId);
        d->m_parents.insert(d->m_changeId, existingChangeId);
    }

    KoChangeTrackerElement *changeElement = new KoChangeTrackerElement(title, KoGenChange::insertChange);

    changeElement->setDate(KDateTime::currentLocalDateTime().toString(KDateTime::ISODate).replace(KGlobal::locale()->decimalSymbol(), QString(".")));
//    changeElement->setDate(KDateTime::currentLocalDateTime().toString("Y-m-dTH:M:Sz")); //i must have misunderstood the API doc but it doesn't work.
    KUser user(KUser::UseRealUserID);
    changeElement->setCreator(user.property(KUser::FullName).toString());

    changeElement->setEnabled(d->m_enabled);

    d->m_changes.insert(d->m_changeId, changeElement);

    return d->m_changeId++;
}

int KoChangeTracker::getDeleteChangeId(QString title, QTextDocumentFragment selection, int existingChangeId)
{
    if ( existingChangeId ) {
        d->children.insert(existingChangeId, d->m_changeId);
        d->m_parents.insert(d->m_changeId, existingChangeId);
    }

    KoChangeTrackerElement *changeElement = new KoChangeTrackerElement(title, KoGenChange::deleteChange);

    changeElement->setDate(KDateTime::currentLocalDateTime().toString(KDateTime::ISODate).replace(KGlobal::locale()->decimalSymbol(), QString(".")));
    KUser user(KUser::UseRealUserID);
    changeElement->setCreator(user.property(KUser::FullName).toString());
    //TODO preserve formating info there. this will do for now
    changeElement->setDeleteData(selection.toPlainText());

    changeElement->setEnabled(d->m_enabled);

    d->m_changes.insert(d->m_changeId, changeElement);

    return d->m_changeId++;
}

KoChangeTrackerElement* KoChangeTracker::elementById(int id)
{
    return d->m_changes.value(id);
}

bool KoChangeTracker::removeById(int id, bool freeMemory)
{
    if (freeMemory) {
      KoChangeTrackerElement *temp = d->m_changes.value(id);
      delete temp;
    }
    return d->m_changes.remove(id);
}

bool KoChangeTracker::containsInlineChanges(const QTextFormat &format)
{
    if (format.property(KoCharacterStyle::ChangeTrackerId).toInt())
        return true;

    return false;
}

int KoChangeTracker::mergeableId(KoGenChange::Type type, QString &title, int existingId)
{
    if (!existingId || !d->m_changes.value(existingId))
        return 0;

    if (d->m_changes.value(existingId)->getChangeType() == type && d->m_changes.value(existingId)->getChangeTitle() == title)
        return existingId;
    else
        if (d->m_parents.contains(existingId))
            return mergeableId(type, title, d->m_parents.value(existingId));
        else
            return 0;
}

int KoChangeTracker::split(int changeId)
{
    KoChangeTrackerElement *element = new KoChangeTrackerElement(*d->m_changes.value(changeId));
    d->m_changes.insert(d->m_changeId, element);
    return d->m_changeId++;
}

bool KoChangeTracker::isParent(int testedId, int baseId)
{
    if (testedId == baseId)
        return true;
    else if (d->m_parents.contains(baseId))
        return isParent(testedId, d->m_parents.value(baseId));
    else
        return false;
}

void KoChangeTracker::setParent(int child, int parent)
{
    if (!d->children.values(parent).contains(child)) {
        d->children.insert(parent, child);
    }
    if (!d->m_parents.contains(child)) {
        d->m_parents.insert(child, parent);
    }
}

bool KoChangeTracker::saveInlineChange(int changeId, KoGenChange &change)
{
    if (!d->m_changes.contains(changeId))
        return false;

    change.setType(d->m_changes.value(changeId)->getChangeType());
    change.addChangeMetaData("dc-creator", d->m_changes.value(changeId)->getCreator());
    change.addChangeMetaData("dc-date", d->m_changes.value(changeId)->getDate());
    if (d->m_changes.value(changeId)->hasExtraMetaData())
        change.addChildElement("changeMetaData", d->m_changes.value(changeId)->getExtraMetaData());

    if (d->m_changes.value(changeId)->hasDeleteData())
        change.addChildElement("deletedData", d->m_changes.value(changeId)->getDeleteData());

    return true;
}

void KoChangeTracker::loadOdfChanges(const KoXmlElement& element)
{
    KoXmlElement tag;
    forEachElement(tag, element) {
        if (! tag.isNull()) {
            const QString localName = tag.localName();
            if (localName == "changed-region") {
                KoChangeTrackerElement *changeElement;
                KoXmlElement region;
                forEachElement(region, tag) {
                    if (!region.isNull()) {
                        if (region.localName() == "insertion") {
                            changeElement = new KoChangeTrackerElement(tag.attributeNS(KoXmlNS::text,"id"),KoGenChange::insertChange);
                        } else if (region.localName() == "format-change") {
                            changeElement = new KoChangeTrackerElement(tag.attributeNS(KoXmlNS::text,"id"),KoGenChange::formatChange);
                        } else if (region.localName() == "deletion") {
                            changeElement = new KoChangeTrackerElement(tag.attributeNS(KoXmlNS::text,"id"),KoGenChange::deleteChange);
                            KoXmlElement deletedData = region.namedItemNS(KoXmlNS::text, "p").toElement();
                            if(!deletedData.isNull())
                              changeElement->setDeleteData(deletedData.text());
                        }
                        KoXmlElement metadata = region.namedItemNS(KoXmlNS::office,"change-info").toElement();
                        if (!metadata.isNull()) {
                            KoXmlElement date = metadata.namedItem("dc:date").toElement();
                            if (!date.isNull()) {
                                changeElement->setDate(date.text());
                            }
                            KoXmlElement creator = metadata.namedItem("dc:creator").toElement();
                            if (!date.isNull()) {
                                changeElement->setCreator(creator.text());
                            }
                            //TODO load comments
/*                            KoXmlElement extra = metadata.namedItem("dc-").toElement();
                            if (!date.isNull()) {
                                kDebug() << "creator: " << creator.text();
                                changeElement->setCreator(creator.text());
                            }*/
                        }
                        changeElement->setEnabled(d->m_enabled);
                        d->m_changes.insert( d->m_changeId, changeElement);
                        d->m_loadedChanges.insert(tag.attributeNS(KoXmlNS::text,"id"), d->m_changeId++);
                    }
                }
            }
        }
    }
}

int KoChangeTracker::getLoadedChangeId(QString odfId)
{
    return d->m_loadedChanges.value(odfId);
}

bool KoChangeTracker::completeLoading(KoStore *)
{
    return true;
}

bool KoChangeTracker::completeSaving(KoStore *, KoXmlWriter *, KoShapeSavingContext *)
{
    return true;
}

int KoChangeTracker::getDeletedChanges(QVector<KoChangeTrackerElement *>& deleteVector)
{
    int numAppendedItems = 0;
    foreach(KoChangeTrackerElement *element, d->m_changes.values())
    {
        if(element->getChangeType() == KoGenChange::deleteChange)
        {
          deleteVector << element;
          numAppendedItems++;
        }
    }

    return numAppendedItems;
}

#include "KoChangeTracker.moc"
