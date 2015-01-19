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

#ifndef KIS_SHADE_SELECTOR_LINE_H
#define KIS_SHADE_SELECTOR_LINE_H

#include <QWidget>
#include <KoColor.h>
#include "kis_types.h"

class KisCanvas2;
class KisShadeSelectorLineComboBox;
class KisColorSelectorBaseProxy;

class KisShadeSelectorLineBase : public QWidget {
public:
    KisShadeSelectorLineBase(QWidget* parent) : QWidget(parent)
    {}

    void setLineNumber(int n) {m_lineNumber=n;}
    virtual QString toString() const = 0;
    virtual void fromString(const QString& string) = 0;

protected:
    int m_lineNumber;
};

class KisShadeSelectorLine : public KisShadeSelectorLineBase
{
    Q_OBJECT
public:

    explicit KisShadeSelectorLine(KisColorSelectorBaseProxy *parentProxy,
                                  QWidget *parent = 0);
    explicit KisShadeSelectorLine(qreal hueDelta, qreal satDelta, qreal valDelta,
                                  KisColorSelectorBaseProxy *parentProxy, QWidget *parent = 0, qreal hueShift = 0, qreal satShift = 0, qreal valShift = 0);

    ~KisShadeSelectorLine();

    void setParam(qreal hue, qreal sat, qreal val, qreal hueShift, qreal satShift, qreal shiftVal);
    void setColor(const KoColor& color);
    void updateSettings();
    void setCanvas(KisCanvas2* canvas);
    void showHelpText() {m_displayHelpText=true;}
    QString toString() const;
    void fromString(const QString& string);

    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);

private:
    qreal m_hueDelta;
    qreal m_saturationDelta;
    qreal m_valueDelta;

    qreal m_hueShift;
    qreal m_saturationShift;
    qreal m_valueShift;

    KoColor m_realColor;
    KisPaintDeviceSP m_realPixelCache;

    bool m_gradient;
    int m_patchCount;
    int m_lineHeight;
    bool m_displayHelpText;

    friend class KisShadeSelectorLineComboBox;

    KisColorSelectorBaseProxy* m_parentProxy;
};

#endif // KIS_SHADE_SELECTOR_LINE_H
