/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISSURFACECOLORMANAGERINTERFACE_H
#define KISSURFACECOLORMANAGERINTERFACE_H

#include "KisSurfaceColorimetry.h"

#include <QObject>
#include <QFuture>

class QWindow;
class QScreen;


/**
 * An interface for fetching and setting the colorimetry information
 * of a surface underlying a QWindow
 *
 * Due to complications of some protocols (i.e. Wayland), the construction
 * and initialization of the object may be not straight forward and may
 * include several steps:
 *
 * 1) Initially, When KisSurfaceColorManagerInterface is created, it is
 *    created in a "not ready" state:
 *
 *    * \ref isReady() will return false
 *    * \ref surfaceDescription() will return std::nullopt
 *    * \ref preferredSurfaceDescription() will return std::nullopt
 *    * the calls to \ref setSurfaceDescription() and \ref unsetSurfaceDescription()
 *      do nothing
 *
 * 2) When initialization is completed, the interface emits \ref sigReadyChanged()
 *    with argument set to true. At this moment all the functions of the interface
 *    are supposed to work and do what expected.
 *
 *    WARNING: Please take it into account that \ref sigPreferredSurfaceDescriptionChanged()
 *             is not emitted during the initialization process. You should handle both
 *             the signals.
 *
 * 3) When the preferred surface color space is changed, the interface emits
 *    \ref sigPreferredSurfaceDescriptionChanged(). This may happen when the
 *    QWindow is moved to a different screen or when the user changes the settings
 *    of the current screen.
 *
 * When the connection to the compositor is lost, the interface first
 * emits \ref sigReadyChanged() with argument set to false, and restarts
 * the initialization process.
 */
class KRITASURFACECOLORMANAGEMENTAPI_EXPORT KisSurfaceColorManagerInterface : public QObject
{
    Q_OBJECT
public:
    KisSurfaceColorManagerInterface(QWindow *window, QObject *parent = nullptr);
    virtual ~KisSurfaceColorManagerInterface();

    /**
     * \return true when the interface is considered as "fully initialized",
     *         i.e. all the methods of the interface are supposed to work
     *         as expected
     */
    virtual bool isReady() const = 0;

    /**
     * Test if the surface description is supported by the compositor
     */
    virtual bool supportsSurfaceDescription(const KisSurfaceColorimetry::SurfaceDescription &desc) = 0;

    /**
     * Test if the rendering intent is supported by the compositor
     */
    virtual bool supportsRenderIntent(const KisSurfaceColorimetry::RenderIntent &intent) = 0;

    /**
     * Sets the surface description of the linked QWindow
     *
     * The operation may be asynchronous on some platforms, so you should
     * handle a QFuture<bool> object to get the actual result.
     *
     * \return a future telling if the description has already been set or not.
     *
     * WARNING: Please do NOT call future.waitForFinished() from the GUI thread.
     * On some platforms (e.g. wayland) a signal from the even loop should be processed
     * to actually set the surface description, so you'll get a deadlock if you try to
     * wait for it in the event loop's thread.
     */
    virtual QFuture<bool> setSurfaceDescription(const KisSurfaceColorimetry::SurfaceDescription &desc, KisSurfaceColorimetry::RenderIntent intent) = 0;

    /**
     * Unset the surface description of the underlying surface. In most of the compositors
     * it means that the surface will be treated as sRGB.
     */
    virtual void unsetSurfaceDescription() = 0;

    /**
     * \return the current description of the underlying surface
     *
     * It can return std::nullopt if the surface discription is unset or
     * if the connection to the compositor has been lost.
     */
    virtual std::optional<KisSurfaceColorimetry::SurfaceDescription> surfaceDescription() const = 0;

    /**
     * \return the preferred description for the underlying surface
     *         from the viewpoint of the compositor
     *
     * It can return std::nullopt only when the interface is not ready
     */
    virtual std::optional<KisSurfaceColorimetry::SurfaceDescription> preferredSurfaceDescription() const = 0;

Q_SIGNALS:
    void sigReadyChanged(bool value);
    void sigPreferredSurfaceDescriptionChanged(const KisSurfaceColorimetry::SurfaceDescription &desc);

protected:
    QWindow *m_window;
};

#endif /* KISSURFACECOLORMANAGERINTERFACE_H */