/*
 *  Copyright (c) 2010 Adam Celarek <kdedev at xibo dot at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_common_colors.h"
#include <cmath>
#include <QPixmap>
#include <QHash>
#include <QImage>
#include <QList>
#include <QPushButton>
#include <QColor>
#include <QPainter>
#include <QResizeEvent>

#include <KDebug>

#include <KConfig>
#include <KConfigGroup>
#include <KComponentData>
#include <KGlobal>

#include "KoColor.h"
#include "kis_canvas2.h"
#include "kis_image.h"


KisCommonColors::KisCommonColors(QWidget *parent) :
    KisColorPatches("commonColors", parent)
{
    setColors(extractColors());
//    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    
    QPushButton* reloadButton = new QPushButton();
    reloadButton->setIcon(KIcon("view-refresh"));
    connect(reloadButton, SIGNAL(clicked()), this, SLOT(recalculate()));
    
    QList<QWidget*> tmpList;
    tmpList.append(reloadButton);
    setAdditionalButtons(tmpList);
    updateSettings();
}

void KisCommonColors::setCanvas(KisCanvas2 *canvas)
{
    KisColorPatches::setCanvas(canvas);

    // todo:
    // make clean
    // make optional
    KConfigGroup cfg = KGlobal::config()->group("advancedColorSelector");
    if(cfg.readEntry("commonColorsAutoUpdate", false))
        connect(m_canvas->image(), SIGNAL(sigImageModified()), this, SLOT(recalculate()));
}

void KisCommonColors::recalculate()
{
    setColors(extractColors());
}

enum ColorAxis {RedAxis=0, GreenAxis, BlueAxis};

class Color
{
public:
    Color(QRgb rgb) : r(qRed(rgb)), g(qGreen(rgb)), b(qBlue(rgb)) {}
    unsigned char r;
    unsigned char g;
    unsigned char b;
    inline unsigned char operator[](ColorAxis i) const
    {
        if(i==RedAxis) return r;
        if(i==GreenAxis) return g;
        return b;
    }
};

class VBox
{
    QList<Color> m_colors;
public:
    VBox(QList<QRgb> rgbList)
    {
        QList<Color> colorList;
        for(int i=0; i<rgbList.size(); i++) {
            colorList.append(Color(rgbList.at(i)));
        }
        m_colors = colorList;
    }

    VBox(QList<Color> colorList) : m_colors(colorList) {}

    int population() const { return m_colors.size(); }

    VBox divide()
    {
        ColorAxis axis = biggestAxis();
        Q_ASSERT(axisSize(axis)>=3);

        unsigned char divpos = divPos(axis);
        QList<Color> newVBoxColors;
        for(int i=m_colors.size()-1; i>=0; i--) {
            Color c = m_colors.at(i);
            if(c[axis]>divpos) {
                m_colors.removeAt(i);
                newVBoxColors.append(c);
            }
        }

        return VBox(newVBoxColors);
    }

    QRgb mean() const
    {
        int r=0;
        int g=0;
        int b=0;
        for(int i=0;i<m_colors.size(); i++) {
            r+=(int) m_colors.at(i)[RedAxis];
            g+=(int) m_colors.at(i)[GreenAxis];
            b+=(int) m_colors.at(i)[BlueAxis];
        }
        int size = m_colors.size();
        Q_ASSERT(size>0);

        return qRgb(r/size, g/size, b/size);
    }

    unsigned char axisSize(ColorAxis axis) const
    {
        unsigned char valMin = 255;
        unsigned char valMax = 0;
        for(int i=0; i<m_colors.size(); i++) {
            if(m_colors.at(i)[axis]>valMax)
                valMax=m_colors.at(i)[axis];
            if(m_colors.at(i)[axis]<valMin)
                valMin=m_colors.at(i)[axis];
        }
        return valMax-valMin;
    }

    ColorAxis biggestAxis() const
    {
        unsigned char sR = axisSize(RedAxis);
        unsigned char sG = axisSize(GreenAxis);
        unsigned char sB = axisSize(BlueAxis);
        if(sR>sG && sR>sB) return RedAxis;
        if(sG>sR && sG>sB) return GreenAxis;
        return BlueAxis;
    }

private:
//    unsigned char divPos(ColorAxis axis) const
//    {
//        QList<unsigned char> values;
//        for(int i=0;i<m_colors.size(); i++) {
//            values.append(m_colors.at(i)[axis]);
//        }
//        qSort(values);
//        return values.at(values.size()*4/5);
//    }
    unsigned char divPos(ColorAxis axis) const
    {
        short min=m_colors.at(0)[axis];
        short max=m_colors.at(0)[axis];
        for(int i=0;i<m_colors.size(); i++) {
            if(min>m_colors.at(i)[axis]) min=m_colors.at(i)[axis];
            if(max<m_colors.at(i)[axis]) max=m_colors.at(i)[axis];
        }

        return (min+max)/2;
    }
};


QList<KoColor> KisCommonColors::extractColors()
{
    QList<QRgb> colors = getColors();

    VBox startBox(colors);
    QList<VBox> boxes;
    boxes.append(startBox);
    
    int numColors=patchCount();

    while (boxes.size()<numColors*3/5 && colors.size()>numColors*3/5) {
        int biggestBox=-1;
        int biggestBoxPopulation=-1;

        for(int i=0; i<boxes.size(); i++) {
            if(boxes.at(i).population()>biggestBoxPopulation &&
               boxes.at(i).axisSize(boxes.at(i).biggestAxis())>=3) {
                biggestBox=i;
                biggestBoxPopulation=boxes.at(i).population();
            }
        }

        if(biggestBox==-1 || boxes[biggestBox].population()<=3)
            break;
        VBox newBox = boxes[biggestBox].divide();
        boxes.append(newBox);
    }


    while (boxes.size()<numColors && colors.size()>numColors) {
        int biggestBox=-1;
        int biggestBoxAxisSize=-1;

        for(int i=0; i<boxes.size(); i++) {
            if(boxes.at(i).axisSize(boxes.at(i).biggestAxis())>biggestBoxAxisSize &&
               boxes.at(i).axisSize(boxes.at(i).biggestAxis())>=3) {
                biggestBox=i;
                biggestBoxAxisSize=boxes.at(i).axisSize(boxes.at(i).biggestAxis());
            }
        }

        if(biggestBox==-1 || boxes[biggestBox].population()<=3)
            break;
        VBox newBox = boxes[biggestBox].divide();
        boxes.append(newBox);
    }

    QList<KoColor> colorList;
    for(int i=0; i<boxes.size(); i++) {
        if(boxes.at(i).population()>=1) {
            colorList.append(KoColor(QColor(boxes.at(i).mean()), m_canvas->image()->colorSpace()));
//            if(colorList.last()==QColor(0,0,0))
//                kDebug()<<"dude!";
        }
    }

    return colorList;
}

QList<QRgb> KisCommonColors::getColors()
{
    if(m_canvas == 0) return QList<QRgb>();
    KisImageWSP kisImage = m_canvas->image();

    QImage qImage = kisImage->convertToQImage(0,0, kisImage->width(), kisImage->height(), kisImage->profile());

    QPixmap pixmap = QPixmap::fromImage(qImage);

    int width = pixmap.width();
    int height = pixmap.height();

    int pixelCount = height*width;
    if(pixelCount> (1<<16)) {
        qreal factor = sqrt((1<<16)/(qreal) pixelCount);
        pixmap=pixmap.scaledToWidth(width*factor);
        width=pixmap.width();
        height=pixmap.height();
    }

    QImage image = pixmap.toImage();
    QSet<QRgb> colorList;

    for (int i=0; i<width; i++) {
        for (int j=0; j<height; j++) {
            colorList.insert(image.pixel(i, j)|qRgba(0,0,0,255));
        }
    }

    return colorList.toList();
}
