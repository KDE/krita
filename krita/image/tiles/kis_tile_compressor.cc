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

#include "kis_tile_compressor.h"

#include "kis_tile.h"
#include "kis_tilemanager.h"

#include "kis_debug.h"

#define HASH_LOG  12
#define HASH_SIZE (1<< HASH_LOG)
#define HASH_MASK  (HASH_SIZE-1)

#define UPDATE_HASH(v,p) { v = *((quint16*)p); v ^= *((quint16*)(p+1))^(v>>(16-HASH_LOG)); }

#define MAX_COPY       32
#define MAX_LEN       264  /* 256 + 8 */
#define MAX_DISTANCE 8192

// Lossless compression using LZF algorithm, this is faster on modern CPU than
// the original implementation in http://liblzf.plan9.de/
int lzff_compress(const void* input, int length, void* output, int /*maxout*/)
{
    const quint8* ip = (const quint8*) input;
    const quint8* ip_limit = ip + length - MAX_COPY - 4;
    quint8* op = (quint8*) output;

    const quint8* htab[HASH_SIZE];
    const quint8** hslot;
    quint32 hval;

    quint8* ref;
    qint32 copy;
    qint32 len;
    qint32 distance;
    quint8* anchor;

    /* initializes hash table */
    for (hslot = htab; hslot < htab + HASH_SIZE; hslot++)
        *hslot = ip;

    /* we start with literal copy */
    copy = 0;
    *op++ = MAX_COPY - 1;

    /* main loop */
    while (ip < ip_limit) {
        /* find potential match */
        UPDATE_HASH(hval, ip);
        hslot = htab + (hval & HASH_MASK);
        ref = (quint8*) * hslot;

        /* update hash table */
        *hslot = ip;

        /* find itself? then it's no match */
        if (ip == ref)
            goto literal;

        /* is this a match? check the first 2 bytes */
        if (*((quint16*)ref) != *((quint16*)ip))
            goto literal;

        /* now check the 3rd byte */
        if (ref[2] != ip[2])
            goto literal;

        /* calculate distance to the match */
        distance = ip - ref;

        /* skip if too far away */
        if (distance >= MAX_DISTANCE)
            goto literal;

        /* here we have 3-byte matches */
        anchor = (quint8*)ip;
        len = 3;
        ref += 3;
        ip += 3;

        /* now we have to check how long the match is */
        if (ip < ip_limit - MAX_LEN) {
            while (len < MAX_LEN - 8) {
                /* unroll 8 times */
                if (*ref++ != *ip++) break;
                if (*ref++ != *ip++) break;
                if (*ref++ != *ip++) break;
                if (*ref++ != *ip++) break;
                if (*ref++ != *ip++) break;
                if (*ref++ != *ip++) break;
                if (*ref++ != *ip++) break;
                if (*ref++ != *ip++) break;
                len += 8;
            }
            ip--;
        }
        len = ip - anchor;

        /* just before the last non-matching byte */
        ip = anchor + len;

        /* if we have copied something, adjust the copy count */
        if (copy) {
            /* copy is biased, '0' means 1 byte copy */
            anchor = anchor - copy - 1;
            *(op - copy - 1) = copy - 1;
            copy = 0;
        } else
            /* back, to overwrite the copy count */
            op--;

        /* length is biased, '1' means a match of 3 bytes */
        len -= 2;

        /* distance is also biased */
        distance--;

        /* encode the match */
        if (len < 7)
            *op++ = (len << 5) + (distance >> 8);
        else {
            *op++ = (7 << 5) + (distance >> 8);
            *op++ = len - 7;
        }
        *op++ = (distance & 255);

        /* assuming next will be literal copy */
        *op++ = MAX_COPY - 1;

        /* update the hash at match boundary */
        --ip;
        UPDATE_HASH(hval, ip);
        htab[hval & HASH_MASK] = ip;
        ip++;

        continue;

    literal:
        *op++ = *ip++;
        copy++;
        if (copy >= MAX_COPY) {
            copy = 0;
            *op++ = MAX_COPY - 1;
        }
    }

    /* left-over as literal copy */
    ip_limit = (const quint8*)input + length;
    while (ip < ip_limit) {
        *op++ = *ip++;
        copy++;
        if (copy == MAX_COPY) {
            copy = 0;
            *op++ = MAX_COPY - 1;
        }
    }

    /* if we have copied something, adjust the copy length */
    if (copy)
        *(op - copy - 1) = copy - 1;
    else
        op--;

    return op - (quint8*)output;
}

