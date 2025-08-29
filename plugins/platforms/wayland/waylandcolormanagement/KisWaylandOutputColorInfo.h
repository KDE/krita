/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISWAYLANDOUTPUTCOLORINFO_H
#define KISWAYLANDOUTPUTCOLORINFO_H

#include <surfacecolormanagement/KisOutputColorInfoInterface.h>

#include <unordered_map>

class KisWaylandAPIOutput;
class KisWaylandAPIColorManager;

class KisWaylandOutputColorInfo : public KisOutputColorInfoInterface
{
    Q_OBJECT
public:
    KisWaylandOutputColorInfo(QObject *parent = nullptr);
    ~KisWaylandOutputColorInfo() override;

    bool isReady() const override;
    std::optional<KisSurfaceColorimetry::SurfaceDescription> outputDescription(const QScreen *screen) const override;

private:
    void reinitialize();
    void initScreenConnection(QScreen *screen);
    void slotScreenAdded(QScreen *screen);
    void slotScreenRemoved(QScreen *screen);

    void setReadyImpl(bool value);
    bool checkIfAllReady() const;

private:
    std::shared_ptr<KisWaylandAPIColorManager> m_waylandManager;
    std::unordered_map<const QScreen*, std::unique_ptr<KisWaylandAPIOutput>> m_waylandOutputs;
    bool m_isReady {false};
};

#endif /* KISWAYLANDOUTPUTCOLORINFO_H */