/*  This file is part of the KDE project

    Copyright (c) 2013 Sascha Suelzer <s.suelzer@gmail.com>

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

#include "KoResourceFiltering.h"

#include "KoResourceServer.h"

#include <QStringList>
#include <QString>


class Q_DECL_HIDDEN KoResourceFiltering::Private
{
public:
    Private()
    : isTag("\\[([\\w\\s]+)\\]")
    , isExactMatch("\"([\\w\\s]+)\"")
    , searchTokenizer("\\s*,+\\s*")
    , hasNewFilters(false)
    , name(true)
    , filename(true)
    , resourceServer(0)
    {}
    QRegExp isTag;
    QRegExp isExactMatch;
    QRegExp searchTokenizer;
    bool hasNewFilters;
    bool name,filename;
    KoResourceServerBase *resourceServer;
    QStringList tagSetFilenames;
    QStringList includedNames;
    QStringList excludedNames;
    QString currentTag;
};

KoResourceFiltering::KoResourceFiltering() : d(new Private())
{}

KoResourceFiltering::~KoResourceFiltering()
{
    delete d;
}

void KoResourceFiltering::configure(int filterType, bool enable) {
    switch (filterType) {
    case 0:
        d->name=true;
        d->filename=enable;
        break;
    case 1:
        d->name=enable;
        break;
    case 2:
        d->filename=enable;
        break;
    }
}

void KoResourceFiltering::setChanged()
{
    d->hasNewFilters = true;
}

void KoResourceFiltering::setTagSetFilenames(const QStringList& filenames)
{
    d->tagSetFilenames = filenames;
    d->excludedNames.clear();
    d->includedNames.clear();
    setChanged();
}

bool KoResourceFiltering::matchesResource(const QStringList &filteredList,const QStringList &filterList) const
{
    Qt::CaseSensitivity sensitivity = Qt::CaseInsensitive;
    foreach (QString filter, filterList) {
        if (!filter.startsWith('"')) {
            foreach (QString filtered, filteredList) {
                if (filtered.contains(filter,sensitivity)) {
                    return true;
                }
            }
        }
        else if (d->name) {
            filter.remove('"');
            if (!filteredList.at(0).compare(filter)) {
                return true;
            }
        }
    }
    return false;
}

void KoResourceFiltering::sanitizeExclusionList()
{
   if(!d->includedNames.isEmpty()) {

        foreach (const QString &exclusion, d->excludedNames) {
            if (!excludeFilterIsValid(exclusion))
                d->excludedNames.removeAll(exclusion);
        }
    }
}

QStringList KoResourceFiltering::tokenizeSearchString(const QString& searchString) const
{
    return searchString.split(d->searchTokenizer, QString::SkipEmptyParts);
}

void KoResourceFiltering::populateIncludeExcludeFilters(const QStringList& filteredNames)
{
    foreach (QString name, filteredNames) {
        QStringList* target;

        if(name.startsWith('!')) {
            name.remove('!');
            target = &d->excludedNames;
        } else {
            target = &d->includedNames;
        }

        if(!name.isEmpty()) {
            if (name.startsWith('[')) {
                if (d->isTag.exactMatch(name) && d->resourceServer) {
                    name = d->isTag.cap(1);
                    (*target) += d->resourceServer->queryResources(name);
                }
            }
            else if (name.startsWith('"')) {
                if (d->isExactMatch.exactMatch(name)) {
                    target->push_back(name);
                }
            }
            else {
                target->push_back(name);
            }
        }
    }
    sanitizeExclusionList();
}

bool KoResourceFiltering::hasFilters() const
{
    return (!d->tagSetFilenames.isEmpty() || !d->includedNames.isEmpty() || !d->excludedNames.isEmpty());
}

bool KoResourceFiltering::filtersHaveChanged() const
{
    return d->hasNewFilters;
}

void KoResourceFiltering::setFilters(const QString &searchString)
{
    d->excludedNames.clear();
    d->includedNames.clear();
    QStringList filteredNames = tokenizeSearchString(searchString);
    populateIncludeExcludeFilters(filteredNames);
    setChanged();
}

bool KoResourceFiltering::presetMatchesSearch(KoResource * resource) const
{
    QList<QString> filteredList;

    QString resourceFileName = resource->shortFilename();
    QString resourceName = resource->name();

    if (d->name) {
        filteredList.push_front(resourceName);
    }

    if (d->filename) {
        filteredList.push_back(resourceFileName);
    }

    if (matchesResource(filteredList,d->excludedNames)) {
        return false;
    }

    if (matchesResource(filteredList,d->includedNames)) {
        return true;
    }

    foreach (const QString &filter, d->tagSetFilenames) {
        if (!resourceFileName.compare(filter) || !resourceName.compare(filter)) {
            return true;
        }
    }

    return false;
}

void KoResourceFiltering::setInclusions(const QStringList &inclusions)
{
    d->includedNames = inclusions;
    setChanged();
}

void KoResourceFiltering::setExclusions(const QStringList &exclusions)
{
    d->excludedNames = exclusions;
    setChanged();
}

bool KoResourceFiltering::excludeFilterIsValid(const QString &exclusion)
{
    Q_FOREACH (const QString &inclusion, d->includedNames) {
        if ((inclusion.startsWith(exclusion)  && exclusion.size() <= inclusion.size())) {
            return false;
        }
    }
    return true;
}

QList< KoResource* > KoResourceFiltering::filterResources(QList< KoResource* > resources)
{

    Q_FOREACH (KoResource* resource, resources) {
        if(!presetMatchesSearch(resource)) {
            resources.removeAll(resource);
        }
    }
    setDoneFiltering();
    return resources;
}

void KoResourceFiltering::setDoneFiltering()
{
    d->hasNewFilters = false;
}

void KoResourceFiltering::rebuildCurrentTagFilenames()
{
    d->tagSetFilenames = d->resourceServer->queryResources(d->currentTag);
}

void KoResourceFiltering::setCurrentTag(const QString& tagSet)
{
    d->currentTag = tagSet;
    rebuildCurrentTagFilenames();
}

void KoResourceFiltering::setResourceServer(KoResourceServerBase* resourceServer)
{
    d->resourceServer = resourceServer;
}
