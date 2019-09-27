/*
 *  Copyright (c) 2014 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KIS_SPLASH_SCREEN_H
#define KIS_SPLASH_SCREEN_H

#include <QWidget>
#include <QTimer>

#include "ui_wdgsplash.h"

class QPixmap;

#include "kritaui_export.h"

class KRITAUI_EXPORT KisSplashScreen : public QWidget, public Ui::WdgSplash
{
    Q_OBJECT
public:
    explicit KisSplashScreen(const QString &m_version, const QPixmap &m_pixmap, const QPixmap &pixmap_x2, bool themed = false, QWidget *parent = 0, Qt::WindowFlags f = 0);

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

private:

    QTimer m_timer;
    bool m_themed;
    QImage m_splashImage;
    int m_textTop;
    qreal m_scaleFactor;
};

#endif // KIS_SPLASH_SCREEN_H
