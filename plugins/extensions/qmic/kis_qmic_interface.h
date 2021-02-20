/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2020 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_IMAGE_INTERFACE_H
#define __KIS_IMAGE_INTERFACE_H

#include "kritaqmicinterface_export.h"

#include <QDebug>
#include <QMutex>
#include <QObject>
#include <QRect>
#include <QScopedPointer>
#include <QSharedPointer>
#include <QSize>
#include <QVector>

class KisQmicApplicator;
class KisViewManager;

struct KRITAQMICINTERFACE_EXPORT KisQMicImage {
    QMutex m_mutex;
    QString m_layerName;
    int m_width;
    int m_height;
    int m_spectrum;
    float* m_data;

    KisQMicImage(QString layerName, int width, int height, int spectrum = 4)
        : m_mutex()
        , m_layerName(layerName)
        , m_width(width)
        , m_height(height)
        , m_spectrum(spectrum)
        , m_data(new float[width * height * spectrum])
    {
    }

    ~KisQMicImage() {
        delete[] m_data;
    }

    const float* constData() const
    {
        return m_data;
    }

    size_t size() const
    {
        return m_width * m_height * m_spectrum * sizeof(float);
    }
};

QDebug operator<<(QDebug d, const KisQMicImage &model);

using KisQMicImageSP = QSharedPointer<KisQMicImage>;

class KRITAQMICINTERFACE_EXPORT KisImageInterface : public QObject
{
    Q_OBJECT

public:
    KisImageInterface(KisViewManager *parent = nullptr);
    ~KisImageInterface() override;

    QSize gmic_qt_get_image_size();
    QVector<KisQMicImageSP> gmic_qt_get_cropped_images(int mode, QRectF &cropRect);
    void gmic_qt_output_images(int mode, QVector<KisQMicImageSP> layers);
    void gmic_qt_detach();

private:
    struct Private;
    const QScopedPointer<Private> p;

private Q_SLOTS:
    void slotStartApplicator(QVector<KisQMicImageSP> gmicImages);
    void slotGmicFinished(bool successfully, int milliseconds, const QString &msg);
};

#endif
