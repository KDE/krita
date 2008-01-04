/* This file is part of the KDE project.
 *
 * Copyright (C) 2008 Boudewijn Rempt <boud@valdyas.org>
 * Copyright (C) 2008 Inge Wallin     <inge@lysator.liu.se>
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

#ifndef TABLESHAPE_H
#define TABLESHAPE_H

#include <KoShape.h>

#define TABLESHAPEID "TableShapeID"


class QTextDocument;
class QTextTable;

class KoTextShapeData;

class TableShape : public KoShape
{
public:
    explicit TableShape();
    virtual ~TableShape();

    // reimplemented
    virtual void paint( QPainter& painter, const KoViewConverter& converter );
    // reimplemented
    virtual void saveOdf( KoShapeSavingContext & context ) const;
    // reimplemented
    virtual bool loadOdf( const KoXmlElement    &element, 
                          KoShapeLoadingContext &context );

private:

    void createExampleData();
    void initTableFrames();
    void recalculateCellPositions();
    double recalculateCellHeight(int row, int column);
    
    QTextTable     * m_table;
    QTextDocument  * m_textDocument;
    KoTextShapeData * m_textShapeData;
    class TableCellFrame;
    QVector< QVector<TableCellFrame*> > m_tableFrames;
    
};


#endif
