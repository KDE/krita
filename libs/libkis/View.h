/*
 *  SPDX-FileCopyrightText: 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef LIBKIS_VIEW_H
#define LIBKIS_VIEW_H

#include <QObject>

#include "kritalibkis_export.h"
#include "libkis.h"

class ManagedColor;
class Resource;
class Scratchpad;
class Node;
class KisView;

/**
 * View represents one view on a document. A document can be
 * shown in more than one view at a time.
 */
class KRITALIBKIS_EXPORT View : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(View)

public:
    explicit View(KisView *view, QObject *parent = 0);
    ~View() override;

    bool operator==(const View &other) const;
    bool operator!=(const View &other) const;

public Q_SLOTS:

    /**
     * @return the window this view is shown in.
     */
    Window* window() const;

    /**
     * @return the document this view is showing.
     */
    Document* document() const;

    /**
     * Reset the view to show @p document.
     */
    void setDocument(Document *document);

    /**
     * @return True if the current view is visible, False if not.
     */
    bool visible() const;

    /**
     * Make the current view visible.
     */
    void setVisible();

    /**
     * @return the canvas this view is showing. The canvas controls
     * things like zoom and rotation.
     */
    Canvas* canvas() const;

    /**
     * @brief activateResource activates the given resource.
     * @param resource: a pattern, gradient or paintop preset
     */
    void activateResource(Resource *resource);



    /**
     * @brief foregroundColor allows access to the currently active foreground color.
     * This is nominally per canvas/view, but in practice per mainwindow.
     *
     * @code
color = Application.activeWindow().activeView().foregroundColor()
components = color.components()
components[0] = 1.0
components[1] = 0.6
components[2] = 0.7
color.setComponents(components)
Application.activeWindow().activeView().setForeGroundColor(color)
     * @endcode
     *
     * @return The current foreground color
     */
    ManagedColor *foregroundColor() const;
    void setForeGroundColor(ManagedColor *color);

    /**
     * @brief backgroundColor allows access to the currently active background color.
     * This is nominally per canvas/view, but in practice per mainwindow.
     *
     * @return The current background color
     */
    ManagedColor *backgroundColor() const;
    void setBackGroundColor(ManagedColor *color);

    /**
     * @brief return the current selected preset
     *
     * @return the current preset (Resource type = 'preset')
     */
    Resource *currentBrushPreset() const;

    /**
     * @brief set the current selected preset
     *
     * @param resource the current preset to set (Resource type = 'preset')
     */
    void setCurrentBrushPreset(Resource *resource);

    /**
     * @brief return the current selected pattern
     *
     * @return the current pattern (Resource type = 'pattern')
     */
    Resource *currentPattern() const;

    /**
     * @brief set the current selected pattern
     *
     * @param resource the current pattern to set (Resource type = 'pattern')
     */
    void setCurrentPattern(Resource *resource);

    /**
     * @brief return the current selected gradient
     *
     * @return the current gradient (Resource type = 'gradient')
     */
    Resource *currentGradient() const;

    /**
     * @brief set the current selected gradient
     *
     * @param resource the current gradient to set (Resource type = 'gradient')
     */
    void setCurrentGradient(Resource *resource);

    /**
     * @brief return the current blending mode for brush
     *
     * @return the current blending mode identifier
     */
    QString currentBlendingMode() const;

    /**
     * @brief set the current blending mode for brush
     *
     * @param blendingMode the current belding mode identifier
     */
    void setCurrentBlendingMode(const QString &blendingMode);

    /**
     * @return the current HDR Exposure value
     */
    float HDRExposure() const;

    /**
     * @brief set the current HDR Exposure value
     *
     * @param exposure the HDR Exposure to set
     */
    void setHDRExposure(float exposure);

    /**
     * @return the current HDR Gamma value
     */
    float HDRGamma() const;

    /**
     * @brief set the current HDR Gamma value
     *
     * @param exposure the HDR Gamma to set
     */
    void setHDRGamma(float gamma);

    /**
     * @brief return the current opacity for brush
     *
     * @return the brush opacity value (0.00=fully transparent - 1.00=fully opaque)
     */
    qreal paintingOpacity() const;

    /**
     * @brief set the current opacity for brush
     *
     * @param opacity the opacity value (0.00=fully transparent - 1.00=fully opaque)
     */
    void setPaintingOpacity(qreal opacity);

    /**
     * @brief return the current size for brush
     *
     * @return the brush size value (in pixels)
     */
    qreal brushSize() const;

    /**
     * @brief set the current size for brush
     *
     * @param brushSize the brush size (in pixels)
     */
    void setBrushSize(qreal brushSize);

    /**
     * @brief return the current fade for brush
     *
     * @return the brush fade value (0.00 - 1.00)
     */
    qreal brushFade() const;

    /**
     * @brief set the current fade for brush
     *
     * @param brushFade the brush fade (0.00 - 1.00)
     */
    void setBrushFade(qreal brushFade);

    /**
     * @brief return the current rotation for brush tip
     *
     * @return the brush tip rotation value (in degrees)
     */
    qreal brushRotation() const;

    /**
     * @brief set the current rotation for brush tip
     *
     * @param brushRotation the brush tip rotation (in degrees)
     */
    void setBrushRotation(qreal brushRotation);

    /**
     * @brief return the current flow for brush
     *
     * @return the brush flow value
     */
    qreal paintingFlow() const;

    /**
     * @brief set the current flow value for brush
     *
     * @param flow the brush flow
     */
    void setPaintingFlow(qreal flow);

    /**
     * @brief return the current pattern size for brush
     *
     * @return the brush pattern size value
     */
    qreal patternSize() const;

    /**
     * @brief set the current pattern size value for brush
     *
     * @param flow the brush pattern size
     */
    void setPatternSize(qreal size);

    /**
     * @brief return current eraser mode status (active/inactive)
     *
     * @return True if eraser mode is active, otherwise False
     */
    bool eraserMode() const;

    /**
     * @brief set current eraser active/inactive
     *
     * @param value Set to True to activate eraser mode, False to deactivate
     */
    void setEraserMode(bool value);

    /**
     * @brief return current global alpha lock mode (active/inactive)
     *
     * @return True if is active, otherwise False
     */
    bool globalAlphaLock() const;

    /**
     * @brief set current global alpha lock mode active/inactive
     *
     * @param value Set to True to lock global alpha mode, False to unlock
     */
    void setGlobalAlphaLock(bool value);

    /**
     * @brief return current disabled pressure status
     *
     * @return True if is pressure is disabled, otherwise False
     */
    bool disablePressure() const;

    /**
     * @brief set current disabled pressure status
     *
     * @param value Set to True to disable pressure, False to enabled pressure
     */
    void setDisablePressure(bool value);

    /**
     * @brief showFloatingMessage displays a floating message box on the top-left corner of the canvas
     * @param message: Message to be displayed inside the floating message box
     * @param icon: Icon to be displayed inside the message box next to the message string
     * @param timeout: Milliseconds until the message box disappears
     * @param priority: 0 = High, 1 = Medium, 2 = Low. Higher priority
     * messages will be displayed in place of lower priority messages
     */
    void showFloatingMessage(const QString &message, const QIcon& icon, int timeout, int priority);

    /**
     * @brief selectedNodes returns a list of Nodes that are selected in this view.
     *
     *
@code
from krita import *
w = Krita.instance().activeWindow()
v = w.activeView()
selected_nodes = v.selectedNodes()
print(selected_nodes)
@endcode
     *
     *
     * @return a list of Node objects which may be empty.
     */
    QList<Node *> selectedNodes() const;

    /**
     * @brief flakeToDocumentTransform
     * The transformation of the document relative to the view without rotation and mirroring
     * @return QTransform
     */
    QTransform flakeToDocumentTransform() const;

    /**
     * @brief flakeToCanvasTransform
     * The transformation of the canvas relative to the view without rotation and mirroring
     * @return QTransform
     */
    QTransform flakeToCanvasTransform() const;

    /**
     * @brief flakeToImageTransform
     * The transformation of the image relative to the view without rotation and mirroring
     * @return QTransform
     */
    QTransform flakeToImageTransform() const;

private:

    friend class Window;
    friend class Scratchpad;


    KisView *view();

    struct Private;
    Private *const d;

};

#endif // LIBKIS_VIEW_H
