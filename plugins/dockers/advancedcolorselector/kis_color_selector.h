/*
 *  Copyright (c) 2010 Adam Celarek <kdedev at xibo dot at>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_COLOR_SELECTOR_H
#define KIS_COLOR_SELECTOR_H

#include "kis_color_selector_base.h"

class KisColorSelectorRing;
class KisColorSelectorComponent;
class KisColorSelectorSimple;
class KisColorSelectorWheel;
class QPushButton;
class KisSignalCompressor;

class KisColorSelector : public KisColorSelectorBase
{
    Q_OBJECT
public:
    enum Type {Ring, Square, Wheel, Triangle, Slider};
    enum Parameters {H, hsvS, V, hslS, L, SL, SV, SV2, hsvSH, hslSH, VH, LH, SI, SY, hsiSH, hsySH, I, Y, IH, YH, hsiS, hsyS};
    struct Configuration {
        Type mainType;
        Type subType;
        Parameters mainTypeParameter;
        Parameters subTypeParameter;
        Configuration(Type mainT = Triangle,
                              Type subT = Ring,
                              Parameters mainTP = SL,
                              Parameters subTP = H)
                                  : mainType(mainT),
                                  subType(subT),
                                  mainTypeParameter(mainTP),
                                  subTypeParameter(subTP)
        {}
        Configuration(QString string)
        {
            readString(string);
        }

        QString toString() const
        {
            return QString("%1|%2|%3|%4").arg(mainType).arg(subType).arg(mainTypeParameter).arg(subTypeParameter);
        }
        void readString(QString string)
        {
            QStringList strili = string.split('|');
            if(strili.length()!=4) return;

            int imt=strili.at(0).toInt();
            int ist=strili.at(1).toInt();
            int imtp=strili.at(2).toInt();
            int istp=strili.at(3).toInt();

            if(imt>Slider || ist>Slider || imtp>hsyS || istp>hsyS)//this was LH before
                return;

            mainType = Type(imt);
            subType = Type(ist);
            mainTypeParameter = Parameters(imtp);
            subTypeParameter = Parameters(istp);
        }
        static Configuration fromString(QString string)
        {
            Configuration ret;
            ret.readString(string);
            return ret;
        }
    };

//    enum MainType {Ring, Square, Wheel};
//    enum SubType {Triangle, Square, Slider};
//    enum MainTypeParameter {SL, SV, SH, VH, LH, VSV/*experimental*/, SI, SY, YH, IH};
//    enum SubTypeParameter {H, S, V, L, I, Y, hsiS, hsyS};

    KisColorSelector(Configuration conf, QWidget* parent = 0);
    KisColorSelector(QWidget* parent=0);
    KisColorSelectorBase* createPopup() const;

    void setConfiguration(Configuration conf);
    Configuration configuration() const;
    void setColor(const KoColor &color);

public Q_SLOTS:
    void reset();
    void updateSettings();

Q_SIGNALS:
    void settingsButtonClicked();

protected:
    void paintEvent(QPaintEvent*);
    void resizeEvent(QResizeEvent*);
    void mousePressEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    bool displaySettingsButton();


private:
    void mouseEvent(QMouseEvent* e);
    void init();

    KisColorSelectorRing* m_ring;
    KisColorSelectorComponent* m_triangle;
    KisColorSelectorSimple* m_slider;
    KisColorSelectorSimple* m_square;
    KisColorSelectorWheel* m_wheel;
    QPushButton* m_button;
    KisColorSelectorComponent* m_mainComponent;
    KisColorSelectorComponent* m_subComponent;
    KisColorSelectorComponent* m_grabbingComponent;

    KisSignalCompressor *m_signalCompressor;

    Configuration m_configuration;

    KoColor m_lastRealColor;
    KoColor m_currentRealColor;
    bool m_blipDisplay;
    Acs::ColorRole m_lastColorRole;


public:
    void setDisplayBlip(bool disp) {m_blipDisplay = disp;}
    bool displayBlip() const {return m_blipDisplay;}
};

#endif // KIS_COLSELNG_COLOR_SELECTOR_H
