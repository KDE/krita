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

#ifndef __rdf_RdfCalendarEvent_h__
#define __rdf_RdfCalendarEvent_h__

#include "komain_export.h"
#include "rdf/RdfSemanticItem.h"
#include <QSharedPointer>

namespace KCal
{
    class Event;
};


/**
 * @short Calendar event information from Rdf (ical/vevent).
 * @author Ben Martin <ben.martin@kogmbh.com>
 *
 * see the following places for more information
 * http://www.w3.org/TR/rdfcal/
 * http://www.w3.org/2002/12/cal/
 * http://www.w3.org/2002/12/cal/test/
 *
 */
class KOMAIN_EXPORT RdfCalendarEvent : public RdfSemanticItem
{
    Q_OBJECT;
    struct Private;
    QSharedPointer<Private> d;

    KCal::Event* toKEvent();
    void fromKEvent(KCal::Event* e);

public:
    RdfCalendarEvent(QObject* parent, KoDocumentRdf* m_rdf = 0);
    RdfCalendarEvent(QObject* parent, KoDocumentRdf* m_rdf, Soprano::QueryResultIterator& it);
    virtual ~RdfCalendarEvent();

    // inherited and reimplemented...

    virtual void exportToFile(const QString& fileName = "");
    virtual void importFromData(const QByteArray& ba, KoDocumentRdf* m_rdf = 0, KoCanvasBase* host = 0);
    virtual QWidget* createEditor(QWidget *parent);
    virtual void updateFromEditorData();
    virtual RdfSemanticTreeWidgetItem* createQTreeWidgetItem(QTreeWidgetItem* parent = 0);
    virtual Soprano::Node linkingSubject() const;
    virtual void setupStylesheetReplacementMapping(QMap< QString, QString >& m);
    virtual void exportToMime(QMimeData* md);
    virtual QList<SemanticStylesheet*>& stylesheets();
    virtual QList<SemanticStylesheet*>& userStylesheets();

    /**
     * Save ourself to the users KDE events calendar.
     */
    virtual void saveToKCal();

    // accessor methods...

    QString name();
    QString location();
    QString summary();
    QString uid();
    KDateTime start();
    KDateTime end();
};
#endif
