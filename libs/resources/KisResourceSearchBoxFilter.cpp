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

    QSet<QString> tagExactMatchesIncluded;
    QSet<QString> tagExactMatchesExcluded;
    QSet<QString> resourceExactMatchesIncluded;
    QSet<QString> resourceExactMatchesExcluded;

    QList<QString> resourceNamesPartialIncluded;
    QList<QString> resourceNamesPartialExcluded;
    QList<QString> tagsPartialIncluded;
    QList<QString> tagsPartialExcluded;

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

bool checkDelimetersAndCut(const QChar& beginEnd, QString& token) {
    return checkDelimetersAndCut(beginEnd, beginEnd, token);
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
    if (m_d->resourceNamesPartialIncluded.count() > 0) {
        Q_FOREACH(const QString& partialName, m_d->resourceNamesPartialIncluded) {
            if (!resourceName.contains(partialName) && tagList.filter(partialName, Qt::CaseInsensitive).isEmpty()) {
                return false;
            }
        }
    }

    Q_FOREACH(const QString& partialName, m_d->resourceNamesPartialExcluded) {
        if (resourceName.contains(partialName) || tagList.filter(partialName, Qt::CaseInsensitive).size() > 0) {
            return false;
        }
    }

    // Tag partial matches
    if (m_d->tagsPartialIncluded.count() > 0 ) {
        Q_FOREACH(const QString& partialTag, m_d->tagsPartialIncluded) {
            if (tagList.filter(partialTag, Qt::CaseInsensitive).isEmpty()) {
                return false;
            }
        }
    }

    if (m_d->tagsPartialExcluded.count() > 0) {
        Q_FOREACH(const QString& partialTag, m_d->tagsPartialExcluded) {
            if (tagList.filter(partialTag, Qt::CaseInsensitive).size() > 0) {
                return false;
            }
        }
    }

    // Tag exact matches
    if (m_d->tagExactMatchesIncluded.count() > 0) {
        Q_FOREACH(const QString& tagName, m_d->tagExactMatchesIncluded) {
            if (!tagList.contains(tagName, Qt::CaseInsensitive)) {
                return false;
            }
        }
    }

    if (m_d->tagExactMatchesExcluded.count() > 0) {
        Q_FOREACH(const QString excludedTag, m_d->tagExactMatchesExcluded) {
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
    m_d->resourceExactMatchesIncluded.clear();
    m_d->resourceExactMatchesExcluded.clear();
    m_d->tagExactMatchesIncluded.clear();
    m_d->tagExactMatchesExcluded.clear();

    m_d->resourceNamesPartialIncluded.clear();
    m_d->resourceNamesPartialExcluded.clear();
    m_d->tagsPartialIncluded.clear();
    m_d->tagsPartialExcluded.clear();
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
            if (checkDelimetersAndCut(m_d->exactMatchBeginEnd, workingToken)) {
                if (included) {

                    m_d->tagExactMatchesIncluded.insert(workingToken);
                } else {

                    m_d->tagExactMatchesExcluded.insert(workingToken);
                }
            } else {
                if (included) {

                    m_d->tagsPartialIncluded.append(workingToken);
                } else {

                    m_d->tagsPartialExcluded.append(workingToken);
                }
            }
        } else if (checkDelimetersAndCut(m_d->exactMatchBeginEnd, workingToken)) {
            if (included) {

                m_d->resourceExactMatchesIncluded.insert(workingToken);
            } else {

                m_d->resourceExactMatchesExcluded.insert(workingToken);
            }

        } else {
            if (included) {

                m_d->resourceNamesPartialIncluded.append(workingToken);
            } else {

                m_d->resourceNamesPartialExcluded.append(workingToken);
            }
        }
    }
}
