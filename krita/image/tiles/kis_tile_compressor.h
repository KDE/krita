/*
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_TILE_COMPRESSOR_H
#define KIS_TILE_COMPRESSOR_H

#include <QQueue>
#include <QByteArray>
#include <QThread>
#include <QMutex>

class KisTile;

class KisTileCompressor : public QThread
{

public:

    KisTileCompressor();
    ~KisTileCompressor();

    void enqueue( KisTile * tile );
    void dequeue( KisTile * tile );

    virtual void run();
    void stop() { m_stopped = true; };

    static void decompress( KisTile * tile );

private:

    KisTileCompressor( const KisTileCompressor & );

    static QByteArray compress(const QByteArray&);
    static void decompress(const QByteArray&, QByteArray&);

private:
    bool m_stopped;
    QQueue<KisTile*> m_tileQueue;
    QMutex m_queueLock;
};


#endif
