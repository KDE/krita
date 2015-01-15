/*
 *  Copyright (c) 2010 Carlos Licea <carlos@kdab.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation; either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KOTABLEPROPERTIES_H
#define KOTABLEPROPERTIES_H

#include "KoTblStyle.h"

class KoTableTemplate;

class TableProperties
{
public:
    TableStyle* tableStyle();
    void setTableStyle(TableStyle* style);

    TableTemplate* tableTemplate();
    void setTableTemplate(KoTableTemplate* tableTemplate);
    TableTemplateFlags templateFlags();
    void setTemplateFlags(TableTemplateFlags templateFlags);

    bool printable() const;
    void setPrintable(bool printable);

    void setPrintRange(CellRange cellRange);
    CellRange printRange() const;

    void setName(QString name);
    QString name() const;

    void setProtected(bool isProtected);
    bool isPprotected() const;
    void setPlainPassword(QString password, QString uri = "http://www.w3.org/2000/09/xmldsig#sha1");

private:
    void saveOdf(KoXmlWriter* writer, KoGenStyles* styles);

    TableStyle* m_style;
    TableTemplate* m_template;
    TableTemplateFlags m_templateFlags;

    bool m_printable;
    CellRange m_printRange;
    QString m_name;

    bool m_protected;
    QString m_password;
};

#endif
