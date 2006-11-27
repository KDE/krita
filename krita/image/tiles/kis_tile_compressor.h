#ifndef KIS_TILE_COMPRESSOR_H
#define KIS_TILE_COMPRESSOR_H

#include <QQueue>
#include <QByteArray>
#include <QThread>

class KisTileCompressor : QThread
{

public:

    KisTileCompressor();
    ~KisTileCompressor();

    void enqueue( KisTile * tile );
    virtual void run();

private:

    KisTileCompressor( const KisTileCompressor & );

    static QByteArray compress(const QByteArray&);
    static void decompress(const QByteArray&, QByteArray&);

private:

    QQueue m_tileQueue;
};


#endif