int lzff_decompress(const void* input, int length, void* output, int maxout)
{
    const quint8* ip = (const quint8*) input;
    const quint8* ip_limit  = ip + length - 1;
    quint8* op = (quint8*) output;
    quint8* op_limit = op + maxout;
    quint8* ref;

    while (ip < ip_limit) {
        quint32 ctrl = (*ip) + 1;
        quint32 ofs = ((*ip) & 31) << 8;
        quint32 len = (*ip++) >> 5;

        if (ctrl < 33) {
            /* literal copy */
            if (op + ctrl > op_limit)
                return 0;

            /* crazy unrolling */
            if (ctrl) {
                *op++ = *ip++;
                ctrl--;

                if (ctrl) {
                    *op++ = *ip++;
                    ctrl--;

                    if (ctrl) {
                        *op++ = *ip++;
                        ctrl--;

                        for (; ctrl; ctrl--)
                            *op++ = *ip++;
                    }
                }
            }
        } else {
            /* back reference */
            len--;
            ref = op - ofs;
            ref--;

            if (len == 7 - 1)
                len += *ip++;

            ref -= *ip++;

            if (op + len + 3 > op_limit)
                return 0;

            if (ref < (quint8 *)output)
                return 0;

            *op++ = *ref++;
            *op++ = *ref++;
            *op++ = *ref++;
            if (len)
                for (; len; --len)
                    *op++ = *ref++;
        }
    }

    return op - (quint8*)output;
}


KisTileCompressor::KisTileCompressor()
        : QThread(0)
        , m_stopped(false)
{
}

KisTileCompressor::~KisTileCompressor()
{
}

void KisTileCompressor::enqueue(KisTile * tile)
{
    dbgImage << "Enqueueing tile" << tile << " for compression";
    tile->setTileState(QUEUED);
    m_tileQueue.enqueue(tile);
}

void KisTileCompressor::dequeue(KisTile * tile)
{
    m_queueLock.lock();
    dbgImage << "Dequeueing tile" << tile << " for compression";
    if (int i =  m_tileQueue.indexOf(tile) > -1)
        m_tileQueue.removeAt(i);
    m_queueLock.unlock();
}

void KisTileCompressor::run()
{
    while (!m_stopped) {
        KisTile * tile = 0;
        if (m_queueLock.tryLock() && !m_tileQueue.isEmpty()) {
            tile = m_tileQueue.dequeue();
            // XXX: hard lock this tile!
            m_queueLock.unlock();
        }
        if (tile && tile->tileState() == QUEUED) {
            dbgImage << "Going to compress tile" << tile << ", state:" << tile->tileState();
            tile->setTileState(COMPRESSED);
            KisTileManager::instance()->maySwapTile(tile);
        }
        sleep(1);   // Sleep a second
    }
}

void KisTileCompressor::decompress(KisTile * tile)
{
    dbgImage << "Decompressing tile" << tile;
    tile->setTileState(UNCOMPRESSED);
}

QByteArray KisTileCompressor::compress(const QByteArray& input)
{
    const void* const in_data = (const void*) input.constData();
    unsigned int in_len = (unsigned int)input.size();

    QByteArray output;
    output.resize(in_len + 4 + 1);

    // we use 4 bytes to store uncompressed length
    // and 1 extra byte as flag (0=uncompressed, 1=compressed)
    output[0] = in_len & 255;
    output[1] = (in_len >> 8) & 255;
    output[2] = (in_len >> 16) & 255;
    output[3] = (in_len >> 24) & 255;
    output[4] = 1;

    unsigned int out_len = in_len - 1;
    unsigned char* out_data = (unsigned char*) output.data() + 5;

    unsigned int len = lzff_compress(in_data, in_len, out_data, out_len);
    out_len = len;

    if ((len > out_len) || (len == 0)) {
        // output buffer is too small, likely because the data can't
        // be compressed. so here just copy without compression
        out_len = in_len;
        output.insert(5, input);

        // flag must indicate "uncompressed block"
        output[4] = 0;
    }

    // minimize memory
    output.resize(out_len + 4 + 1);
    output.squeeze();

    return output;
}

// will not squeeze output
void KisTileCompressor::decompress(const QByteArray& input, QByteArray& output)
{
    // read out first how big is the uncompressed size
    unsigned int unpack_size = 0;
    unpack_size |= ((quint8)input[0]);
    unpack_size |= ((quint8)input[1]) << 8;
    unpack_size |= ((quint8)input[2]) << 16;
    unpack_size |= ((quint8)input[3]) << 24;

    // prepare the output
    output.reserve(unpack_size);

    // compression flag
    quint8 flag = (quint8)input[4];

    // prepare for lzf
    const void* const in_data = (const void*)(input.constData() + 5);
    unsigned int in_len = (unsigned int)input.size() - 5;
    unsigned char* out_data = (unsigned char*) output.data();
    unsigned int out_len = (unsigned int)unpack_size;

    if (flag == 0)
        memcpy(output.data(), in_data, in_len);
    else {
        unsigned int len = lzff_decompress(in_data, in_len, out_data, out_len);
        output.resize(len);
        output.squeeze();
    }
}
