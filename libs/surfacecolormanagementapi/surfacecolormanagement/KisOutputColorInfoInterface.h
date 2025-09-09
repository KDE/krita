/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISOUTPUTCOLORINFOINTERFACE_H
#define KISOUTPUTCOLORINFOINTERFACE_H

#include "KisSurfaceColorimetry.h"

#include <QObject>
#include <QFuture>

class QWindow;
class QScreen;

/**
 * A basic interface for fetching colorimetry information from a QScreen
 * object.
 *
 * Due to complications of some protocols (i.e. Wayland), the construction
 * and initialization of the object may be not straight forward and may
 * include several steps:
 *
 * 1) Initially, when KisOutputColorInfoInterface is created, it is created
 *    in a "not ready" state. \ref isReady() will return false and
 *    \ref outputDescription() for any screen may (or may not) return
 *    std::nullopt.
 *
 * 2) Then the screens will asynchronously initialize. The interface will
 *    emit \ref sigOutputDescriptionChanged() for each known interfrace.
 *
 * 3) When all known screens are initialized (and all
 *    \ref sigOutputDescriptionChanged() signals are emitted), the interface
 *    emits \ref sigReadyChanged() with argument set to true. At this point
 *    \ref outputDescription() will return a correct description for every
 *    known screen.
 *
 * When the connection to the compositor is lost, the interface first
 * emits \ref sigReadyChanged() with argument set to false, and restarts
 * the initialization process.
 */
class KRITASURFACECOLORMANAGEMENTAPI_EXPORT KisOutputColorInfoInterface : public QObject
{
    Q_OBJECT
public:
    KisOutputColorInfoInterface(QObject *parent = nullptr);
    virtual ~KisOutputColorInfoInterface();

    /**
     * \return true when the interface is considered as "fully initialized",
     *         i.e. when all screen descriptions are ready.
     */
    virtual bool isReady() const = 0;

    /**
     * \return a color space description for the specified screen
     *
     * When \ref isReady() is set to false may or may not return std::nullopt,
     * wait for \ref sigReadyChanged() to make sure all screens are initialized.
     */
    virtual std::optional<KisSurfaceColorimetry::SurfaceDescription> outputDescription(const QScreen *screen) const = 0;

Q_SIGNALS:
    /**
     * Emits when the interface transitions between ready and non-ready states.
     *
     * The interface becomes ready when the connection to the compositor is
     * established and information about all screens is fetched.
     *
     * The interface may become non-ready at any moment of time when the
     * connection to the compositor is lost. You should wait for
     * sigReadyChanged(true) in such a case.
     */
    void sigReadyChanged(bool isReady);

    /**
     * Emits when a screen has changed its color description, e.g. when
     * the user assigned a new color profile to this screen in the compositor
     *
     * Please note that during the initialization process this signal is emitted
     * for every screen **before** sigReadyChanged(true).
     */
    void sigOutputDescriptionChanged(QScreen *screen, const KisSurfaceColorimetry::SurfaceDescription &desc);

protected:
    QWindow *m_window;
};

#endif /* KISOUTPUTCOLORINFOINTERFACE_H */