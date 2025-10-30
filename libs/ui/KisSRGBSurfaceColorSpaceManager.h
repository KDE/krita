/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISSRGBSURFACECOLORSPACEMANAGER_H
#define KISSRGBSURFACECOLORSPACEMANAGER_H

#include <KisCanvasSurfaceColorSpaceManager.h>


class KRITAUI_EXPORT KisSRGBSurfaceColorSpaceManager : public KisCanvasSurfaceColorSpaceManager
{
    Q_OBJECT
public:
    KisSRGBSurfaceColorSpaceManager(KisSurfaceColorManagerInterface *interface, QObject *parent = nullptr);
    ~KisSRGBSurfaceColorSpaceManager();

    static KisSRGBSurfaceColorSpaceManager* tryCreateForCurrentPlatform(QWidget *widget);

protected:
    void slotConfigChanged();
};

#endif /* KISSRGBSURFACECOLORSPACEMANAGER_H */
