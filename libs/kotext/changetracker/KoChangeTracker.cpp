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
#include <QColor>

class KoChangeTracker::Private
{
public:
    Private()
      : changeId(1),
        recordChanges(false),
        displayChanges(false),
        insertionBgColor(0,255,0),
        deletionBgColor(255,0,0),
        formatChangeBgColor(0,0,255)
    {
    }
    ~Private() { }

    QMultiHash<int, int> children;
    QHash<int, int> parents;
    QHash<int, KoChangeTrackerElement *> changes;
    QHash<QString, int> loadedChanges;
    QList<int> saveChanges;
    QList<int> acceptedRejectedChanges;
    int changeId;
    bool recordChanges;
    bool displayChanges;
    QColor insertionBgColor, deletionBgColor, formatChangeBgColor;
};

KoChangeTracker::KoChangeTracker(QObject *parent)
    : QObject(parent),
    d(new Private())
{
    d->changeId = 1;
}

KoChangeTracker::~KoChangeTracker()
{
    delete d;
}

void KoChangeTracker::setRecordChanges(bool enabled)
{
    d->recordChanges = enabled;
}

bool KoChangeTracker::recordChanges()
{
    return d->recordChanges;
}

void KoChangeTracker::setDisplayChanges(bool enabled)
{
    d->displayChanges = enabled;
}

bool KoChangeTracker::displayChanges()
{
    return d->displayChanges;
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
        d->children.insert(existingChangeId, d->changeId);
        d->parents.insert(d->changeId, existingChangeId);
    }

    KoChangeTrackerElement *changeElement = new KoChangeTrackerElement(title, KoGenChange::formatChange);
    changeElement->setChangeFormat(format);
    changeElement->setPrevFormat(prevFormat);

    changeElement->setDate(KDateTime::currentLocalDateTime().toString(KDateTime::ISODate).replace(KGlobal::locale()->decimalSymbol(), QString(".")));

    KUser user(KUser::UseRealUserID);
    changeElement->setCreator(user.property(KUser::FullName).toString());

    changeElement->setEnabled(d->recordChanges);

    d->changes.insert(d->changeId, changeElement);

    return d->changeId++;
}

int KoChangeTracker::getInsertChangeId(QString title, int existingChangeId)
{
    if ( existingChangeId ) {
        d->children.insert(existingChangeId, d->changeId);
        d->parents.insert(d->changeId, existingChangeId);
    }

    KoChangeTrackerElement *changeElement = new KoChangeTrackerElement(title, KoGenChange::insertChange);

    changeElement->setDate(KDateTime::currentLocalDateTime().toString(KDateTime::ISODate).replace(KGlobal::locale()->decimalSymbol(), QString(".")));
//    changeElement->setDate(KDateTime::currentLocalDateTime().toString("Y-m-dTH:M:Sz")); //i must have misunderstood the API doc but it doesn't work.
    KUser user(KUser::UseRealUserID);
    changeElement->setCreator(user.property(KUser::FullName).toString());

    changeElement->setEnabled(d->recordChanges);

    d->changes.insert(d->changeId, changeElement);

    return d->changeId++;
}

int KoChangeTracker::getDeleteChangeId(QString title, QTextDocumentFragment selection, int existingChangeId)
{
    if ( existingChangeId ) {
        d->children.insert(existingChangeId, d->changeId);
        d->parents.insert(d->changeId, existingChangeId);
    }

    KoChangeTrackerElement *changeElement = new KoChangeTrackerElement(title, KoGenChange::deleteChange);

    changeElement->setDate(KDateTime::currentLocalDateTime().toString(KDateTime::ISODate).replace(KGlobal::locale()->decimalSymbol(), QString(".")));
    KUser user(KUser::UseRealUserID);
    changeElement->setCreator(user.property(KUser::FullName).toString());
    //TODO preserve formating info there. this will do for now
    changeElement->setDeleteData(selection.toPlainText());

    changeElement->setEnabled(d->recordChanges);

    d->changes.insert(d->changeId, changeElement);

    return d->changeId++;
}

KoChangeTrackerElement* KoChangeTracker::elementById(int id)
{
    return d->changes.value(id);
}

bool KoChangeTracker::removeById(int id, bool freeMemory)
{
    if (freeMemory) {
      KoChangeTrackerElement *temp = d->changes.value(id);
      delete temp;
    }
    return d->changes.remove(id);
}

bool KoChangeTracker::containsInlineChanges(const QTextFormat &format)
{
    if (format.property(KoCharacterStyle::ChangeTrackerId).toInt())
        return true;

    return false;
}

