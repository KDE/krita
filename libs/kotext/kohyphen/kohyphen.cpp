/******************************************************************************
 *   Copyright (C) 2002 by Alexander Dymo <cloudtemple@mskat.net>             *
 *   Copyright (C) 2002-2003 by Lukas Tinkl <lukas@kde.org>                   *
 *   Copyright (C) 2003 David Faure <faure@kde.org>                           *
 *                                                                            *
 *   This program is free software; you can redistribute it and/or modify     *
 *   it under the terms of the GNU Library General Public License as          *
 *   published by the Free Software Foundation; either version 2 of the       *
 *   License, or (at your option) any later version.                          *
 *                                                                            *
 *   This library is distributed in the hope that it will be useful,          *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 *  Library General Public License for more details.                          *
 *                                                                            *
 *  You should have received a copy of the GNU Library General Public License *
 *  along with this library; see the file COPYING.LIB.  If not, write to      *
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.                                               *
 ******************************************************************************/

#include <qdom.h>
#include <QFile>
#include <qtextcodec.h>
#include <QString>

#include <klocale.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <kstaticdeleter.h>

#include "kohyphen.h"
#include <kdebug.h>


//#define DEBUG_HYPHENATOR 1

KoHyphenator* KoHyphenator::s_self;
static KStaticDeleter<KoHyphenator> kohyphensd;

KoHyphenator* KoHyphenator::self()
{
    if ( !s_self )
        kohyphensd.setObject( s_self, new KoHyphenator );
    return s_self;
}

KoHyphenator::KoHyphenator()
{
/*  Reading config for dictionary encodings from file...*/

    QString path = kapp->dirs()->findResource("data", "koffice/hyphdicts/dicts.xml");
#ifdef DEBUG_HYPHENATOR
    kDebug() << path << endl;
#endif

    QFile *f;
    if (!path.isNull())
        f = new QFile(path);
    else
        throw KoHyphenatorException( "Could not create KoHyphenator instance." );

    QDomDocument config;
    QDomNodeList records;
    config.setContent(f);

    for (QDomNode n = config.firstChild(); !n.isNull(); n = n.nextSibling())
        if (n.nodeName() == "dicts")
        {
            records = n.childNodes();
            for (uint i = 0; i < records.count(); i++)
            {
                QDomNamedNodeMap attr = records.item(i).attributes();
                if (attr.contains("lang") && attr.contains("encoding")) {
                    QString lang = attr.namedItem("lang").nodeValue();
                    QString encoding = attr.namedItem("encoding").nodeValue();
#ifdef DEBUG_HYPHENATOR
                    kDebug() << "KoHyphenator: found lang=" << lang << " encoding=" << encoding << endl;
#endif
                    encodings.insert( lang,
                                      EncodingStruct( encoding.latin1() ) );
                }
            }
        }

    delete f;
}

KoHyphenator::~KoHyphenator()
{
    for (QMap<QString, HyphenDict*>::iterator it = dicts.begin(); it != dicts.end(); ++it)
    {
        if ((*it) != 0)
            hnj_hyphen_free((*it));
    }
}

char *KoHyphenator::hyphens(const QString& str, const QString& lang) const
{
    char *x = new char[str.length()+1];
    try
    {
        QTextCodec *codec = codecForLang(lang);
        hnj_hyphen_hyphenate(dict(lang), (const char *)(codec->fromUnicode(str)), str.length(), x);
    }
    catch (KoHyphenatorException &e)
    {
#ifdef DEBUG_HYPHENATOR
        kDebug() << e.message().latin1() << endl;
#endif
        for (uint j = 0; j < str.length(); j++)
            x[j] = '0';
        x[str.length()] = '\0';
    }
    return x;
}

QString KoHyphenator::hyphenate(const QString& str, const QString& lang) const
{
    char* x = new char[str.length()+1];
    QString res = str;
    try
    {
        QTextCodec *codec = codecForLang(lang);
        hnj_hyphen_hyphenate(dict(lang), (const char *)(codec->fromUnicode(str)), str.length(), x);
    }
    catch (KoHyphenatorException &e)
    {
#ifdef DEBUG_HYPHENATOR
        kDebug() << e.message() << endl;
#endif
        delete[] x;
        return str;
    }
    int i = 0, j = 0;
    int len = strlen(x);
    for (; i < len; i++)
    {
#ifdef DEBUG_HYPHENATOR
        kDebug() << "loop: i=" << i << ", j=" << j << ", x=" << x << ", res=" << res << endl;
#endif
        if ((x[i] % 2) != 0)
        {
            res.insert(j+1, QChar(0xad));
            j++;
        }
        j++;
    }
    delete[] x;
    return res;
}

bool KoHyphenator::checkHyphenPos(const QString& str, int pos, const QString& lang) const
{
#ifdef DEBUG_HYPHENATOR
    kDebug() << "string: " << str << endl;
#endif

    char *hyph = hyphens(str, lang);

#ifdef DEBUG_HYPHENATOR
    kDebug() << "result: " << hyph << endl;
    kDebug() << "checked position: " << pos << endl;
#endif
    bool ret = ((hyph[pos] % 2) != 0);
    delete[] hyph;
    return ret;
}

HyphenDict *KoHyphenator::dict(const QString &_lang) const
{
    QString lang( _lang );
    //only load dictionary when encoding info is present
    if (encodings.find(lang) == encodings.end())
    {
        int underscore = lang.find('_');
        if ( underscore > -1 ) {
            lang.truncate( underscore );
            if (encodings.find(lang) == encodings.end())
                throw KoHyphenatorException( QString("No dictionary for %1").arg(lang) );
        }
        else
            throw KoHyphenatorException( QString("No dictionary for %1").arg(lang) );
    }
    if (dicts.find(lang) == dicts.end())
    {
#ifdef DEBUG_HYPHENATOR
        kDebug() << "Searching dictionary for '" << lang << "' language..." << endl;
#endif
        QString path = kapp->dirs()->findResource("data", "koffice/hyphdicts/hyph_" + lang + ".dic");
        if (!path.isNull())
        {
#ifdef DEBUG_HYPHENATOR
            kDebug() << "Loading dictionary for '" << lang << "' language: path = " << path << endl;
#endif
            const_cast<KoHyphenator*>(this)->dicts.insert( lang, hnj_hyphen_load(QFile::encodeName(path)) );
            if (dicts.find(lang) == dicts.end())
            {
#ifdef DEBUG_HYPHENATOR
                kDebug() << "No dictionary loaded" << endl;
#endif
                throw(KoHyphenatorException( QString("Could not load dictionary for the language: %1").arg(lang) ));
            }
        }
        else
            throw(KoHyphenatorException( QString("Could not load dictionary for the language: %1").arg(lang) ));
    }
    return dicts[lang];
}

QTextCodec* KoHyphenator::codecForLang(const QString& lang) const
{
    EncodingMap::Iterator it = encodings.find(lang);
    if (it == encodings.end())
    {
        int underscore = lang.find('_');
        if ( underscore > -1 ) {
            QString _lang( lang );
            _lang.truncate( underscore );
            it = encodings.find(_lang);
        }
    }
    if (it != encodings.end())
    {
        if ( (*it).codec )
            return (*it).codec;
        (*it).codec = QTextCodec::codecForName((*it).encoding);
        return (*it).codec;
    }
    return QTextCodec::codecForMib(106); // utf-8
}
