/*
 *  SPDX-FileCopyrightText: 2014 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2021 Alvin Wong <alvin@alvinhc.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_SPLASH_SCREEN_H
#define KIS_SPLASH_SCREEN_H

#include <QWidget>
#include <QTimer>

class QPixmap;
class QSvgWidget;
class QLabel;

#include "kritaui_export.h"

class KRITAUI_EXPORT KisSplashScreen : public QWidget
{
    Q_OBJECT
public:
    struct Source {
        QString resourcePath;
        QString artistCredit;
    };

    explicit KisSplashScreen(int height, QWidget *parent = 0,
        Qt::WindowFlags f = Qt::WindowFlags(Qt::SplashScreen | Qt::FramelessWindowHint));

    void repaint();

    void show();

    void setLoadingText(QString text);

    static Source getImageSource();

private:
    void updateSplashImage(const int height);

private:

    QTimer m_timer;
    bool m_displayLinks { false };
    QLabel* m_lblSplash;
    QSvgWidget *m_brandingSvg;
    QSvgWidget *m_bannerSvg;
    QLabel *m_loadingTextLabel;
    QLabel *m_artCreditsLabel;
    QString m_versionHtml;
};

#endif // KIS_SPLASH_SCREEN_H
