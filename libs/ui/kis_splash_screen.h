/*
 *  SPDX-FileCopyrightText: 2014 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_SPLASH_SCREEN_H
#define KIS_SPLASH_SCREEN_H

#include <QWidget>
#include <QTimer>

#include "ui_wdgsplash.h"

class QPixmap;
class QSvgWidget;

#include "kritaui_export.h"

class KRITAUI_EXPORT KisSplashScreen : public QWidget, public Ui::WdgSplash
{
    Q_OBJECT
public:
    explicit KisSplashScreen(bool themed = false, QWidget *parent = 0, Qt::WindowFlags f = Qt::WindowFlags());

    void repaint();

    void show();
    void displayLinks(bool show);
    void displayRecentFiles(bool show);

    void setLoadingText(QString text);

private Q_SLOTS:

    void toggleShowAtStartup(bool toggle);
    void linkClicked(const QString &link);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void updateText();
    QString colorString() const;
    void updateSplashImage();

private:

    QTimer m_timer;
    bool m_themed;
    bool m_displayLinks { false };
    QSvgWidget *m_brandingSvg;
    QSvgWidget *m_bannerSvg;
    QLabel *m_loadingTextLabel;
    QLabel *m_artCreditsLabel;
    QString m_versionHtml;
};

#endif // KIS_SPLASH_SCREEN_H
