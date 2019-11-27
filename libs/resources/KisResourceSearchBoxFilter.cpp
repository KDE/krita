/*
 *  Copyright (c) 2019 Agata Cacko <cacko.azh@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
    : isTag("\\[([\\w\\s]+)\\]")
    , isExactMatch("\"(.+)\"")
    , searchTokenizer("\\s*,+\\s*")
    , filter("")
    , tagNamesIncluded()
    , tagNamesExcluded()
    , resourceNamesPartsIncluded()
    , resourceNamesPartsExcluded()
    , resourceExactMatchesIncluded()
    , resourceExactMatchesExcluded()
    {}

    QRegularExpression isTag;
    QRegularExpression isExactMatch;
    QRegularExpression searchTokenizer;

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

void KisResourceSearchBoxFilter::setFilter(const QString& filter)
{
    d->tagNamesIncluded.clear();
    d->tagNamesExcluded.clear();
    d->resourceNamesPartsIncluded.clear();
    d->resourceNamesPartsExcluded.clear();
    d->resourceExactMatchesIncluded.clear();
    d->resourceExactMatchesExcluded.clear();

    QString tempFilter(filter);
    d->filter = QString(tempFilter);

    QStringList parts = tempFilter.split(d->searchTokenizer, QString::SkipEmptyParts);
    Q_FOREACH(const QString& partFor, parts) {
        QString part(partFor);

        fprintf(stderr, "Filter: part: %s\n", part.toStdString().c_str());

        bool included = true;
        if (part.startsWith('!')) {
            part.remove(0, 1);
            included = false;
        }

        QRegularExpressionMatch tagMatch = d->isTag.match(part, 0, QRegularExpression::NormalMatch, QRegularExpression::AnchoredMatchOption);
        QRegularExpressionMatch exactMatchMatch = d->isExactMatch.match(part, 0, QRegularExpression::NormalMatch, QRegularExpression::AnchoredMatchOption);

        if (tagMatch.hasMatch()) {
            QString tagMatchCaptured = tagMatch.captured();
            fprintf(stderr, "Filter: part: %s - it is a tag! %s\n", part.toStdString().c_str(), tagMatchCaptured.toStdString().c_str());
            if (included) {
                d->tagNamesIncluded.insert(tagMatchCaptured);
            } else {
                d->tagNamesExcluded.insert(tagMatchCaptured);
            }
        } else if (exactMatchMatch.hasMatch()) {
            QString exactMatchMatchCaptured = exactMatchMatch.captured();

            QString captured = exactMatchMatchCaptured;
            captured = captured.remove(0, 1);
            captured = captured.left(captured.length() - 1);
            fprintf(stderr, "Filter: part: %s - it is an exact Match! %s %s\n", part.toStdString().c_str(), exactMatchMatchCaptured.toStdString().c_str(), captured.toStdString().c_str());
            if (included) {
                d->resourceExactMatchesIncluded.insert(captured);
            } else {
                d->resourceExactMatchesExcluded.insert(captured);
            }
        } else {
            fprintf(stderr, "Filter: part: %s - it is partial\n", part.toStdString().c_str());
            if (included) {
                d->resourceNamesPartsIncluded.append(part);
            } else {
                d->resourceNamesPartsExcluded.append(part);
            }
        }
    }


}


bool KisResourceSearchBoxFilter::matchesResource(const QString &resourceName)
{
    fprintf(stderr, "## resource name: %s ", resourceName.toStdString().c_str());
    // exact name matches
    fprintf(stderr, "## 1 ");
    if (d->resourceExactMatchesIncluded.count() > 0
            && !d->resourceExactMatchesIncluded.contains(resourceName)) {
        return false;
    }
    fprintf(stderr, "## 2 ");
    if (d->resourceExactMatchesExcluded.contains(resourceName)) {
        return false;
    }
    fprintf(stderr, "## 3-%d ", d->resourceNamesPartsIncluded.count());
    // partial name matches
    if (d->resourceNamesPartsIncluded.count() > 0) {
        fprintf(stderr, "## 3a ");
        Q_FOREACH(const QString& partialName, d->resourceNamesPartsIncluded) {
            fprintf(stderr, "checking for partial: %s\n", partialName.toStdString().c_str());
            if (!resourceName.contains(partialName)) {
                fprintf(stderr, "it doesn't contain it!\n");
                return false;
            } else {
                fprintf(stderr, "it DOES contain it!\n");
            }
        }
    }
    fprintf(stderr, "## 4 ");
    Q_FOREACH(const QString& partialName, d->resourceNamesPartsExcluded) {
        if (resourceName.contains(partialName)) {
            return false;
        }
    }
    fprintf(stderr, "## 5 \n");
    // tags matches

    return true;
}