int KoChangeTracker::mergeableId(KoGenChange::Type type, QString &title, int existingId)
{
    if (!existingId || !d->changes.value(existingId))
        return 0;

    if (d->changes.value(existingId)->getChangeType() == type && d->changes.value(existingId)->getChangeTitle() == title)
        return existingId;
    else
        if (d->parents.contains(existingId))
            return mergeableId(type, title, d->parents.value(existingId));
        else
            return 0;
}

int KoChangeTracker::split(int changeId)
{
    KoChangeTrackerElement *element = new KoChangeTrackerElement(*d->changes.value(changeId));
    d->changes.insert(d->changeId, element);
    return d->changeId++;
}

bool KoChangeTracker::isParent(int testedParentId, int testedChildId)
{
    if ((testedParentId == testedChildId) && !d->acceptedRejectedChanges.contains(testedParentId))
        return true;
    else if (d->parents.contains(testedChildId))
        return isParent(testedParentId, d->parents.value(testedChildId));
    else
        return false;
}

void KoChangeTracker::setParent(int child, int parent)
{
    if (!d->children.values(parent).contains(child)) {
        d->children.insert(parent, child);
    }
    if (!d->parents.contains(child)) {
        d->parents.insert(child, parent);
    }
}

int KoChangeTracker::parent(int changeId)
{
    if (!d->parents.contains(changeId))
        return 0;
    if (d->acceptedRejectedChanges.contains(d->parents.value(changeId)))
        return parent(d->parents.value(changeId));
    return d->parents.value(changeId);
}

void KoChangeTracker::acceptRejectChange(int changeId, bool set)
{
    if (set) {
        if (!d->acceptedRejectedChanges.contains(changeId))
            d->acceptedRejectedChanges.append(changeId);
    }
    else {
        if (d->acceptedRejectedChanges.contains(changeId))
            d->acceptedRejectedChanges.removeAll(changeId);
    }

    d->changes.value(changeId)->setAcceptedRejected(set);
}

bool KoChangeTracker::saveInlineChange(int changeId, KoGenChange &change)
{
    if (!d->changes.contains(changeId))
        return false;

    change.setType(d->changes.value(changeId)->getChangeType());
    change.addChangeMetaData("dc-creator", d->changes.value(changeId)->getCreator());
    change.addChangeMetaData("dc-date", d->changes.value(changeId)->getDate());
    if (d->changes.value(changeId)->hasExtraMetaData())
        change.addChildElement("changeMetaData", d->changes.value(changeId)->getExtraMetaData());

    if (d->changes.value(changeId)->hasDeleteData())
        change.addChildElement("deletedData", d->changes.value(changeId)->getDeleteData());

    return true;
}

void KoChangeTracker::loadOdfChanges(const KoXmlElement& element)
{
    KoXmlElement tag;
    forEachElement(tag, element) {
        if (! tag.isNull()) {
            const QString localName = tag.localName();
            if (localName == "changed-region") {
                KoChangeTrackerElement *changeElement = 0;
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
                        changeElement->setEnabled(d->recordChanges);
                        d->changes.insert( d->changeId, changeElement);
                        d->loadedChanges.insert(tag.attributeNS(KoXmlNS::text,"id"), d->changeId++);
                    }
                }
            }
        }
    }
}

int KoChangeTracker::getLoadedChangeId(QString odfId)
{
    return d->loadedChanges.value(odfId);
}

int KoChangeTracker::getDeletedChanges(QVector<KoChangeTrackerElement *>& deleteVector)
{
    int numAppendedItems = 0;
    foreach(KoChangeTrackerElement *element, d->changes.values())
    {
        if(element->getChangeType() == KoGenChange::deleteChange && !element->acceptedRejected())
        {
          deleteVector << element;
          numAppendedItems++;
        }
    }

    return numAppendedItems;
}

const QColor& KoChangeTracker::getInsertionBgColor()
{
    return d->insertionBgColor;
}

const QColor& KoChangeTracker::getDeletionBgColor()
{
    return d->deletionBgColor;
}

const QColor& KoChangeTracker::getFormatChangeBgColor()
{
    return d->formatChangeBgColor;
}

void KoChangeTracker::setInsertionBgColor(const QColor& bgColor)
{
    d->insertionBgColor = bgColor;
}

void KoChangeTracker::setDeletionBgColor(const QColor& bgColor)
{
    d->deletionBgColor = bgColor;
}

void KoChangeTracker::setFormatChangeBgColor(const QColor& bgColor)
{
    d->formatChangeBgColor = bgColor;
}

#include <KoChangeTracker.moc>
