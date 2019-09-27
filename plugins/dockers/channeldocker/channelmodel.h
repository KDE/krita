/*
 *  Copyright (c) 2011 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or
 *  (at your option) any later version.
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
#include <QPointer>

#include <kis_canvas2.h>
#include <kis_types.h>

class KoColorSpace;


class ChannelModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    ChannelModel(QObject* parent = 0);
    ~ChannelModel() override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
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
    QPointer<KisCanvas2> m_canvas;
    QVector<QImage> m_thumbnails;
    QSize m_thumbnailSizeLimit;
    int m_oversampleRatio;
    int m_channelCount;
};


#endif // CHANNELMODEL_H
