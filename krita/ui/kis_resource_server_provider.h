/*
 *  kis_resource_server_provider.h - part of KImageShop
 *
 *  Copyright (c) 1999 Matthias Elter <elter@kde.org>
 *  Copyright (c) 2003 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2005 Sven Langkamp <sven.langkamp@gmail.com>
 *  Copyright (c) 2003-2008 Boudewijn Rempt <boud@valdyas.org>
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
class KisPattern;
class KisPaintOpPreset;

class KRITAUI_EXPORT KisResourceServerProvider : public QObject
{
    Q_OBJECT

public:
    virtual ~KisResourceServerProvider();

    static KisResourceServerProvider* instance();

    KoResourceServer<KisPattern>* patternServer();
    KoResourceServer<KisPaintOpPreset>* paintOpPresetServer();

private:

    KisResourceServerProvider();
    KisResourceServerProvider(const KisResourceServerProvider&);
    KisResourceServerProvider operator=(const KisResourceServerProvider&);

    KoResourceServer<KisPattern>* m_patternServer;
    KoResourceServer<KisPaintOpPreset>* m_paintOpPresetServer;

private slots:

    void patternThreadDone();
    void paintOpPresetThreadDone();

private:

    QThread * patternThread;
    QThread * paintOpPresetThread;
};

#endif // KIS_RESOURCESERVERPROVIDER_H_
