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

#ifndef CHANNELMODEL_H
#define CHANNELMODEL_H

#include <QModelIndex>
#include <QSize>
#include <kis_types.h>

class KoColorSpace;
class KisCanvas2;

class ChannelModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    ChannelModel(QObject* parent = 0);
    virtual ~ChannelModel();
    
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
    virtual Qt::ItemFlags flags(const QModelIndex& index) const;

    //set maximum size of the thumbnail image. This should be set based on screen resolution, etc.
    virtual void setThumbnailSizeLimit(QSize size);

public Q_SLOTS:
    void slotSetCanvas(KisCanvas2* canvas);
    void slotColorSpaceChanged(const KoColorSpace *colorSpace);
    void updateData( KisCanvas2 *canvas );
    void rowActivated(const QModelIndex &index);

Q_SIGNALS:
    void channelFlagsChanged();

private:
    void updateThumbnails( );
    void initThumbnailImages(QSize size, int nChannels );

private:
    KisCanvas2* m_canvas;
    QVector<QImage> m_thumbnails;
    QSize m_thumbnailSizeLimit;
    int m_skipCount;
    int m_oversampleRatio;
};


#endif // CHANNELMODEL_H
