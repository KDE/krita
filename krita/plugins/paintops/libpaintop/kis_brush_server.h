/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_BRUSH_SERVER_H
#define KIS_BRUSH_SERVER_H

#include <QString>
#include <QStringList>
#include <QList>

#include <KoResourceServer.h>

class KoResource;
class KisBrush;

/**
 *
 */
class KisBrushServer : public QObject {

    Q_OBJECT

public:

    virtual ~KisBrushServer();
    KoResourceServer<KisBrush>* brushServer();

    static KisBrushServer* instance();

private:

    KisBrushServer();
    KisBrushServer(const KisBrushServer&);
    KisBrushServer operator=(const KisBrushServer&);

    static KisBrushServer *m_singleton;

    KoResourceServer<KisBrush>* m_brushServer;

private slots:

    void brushThreadDone();

private:

    QThread * brushThread;

};

#endif
