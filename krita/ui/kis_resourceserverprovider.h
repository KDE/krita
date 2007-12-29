/*
 *  kis_resourceserverprovider.h - part of KImageShop
 *
 *  Copyright (c) 1999 Matthias Elter <elter@kde.org>
 *  Copyright (c) 2003 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2005 Sven Langkamp <sven.langkamp@gmail.com>
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
#ifndef KIS_RESOURCESERVERPROVIDER_H_
#define KIS_RESOURCESERVERPROVIDER_H_

#include <QString>
#include <QStringList>
#include <QList>

#include <KoResourceServer.h>

#include <krita_export.h>

class KoResource;
class KisBrush;
class KisImagePipeBrush;
class KisPattern;

class KRITAUI_EXPORT KisResourceServerProvider : public QObject
{
    Q_OBJECT

public:
    virtual ~KisResourceServerProvider();

    static KisResourceServerProvider* instance();

    KoResourceServer<KisBrush>* brushServer();
    KoResourceServer<KisPattern>* patternServer();
private:
    KisResourceServerProvider();
    KisResourceServerProvider(const KisResourceServerProvider&);
    KisResourceServerProvider operator=(const KisResourceServerProvider&);

    static KisResourceServerProvider *m_singleton;
    KoResourceServer<KisBrush>* m_brushServer;
    KoResourceServer<KisPattern>* m_patternServer;

private slots:

    void brushThreadDone();
    void patternThreadDone();

private:
    
    QThread * brushThread;
    QThread * patternThread;
};

#endif // KIS_RESOURCESERVERPROVIDER_H_
