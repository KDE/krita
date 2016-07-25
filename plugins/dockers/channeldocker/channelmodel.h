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

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
    Qt::ItemFlags flags(const QModelIndex& index) const;
    void unsetCanvas( void );

    //set maximum size of the thumbnail image. This should be set based on screen resolution, etc.
    void setThumbnailSizeLimit(QSize size);

public Q_SLOTS:
    void slotSetCanvas(KisCanvas2* canvas);
    void slotColorSpaceChanged(const KoColorSpace *colorSpace);
    void updateData(KisCanvas2 *canvas);
    void rowActivated(const QModelIndex &index);

Q_SIGNALS:
    void channelFlagsChanged();

private:
    void updateThumbnails();

private:
    KisCanvas2* m_canvas;
    QVector<QImage> m_thumbnails;
    QSize m_thumbnailSizeLimit;
    int m_oversampleRatio;
    int m_channelCount;
};


#endif // CHANNELMODEL_H
