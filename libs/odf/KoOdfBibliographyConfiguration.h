/* This file is part of the KDE project
 * Copyright (C) 2011 Smit Patel <smitpatel24@gmail.com>
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
#ifndef KOODFBIBLIOGRAPHYCONFIGURATION_H
#define KOODFBIBLIOGRAPHYCONFIGURATION_H

#include <QString>
#include <QMetaType>
#include <QObject>

#include "KoXmlReaderForward.h"
#include "kritaodf_export.h"

class KoXmlWriter;

typedef QPair<QString, Qt::SortOrder> SortKeyPair;

/**
 * load and save the bibliography-configuration element from the text: namespace.
 * • Prefix
 * • Suffix
 * • Numbered entries
 * • Sort by position
 * • Sort algorithm
 */
class KRITAODF_EXPORT KoOdfBibliographyConfiguration : public QObject
{
    Q_OBJECT
public:

    KoOdfBibliographyConfiguration();
    ~KoOdfBibliographyConfiguration() override;
    KoOdfBibliographyConfiguration(const KoOdfBibliographyConfiguration &other);
    KoOdfBibliographyConfiguration &operator=(const KoOdfBibliographyConfiguration &other);

    static const QList<QString> bibTypes;
    static const QList<QString> bibDataFields;

    /**
     * load the bibliography-configuration element
     */
    void loadOdf(const KoXmlElement &element);

    /**
     * save the bibliography-configuration element
     */
    void saveOdf(KoXmlWriter * writer) const;

    /**
     * Sort by position
     * If text:sort-by-position attribute is true then bibliography entries or citations will be
     * sorted according to their position in document.
     */
    bool sortByPosition() const;
    void setSortByPosition(bool enable);

    /**
     * Numbered entries
     * If text:numbered-entries attribute is true then bibliography entries or citations will be numbered.
     */
    bool numberedEntries() const;
    void setNumberedEntries(bool enable);

    /**
     * Prefix
     * The text:prefix attribute specifies prefix of bibliography entry or citation(text:bibliography-mark)
     */
    QString prefix() const;
    void setPrefix(const QString &prefixValue);

    /**
     * Suffix
     * The text:suffix attribute specifies suffix of bibliography entry or citation(text:bibliography-mark)
     */
    QString suffix() const;
    void setSuffix(const QString &suffixValue);

    /**
     * Sort algorithm
     * The text:sort-algorithm attribute specifies sorting algorithm for bibliography entry
     */
    QString sortAlgorithm() const;
    void setSortAlgorithm(const QString &algorithm);

    /**
     * Sort Keys
     * The text:sort-key attribute specifies sort key for bibliography entry
     */
    QList<SortKeyPair> sortKeys() const;
    void setSortKeys(const QList<SortKeyPair> &sortKeys);

private:

    class Private;
    Private * const d;

};

Q_DECLARE_METATYPE(KoOdfBibliographyConfiguration*)

#endif // KOODFBIBLIOGRAPHYCONFIGURATION_H
