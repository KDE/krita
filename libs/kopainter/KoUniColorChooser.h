/**
 * Copyright (c) 2006 Casper Boemann (cbr@boemann.dk)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/
#ifndef KOUNICOLORCHOOSER_H
#define KOUNICOLORCHOOSER_H

#include "qwidget.h"
#include <QLabel>

#include <KoColor.h>

#include <koffice_export.h>

class KValueSelector;
class KoXYColorSelector;
class KoColorSlider;
class KoColorPatch;
class QFrame;
class QLabel;
class QSpinBox;
class QRadioButton;

class KOPAINTER_EXPORT KoUniColorChooser
     : public QWidget
{
    Q_OBJECT
    typedef QWidget super;

public:
    KoUniColorChooser(KoColorSpaceFactoryRegistry* csFactoryRegistry, QWidget *parent = 0L);
    virtual ~KoUniColorChooser() {}

public slots:

signals:

    /**
     * Emitted when the current color is changed.
     */
    void sigColorChanged(const QColor & c);

protected slots:

    virtual void slotHChanged(int h);
    virtual void slotSChanged(int s);
    virtual void slotVChanged(int v);
    virtual void slotRGBChanged();
    virtual void slotRSelected(bool s);
    virtual void slotGSelected(bool s);
    virtual void slotBSelected(bool s);
    virtual void slotSliderChanged(int v);
    virtual void slotXYChanged(int u, int v);

private:
    enum ChannelType {CHANNEL_H, CHANNEL_S, CHANNEL_V, CHANNEL_R, CHANNEL_G, CHANNEL_B,CHANNEL_L,CHANNEL_a,CHANNEL_b};

    ChannelType m_activeChannel;
    KoColor m_currentColor;

    KoColorSpace *rgbColorSpace();
    KoColorSpace *labColorSpace();
    KoColorSpace *cmykColorSpace();

    void announceColor();
    void updateValues();
    void updateSelectorsR();
    void updateSelectorsG();
    void updateSelectorsB();
    void updateSelectorsCurrent();

    KoXYColorSelector *m_xycolorselector;
    KoColorSlider *m_colorSlider;
    KoColorPatch *m_colorpatch;
    QLabel *m_HLabel;
    QLabel *m_SLabel;
    QLabel *m_VLabel;
    QLabel *m_RLabel;
    QLabel *m_GLabel;
    QLabel *m_BLabel;
    QLabel *m_CLabel;
    QLabel *m_MLabel;
    QLabel *m_YLabel;
    QLabel *m_KLabel;
    QLabel *m_LLabel;
    QLabel *m_aLabel;
    QLabel *m_bLabel;
    QSpinBox *m_HIn;
    QSpinBox *m_SIn;
    QSpinBox *m_VIn;
    QSpinBox *m_RIn;
    QSpinBox *m_GIn;
    QSpinBox *m_BIn;
    QSpinBox *m_CIn;
    QSpinBox *m_MIn;
    QSpinBox *m_YIn;
    QSpinBox *m_KIn;
    QSpinBox *m_LIn;
    QSpinBox *m_aIn;
    QSpinBox *m_bIn;
    QRadioButton *m_HRB;
    QRadioButton *m_SRB;
    QRadioButton *m_VRB;
    QRadioButton *m_RRB;
    QRadioButton *m_GRB;
    QRadioButton *m_BRB;
    QRadioButton *m_LRB;
    QRadioButton *m_aRB;
    QRadioButton *m_bRB;

    KoColorSpaceFactoryRegistry* m_csFactoryRegistry;
};

#endif
