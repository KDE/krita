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

#ifndef __rdf_KoRdfFoaFTreeWidgetItem_h__
#define __rdf_KoRdfFoaFTreeWidgetItem_h__

#include "KoRdfFoaF.h"
#include "KoRdfSemanticTreeWidgetItem.h"

class KoCanvasBase;

/**
 * @short Display Contact/FOAF semantic data with a context menu tailored
 *        to such infomartion.
 * @author Ben Martin <ben.martin@kogmbh.com>
 */
class KoRdfFoaFTreeWidgetItem : public KoRdfSemanticTreeWidgetItem
{
    Q_OBJECT
public:
    KoRdfFoaFTreeWidgetItem(QTreeWidgetItem *parent, hKoRdfFoaF foaf);

    // inherited and reimplemented...

    hKoRdfFoaF foaf() const;
    virtual QList<QAction *> actions(QWidget *parent, KoCanvasBase *host = 0);
    virtual void insert(KoCanvasBase *host);

public Q_SLOTS:
    void importSelectedSemanticViewContact();
    void exportToFile();

protected:
    virtual hKoRdfSemanticItem semanticItem() const;
    virtual QString uIObjectName() const;

private:
    hKoRdfFoaF m_foaf;
};

#endif
