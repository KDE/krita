/*
 *  Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
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
#ifndef LIBKIS_VIEW_H
#define LIBKIS_VIEW_H

#include <QObject>

#include "kritalibkis_export.h"
#include "libkis.h"

class ManagedColor;
class Resource;
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
     * @return true if the current view is visible, false if not.
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
     * @brief foregroundColor allows access to the currently active color.
     * This is nominally per canvas/view, but in practice per mainwindow.
     * @code
color = Application.activeWindow().activeView().foregroundColor()
components = color.components()
components[0] = 1.0
components[1] = 0.6
components[2] = 0.7
color.setComponents(components)
Application.activeWindow().activeView().setForeGroundColor(color)
     * @endcode
     */
    ManagedColor *foregroundColor() const;
    void setForeGroundColor(ManagedColor *color);

    ManagedColor *backgroundColor() const;
    void setBackGroundColor(ManagedColor *color);

    Resource *currentBrushPreset() const;
    void setCurrentBrushPreset(Resource *resource);

    Resource *currentPattern() const;
    void setCurrentPattern(Resource *resource);

    Resource *currentGradient() const;
    void setCurrentGradient(Resource *resource);

    QString currentBlendingMode() const;
    void setCurrentBlendingMode(const QString &blendingMode);

    float HDRExposure() const;
    void setHDRExposure(float exposure);

    float HDRGamma() const;
    void setHDRGamma(float gamma);

    qreal paintingOpacity() const;
    void setPaintingOpacity(qreal opacity);

    qreal brushSize() const;
    void setBrushSize(qreal brushSize);

    qreal paintingFlow() const;
    void setPaintingFlow(qreal flow);

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

private:

    friend class Window;
    KisView *view();

    struct Private;
    Private *const d;

};

#endif // LIBKIS_VIEW_H
