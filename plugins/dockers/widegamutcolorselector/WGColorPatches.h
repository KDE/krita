/*
 * SPDX-FileCopyrightText: 2020 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef WGCOLORPATCHES_H
#define WGCOLORPATCHES_H

#include "WGSelectorWidgetBase.h"

#include <kis_types.h>

class KoColorDisplayRendererInterface;

class WGColorPatches : public WGSelectorWidgetBase
{
    Q_OBJECT
public:
    explicit WGColorPatches(QWidget *parent = nullptr);

    void addUniqueColor(const KoColor& color);
    QList<KoColor> colors() const { return m_colors; }
    /*! set buttons, that should be drawn additionally to the patches
     * this class takes ownership of them and will delete them
     * they will be resized to the patchsize */
    void setAdditionalButtons(QList<QWidget*> buttonList);
    void setColors(QList<KoColor>colors);
protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    QSize sizeHint() const override;

    bool colorAt(const QPoint &pos, KoColor &result) const;
    int indexAt(const QPoint &pos) const;
    int maxScroll() const;
    QRect patchRect(int gridIndex) const;

private:
    QList<KoColor> m_colors;
    QList<QWidget*> m_buttonList;
    Qt::Orientation m_orientation {Qt::Horizontal};
    int m_numLines {1};
    int m_patchWidth {16};
    int m_patchHeight {16};
    int m_patchCount {20};
    int m_scrollValue {0};
    int m_mouseIndex {-1};
    bool m_allowScrolling {true};

Q_SIGNALS:
    void sigColorChanged(const KoColor &color);
    void sigInteraction(bool active);

};

#endif // WGCOLORPATCHES_H
