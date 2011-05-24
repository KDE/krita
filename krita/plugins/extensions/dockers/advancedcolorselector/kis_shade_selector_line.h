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

#ifndef KIS_SHADE_SELECTOR_LINE_H
#define KIS_SHADE_SELECTOR_LINE_H

#include <QWidget>

class KisCanvas2;
class KisShadeSelectorLineComboBox;

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
    explicit KisShadeSelectorLine(QWidget *parent = 0);
    explicit KisShadeSelectorLine(qreal hueDelta, qreal satDelta, qreal valDelta, QWidget *parent = 0, qreal hueShift = 0, qreal satShift = 0, qreal shiftVal = 0);
    void setParam(qreal hue, qreal sat, qreal val, qreal hueShift, qreal satShift, qreal shiftVal);
    void setColor(const QColor& color);
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

    QColor m_color;
    QColor m_backgroundColor;

    QImage m_pixelCache;

    bool m_gradient;
    int m_patchCount;
    int m_lineHeight;
    bool m_displayHelpText;

    friend class KisShadeSelectorLineComboBox;
};

#endif // KIS_SHADE_SELECTOR_LINE_H
