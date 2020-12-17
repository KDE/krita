/*
 *  SPDX-FileCopyrightText: 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef LIBKIS_DOCKWIDGET_H
#define LIBKIS_DOCKWIDGET_H

#include <QDockWidget>

#include "kritalibkis_export.h"
#include "libkis.h"

#include <KoCanvasObserverBase.h>

class KoCanvasBase;

/**
 * DockWidget is the base class for custom Dockers. Dockers are created by a
 * factory class which needs to be registered by calling Application.addDockWidgetFactory:
 *
 * @code
 * class HelloDocker(DockWidget):
 *   def __init__(self):
 *       super().__init__()
 *       label = QLabel("Hello", self)
 *       self.setWidget(label)
 *       self.label = label
 *       self.setWindowTitle("Hello Docker")
 *
 * def canvasChanged(self, canvas):
 *       self.label.setText("Hellodocker: canvas changed");
 *
 * Application.addDockWidgetFactory(DockWidgetFactory("hello", DockWidgetFactoryBase.DockRight, HelloDocker))
 *
 * @endcode
 *
 * One docker per window will be created, not one docker per canvas or view. When the user
 * switches between views/canvases, canvasChanged will be called. You can override that
 * method to reset your docker's internal state, if necessary.
 */
class KRITALIBKIS_EXPORT DockWidget : public QDockWidget, public KoCanvasObserverBase
{
    Q_OBJECT
    Q_DISABLE_COPY(DockWidget)

public:
    explicit DockWidget();
    ~DockWidget() override;

protected Q_SLOTS: // Krita API

    void setCanvas(KoCanvasBase* canvas) override;
    void unsetCanvas() override;

protected Q_SLOTS: // PyKrita API

    /**
     * @@return the canvas object that this docker is currently associated with
     */
    Canvas* canvas() const;

    /**
     * @brief canvasChanged is called whenever the current canvas is changed
     * in the mainwindow this dockwidget instance is shown in.
     * @param canvas The new canvas.
     */
    virtual void canvasChanged(Canvas *canvas) = 0;

private:
    struct Private;
    Private *const d;

};

#endif // LIBKIS_DOCKWIDGET_H
