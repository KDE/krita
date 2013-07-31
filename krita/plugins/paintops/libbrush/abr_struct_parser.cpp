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

#include <QString>
#include <QFile>
#include <QDataStream>
#include <QDebug>

#include <QDomDocument>

#include "abr_struct_parser.h"
#include "kis_abr_translator.h"


AbrStructParser::AbrStructParser()
{
    m_types.insert(PATT, P_PATT);
    m_types.insert(DESC, P_DESC);
    m_types.insert(VLLS, P_VLLS);
    m_types.insert(TEXT, P_TEXT);
    m_types.insert(OBJC, P_OBJC);
    m_types.insert(UNTF, P_UNTF);
    m_types.insert(BOOL, P_BOOL);
    m_types.insert(LONG, P_LONG);
    m_types.insert(DOUB, P_DOUB);
    m_types.insert(ENUM, P_ENUM);
    m_types.insert(TDTA, P_TDTA);
}

AbrStructParser::~AbrStructParser()
{

}

QString AbrStructParser::p_patt(QDataStream &buf){
    // didn't rev.engineered yet
    Q_UNUSED(buf);
    return QString("");
}

QString AbrStructParser::p_tdta(QDataStream &buf){
    quint32 size;
    buf >> size;

    ushort * text = new ushort[size];
    for (quint32 i = 0; i < size;i++){
        buf >> text[i];
    }
        
    return "(tdta:" + QString::number(size) + ')' + QString::fromUtf16(text,size);
}

QString AbrStructParser::p_desc(QDataStream &buf){
    // convert 4 bytes as big-endian unsigned long
    quint32 size;
    // 22 + 4
    buf >> size;
    buf.skipRawData(22);
    return QString::number( size );
}

QString AbrStructParser::p_long(QDataStream &buf){
    quint32 size;
    buf >> size;
    return QString::number(size);
}

QString AbrStructParser::p_vlls(QDataStream &buf){
    quint32 size;
    buf >> size;
    return QString::number(size);
}

QString AbrStructParser::p_text(QDataStream &buf){
    quint32 size;
    buf >> size;
    
    ushort * text = new ushort[size+1];
    for (quint32 i = 0; i < size;i++){
        buf >> text[i];
    }
    text[size] = '\0';
    return QString::fromUtf16(text);
}

QString AbrStructParser::p_objc(QDataStream &buf){
    // here we lost some data definitly 
    // objnamelen is always 1 and objname is empty string
    quint32 objnamelen;
    buf >> objnamelen;
    
    char * objname = new char[objnamelen*2+1];
    buf.readRawData(objname,objnamelen*2);
    objname[ objnamelen * 2 ] = '\0';
    
    Q_ASSERT(objnamelen == 1);
    
    quint32 objtypelen;
    buf >> objtypelen;
    if (objtypelen == 0){
        objtypelen = 4;
    }
    
    char * typeName = new char[objtypelen+1];
    buf.readRawData(typeName,objtypelen);
    typeName [objtypelen] = '\0';
    
    quint32 value;
    buf >> value;
    //return QString::fromLatin1( objname ) + ' ' + QString::fromLatin1(typeName) + ' ' + QString::number(value);
    return QString::fromLatin1(typeName) + ' ' + QString::number(value);
}
    
QString AbrStructParser::p_untf(QDataStream &buf){
    char * type = new char[5];
    buf.readRawData(type, 4);
    type[4] = '\0';
    double value;
    buf >> value;
    return QString::fromLatin1(type) + ' ' + QString::number(value);
}

QString AbrStructParser::p_bool(QDataStream &buf){
    //# ord converts 1 byte number
    char byte;
    buf.device()->getChar(&byte);
    if (byte) return QString("1");
    else return QString("0");
}

QString AbrStructParser::p_doub(QDataStream &buf){
    // unpack 8 bytes ieee 754 value to floating point number
    double value;
    buf >> value;
    return QString::number(value);
}

QString AbrStructParser::p_enum(QDataStream &buf){
    quint32 size1, size2;
    buf >> size1;

    if (size1 == 0){            
        size1 = 4;
    }
    char * name1 = new char[size1+1];
    buf.readRawData(name1,size1);
    name1[size1] = '\0';

    buf >> size2 ;
    if (size2 == 0){
        size2 = 4;
    }

    char * name2 = new char[size2+1];
    buf.readRawData(name2,size2);
    name2[size2] = '\0';
    
    return QString::fromLatin1(name1) + ' ' + QString::fromLatin1(name2);
}

quint32 AbrStructParser::parseEntry(QDataStream &buf){
    quint32 nlen;
    buf >> nlen;
    if (nlen == 0){
        nlen = 4;
    }

    QString value;

    if (nlen == MAGIC_OBJC_LENGTH){
        value = p_objc(buf);
        qDebug() << ABR_PRESET_START  << ABR_OBJECT << value;
        // start to create the preset here
        m_translator.addEntry(ABR_PRESET_START, ABR_OBJECT, value);
    }else{
        // read char with nlen bytes and convert to String
        char * name = new char[ nlen+1 ];
        int status = buf.readRawData(name, nlen);
        if (status == -1){
            qDebug() << "Error, name can't be readed";
        }
        name[nlen] = '\0';

        char * type = new char[5];
        status = buf.readRawData(type, 4);
        type[4] = '\0';
        QString key = QString::fromLatin1(type);
        
        if (m_types.contains(key))
        {
            enumFuncNames enumName = m_types[key];
            
            switch (enumName){
                case P_PATT: value = p_patt(buf); break;
                case P_DESC: value = p_desc(buf); break;
                case P_VLLS: value = p_vlls(buf); break;
                case P_TEXT: value = p_text(buf); break;
                case P_OBJC: value = p_objc(buf); break;
                case P_UNTF: value = p_untf(buf); break;
                case P_BOOL: value = p_bool(buf); break;
                case P_LONG: value = p_long(buf); break; 
                case P_DOUB: value = p_doub(buf); break;
                case P_ENUM: value = p_enum(buf); break;
                case P_TDTA: value = p_tdta(buf); break;
                default: qDebug() << "Freak error occurred!"; break;
            }
            
            QString attributeName = QString::fromLatin1(name);
            //qDebug() << attributeName << key << value;
            m_translator.addEntry(attributeName, key, value);
            
            // airbrush is the last parsed attribute of the preset
            if (attributeName == ABR_AIRBRUSH)    {
                m_translator.finishPreset();
                qDebug() << m_translator.toString();
            }
                
        }else
        {
            qDebug() << "Unknown key:\t" << name << type;
            //qDebug() << p_unkn(buf);
            return -1;
        }
    
    }
    return 0;
}


void AbrStructParser::parse(QString fileName){
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Can't open file " << fileName;
        return;
    }
    QDataStream buf(&file); 

    
    // offset in bytes
    short int vermaj, vermin; 
    buf >> vermaj;
    buf >> vermin;
    qDebug() << "Version: " << vermaj << "." << vermin;
    
    int index = file.readAll().indexOf("8BIMdesc");
    buf.device()->seek(index);
    int status = 0;
    while (!buf.atEnd()){
        status = parseEntry(buf);
        if (status == -1){
            // something to break the parsing with fail?
            qDebug() << "Finishing with fail...";
            break;
        }
    }
    qDebug() << m_doc.toString();
}



int main(int argc, const char * argv[] ){
    QString fileName;
    if (argc != 2) {
        fileName = "test.abr";
    }else{
        fileName = QString::fromLatin1(argv[1]);
    }
    
    AbrStructParser parser;
    parser.parse(fileName);
    
    return 0;
}
