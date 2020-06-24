/*
 * SPDX-FileCopyrightText: 2016 Wolthera van Hovell tot Westerflier <griffinvalley@gmail.com>
 * SPDX-FileCopyrightText: 2020 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_VISUAL_COLOR_SELECTOR_H
#define KIS_VISUAL_COLOR_SELECTOR_H

#include <QWidget>
#include <QScopedPointer>

#include "KisColorSelectorInterface.h"
#include "KisVisualColorModel.h"
#include "kritawidgets_export.h"

class QVector4D;

/**
 * @brief The KisVisualColorSelector class
 *
 * This gives a color selector box that draws gradients and everything.
 *
 * Unlike other color selectors, this one draws the full gamut of the given
 * colorspace.
 */
class KRITAWIDGETS_EXPORT KisVisualColorSelector : public KisColorSelectorInterface
{
    Q_OBJECT
public:

    enum RenderMode {
        StaticBackground,
        DynamicBackground,
        CompositeBackground
    };

    explicit KisVisualColorSelector(QWidget *parent = 0);
    ~KisVisualColorSelector() override;

    QSize minimumSizeHint() const override;
    void setSelectorModel(KisVisualColorModel *model);
    KisVisualColorModel *selectorModel() const;
    /**
     * @brief setConfig
     * @param forceCircular
     * Force circular is for space where you only have room for a circular selector.
     * @param forceSelfUpdate
     * ignored, can possibly be removed from parent class now
     */
    void setConfig(bool forceCircular, bool forceSelfUpdate) override;
    void setAcceptTabletEvents(bool on);
    KoColor getCurrentColor() const override;
    RenderMode renderMode() const;
    void setRenderMode(RenderMode mode);

public Q_SLOTS:
    void slotSetColor(const KoColor &c) override;
    void slotSetColorSpace(const KoColorSpace *cs) override;
    void slotConfigurationChanged();
    void setDisplayRenderer(const KoColorDisplayRendererInterface *displayRenderer) override;

private Q_SLOTS:
    void slotChannelValuesChanged(const QVector4D &values);
    void slotColorModelChanged();
    void slotCursorMoved(QPointF pos);
    void slotDisplayConfigurationChanged();
    void slotRebuildSelectors();

protected:
    void resizeEvent(QResizeEvent *) override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KIS_VISUAL_COLOR_SELECTOR_H
