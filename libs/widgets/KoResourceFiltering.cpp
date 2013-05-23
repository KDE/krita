/*  This file is part of the KDE project

    Copyright (C) 2013 Sascha Suelzer <s.suelzer@lavabit.com>

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
    : m_isTag("\\[([\\w\\s]+)\\]")
    , m_isExactMatch("\"([\\w\\s]+)\"")
    , m_searchTokenizer("\\s*,+\\s*")
    , m_hasNewFilters(false)
    {}
    QRegExp m_isTag;
    QRegExp m_isExactMatch;
    QRegExp m_searchTokenizer;
    bool m_hasNewFilters;
    QStringList m_rootTagList;
    QStringList m_includedNames;
    QStringList m_excludedNames;

};

KoResourceFiltering::KoResourceFiltering() : d(new Private())
{}

KoResourceFiltering::~KoResourceFiltering()
{
    delete d;
}
void KoResourceFiltering::setChanged()
{
    d->m_hasNewFilters = true;
}

void KoResourceFiltering::setRootResourceFilenames(QStringList tag)
{
    d->m_rootTagList = tag;
    d->m_excludedNames.clear();
    d->m_includedNames.clear();
    setChanged();
}

bool KoResourceFiltering::matchesResource(QString &resourceName, QString &resourceFileName,const QStringList &filterList) const
{
    Qt::CaseSensitivity sensitivity = Qt::CaseInsensitive;
    foreach (QString filter, filterList) {
        if (!filter.startsWith("\"")) {
        if (resourceName.contains(filter,sensitivity) || resourceFileName.contains(filter,sensitivity))
            return true;
        }
        else {
            filter = filter.replace("\"","");
            if (!resourceName.compare(filter)) {
                return true;
            }
        }
    }
    return false;
}

void KoResourceFiltering::sanitizeExclusionList()
{
   if(!d->m_includedNames.isEmpty()) {

        foreach (QString exclusion, d->m_excludedNames) {
            if (!excludeFilterIsValid(exclusion))
                d->m_excludedNames.removeAll(exclusion);
        }
    }
}

QStringList KoResourceFiltering::tokenizeSearchString(const QString searchString)
{
    return searchString.split(d->m_searchTokenizer, QString::SkipEmptyParts);
}

void KoResourceFiltering::populateIncludeExcludeFilters(const QStringList& filteredNames, KoResourceTagging* tagObject)
{
    foreach (QString name, filteredNames) {
        QStringList* target;

        if(name.startsWith("!")) {
            name.remove("!");
            target = &d->m_excludedNames;
        } else {
            target = &d->m_includedNames;
        }

        if(!name.isEmpty()) {
            if (name.startsWith("[")) {
                if (d->m_isTag.exactMatch(name) && tagObject) {
                    name = d->m_isTag.cap(1);
                    (*target) += tagObject->searchTag(name);
                }
            }
            else if (name.startsWith("\"")) {
                if (d->m_isExactMatch.exactMatch(name)) {
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

bool KoResourceFiltering::hasFilters()
{
    return (!d->m_rootTagList.isEmpty() || !d->m_includedNames.isEmpty() || !d->m_excludedNames.isEmpty());
}

bool KoResourceFiltering::filtersHaveChanged()
{
    return d->m_hasNewFilters;
}

void KoResourceFiltering::setFilters(const QString &searchString, KoResourceTagging * tagObject)
{
    d->m_excludedNames.clear();
    d->m_includedNames.clear();
    QStringList filteredNames = tokenizeSearchString(searchString);
    populateIncludeExcludeFilters(filteredNames,tagObject);
    setChanged();
}

bool KoResourceFiltering::presetMatchesSearch(KoResource * resource) const
{

    QString resourceName = resource->name();
    QString resourceFileName = resource->filename();
    if (matchesResource(resourceName,resourceFileName,d->m_excludedNames))
        return false;
    if (matchesResource(resourceName,resourceFileName,d->m_includedNames))
        return true;
    foreach (QString filter, d->m_rootTagList) {
        if (!resourceFileName.compare(filter) || !resourceName.compare(filter))
            return true;
    }
    return false;
}
void KoResourceFiltering::setInclusions(QStringList inclusions)
{
    d->m_includedNames = inclusions;
    setChanged();
}
void KoResourceFiltering::setExclusions(QStringList exclusions)
{
    d->m_excludedNames = exclusions;
    setChanged();
}

bool KoResourceFiltering::excludeFilterIsValid(const QString &exclusion)
{
    foreach(QString inclusion, d->m_includedNames) {
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
    d->m_hasNewFilters = false;
}

