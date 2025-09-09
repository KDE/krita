/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISWAYLANDAPIOUTPUT_H
#define KISWAYLANDAPIOUTPUT_H

#include <QObject>
#include <qwayland-color-management-v1.h>

class KisWaylandAPIImageDescription;

class KisWaylandAPIOutput : public QObject, public QtWayland::wp_color_management_output_v1
{
    Q_OBJECT
public:
    explicit KisWaylandAPIOutput(::wp_color_management_output_v1 *obj);
    ~KisWaylandAPIOutput();

    std::unique_ptr<KisWaylandAPIImageDescription> m_imageDescription;

Q_SIGNALS:
    void outputImageDescriptionChanged();

protected:
    void wp_color_management_output_v1_image_description_changed() override;

};

#endif /* KISWAYLANDAPIOUTPUT_H */