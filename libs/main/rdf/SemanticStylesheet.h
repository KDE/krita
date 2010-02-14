/* This file is part of the KDE project
   Copyright (C) 2010 KO GmbH <ben.martin@kogmbh.com>

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

#ifndef __rdf_SemanticStylesheet_h__
#define __rdf_SemanticStylesheet_h__

#include "komain_export.h"

#include <QObject>
#include <QSharedData>
#include <QList>
#include <QString>
#include "RdfForward.h"

/**
 * @short A stylesheet that knows how to format a given RdfSemanticItem
 *
 * If you are looking to apply a stylesheet you should use the RdfSemanticItemViewSite
 * class. For example:
 *
 * RdfSemanticItemViewSite vs( SemanticItemPtr, xmlid );
 * vs.applyStylesheet( canvas, StylesheetPtr );
 *
 * @author Ben Martin <ben.martin@kogmbh.com>
 * @see RdfSemanticItem
 * @see RdfSemanticItemViewSite
 * @see KoDocumentRdf
 */
class KOMAIN_EXPORT SemanticStylesheet : public QObject
{
    Q_OBJECT
protected:

    // Restrict who can make us
    friend class RdfSemanticItem;
    friend class RdfCalendarEvent;
    friend class RdfFoaF;
    friend class RdfLocation;
    SemanticStylesheet(const QString &uuid, const QString &name, const QString &templateString,
                       const QString &type = "System", bool isMutable = false);

    friend class RdfSemanticItemViewSite;
    /**
     * Only called from RdfSemanticItemViewSite, this method actually
     * applies the stylesheet to a specific reference to a semantic
     * item in the document.
     */
    void format(RdfSemanticItem *obj, KoTextEditor *editor, const QString& xmlid = QString());

public:
    static QString TYPE_SYSTEM; // TODO static QStrings are not safe. Also fix CamelCase
    static QString TYPE_USER;

    QString uuid();
    QString name();
    QString templateString();
    QString type();
    bool isMutable();

    // if (isMutable()) these methods update
    // user stylesheets are mutable, system ones are not
    void name(const QString &v);
    void templateString(const QString &v);

signals:
    void nameChanging(SemanticStylesheet*, QString oldName, QString newName);

private:
    QString m_uuid;
    QString m_name;
    QString m_templateString;
    QString m_type;
    bool m_isMutable;

};

#include <QMetaType>
Q_DECLARE_METATYPE(SemanticStylesheet*)
#endif
