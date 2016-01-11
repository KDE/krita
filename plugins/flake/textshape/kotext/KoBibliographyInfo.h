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
#ifndef KOBIBLIOGRAPHYINFO_H
#define KOBIBLIOGRAPHYINFO_H

#include "kritatext_export.h"
#include <KoXmlReaderForward.h>
#include "ToCBibGeneratorInfo.h"

class KoTextSharedLoadingData;
class KoXmlWriter;

class KRITATEXT_EXPORT BibliographyGeneratorInterface {
public:
    BibliographyGeneratorInterface() {}
    virtual ~BibliographyGeneratorInterface() {}
    //virtual void setMaxTabPosition(qreal maxTabPosition) = 0;
};

class KRITATEXT_EXPORT KoBibliographyInfo
{
public:
    KoBibliographyInfo();

    ~KoBibliographyInfo();

    void loadOdf(KoTextSharedLoadingData *sharedLoadingData, const KoXmlElement &element);
    void saveOdf(KoXmlWriter *writer) const;

    void setGenerator(BibliographyGeneratorInterface *generator);
    void setEntryTemplates(QMap<QString, BibliographyEntryTemplate> &entryTemplates);

    KoBibliographyInfo *clone();
    BibliographyGeneratorInterface *generator() const;

    QString m_name;
    QString m_styleName;
    IndexTitleTemplate m_indexTitleTemplate;
    QMap<QString, BibliographyEntryTemplate> m_entryTemplate;

private:
    int styleNameToStyleId(KoTextSharedLoadingData *sharedLoadingData, const QString &styleName);
    BibliographyGeneratorInterface *m_generator;
};

Q_DECLARE_METATYPE(KoBibliographyInfo *)

#endif // KOBIBLIOGRAPHYINFO_H
