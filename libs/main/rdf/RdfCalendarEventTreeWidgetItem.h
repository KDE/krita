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

#ifndef __rdf_RdfCalendarEventTreeWidgetItem_h__
#define __rdf_RdfCalendarEventTreeWidgetItem_h__

#include "komain_export.h"
#include "rdf/RdfForward.h"
#include "rdf/RdfCalendarEvent.h"
#include "rdf/RdfSemanticTreeWidgetItem.h"
#include "rdf/RdfFoaFTreeWidgetItem.h"

class KoCanvasBase;

/**
 * @short Display ical/vevent semantic data with a context menu tailored
 *        to such infomartion.
 * @author Ben Martin <ben.martin@kogmbh.com>
 */
class KOMAIN_EXPORT RdfCalendarEventTreeWidgetItem : public RdfSemanticTreeWidgetItem
{
    Q_OBJECT;

public:
    enum {
        Type = RdfFoaFTreeWidgetItem::Type + 1
    };
    RdfCalendarEventTreeWidgetItem(QTreeWidgetItem* parent, RdfCalendarEvent* ev);

    // inherited and reimplemented...

    RdfCalendarEvent* semanticObject();
    virtual QList<KAction *> actions(QWidget *parent, KoCanvasBase* host = 0);
    virtual void insert(KoCanvasBase* host);

public slots:
    void saveToKCal();
    void exportToFile();

private:

    RdfCalendarEvent* m_semanticObject;
protected:
    virtual RdfSemanticItem* semanticItem();
    virtual QString UIObjectName();

};
#endif
