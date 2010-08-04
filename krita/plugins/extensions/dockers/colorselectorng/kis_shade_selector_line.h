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

class KisShadeSelectorLine : public QWidget
{
    Q_OBJECT
public:
    explicit KisShadeSelectorLine(QWidget *parent = 0);
    explicit KisShadeSelectorLine(qreal hueDelta, qreal satDelta, qreal valDelta, QWidget *parent = 0);
    void setDelta(qreal hue, qreal sat, qreal val);
    void setColor(const QColor& color);
    void updateSettings();
    void setCanvas(KisCanvas2* canvas);
    void setLineNumber(int n);
    QString toString() const;
    void fromString(const QString& string);

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);

protected slots:
    void resourceChanged(int key, const QVariant &v);

private:
    qreal m_hueDelta;
    qreal m_saturationDelta;
    qreal m_valueDelta;

    QColor m_color;
    QColor m_backgroundColor;

    QImage m_pixelCache;

    bool m_gradient;
    int m_patchCount;
    int m_lineHeight;
    int m_lineNumber;

    KisCanvas2* m_canvas;

    friend class KisShadeSelectorLineComboBox;
};

#endif // KIS_SHADE_SELECTOR_LINE_H
