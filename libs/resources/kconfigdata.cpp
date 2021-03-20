/*
   This file is part of the KDE libraries
   SPDX-FileCopyrightText: 2006, 2007 Thomas Braxton <kde.braxton@gmail.com>
   SPDX-FileCopyrightText: 1999-2000 Preston Brown <pbrown@kde.org>
   SPDX-FileCopyrightText: 1996-2000 Matthias Kalle Dalheimer <kalle@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <kconfigdata.h>

QDebug operator<<(QDebug dbg, const KEntryKey &key)
{
    dbg.nospace() << "[" << key.mGroup << ", " << key.mKey << (key.bLocal ? " localized" : "") <<
                  (key.bDefault ? " default" : "") << (key.bRaw ? " raw" : "") << "]";
    return dbg.space();
}

QDebug operator<<(QDebug dbg, const KEntry &entry)
{
    dbg.nospace() << "[" << entry.mValue << (entry.bDirty ? " dirty" : "") <<
                  (entry.bGlobal ? " global" : "") << (entry.bImmutable ? " immutable" : "") <<
                  (entry.bDeleted ? " deleted" : "") << (entry.bReverted ? " reverted" : "") <<
                  (entry.bExpand ? " expand" : "") << "]";

    return dbg.space();
}

QMap< KEntryKey, KEntry >::Iterator KEntryMap::findExactEntry(const QByteArray &group, const QByteArray &key, KEntryMap::SearchFlags flags)
{
    KEntryKey theKey(group, key, bool(flags & SearchLocalized), bool(flags & SearchDefaults));
    return find(theKey);
}

QMap< KEntryKey, KEntry >::Iterator KEntryMap::findEntry(const QByteArray &group, const QByteArray &key, KEntryMap::SearchFlags flags)
{
    KEntryKey theKey(group, key, false, bool(flags & SearchDefaults));

    // try the localized key first
    if (flags & SearchLocalized) {
        theKey.bLocal = true;

        Iterator it = find(theKey);
        if (it != end()) {
            return it;
        }

        theKey.bLocal = false;
    }
    return find(theKey);
}

QMap< KEntryKey, KEntry >::ConstIterator KEntryMap::findEntry(const QByteArray &group, const QByteArray &key, KEntryMap::SearchFlags flags) const
{
    KEntryKey theKey(group, key, false, bool(flags & SearchDefaults));

    // try the localized key first
    if (flags & SearchLocalized) {
        theKey.bLocal = true;

        ConstIterator it = find(theKey);
        if (it != constEnd()) {
            return it;
        }

        theKey.bLocal = false;
    }
    return find(theKey);
}

bool KEntryMap::setEntry(const QByteArray &group, const QByteArray &key, const QByteArray &value, KEntryMap::EntryOptions options)
{
    KEntryKey k;
    KEntry e;
    bool newKey = false;

    const Iterator it = findExactEntry(group, key, SearchFlags(options >> 16));

    if (key.isEmpty()) { // inserting a group marker
        k.mGroup = group;
        e.bImmutable = (options & EntryImmutable);
        if (options & EntryDeleted) {
            qWarning("Internal KConfig error: cannot mark groups as deleted");
        }
        if (it == end()) {
            insert(k, e);
            return true;
        } else if (it.value() == e) {
            return false;
        }

        it.value() = e;
        return true;
    }

    if (it != end()) {
        if (it->bImmutable) {
            return false;    // we cannot change this entry. Inherits group immutability.
        }
        k = it.key();
        e = *it;
        //qDebug() << "found existing entry for key" << k;
    } else {
        // make sure the group marker is in the map
        KEntryMap const *that = this;
        ConstIterator cit = that->findEntry(group);
        if (cit == constEnd()) {
            insert(KEntryKey(group), KEntry());
        } else if (cit->bImmutable) {
            return false;    // this group is immutable, so we cannot change this entry.
        }

        k = KEntryKey(group, key);
        newKey = true;
    }

    // set these here, since we may be changing the type of key from the one we found
    k.bLocal = (options & EntryLocalized);
    k.bDefault = (options & EntryDefault);
    k.bRaw = (options & EntryRawKey);

    e.mValue = value;
    e.bDirty = e.bDirty || (options & EntryDirty);
    e.bGlobal = (options & EntryGlobal); //we can't use || here, because changes to entries in
    //kdeglobals would be written to kdeglobals instead
    //of the local config file, regardless of the globals flag
    e.bImmutable = e.bImmutable || (options & EntryImmutable);
    if (value.isNull()) {
        e.bDeleted = e.bDeleted || (options & EntryDeleted);
    } else {
        e.bDeleted = false;    // setting a value to a previously deleted entry
    }
    e.bExpand = (options & EntryExpansion);
    e.bReverted = false;
    if (options & EntryLocalized) {
        e.bLocalizedCountry = (options & EntryLocalizedCountry);
    } else {
        e.bLocalizedCountry = false;
    }

    if (newKey) {
        //qDebug() << "inserting" << k << "=" << value;
        insert(k, e);
        if (k.bDefault) {
            k.bDefault = false;
            //qDebug() << "also inserting" << k << "=" << value;
            insert(k, e);
        }
        // TODO check for presence of unlocalized key
        return true;
    } else {
//                KEntry e2 = it.value();
        if (options & EntryLocalized) {
            // fast exit checks for cases where the existing entry is more specific
            const KEntry &e2 = it.value();
            if (e2.bLocalizedCountry && !e.bLocalizedCountry) {
                // lang_COUNTRY > lang
                return false;
            }
        }
        if (it.value() != e) {
            //qDebug() << "changing" << k << "from" << e.mValue << "to" << value;
            it.value() = e;
            if (k.bDefault) {
                KEntryKey nonDefaultKey(k);
                nonDefaultKey.bDefault = false;
                insert(nonDefaultKey, e);
            }
            if (!(options & EntryLocalized)) {
                KEntryKey theKey(group, key, true, false);
                //qDebug() << "non-localized entry, remove localized one:" << theKey;
                remove(theKey);
                if (k.bDefault) {
                    theKey.bDefault = true;
                    remove(theKey);
                }
            }
            return true;
        } else {
            //qDebug() << k << "was already set to" << e.mValue;
            if (!(options & EntryLocalized)) {
                //qDebug() << "unchanged non-localized entry, remove localized one.";
                KEntryKey theKey(group, key, true, false);
                bool ret = false;
                Iterator cit = find(theKey);
                if (cit != end()) {
                    erase(cit);
                    ret = true;
                }
                if (k.bDefault) {
                    theKey.bDefault = true;
                    Iterator cit = find(theKey);
                    if (cit != end()) {
                        erase(cit);
                        return true;
                    }
                }
                return ret;
            }
            //qDebug() << "localized entry, unchanged, return false";
            // When we are writing a default, we know that the non-
            // default is the same as the default, so we can simply
            // use the same branch.
            return false;
        }
    }
}

QString KEntryMap::getEntry(const QByteArray &group, const QByteArray &key, const QString &defaultValue, KEntryMap::SearchFlags flags, bool *expand) const
{
    const ConstIterator it = findEntry(group, key, flags);
    QString theValue = defaultValue;

    if (it != constEnd() && !it->bDeleted) {
        if (!it->mValue.isNull()) {
            const QByteArray data = it->mValue;
            theValue = QString::fromUtf8(data.constData(), data.length());
            if (expand) {
                *expand = it->bExpand;
            }
        }
    }

    return theValue;
}

bool KEntryMap::hasEntry(const QByteArray &group, const QByteArray &key, KEntryMap::SearchFlags flags) const
{
    const ConstIterator it = findEntry(group, key, flags);
    if (it == constEnd()) {
        return false;
    }
    if (it->bDeleted) {
        return false;
    }
    if (key.isNull()) { // looking for group marker
        return it->mValue.isNull();
    }
    // if it->bReverted, we'll just return true; the real answer depends on lookup up with SearchDefaults, though.
    return true;
}

bool KEntryMap::getEntryOption(const QMap< KEntryKey, KEntry >::ConstIterator &it, KEntryMap::EntryOption option) const
{
    if (it != constEnd()) {
        switch (option) {
        case EntryDirty:
            return it->bDirty;
        case EntryLocalized:
            return it.key().bLocal;
        case EntryGlobal:
            return it->bGlobal;
        case EntryImmutable:
            return it->bImmutable;
        case EntryDeleted:
            return it->bDeleted;
        case EntryExpansion:
            return it->bExpand;
        default:
            break; // fall through
        }
    }

    return false;
}

void KEntryMap::setEntryOption(QMap< KEntryKey, KEntry >::Iterator it, KEntryMap::EntryOption option, bool bf)
{
    if (it != end()) {
        switch (option) {
        case EntryDirty:
            it->bDirty = bf;
            break;
        case EntryGlobal:
            it->bGlobal = bf;
            break;
        case EntryImmutable:
            it->bImmutable = bf;
            break;
        case EntryDeleted:
            it->bDeleted = bf;
            break;
        case EntryExpansion:
            it->bExpand = bf;
            break;
        default:
            break; // fall through
        }
    }
}

bool KEntryMap::revertEntry(const QByteArray &group, const QByteArray &key, KEntryMap::SearchFlags flags)
{
    Q_ASSERT((flags & KEntryMap::SearchDefaults) == 0);
    Iterator entry = findEntry(group, key, flags);
    if (entry != end()) {
        //qDebug() << "reverting" << entry.key() << " = " << entry->mValue;
        if (entry->bReverted) { // already done before
            return false;
        }

        KEntryKey defaultKey(entry.key());
        defaultKey.bDefault = true;
        //qDebug() << "looking up default entry with key=" << defaultKey;
        const ConstIterator defaultEntry = constFind(defaultKey);
        if (defaultEntry != constEnd()) {
            Q_ASSERT(defaultEntry.key().bDefault);
            //qDebug() << "found, update entry";
            *entry = *defaultEntry; // copy default value, for subsequent lookups
        } else {
            entry->mValue = QByteArray();
        }
        entry->bDirty = true;
        entry->bReverted = true; // skip it when writing out to disk

        //qDebug() << "Here's what we have now:" << *this;
        return true;
    }
    return false;
}
