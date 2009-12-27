/*
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KIS_EXPERIMENT_SHAPE_OPTION_H
#define KIS_EXPERIMENT_SHAPE_OPTION_H

#include <kis_paintop_option.h>
#include <QImage>

class KisShapeOptionsWidget;

class KisExperimentShapeOption : public KisPaintOpOption
{

    Q_OBJECT

public:
    KisExperimentShapeOption();
    ~KisExperimentShapeOption();

    /// 0 - ellipse, 1 - rectangle, 2 - anti-aliased pixel, 2 - pixel
    int shape() const;
    
    /// size settings
    bool proportional() const;
    int width() const;
    int height() const;
    
    /// random size 
    bool jitterShapeSize() const;

    bool fixedRotation() const;
    int fixedAngle() const;
   
    bool randomRotation() const;
    qreal randomRotationWeight() const;
    
    bool followCursor() const;
    qreal followCursorWeigth() const;
    
    QImage image() const { return m_image; }

    /// TODO: serialization 
    void writeOptionSetting(KisPropertiesConfiguration* setting) const;
    void readOptionSetting(const KisPropertiesConfiguration* setting);
private:
    KisShapeOptionsWidget * m_options;
    QImage m_image;
    bool m_useAspect;
    qreal m_aspect;
    
    int m_maxSize;
private:
    void setupBrushPreviewSignals();
    void computeAspect();

private slots:
            void randomValueChanged(int value);
            void followValueChanged(int value);
            void prepareImage();
            void aspectToggled(bool toggled);
            void updateHeight(int value);
            void updateWidth(int value);
            
            void changeSizeUI(bool proportionalSize);
};

#endif // KIS_EXPERIMENT_SHAPE_OPTION_H

