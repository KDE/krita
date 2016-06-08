/* This file is part of the KDE project
Copyright (C) 2004-2006 David Faure <faure@kde.org>
Copyright (C) 2007-2008 Thorsten Zachmann <zachmann@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public License
along with this library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
* Boston, MA 02110-1301, USA.
*/

#ifndef KOTEXTSHAREDSAVINGDATA_H
#define KOTEXTSHAREDSAVINGDATA_H

#include <KoSharedSavingData.h>
#include "kritatext_export.h"

#include <QMap>
#include <QSharedPointer>

#define KOTEXT_SHARED_SAVING_ID "KoTextSharedSavingId"

class KoGenChanges;

namespace Soprano
{
class Model;
}

class QString;


class KRITATEXT_EXPORT KoTextSharedSavingData : public KoSharedSavingData
{
public:
    KoTextSharedSavingData();
    virtual ~KoTextSharedSavingData();

    void setGenChanges(KoGenChanges &changes);

    KoGenChanges& genChanges() const;

    void addRdfIdMapping(const QString &oldid, const QString &newid);
    QMap<QString, QString> getRdfIdMapping() const;

    /**
     * The Rdf Model ownership is not taken, you must still delete it,
     * and you need to ensure that it lives longer than this object
     * unless you reset the model to 0.
     */
#ifdef SHOULD_BUILD_RDF
    void setRdfModel(QSharedPointer<Soprano::Model> m);
    QSharedPointer<Soprano::Model> rdfModel() const;
#endif

    /**
     * Stores the name that written to the file for the style
     *
     * @param styleId the id of the style in KoStyleManger
     * @param savedName the name that is written to the file
     */
    void setStyleName(int styleId, const QString &name);

    /**
     * Style name of the style
     *
     * @param styleId the id of the style in KoStyleManager
     * @return the saved name of the style
     */
    QString styleName(int styleId) const;

    /**
     * @brief styleNames List of all names of the styles that are saved
     * @return All the names of styles that are saved in the style manager
     */
    QList<QString> styleNames() const;

private:

    class Private;
    QScopedPointer<Private> d;
};

#endif // KOTEXTSHAREDSAVINGDATA_H
