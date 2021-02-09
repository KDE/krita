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
#include <kis_debug.h>

class Q_DECL_HIDDEN KisResourceSearchBoxFilter::Private
{
public:
    Private()
        : searchTokenizer("\\s*,+\\s*")
    {}

    QRegularExpression searchTokenizer;

    QChar excludeBegin {'!'};
    QChar tagBegin {'#'};
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
    : m_d(new Private())
{
}

KisResourceSearchBoxFilter::~KisResourceSearchBoxFilter()
{

}

bool checkDelimetersAndCut(const QChar& begin, const QChar& end, QString& token) {
    if (token.startsWith(begin) && token.endsWith(end)) {
        token.remove(0, 1);
        token = token.left(token.length() - 1);
        return true;
    } else {
        return false;
    }
}

bool checkPrefixAndCut(QChar& prefix, QString& token) {
    if (token.startsWith(prefix)) {
        token.remove(0, 1);
        return true;
    } else {
        return false;
    }
}

void KisResourceSearchBoxFilter::setFilter(const QString& filter)
{
    m_d->filter = QString(filter);
    initializeFilterData();
}


bool KisResourceSearchBoxFilter::matchesResource(const QString &_resourceName, const QStringList &tagList)
{
    // exact matches
    QString resourceName = _resourceName.toLower();
    if (m_d->resourceExactMatchesIncluded.count() > 0
            && !m_d->resourceExactMatchesIncluded.contains(resourceName)) {
        return false;
    }
    if (m_d->resourceExactMatchesExcluded.contains(resourceName)) {
        return false;
    }

    // partial name matches
    if (m_d->resourceNamesPartsIncluded.count() > 0) {
        Q_FOREACH(const QString& partialName, m_d->resourceNamesPartsIncluded) {
            if (!resourceName.contains(partialName) && tagList.filter(partialName, Qt::CaseInsensitive).isEmpty()) {
                return false;
            }
        }
    }

    Q_FOREACH(const QString& partialName, m_d->resourceNamesPartsExcluded) {
        if (resourceName.contains(partialName) || tagList.filter(partialName, Qt::CaseInsensitive).size() > 0) {
            return false;
        }
    }

    // Tag exact matches
    if (m_d->tagNamesIncluded.count() > 0) {
        Q_FOREACH(const QString& tagName, m_d->tagNamesIncluded) {
            if (!tagList.contains(tagName, Qt::CaseInsensitive)) {
                return false;
            }
        }
    }

    if (m_d->tagNamesExcluded.count() > 0) {
        Q_FOREACH(const QString excludedTag, m_d->tagNamesExcluded) {
            if (tagList.contains(excludedTag, Qt::CaseInsensitive)) {
                return false;
            }
        }
    }

    return true;
}

bool KisResourceSearchBoxFilter::isEmpty()
{
    return m_d->filter.isEmpty();
}

void KisResourceSearchBoxFilter::clearFilterData()
{

    m_d->tagNamesIncluded.clear();
    m_d->tagNamesExcluded.clear();
    m_d->resourceNamesPartsIncluded.clear();
    m_d->resourceNamesPartsExcluded.clear();
    m_d->resourceExactMatchesIncluded.clear();
    m_d->resourceExactMatchesExcluded.clear();

}

void KisResourceSearchBoxFilter::initializeFilterData()
{
    clearFilterData();

    QString tempFilter(m_d->filter);

    QStringList tokens = tempFilter.split(m_d->searchTokenizer, QString::SkipEmptyParts);
    Q_FOREACH(const QString& token, tokens) {
        QString workingToken(token.toLower());
        const bool included = !checkPrefixAndCut(m_d->excludeBegin, workingToken);

        if (checkPrefixAndCut(m_d->tagBegin, workingToken)) {
            if (included) {
                m_d->tagNamesIncluded.insert(workingToken);
            } else {
                m_d->tagNamesExcluded.insert(workingToken);
            }

        } else if (checkDelimetersAndCut(m_d->exactMatchBeginEnd, m_d->exactMatchBeginEnd, workingToken)) {
            if (included) {
                m_d->resourceExactMatchesIncluded.insert(workingToken);
            } else {
                m_d->resourceExactMatchesExcluded.insert(workingToken);
            }

        } else {
            if (included) {
                m_d->resourceNamesPartsIncluded.append(workingToken);
            } else {
                m_d->resourceNamesPartsExcluded.append(workingToken);
            }
        }
    }
}
