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

#include <koColor.h>

#include <koffice_export.h>

class KValueSelector;
class KoXYColorSelector;
class KoColorSlider;
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
    KoUniColorChooser(KoColorSpaceFactoryRegistry* csFactoryRegistry, QWidget *parent = 0L, const char *name = 0);
    virtual ~KoUniColorChooser() {}

public slots:
    /**
     * Set the current color to c. Do not emit the color changed signals
     */
    virtual void setColor(const QColor & c);

signals:

    /**
     * Emitted when the current color is changed.
     */
    void sigColorChanged(const QColor & c);

protected slots:

    virtual void slotHChanged(int h);
    virtual void slotSChanged(int s);
    virtual void slotVChanged(int v);
    virtual void slotWheelChanged(const KoOldColor& c);

    void slotSetColor(const QColor& c);

private:
    void changedFgColor();

    void update(const KoOldColor & color);

    KoXYColorSelector *m_xycolorselector;
    KoColorSlider *m_colorSlider;
    QFrame *m_colorpatch;
    QLabel *m_HLabel;
    QLabel *m_SLabel;
    QLabel *m_VLabel;
    QLabel *m_RLabel;
    QLabel *m_GLabel;
    QLabel *m_BLabel;
    QLabel *m_LLabel;
    QLabel *m_aLabel;
    QLabel *m_bLabel;
    QSpinBox *m_HIn;
    QSpinBox *m_SIn;
    QSpinBox *m_VIn;
    QSpinBox *m_RIn;
    QSpinBox *m_GIn;
    QSpinBox *m_BIn;
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

    KoOldColor m_fgColor;

    bool m_autovalue;
};

#endif
