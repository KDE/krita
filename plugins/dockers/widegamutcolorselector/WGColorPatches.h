/*
 * SPDX-FileCopyrightText: 2020 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef WGCOLORPATCHES_H
#define WGCOLORPATCHES_H

#include "WGSelectorWidgetBase.h"

#include <kis_types.h>

class KisUniqueColorSet;
class KoColorDisplayRendererInterface;

namespace WGConfig {
struct ColorPatches;
}

class WGColorPatches : public WGSelectorWidgetBase
{
    Q_OBJECT
public:
    explicit WGColorPatches(KisUniqueColorSet *history, QWidget *parent = nullptr);

    KisUniqueColorSet* colorHistory() const;
    void updateSettings() override;
    void setConfigSource(const WGConfig::ColorPatches *source);
    QPoint popupOffset() const override;
    /*! set buttons, that should be drawn additionally to the patches
     * this class takes ownership of them and will delete them
     * they will be resized to the patchsize */
    void setAdditionalButtons(QList<QWidget*> buttonList);
    void setColorHistory(KisUniqueColorSet *history);
protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    QSize sizeHint() const override;

    bool colorAt(const QPoint &pos, KoColor &result) const;
    int indexAt(const QPoint &widgetPos) const;
    int maxScroll() const;
    QRect patchRect(int gridIndex) const;

private:
    QPointer<KisUniqueColorSet> m_colors;
    QList<QWidget*> m_buttonList;
    Qt::Orientation m_orientation {Qt::Horizontal};
    const WGConfig::ColorPatches *m_configSource {0};
    int m_numLines {1};
    int m_patchWidth {16};
    int m_patchHeight {16};
    int m_patchCount {30};
    int m_scrollValue {0};
    int m_mouseIndex {-1};
    bool m_allowScrolling {true};

Q_SIGNALS:
    void sigColorChanged(const KoColor &color);

};

#endif // WGCOLORPATCHES_H
