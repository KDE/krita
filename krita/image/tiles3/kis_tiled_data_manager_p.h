/*
 *  Copyright (c) 2004 Casper Boemann <cbr@boemann.dk>
 *            (c) 2009 Dmitry  Kazakov <dimula73@gmail.com>
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



/* FIXME: Think over SSE here */
void KisTiledDataManager::writeBytesBody(const quint8 *data,
        qint32 x, qint32 y,
        qint32 width, qint32 height)
{
    if (!data) return;

    width  = width < 0  ? 0 : width;
    height = height < 0 ? 0 : height;

    qint32 dataY = 0;
    qint32 imageY = y;
    qint32 rowsRemaining = height;
    const qint32 pixelSize = this->pixelSize();

    while (rowsRemaining > 0) {

        qint32 dataX = 0;
        qint32 imageX = x;
        qint32 columnsRemaining = width;
        qint32 numContiguousImageRows = numContiguousRows(imageY, imageX,
                                        imageX + width - 1);

        qint32 rowsToWork = qMin(numContiguousImageRows, rowsRemaining);

        while (columnsRemaining > 0) {

            qint32 numContiguousImageColumns =
                numContiguousColumns(imageX, imageY,
                                     imageY + rowsToWork - 1);

            qint32 columnsToWork = qMin(numContiguousImageColumns,
                                        columnsRemaining);

            KisTileDataWrapper tw = pixelPtr(imageX, imageY,
                                             KisTileDataWrapper::WRITE);
            quint8 *tileIt = tw.data();


            const qint32 tileRowStride = rowStride(imageX, imageY);

            const quint8 *dataIt = data +
                                   ((dataX + (dataY * width)) * pixelSize);

            const qint32 dataRowStride = width * pixelSize;

            const qint32 lineSize = columnsToWork * pixelSize;

            for (qint32 row = 0; row < rowsToWork; row++) {
                memcpy(tileIt, dataIt, lineSize);
                tileIt += tileRowStride;
                dataIt += dataRowStride;
            }

            imageX += columnsToWork;
            dataX += columnsToWork;
            columnsRemaining -= columnsToWork;
        }

        imageY += rowsToWork;
        dataY += rowsToWork;
        rowsRemaining -= rowsToWork;
    }
}


void KisTiledDataManager::readBytesBody(quint8 *data,
                                        qint32 x, qint32 y,
                                        qint32 width, qint32 height) const
{
    if (!data) return;

    width  = width < 0  ? 0 : width;
    height = height < 0 ? 0 : height;

    qint32 dataY = 0;
    qint32 imageY = y;
    qint32 rowsRemaining = height;
    const qint32 pixelSize = this->pixelSize();

    while (rowsRemaining > 0) {

        qint32 dataX = 0;
        qint32 imageX = x;
        qint32 columnsRemaining = width;
        qint32 numContiguousImageRows = numContiguousRows(imageY, imageX,
                                        imageX + width - 1);

        qint32 rowsToWork = qMin(numContiguousImageRows, rowsRemaining);

        while (columnsRemaining > 0) {

            qint32 numContiguousImageColumns = numContiguousColumns(imageX, imageY,
                                               imageY + rowsToWork - 1);

            qint32 columnsToWork = qMin(numContiguousImageColumns,
                                        columnsRemaining);

            // XXX: Ugly const cast because of theold pixelPtr design copied from tiles1.
            KisTileDataWrapper tw = const_cast<KisTiledDataManager*>(this)->pixelPtr(imageX, imageY, KisTileDataWrapper::READ);
            quint8 *tileIt = tw.data();


            const qint32 tileRowStride = rowStride(imageX, imageY);

            quint8 *dataIt = data +
                             ((dataX + (dataY * width)) * pixelSize);

            const qint32 dataRowStride = width * pixelSize;

            const qint32 lineSize = columnsToWork * pixelSize;

            for (qint32 row = 0; row < rowsToWork; row++) {
                memcpy(dataIt, tileIt, lineSize);
                tileIt += tileRowStride;
                dataIt += dataRowStride;
            }

            imageX += columnsToWork;
            dataX += columnsToWork;
            columnsRemaining -= columnsToWork;
        }

        imageY += rowsToWork;
        dataY += rowsToWork;
        rowsRemaining -= rowsToWork;
    }
}


#define forEachChannel(_idx, _channelSize)                              \
    for(qint32 _idx=0, _channelSize=channelSizes[_idx];         \
            _idx<numChannels && (_channelSize=channelSizes[_idx], 1);   \
            _idx++)

