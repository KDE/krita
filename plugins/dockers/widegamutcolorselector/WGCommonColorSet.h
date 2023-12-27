/*
 * SPDX-FileCopyrightText: 2021 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef WGCOMMONCOLORSET_H
#define WGCOMMONCOLORSET_H

#include <KisUniqueColorSet.h>
#include <kis_image.h>

#include <QTimer>
#include <QVector>

class WGCommonColorSet: public KisUniqueColorSet
{
    Q_OBJECT
public:
    explicit WGCommonColorSet(QObject *parent = 0);
    void setImage(KisImageSP image);
    void setAutoUpdate(bool enabled);
    void setColorCount(int count) { m_numColors = count; }
public Q_SLOTS:
    void slotUpdateColors();
private Q_SLOTS:
    void slotCalculationDone();
Q_SIGNALS:
    void sigIdle(bool isIdle);
private:
    QTimer m_updateTimer;
    QSharedPointer<QVector<KoColor>> m_commonColors;
    KisImageSP m_image;
    int m_numColors {10};
    bool m_idle {true};
    bool m_autoUpdate {false};
};

#endif // WGCOMMONCOLORSET_H
