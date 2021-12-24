/*
 * SPDX-FileCopyrightText: 2021 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef WGCOMMONCOLORSCALCULATIONRUNNER_H
#define WGCOMMONCOLORSCALCULATIONRUNNER_H

#include <kis_types.h>

#include <QImage>
#include <QObject>
#include <QRgb>
#include <QRunnable>
#include <QSharedPointer>

class KoColor;

class WGCommonColorsCalculationRunner : public QObject, public QRunnable
{
    Q_OBJECT
public:
    WGCommonColorsCalculationRunner(KisImageSP image, int numberOfColors,
                                       QSharedPointer<QVector<KoColor>> colorStore);


    void run() override;
    void extractColors();
    QList<QRgb> getColors();


Q_SIGNALS:
    void sigDone();

private:
    QImage m_imageData;
    int m_numColors;
    QSharedPointer<QVector<KoColor>> m_commonColors;
};

#endif // WGCOMMONCOLORSCALCULATIONRUNNER_H
