/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISWAYLANDAPIIMAGEDESCRIPTIONCREATORPARAMS_H
#define KISWAYLANDAPIIMAGEDESCRIPTIONCREATORPARAMS_H

#include <QObject>
#include <qwayland-color-management-v1.h>

class KisWaylandAPIColorManager;
class KisWaylandAPIImageDescriptionNoInfo;
namespace KisSurfaceColorimetry {
    class WaylandSurfaceDescription;
}

class KisWaylandAPIImageDescriptionCreatorParams : public QObject, public QtWayland::wp_image_description_creator_params_v1
{
    Q_OBJECT
public:
    explicit KisWaylandAPIImageDescriptionCreatorParams(KisWaylandAPIColorManager *colorManager);
    explicit KisWaylandAPIImageDescriptionCreatorParams(::wp_image_description_creator_params_v1 *params, KisWaylandAPIColorManager *colorManager);

    ~KisWaylandAPIImageDescriptionCreatorParams();

    std::unique_ptr<KisWaylandAPIImageDescriptionNoInfo> createImageDescription(const KisSurfaceColorimetry::WaylandSurfaceDescription &data);

private:
    KisWaylandAPIColorManager *m_colorManager;
};

#endif /* KISWAYLANDIMAGEDESCRIPTIONCREATORPARAMS_H */