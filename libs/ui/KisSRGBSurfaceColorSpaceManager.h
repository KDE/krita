/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISSRGBSURFACECOLORSPACEMANAGER_H
#define KISSRGBSURFACECOLORSPACEMANAGER_H

#include <QObject>
#include <QScopedPointer>

#include <surfacecolormanagement/KisSurfaceColorimetry.h>

class KisSurfaceColorManagerInterface;


class KisSRGBSurfaceColorSpaceManager : public QObject
{
    Q_OBJECT
public:
    KisSRGBSurfaceColorSpaceManager(KisSurfaceColorManagerInterface *interface, QObject *parent = nullptr);
    ~KisSRGBSurfaceColorSpaceManager();

    QString colorManagementReport() const;
    QString osPreferredColorSpaceReport() const;

    static KisSRGBSurfaceColorSpaceManager* tryCreateForCurrentPlatform(QWidget *widget);

protected:
    static KisSurfaceColorimetry::RenderIntent calculateConfigIntent();

    void slotConfigChanged();
    void slotInterfaceReadyChanged(bool isReady);
    void reinitializeSurfaceDescription();

protected:
    QScopedPointer<KisSurfaceColorManagerInterface> m_interface;
};

#endif /* KISSRGBSURFACECOLORSPACEMANAGER_H */
