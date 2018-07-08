/*  This file is part of the KDE project
   Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
   Copyright (c) 2016 L. E. Segovia <leo.segovia@siggraph.org>


    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include <resources/KoColorSet.h>

#include <sys/types.h>

#include <QFile>
#include <QVector>
#include <QTextStream>
#include <QTextCodec>
#include <QHash>
#include <QStringList>

#include <DebugPigment.h>
#include <klocalizedstring.h>

#include <QList>
#include "KoColor.h"
#include "KoColorSet.h"
#include "KisSwatch.h"
#include "KoColorSet_p.h"

KoColorSet::KoColorSet(const QString& filename)
    : KoResource(filename)
    , d(new Private(this))
{
}

/// Create an copied palette
KoColorSet::KoColorSet(const KoColorSet& rhs)
    : QObject(0)
    , KoResource(QString())
    , d(new Private(this))
{
    setFilename(rhs.filename());
    d->comment = rhs.d->comment;
    d->groupNames = rhs.d->groupNames;
    d->groups = rhs.d->groups;
    setValid(true);
}

KoColorSet::~KoColorSet()
{ }

bool KoColorSet::load()
{
    QFile file(filename());
    if (file.size() == 0) return false;
    if (!file.open(QIODevice::ReadOnly)) {
        warnPigment << "Can't open file " << filename();
        return false;
    }
    bool res = loadFromDevice(&file);
    file.close();
    return res;
}

bool KoColorSet::loadFromDevice(QIODevice *dev)
{
    if (!dev->isOpen()) dev->open(QIODevice::ReadOnly);

    d->data = dev->readAll();

    Q_ASSERT(d->data.size() != 0);

    return d->init();
}


bool KoColorSet::save()
{
    QFile file(filename());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    saveToDevice(&file);
    file.close();
    return true;
}

bool KoColorSet::saveToDevice(QIODevice *dev) const
{
    bool res;
    switch(d->paletteType) {
    case GPL:
        res = d->saveGpl(dev);
        break;
    default:
        res = d->saveKpl(dev);
    }
    if (res) {
        KoResource::saveToDevice(dev);
    }
    return res;
}

quint32 KoColorSet::nColors()
{
    return d->groups[Private::GLOBAL_GROUP_NAME].nColors();
}

quint32 KoColorSet::nColorsGroup(QString groupName)
{
    if (d->groups.contains(groupName)) {
        return d->groups[groupName].nColors();
    } else if (groupName.isEmpty()) {
        return d->groups[Private::GLOBAL_GROUP_NAME].nColors();
    } else {
        return 0;
    }
}

QString KoColorSet::closestColorName(const KoColor color, bool useGivenColorSpace)
{
    quint8 highestPercentage = 0;
    quint8 testPercentage = 0;
    KoColor compare = color;
    QString res;
    for (int x = 0; x < d->groups[Private::GLOBAL_GROUP_NAME].nColumns(); x++) {
        for (int y = 0; y < rowCount(); y++ ) {
            KisSwatch entry = getColorGlobal(x, y);
            KoColor color = entry.color();
            if (useGivenColorSpace == true && compare.colorSpace() != color.colorSpace()) {
                color.convertTo(compare.colorSpace());

            } else if (compare.colorSpace() != color.colorSpace()) {
                compare.convertTo(color.colorSpace());
            }
            testPercentage = (255 - compare.colorSpace()->difference(compare.data(), color.data()));
            if (testPercentage > highestPercentage)
            {
                res = entry.name();
                highestPercentage = testPercentage;
            }
        }
    }
    return res;
}

void KoColorSet::add(KisSwatch c, const QString &groupName)
{
    KisSwatchGroup &modifiedGroup = d->groups.contains(groupName)
            ? d->groups[groupName] : d->global();
    modifiedGroup.addEntry(c);
}

void KoColorSet::clear()
{
    d->groups.clear();
    d->groupNames.clear();
}

KisSwatch KoColorSet::getColorGlobal(quint32 x, quint32 y)
{
    KisSwatch e;
    int yInGroup = y;
    QString nameGroupFoundIn;
    for (const QString &groupName : d->groupNames) {
        if (yInGroup < d->groups[groupName].nRows()) {
            nameGroupFoundIn = groupName;
            break;
        } else {
            yInGroup -= d->groups[groupName].nRows();
        }
    }
    const KisSwatchGroup &groupFoundIn = nameGroupFoundIn == QString()
            ? d->global() : d->groups[nameGroupFoundIn];
    Q_ASSERT(groupFoundIn.checkEntry(x, yInGroup));
    return groupFoundIn.getEntry(x, yInGroup);
}

KisSwatch KoColorSet::getColorGroup(quint32 x, quint32 y, QString groupName)
{
    KisSwatch e;
    const KisSwatchGroup &sourceGroup = groupName == QString()
            ? d->global() : d->groups[groupName];
    if (sourceGroup.checkEntry(x, y)) {
        e = sourceGroup.getEntry(x, y);
    }
    return e;
}

QStringList KoColorSet::getGroupNames()
{
    if (d->groupNames.size() != d->groups.size()) {
        warnPigment << "mismatch between groups and the groupnames list.";
        warnPigment << "<" << d->groupNames.size() << " " << d->groups.size() << ">";
        return QStringList(d->groups.keys());
    }
    return d->groupNames;
}

bool KoColorSet::changeGroupName(QString oldGroupName, QString newGroupName)
{
    if (d->groupNames.contains(oldGroupName)==false) {
        return false;
    }
    KisSwatchGroup tmp = d->groups.value(oldGroupName);
    d->groups.remove(oldGroupName);
    d->groups[newGroupName] = tmp;
    //rename the string in the stringlist;
    int index = d->groupNames.indexOf(oldGroupName);
    d->groupNames.replace(index, newGroupName);
    return true;
}

void KoColorSet::setColumnCount(int columns)
{
    d->groups[Private::GLOBAL_GROUP_NAME].setNColumns(columns);
    for (KisSwatchGroup &g : d->groups.values()) { // Q_FOREACH doesn't accept non const refs
        g.setNColumns(columns);
    }
}

int KoColorSet::columnCount()
{
    return d->groups[Private::GLOBAL_GROUP_NAME].nColumns();
}

QString KoColorSet::comment()
{
    return d->comment;
}

void KoColorSet::setComment(QString comment)
{
    d->comment = comment;
}

bool KoColorSet::addGroup(const QString &groupName)
{
    if (d->groups.contains(groupName) || d->groupNames.contains(groupName)) {
        return false;
    }
    d->groupNames.append(groupName);
    d->groups[groupName] = KisSwatchGroup();
    return true;
}

bool KoColorSet::moveGroup(const QString &groupName, const QString &groupNameInsertBefore)
{
    if (d->groupNames.contains(groupName)==false || d->groupNames.contains(groupNameInsertBefore)==false) {
        return false;
    }
    d->groupNames.removeAt(d->groupNames.indexOf(groupName));
    int index = d->groupNames.size();
    if (groupNameInsertBefore!=QString()) {
        index = d->groupNames.indexOf(groupNameInsertBefore);
    }
    d->groupNames.insert(index, groupName);
    return true;
}

bool KoColorSet::removeGroup(const QString &groupName, bool keepColors)
{
    if (!d->groups.contains(groupName)) {
        return false;
    }

    if (keepColors) {
        // put all colors directly below global
        int startingRow = d->groups[Private::GLOBAL_GROUP_NAME].nRows();
        for (int x = 0; x < d->groups[groupName].nColumns(); x++) {
            for (int y = 0; y < d->groups[groupName].nRows(); y++) {
                if (d->groups[groupName].checkEntry(x, y)) {
                    d->groups[Private::GLOBAL_GROUP_NAME].setEntry(d->groups[groupName].getEntry(x, y),
                                       x,
                                       y + startingRow);
                }
            }
        }
    }

    for (int n = 0; n<d->groupNames.size(); n++) {
        if (d->groupNames.at(n) == groupName) {
            d->groupNames.removeAt(n);
        }
    }

    d->groups.remove(groupName);
    return true;
}

QString KoColorSet::defaultFileExtension() const
{
    return QString(".kpl");
}


int KoColorSet::rowCount()
{
    int res = 0;
    for (const QString &name : d->groupNames) {
        res += d->groups[name].nRows();
    }
    return res;
}

KisSwatchGroup *KoColorSet::getGroup(const QString &name)
{
    if (name.isEmpty()) {
        return &(d->global());
    }
    if (!d->groups.contains(name)) {
        return Q_NULLPTR;
    }
    return &(d->groups[name]);
}

/*
QString KoColorSet::findGroupByGlobalIndex(quint32 x, quint32 y)
{
    *index = globalIndex;
    QString groupName = QString();
    if ((quint32)d->colors.size()<=*index) {
        *index -= (quint32)d->colors.size();
        if (!d->groups.empty() || !d->groupNames.empty()) {
            QStringList groupNames = getGroupNames();
            Q_FOREACH (QString name, groupNames) {
                quint32 size = (quint32)d->groups.value(name).size();
                if (size<=*index) {
                    *index -= size;
                } else {
                    groupName = name;
                    return groupName;
                }
            }

        }
    }
    return groupName;
}

QString KoColorSet::findGroupByColorName(const QString &name, quint32 *x, quint32 *y)
{
    QString groupName = QString();
    Q_FOREACH (const KisSwatchGroup::Column & col, d->global.colors()) {
        Q_FOREACH (const KisSwatch & entry, col.values()) {
            if (entry.name() == name) {
                *x = static_cast<quint32>(entry.x());
                *y = static_cast<quint32>(entry.y());
                return groupName;
            }
        }
    }
    QStringList groupNames = getGroupNames();
    Q_FOREACH (QString name, groupNames) {
        Q_FOREACH (const KisSwatchGroup::Column & col, d->groups[name].colors()) {
            Q_FOREACH (const KisSwatch & entry, col.values()) {
                if (entry.name() == name) {
                    *x = static_cast<quint32>(entry.x());
                    *y = static_cast<quint32>(entry.y());
                    return groupName;
                }
            }
        }
    }
    return groupName;
}

QString KoColorSet::findGroupByID(const QString &id, quint32 *index) {
    *index = 0;
    QString groupName = QString();
    for (int i = 0; i<d->colors.size(); i++) {
        if(d->colors.at(i).id() == id) {
            *index = (quint32)i;
            return groupName;
        }
    }
    QStringList groupNames = getGroupNames();
    Q_FOREACH (QString name, groupNames) {
        for (int i=0; i<d->groups[name].size(); i++) {
            if(d->groups[name].at(i).id() == id) {
                *index = (quint32)i;
                groupName = name;
                return groupName;
            }
        }
    }
    return groupName;
    return QString();
}

const KisSwatchGroup &KoColorSet::getGroupByName(const QString &groupName, bool &success) const
{
    if (groupName.isEmpty()) {
        success = true;
        return d->global;
    }
    if (d->groupNames.contains(groupName)) {
        success = true;
        return d->groups[groupName];
    }
    success = false;
    return d->global;
}

bool KoColorSet::changeColorSetEntry(KisSwatch entry,
                                     QString groupName)
{
    if (!d->groupNames.contains(groupName)) {
        return false;
    }
    if (groupName.isEmpty()) {
        d->global.setEntry(entry);
    } else {
        d->groups[groupName].setEntry(entry);
    }
    return true;
}

quint32 KoColorSet::getIndexClosestColor(const KoColor color, bool useGivenColorSpace)
{
    quint32 closestIndex = 0;
    quint8 highestPercentage = 0;
    quint8 testPercentage = 0;
    KoColor compare = color;
    for (quint32 i=0; i < nColors(); i++) {
        KoColor entry = getColorGlobal(i).color();
        if (useGivenColorSpace == true && compare.colorSpace() != entry.colorSpace()) {
            entry.convertTo(compare.colorSpace());

        } else if(compare.colorSpace()!=entry.colorSpace()) {
            compare.convertTo(entry.colorSpace());
        }
        testPercentage = (255 - compare.colorSpace()->difference(compare.data(), entry.data()));
        if (testPercentage>highestPercentage)
        {
            closestIndex = i;
            highestPercentage = testPercentage;
        }
    }
    return closestIndex;
}

void KoColorSet::removeAt(quint32 x, quint32 y, QString groupName)
{
    if (d->groups.contains(groupName)){
        if ((quint32)d->groups.value(groupName).size()>x) {
            d->groups[groupName].remove(x);
        }
    } else {
        if ((quint32)d->colors.size()>x) {
            d->colors.remove(x);
        }
    }
    if (x >= d->columns || x < 0)
        return;
    if (d->groups.contains(groupName)){
            d->groups[groupName].removeEntry(x, y);
    } else {
            d->global.removeEntry(x, y);
    }
}

quint32 KoColorSet::insertBefore(const KisSwatch &c, qint32 index, const QString &groupName)
{
    quint32 newIndex = index;
    if (d->groups.contains(groupName)) {
        d->groups[groupName].insert(index, c);
    } else if (groupName.isEmpty()){
        d->colors.insert(index, c);
    } else {
        warnPigment << "Couldn't find group to insert to";
    }
    return newIndex;
    return 0;
}
*/
