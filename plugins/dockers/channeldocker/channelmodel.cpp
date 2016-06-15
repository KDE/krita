/*
 *  Copyright (c) 2011 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "channelmodel.h"
#include <QImage>
#include <KoColorSpace.h>
#include <KoChannelInfo.h>
//#include <kis_layer.h>
#include <kis_painter.h>

#include <kis_group_layer.h>
//#include <kis_paint_layer.h>
#include <kis_paint_device.h>
#include <kis_iterator_ng.h>
#include <kis_default_bounds.h>

#include <kis_canvas2.h>

ChannelModel::ChannelModel(QObject* parent):
    QAbstractTableModel(parent),
    m_canvas(nullptr), m_skipCount(1), m_oversampleRatio(4)
{
    setThumbnailSizeLimit(QSize(64,64));
}

ChannelModel::~ChannelModel()
{
}

QVariant ChannelModel::data(const QModelIndex& index, int role) const
{
    if (m_canvas && index.isValid()) {
        auto rootLayer = m_canvas->image()->rootLayer();
        auto cs = rootLayer->colorSpace();
        auto channels = cs->channels();

        int channelIndex = KoChannelInfo::displayPositionToChannelIndex(index.row(), channels);

        switch (role) {
        case Qt::DisplayRole:
        {
            if( index.column()==2 )
                return channels.at(channelIndex)->name();
            return QVariant();
        }
        case Qt::DecorationRole:
        {
            if( index.column()==1 ){
                Q_ASSERT(m_thumbnails.count()>index.row());
                return QVariant(m_thumbnails.at(index.row()));
            }
            return QVariant();
        }
        case Qt::CheckStateRole: {
            Q_ASSERT(index.row() < rowCount());
            Q_ASSERT(index.column() < columnCount());
            
            if (index.column() == 0) {
                QBitArray flags = rootLayer->channelFlags();
                return (flags.isEmpty() || flags.testBit(channelIndex)) ? Qt::Checked : Qt::Unchecked;
            }
            return QVariant();
        }
        }
    }
    return QVariant();
}

QVariant ChannelModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return QVariant();
}


int ChannelModel::rowCount(const QModelIndex& /*parent*/) const
{
    if (!m_canvas) return 0;

    return m_canvas->image() ? (m_canvas->image()->colorSpace()->channelCount()) : 0;
}

int ChannelModel::columnCount(const QModelIndex& /*parent*/) const
{
    if (!m_canvas) return 0;

    //columns are: checkbox, thumbnail, channel name
    return 3;
}


bool ChannelModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if( m_canvas && m_canvas->image() ){
        auto rootLayer = m_canvas->image()->rootLayer();
        auto cs = rootLayer->colorSpace();
        auto channels = cs->channels();
        Q_ASSERT(index.row() <= channels.count());

        int channelIndex = KoChannelInfo::displayPositionToChannelIndex(index.row(), channels);

        if (role == Qt::CheckStateRole) {
            QBitArray flags = cs->channelFlags(true,true);
            Q_ASSERT(!flags.isEmpty());
            if( flags.isEmpty())
                return false;

            //flags = flags.isEmpty() ? cs->channelFlags(true, true) : flags;
            flags.setBit(channelIndex, value.toInt() == Qt::Checked);
            rootLayer->setChannelFlags(flags);

            emit channelFlagsChanged();
            emit dataChanged(this->index(0,0),this->index(channels.count(),0));
            return true;
        }
    }
    return false;
}


//User double clicked on a row (but on channel checkbox)
//we select this channel, and deselect all other channels (except alpha, which we don't touch)
//this makes it fast to select single color channel
void ChannelModel::rowActivated(const QModelIndex &index)
{
    if( m_canvas && m_canvas->image() ){
        auto rootLayer = m_canvas->image()->rootLayer();
        auto cs = rootLayer->colorSpace();
        auto channels = cs->channels();
        Q_ASSERT(index.row() <= channels.count());

        int channelIndex = KoChannelInfo::displayPositionToChannelIndex(index.row(), channels);

        QBitArray flags = cs->channelFlags(true,true);
        Q_ASSERT(!flags.isEmpty());
        if(flags.isEmpty())
            return;

        for(int i=0; i<channels.count(); ++i){
            if(channels[i]->channelType() != KoChannelInfo::ALPHA){
                flags.setBit( i, (i==channelIndex) );
            }
        }


        rootLayer->setChannelFlags(flags);

        emit channelFlagsChanged();
        emit dataChanged(this->index(0,0),this->index(channels.count(),0));

    }
}


