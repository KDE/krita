/*
 *  Copyright (c) 2010 Valek Filippov <frob@gnome.org>
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef ABR_STRUCT_PARSER_H
#define ABR_STRUCT_PARSER_H

#include <QHash>
#include <QDomDocument>
#include "kis_abr_translator.h"

const quint32 MAGIC_OBJC_LENGTH = 1331849827;
const QString TDTA = "tdta";
const QString PATT = "patt";
const QString DESC = "desc";
const QString VLLS = "VlLs";
const QString TEXT = "TEXT";
const QString OBJC = "Objc";
const QString UNTF = "UntF";
const QString BOOL = "bool"; 
const QString LONG = "long"; 
const QString DOUB = "doub"; 
const QString ENUM = "enum";

enum enumFuncNames  {
    P_PATT,
    P_DESC,
    P_VLLS,
    P_TEXT,
    P_OBJC,
    P_UNTF,
    P_BOOL, 
    P_LONG, 
    P_DOUB, 
    P_ENUM,
    P_TDTA
};

class AbrStructParser{

public:
    AbrStructParser();
    ~AbrStructParser();
    
    void parse(QString fileName);

private:
    // see Valek's script, these methods are ported from Python
    QString p_patt(QDataStream &buf);
    QString p_tdta(QDataStream &buf);
    QString p_desc(QDataStream &buf);
    QString p_long(QDataStream &buf);
    QString p_vlls(QDataStream &buf);
    QString p_text(QDataStream &buf);
    QString p_objc(QDataStream &buf);
    QString p_untf(QDataStream &buf);
    QString p_bool(QDataStream &buf);
    QString p_doub(QDataStream &buf);
    QString p_enum(QDataStream &buf);
    
    quint32 parseEntry(QDataStream &buf);
    
        
private:
    QHash<QString, enumFuncNames> m_types;
    QDomDocument m_doc;
    KisAbrTranslator m_translator;
};

#endif // ABR_STRUCT_PARSER_H
