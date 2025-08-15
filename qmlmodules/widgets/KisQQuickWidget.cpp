/*
 * SPDX-FileCopyrightText: 2025 Wolthera van Hovell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisQQuickWidget.h"


#include <QQmlEngine>
#include <QQmlContext>
#include <QQuickStyle>

#include <KLocalizedContext>

#include <KoResourcePaths.h>
#include <KisSurfaceColorSpaceWrapper.h>

KisQQuickWidget::KisQQuickWidget(QWidget *parent): QQuickWidget(parent)
{
#if !defined(Q_OS_MACOS) || QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QSurfaceFormat format;

    {
        /**
         * Our version of Qt has a special handling of the color
         * space passed to the surface of the QQuickWidget. It will
         * allow it to render correctly on a Rec2020PQ window.
         */
        format.setRedBufferSize(8);
        format.setGreenBufferSize(8);
        format.setBlueBufferSize(8);
        format.setAlphaBufferSize(8);
        format.setColorSpace(KisSurfaceColorSpaceWrapper::makeSRGBColorSpace());
    }

    setFormat(format);
#endif

    engine()->rootContext()->setContextProperty("mainWindow", parent);
    engine()->rootContext()->setContextObject(new KLocalizedContext(parent));

    // Clear color is the 'default background color', which, in qwidget context is the window bg.
    setClearColor(palette().window().color());

    // Default to fusion style unless the user forces another style
    const QString fusion = "Fusion";
    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE") && QQuickStyle::name() != fusion) {
         QQuickStyle::setStyle(fusion);
    }

    engine()->addImportPath(KoResourcePaths::getApplicationRoot() + "/lib/qml/");
    engine()->addImportPath(KoResourcePaths::getApplicationRoot() + "/lib64/qml/");

    engine()->addPluginPath(KoResourcePaths::getApplicationRoot() + "/lib/qml/");
    engine()->addPluginPath(KoResourcePaths::getApplicationRoot() + "/lib64/qml/");


    setResizeMode(QQuickWidget::SizeRootObjectToView);
}

KisQQuickWidget::~KisQQuickWidget()
{
    /// Prevent accessing destroyed objects in QML engine
    /// See:
    ///   * https://invent.kde.org/graphics/krita/-/commit/d8676f4e9cac1a8728e73fec3ff1df1763c713b7
    ///   * https://bugreports.qt.io/browse/QTBUG-81247
    setParent(nullptr);
}