Qt::ItemFlags ChannelModel::flags(const QModelIndex& /*index*/) const
{
    Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
    return flags;
}

void ChannelModel::setThumbnailSizeLimit(QSize size)
{
    m_thumbnailSizeLimit = size;
    updateData(m_canvas);
}

void ChannelModel::slotSetCanvas(KisCanvas2 *canvas)
{
    if( m_canvas != canvas ){
        beginResetModel();
        m_canvas = canvas;
        if( m_canvas && m_canvas->image() ){
            updateThumbnails();
        }
        endResetModel();
    }
}

void ChannelModel::slotColorSpaceChanged(const KoColorSpace *colorSpace)
{
    Q_UNUSED(colorSpace);
    beginResetModel();
    updateThumbnails();
    endResetModel();
}

void ChannelModel::updateData( KisCanvas2 *canvas )
{
    beginResetModel();
    m_canvas = canvas;
    updateThumbnails();
    endResetModel();
}

//Create thumbnails from full image.
//Assumptions: thumbnail size is small compared to the original image and thumbnail quality
//doesn't need to be high, so we use fast but not very accurate algorithm.
void ChannelModel::updateThumbnails(void)
{

    if( m_canvas && m_canvas->image() ){
        auto cs = m_canvas->image()->colorSpace();
        auto channelCount = cs->channelCount();

        KisPaintDeviceSP dev = m_canvas->image()->projection();

        initThumbnailImages( m_canvas->image()->size(), channelCount );

        int width = m_thumbnails[0].width();
        int height = m_thumbnails[0].height();

        QVector<uchar*> thumbnailPointerCache(m_thumbnails.count());

        //step 1 - decimating the image to 4x the thumbnail size. We pick up pixels every skipCount.
        //This is inaccurate, but fast.
        KisHLineConstIteratorSP it = dev->createHLineConstIteratorNG(0,0,m_canvas->image()->width());
        for( int y=0; y<height; y++){
            for(int i=0; i<thumbnailPointerCache.count(); ++i){
                //.scanline() is expensive, cache values for reuse
                //because of alignment, can't use x+width*y, have to ask
                //qimage for first byte of the row.
                thumbnailPointerCache[i]=m_thumbnails[i].scanLine(y);
            }
            for( int x=0; x<width; x++){
                for( quint32 i=0; i<channelCount; ++i){
                    //channel thumbnails are grayscale 8 bit images.
                    quint8 pixel = cs->scaleToU8(it->rawDataConst(),i);
                    *(thumbnailPointerCache[i]+x)=pixel;
                }
                if(!it->nextPixels(m_skipCount))
                    break;
            }
            it->resetPixelPos(); //this is required. otherwise x coordinate of the iterator is not reset.
            for( int i=0; i<m_skipCount; it->nextRow(), ++i ); //since there is no nextRows(n), emulate it here.
        }

        //step 2. Smooth downsampling to the final thumbnail size. Slower, but image is small at this time.
        for(int i=0; i<m_thumbnails.count(); ++i){
            QSize sz = m_thumbnails[i].size();
            sz /= m_oversampleRatio;
            //for better quality we apply smooth transformation
            //speed is not an issue, because thumbnail image has been decimated and is small.
            m_thumbnails[i] = m_thumbnails[i].scaled( sz, Qt::KeepAspectRatio, Qt::SmoothTransformation );
        }
    }
}

//Compute thumbnail size.
//Preallocate qimage for thumbnail generation
void ChannelModel::initThumbnailImages(QSize size, int nChannels)
{
    m_thumbnails.clear();
    m_thumbnails.resize(nChannels);

    QSize thumbnailSizeOversample = m_thumbnailSizeLimit;

    //We allocate space for image that is oversampleRatio times larger than the end image
    thumbnailSizeOversample *= m_oversampleRatio;

    //compute decimation step size
    m_skipCount = static_cast<int>(std::max( size.width()/thumbnailSizeOversample.width(), size.height()/thumbnailSizeOversample.height() ));
    m_skipCount = std::max(m_skipCount, 1); //skip count of 1, means we are getting back the original image. Can't go lower.

    thumbnailSizeOversample = size;
    thumbnailSizeOversample /= m_skipCount;

    for(int i=0; i<nChannels; ++i){
        //channel thumbnails are grayscale images.
        m_thumbnails[i]=QImage(thumbnailSizeOversample,QImage::Format_Grayscale8);
    }
}


#include "moc_channelmodel.cpp"
