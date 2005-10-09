/*
 *  kis_resourceserver.h - part of KImageShop
 *
 *  Copyright (c) 1999 Matthias Elter <elter@kde.org>
 *  Copyright (c) 2003 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2005 Sven Langkamp <longamp@reallygood.de>
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
#ifndef KIS_RESOURCESERVER_H_
#define KIS_RESOURCESERVER_H_

#include <qstring.h>

class KisResource;

class KisResourceServerBase {

public:
    KisResourceServerBase(QString type, QStringList fileExtensions);
    virtual ~KisResourceServerBase();

    void loadResources();
    QValueList<KisResource*> resources();
    QString type() { return m_type; };
    
protected:
    virtual KisResource* createResource( QString filename ) = 0;

private:
    QValueList<KisResource*> m_resources;
    QStringList m_fileExtensions;
    QString m_type;

    bool m_loaded;

};

template <class T> class KisResourceServer : public KisResourceServerBase {
    typedef KisResourceServerBase super;

public:
    KisResourceServer(QString type, QStringList fileExtensions) :super( type, fileExtensions) {}
    virtual ~KisResourceServer(){}

private:
    KisResource* createResource( QString filename ){return new T(filename);}
};

#endif // KIS_RESOURCESERVER_H_

