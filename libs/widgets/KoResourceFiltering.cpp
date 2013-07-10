/*  This file is part of the KDE project

    Copyright (c) 2013 Sascha Suelzer <s_suelzer@lavabit.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "KoResourceFiltering.h"
#include "KoResourceTagging.h"

class KoResourceFiltering::Private
{
public:
    Private()
    : isTag("\\[([\\w\\s]+)\\]")
    , isExactMatch("\"([\\w\\s]+)\"")
    , searchTokenizer("\\s*,+\\s*")
    , hasNewFilters(false)
    , tagObject(0)
    {}
    QRegExp isTag;
    QRegExp isExactMatch;
    QRegExp searchTokenizer;
    bool hasNewFilters;
    KoResourceTagging *tagObject;
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

bool KoResourceFiltering::matchesResource(const QString &resourceName, const QString &resourceFileName,const QStringList &filterList) const
{
    Qt::CaseSensitivity sensitivity = Qt::CaseInsensitive;
    foreach (QString filter, filterList) {
        if (!filter.startsWith('"')) {
            if (resourceName.contains(filter,sensitivity) || resourceFileName.contains(filter,sensitivity))
                return true;
        }
        else {
            filter.remove('"');
            if (!resourceName.compare(filter)) {
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
                if (d->isTag.exactMatch(name) && d->tagObject) {
                    name = d->isTag.cap(1);
                    (*target) += d->tagObject->searchTag(name);
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

    QString resourceName = resource->name();
    QString resourceFileName = resource->filename();
    if (matchesResource(resourceName,resourceFileName,d->excludedNames))
        return false;
    if (matchesResource(resourceName,resourceFileName,d->includedNames))
        return true;
    foreach (const QString &filter, d->tagSetFilenames) {
        if (!resourceFileName.compare(filter) || !resourceName.compare(filter))
            return true;
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
    foreach(const QString &inclusion, d->includedNames) {
        if ((inclusion.startsWith(exclusion)  && exclusion.size() <= inclusion.size())) {
            return false;
        }
    }
    return true;
}

QList< KoResource* > KoResourceFiltering::filterResources(QList< KoResource* > resources)
{

    foreach(KoResource* resource, resources) {
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
    d->tagSetFilenames = d->tagObject->searchTag(d->currentTag);
}

void KoResourceFiltering::setCurrentTag(const QString& tagSet)
{
    d->currentTag = tagSet;
    rebuildCurrentTagFilenames();
}

void KoResourceFiltering::setTagObject(KoResourceTagging* tagObject)
{
    d->tagObject = tagObject;
}
