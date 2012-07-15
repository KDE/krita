/*
 *  Copyright (c) 2010 Carlos Licea <carlos@kdab.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KOTABLE_H
#define KOTABLE_H

class KoColumn;
class KoRow;
class KoCell;
#include "KoTblStyle.h"

class KoXmlWriter;
class KoGenStyles;

#include "koodf_export.h"

#include <QMap>
#include <QVector>
#include <QPair>

/**
* \class KoTable represents a table in an ODF element.
* Note that, at least for now, the class is meant to be used
* only to write tables and as such lacks much of the functionallity
* needed to read written tables.
* 
* All the pointers returned by this class are guaranteed to be not-null.
* Do note, however, that there's no way to clear a cell, row or column.
* 
* The \class KoTable owns all the pointer objects returned by
* its methods.
**/

class KOODF_EXPORT KoTable
{
public:
    KoTable();
    ~KoTable();

    KoRow* rowAt(int row);
    int rowCount() const;

    KoColumn* columnAt(int column);
    int columnCount() const;

    KoCell* cellAt(int row, int column);

    void saveOdf(KoXmlWriter& writer, KoGenStyles& styles);

    KoTblStyle::Ptr tableStyle();
    void setTableStyle(KoTblStyle::Ptr style);

//     KoTableTemplate* tableTemplate();
//     void setTableTemplate(KoTableTemplate* tableTemplate);
//     TableTemplateFlags templateFlags();
//     void setTemplateFlags(TableTemplateFlags templateFlags);

    bool printable() const;
    void setPrintable(bool printable);

//     void setPrintRange(CellRange cellRange);
//     CellRange printRange() const;

    void setName(const QString& name);
    QString name() const;

    void setProtected(bool isProtected);
    bool isPprotected() const;

    void setProtectionKey(QString password, QString protectionAlgorithmUri = "http://www.w3.org/2000/09/xmldsig#sha1");
    QString protectionKey() const;
    QString protectionalgorithm() const;

private:
    QVector<KoColumn*> m_columns;
    QVector<KoRow*> m_rows;

    QMap<QPair<int,int>, KoCell*> m_cells;

    int m_rowCount;
    int m_columnCount;

    KoTblStyle::Ptr m_style;
//     KoTableTemplate* m_template;
//     TableTemplateFlags m_templateFlags;

    bool m_printable;
//     CellRange m_printRange;
    QString m_name;

    bool m_protected;
    QString m_protectionKey;
    QString m_protectionAlgorithm;
};

#endif
