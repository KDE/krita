/* 
 * Copyright (C) 2015 by Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * Based on the Digikam CIE Tongue widget
 * Copyright (C) 2006-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * Any source code are inspired from lprof project and
 * Copyright (C) 1998-2001 Marti Maria
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 **/
 
#ifndef KIS_CIETONGUEWIDGET_H
#define KIS_CIETONGUEWIDGET_H
 
// Qt includes
 
#include <QWidget>
#include <QColor>
#include <QPaintEvent>
 
// KDE includes

#include <KoColor.h>
#include <KoColorSpace.h>
  
#include <kritaui_export.h>
 
class KRITAUI_EXPORT KisCIETongueWidget : public QWidget
{
    Q_OBJECT
 
public:
 
    KisCIETongueWidget(QWidget *parent=0);
    ~KisCIETongueWidget() override;
 
    //this expects a qvector <double> (9), qvector <double> (3) and whether or not there's profile data?;
    void setProfileData(QVector <double> p, QVector <double> w, bool profileData = false);
    void setGamut(QPolygonF gamut);
    void setRGBData(QVector <double> whitepoint, QVector <double> colorants);
    void setCMYKData(QVector <double> whitepoint);
    void setXYZData(QVector <double> whitepoint);
    void setGrayData(QVector <double> whitepoint);
    void setLABData(QVector <double> whitepoint);
    void setYCbCrData(QVector <double> whitepoint);
    void setProfileDataAvailable(bool dataAvailable);
 
    void loadingStarted();
    void loadingFailed();
    void uncalibratedColor();
    
    enum model {RGBA, CMYKA, XYZA, LABA, GRAYA, YCbCrA};
 
protected:
 
    int  grids(double val) const;
 
    void outlineTongue();
    void fillTongue();
    void drawTongueAxis();
    void drawTongueGrid();
    void drawLabels();
 
    QRgb colorByCoord(double x, double y);
    void drawSmallEllipse(QPointF xy, int r, int g, int b, int sz);
 
    void resizeEvent(QResizeEvent* event) override;
    void paintEvent(QPaintEvent*) override;
 
private:
 
    void drawColorantTriangle();
    void drawGamut();
    void drawWhitePoint();
    void drawPatches();
    void updatePixmap();
 
    void mapPoint(int& icx, int& icy, QPointF xy);
    void biasedLine(int x1, int y1, int x2, int y2);
    void biasedText(int x, int y, const QString& txt);
 
private Q_SLOTS:
 
    void slotProgressTimerDone();
 
private :
 
    class Private;
    Private* const d;
};
 
#endif /* KISCIETONGUEWIDGET_H */