void KisTiledDataManager::writePlanarBytesBody(QVector </*const*/ quint8* > planes,
        QVector<qint32> channelSizes,
        qint32 x, qint32 y,
        qint32 width, qint32 height)
{
    Q_ASSERT(planes.size() == channelSizes.size());
    Q_ASSERT(planes.size() > 0);

    width  = width < 0  ? 0 : width;
    height = height < 0 ? 0 : height;

    const qint32 numChannels = planes.size();
    const qint32 pixelSize = this->pixelSize();

    qint32 dataY = 0;
    qint32 imageY = y;
    qint32 rowsRemaining = height;

    while (rowsRemaining > 0) {

        qint32 dataX = 0;
        qint32 imageX = x;
        qint32 columnsRemaining = width;
        qint32 numContiguousImageRows = numContiguousRows(imageY, imageX,
                                        imageX + width - 1);

        qint32 rowsToWork = qMin(numContiguousImageRows, rowsRemaining);

        while (columnsRemaining > 0) {

            qint32 numContiguousImageColumns =
                numContiguousColumns(imageX, imageY,
                                     imageY + rowsToWork - 1);
            qint32 columnsToWork = qMin(numContiguousImageColumns,
                                        columnsRemaining);

            const qint32 dataIdx = dataX + dataY * width;
            const qint32 tileRowStride = rowStride(imageX, imageY) -
                                         columnsToWork * pixelSize;

            KisTileDataWrapper tw = pixelPtr(imageX, imageY,
                                             KisTileDataWrapper::WRITE);
            quint8 *tileItStart = tw.data();


            forEachChannel(i, channelSize) {
                const quint8* planeIt = planes[i] + dataIdx * channelSize;
                qint32 dataStride = (width - columnsToWork) * channelSize;
                quint8* tileIt = tileItStart;

                for (qint32 row = 0; row < rowsToWork; row++) {
                    for (int col = 0; col < columnsToWork; col++) {
                        memcpy(tileIt, planeIt, channelSize);
                        tileIt += pixelSize;
                        planeIt += channelSize;
                    }

                    tileIt += tileRowStride;
                    planeIt += dataStride;
                }
                tileItStart += channelSize;
            }

            imageX += columnsToWork;
            dataX += columnsToWork;
            columnsRemaining -= columnsToWork;
        }


        imageY += rowsToWork;
        dataY += rowsToWork;
        rowsRemaining -= rowsToWork;
    }
}

QVector<quint8*> KisTiledDataManager::
readPlanarBytesBody(QVector<qint32> channelSizes,
                    qint32 x, qint32 y,
                    qint32 width, qint32 height)
{
    Q_ASSERT(channelSizes.size() > 0);

    width  = width < 0  ? 0 : width;
    height = height < 0 ? 0 : height;

    const qint32 numChannels = channelSizes.size();
    const qint32 pixelSize = this->pixelSize();

    QVector<quint8*> planes;
    forEachChannel(i, channelSize) {
        planes.append(new quint8[width * height * channelSize]);
    }

    qint32 dataY = 0;
    qint32 imageY = y;
    qint32 rowsRemaining = height;

    while (rowsRemaining > 0) {

        qint32 dataX = 0;
        qint32 imageX = x;
        qint32 columnsRemaining = width;
        qint32 numContiguousImageRows = numContiguousRows(imageY, imageX,
                                        imageX + width - 1);

        qint32 rowsToWork = qMin(numContiguousImageRows, rowsRemaining);

        while (columnsRemaining > 0) {

            qint32 numContiguousImageColumns =
                numContiguousColumns(imageX, imageY,
                                     imageY + rowsToWork - 1);
            qint32 columnsToWork = qMin(numContiguousImageColumns,
                                        columnsRemaining);

            const qint32 dataIdx = dataX + dataY * width;
            const qint32 tileRowStride = rowStride(imageX, imageY) -
                                         columnsToWork * pixelSize;

            KisTileDataWrapper tw = pixelPtr(imageX, imageY,
                                             KisTileDataWrapper::READ);
            quint8 *tileItStart = tw.data();


            forEachChannel(i, channelSize) {
                quint8* planeIt = planes[i] + dataIdx * channelSize;
                qint32 dataStride = (width - columnsToWork) * channelSize;
                quint8* tileIt = tileItStart;

                for (qint32 row = 0; row < rowsToWork; row++) {
                    for (int col = 0; col < columnsToWork; col++) {
                        memcpy(planeIt, tileIt, channelSize);
                        tileIt += pixelSize;
                        planeIt += channelSize;
                    }

                    tileIt += tileRowStride;
                    planeIt += dataStride;
                }
                tileItStart += channelSize;
            }

            imageX += columnsToWork;
            dataX += columnsToWork;
            columnsRemaining -= columnsToWork;
        }


        imageY += rowsToWork;
        dataY += rowsToWork;
        rowsRemaining -= rowsToWork;
    }
    return planes;
}




