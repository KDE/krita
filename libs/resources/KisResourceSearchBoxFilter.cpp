/*
 *  SPDX-FileCopyrightText: 2019 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisResourceSearchBoxFilter.h"


#include <QRegExp>
#include <QRegularExpression>
#include <QList>
#include <QSet>


class Q_DECL_HIDDEN KisResourceSearchBoxFilter::Private
{
public:
    Private()
        : searchTokenizer("\\s*,+\\s*")
    {}

    QRegularExpression searchTokenizer;

    QChar tagBegin {'['};
    QChar tagEnd {']'};
    QChar exactMatchBeginEnd {'"'};

    QSet<QString> tagNamesIncluded;
    QSet<QString> tagNamesExcluded;

    QList<QString> resourceNamesPartsIncluded;
    QList<QString> resourceNamesPartsExcluded;
    QSet<QString> resourceExactMatchesIncluded;
    QSet<QString> resourceExactMatchesExcluded;

    QString filter;
};


KisResourceSearchBoxFilter::KisResourceSearchBoxFilter()
    : d(new Private())
{

}

KisResourceSearchBoxFilter::~KisResourceSearchBoxFilter()
{

}

QString cutOutDelimeters(QString text)
{
    QString response(text);
    response = response.remove(0, 1);
    response = response.left(response.length() - 1);
    return response;
}

void KisResourceSearchBoxFilter::setFilter(const QString& filter)
{
    d->filter = QString(filter);
    initializeFilterData();
}


bool KisResourceSearchBoxFilter::matchesResource(const QString &_resourceName)
{
    // exact matches
    QString resourceName = _resourceName.toLower();
    if (d->resourceExactMatchesIncluded.count() > 0
            && !d->resourceExactMatchesIncluded.contains(resourceName)) {
        return false;
    }
    if (d->resourceExactMatchesExcluded.contains(resourceName)) {
        return false;
    }
    // partial name matches
    if (d->resourceNamesPartsIncluded.count() > 0) {
        Q_FOREACH(const QString& partialName, d->resourceNamesPartsIncluded) {
            if (!resourceName.contains(partialName)) {
                return false;
            }
        }
    }
    Q_FOREACH(const QString& partialName, d->resourceNamesPartsExcluded) {
        if (resourceName.contains(partialName)) {
            return false;
        }
    }

    return true;
}

bool KisResourceSearchBoxFilter::isEmpty()
{
    return d->filter.isEmpty();
}

void KisResourceSearchBoxFilter::clearFilterData()
{

    d->tagNamesIncluded.clear();
    d->tagNamesExcluded.clear();
    d->resourceNamesPartsIncluded.clear();
    d->resourceNamesPartsExcluded.clear();
    d->resourceExactMatchesIncluded.clear();
    d->resourceExactMatchesExcluded.clear();

}

void KisResourceSearchBoxFilter::initializeFilterData()
{
    clearFilterData();

    QString tempFilter(d->filter);

    QStringList parts = tempFilter.split(d->searchTokenizer, QString::SkipEmptyParts);
    Q_FOREACH(const QString& partFor, parts) {
        QString part(partFor);
        part = part.toLower();

        bool included = true;
        if (part.startsWith('!')) {
            part.remove(0, 1);
            included = false;
        }

        if (part.startsWith(d->tagBegin) && part.endsWith(d->tagEnd)) {
            QString tagMatchCaptured = cutOutDelimeters(part);
            if (included) {
                d->tagNamesIncluded.insert(tagMatchCaptured);
            } else {
                d->tagNamesExcluded.insert(tagMatchCaptured);
            }
        } else if (part.startsWith(d->exactMatchBeginEnd) && part.endsWith(d->exactMatchBeginEnd)) {
            QString exactMatchCaptured = cutOutDelimeters(part);
            if (included) {
                d->resourceExactMatchesIncluded.insert(exactMatchCaptured);
            } else {
                d->resourceExactMatchesExcluded.insert(exactMatchCaptured);
            }
        } else {
            if (included) {
                d->resourceNamesPartsIncluded.append(part);
            } else {
                d->resourceNamesPartsExcluded.append(part);
            }
        }
    }

}
