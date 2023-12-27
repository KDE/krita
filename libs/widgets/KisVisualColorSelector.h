/*
 * SPDX-FileCopyrightText: 2016 Wolthera van Hovell tot Westerflier <griffinvalley@gmail.com>
 * SPDX-FileCopyrightText: 2022 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_VISUAL_COLOR_SELECTOR_H
#define KIS_VISUAL_COLOR_SELECTOR_H

#include <QWidget>
#include <QScopedPointer>

#include <resources/KoGamutMask.h>

#include "KisColorSelectorInterface.h"
#include "KisVisualColorModel.h"
#include "kritawidgets_export.h"

class QVector4D;
class KisColorSelectorConfiguration;

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

    /**
     * @brief KisVisualColorSelector constructor.
     * @param parent The parent widget.
     * @param model The color model to work with. If not provided, a new one will be created.
     */
    explicit KisVisualColorSelector(QWidget *parent = 0, KisVisualColorModelSP model = KisVisualColorModelSP());
    ~KisVisualColorSelector() override;

    QSize minimumSizeHint() const override;
    void setSelectorModel(KisVisualColorModelSP model);
    KisVisualColorModelSP selectorModel() const;
    /**
     * @brief setConfig
     * @param forceCircular
     * Force circular is for space where you only have room for a circular selector.
     * @param forceSelfUpdate
     * ignored, can possibly be removed from parent class now
     */
    void setConfig(bool forceCircular, bool forceSelfUpdate) override;
    const KisColorSelectorConfiguration& configuration() const;
    /**
    * @brief Explicitly set the shape configuration.
    * Accepts all valid combinations of Advanced Color Selector, however parameters
    * will be converted to HSV equivalent since color model is independent.
    * @param config
    * Passing null will load the Advanced Color Selector configuration
    * Note: Null will also set the HSX color model for compatibility reasons,
    * while otherwise shape layout and color model are independent.
    */
    void setConfiguration(const KisColorSelectorConfiguration *config);
    void setAcceptTabletEvents(bool on);
    KoColor getCurrentColor() const override;
    void setMinimumSliderWidth(int width);
    const KoColorDisplayRendererInterface* displayRenderer() const;
    RenderMode renderMode() const;
    void setRenderMode(RenderMode mode);
    /**
     * @brief Get the state of automatic exposure adjustment.
     * If enabled, the selector will set new maximum channel values on the selectorModel
     * whenever the set display renderer signals a configuration change.
     * The default value is true.
     * @return the current state
     */
    bool autoAdjustExposure() const;
    void setAutoAdjustExposure(bool enabled);
    bool proofColors() const;
    void setProofColors(bool enabled);
    /**
     * @brief Set the slider position for slider + square and slider + wheel configurations.
     * @param edge Edge to position the slider; currently only supports LeftEdge and TopEdge
     */
    void setSliderPosition(Qt::Edge edge);
    KoGamutMask* activeGamutMask() const;

public Q_SLOTS:
    void slotSetColor(const KoColor &c) override;
    void slotSetColorSpace(const KoColorSpace *cs) override;
    void slotConfigurationChanged();
    void setDisplayRenderer(const KoColorDisplayRendererInterface *displayRenderer) override;
    void slotGamutMaskChanged(KoGamutMaskSP mask);
    void slotGamutMaskUnset();
    void slotGamutMaskPreviewUpdate();

private Q_SLOTS:
    void slotChannelValuesChanged(const QVector4D &values, quint32 channelFlags);
    void slotColorModelChanged();
    void slotColorSpaceChanged();
    void slotCursorMoved(QPointF pos);
    void slotDisplayConfigurationChanged();
    void slotReloadConfiguration();

Q_SIGNALS:
    /**
     * @brief sigInteraction is emitted whenever mouse interaction status changes
     * @param active when true, the user started dragging a handle, when false the
     *        interaction has just finished
     */
    void sigInteraction(bool active);

protected:
    void resizeEvent(QResizeEvent *) override;

private:
    bool useHorizontalSlider();
    void rebuildSelector();
    void loadACSConfig();
    void switchDisplayRenderer(const KoColorDisplayRendererInterface *displayRenderer);
    QVector4D calculateMaxChannelValues();
    static KisColorSelectorConfiguration validatedConfiguration(const KisColorSelectorConfiguration &cfg);

    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KIS_VISUAL_COLOR_SELECTOR_H
