/*
 *  SPDX-FileCopyrightText: 2011 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
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
