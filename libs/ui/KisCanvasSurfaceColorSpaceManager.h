/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISCANVASSURFACECOLORSPACEMANAGER_H
#define KISCANVASSURFACECOLORSPACEMANAGER_H

#include <QObject>

#include <surfacecolormanagement/KisSurfaceColorimetry.h>

#include <KisDisplayConfig.h>
#include <kis_types.h>


class KoColorProfile;
class KisSurfaceColorManagerInterface;

class KisCanvasSurfaceColorSpaceManager : public QObject
{
    Q_OBJECT
public:
    KisCanvasSurfaceColorSpaceManager(KisSurfaceColorManagerInterface *interface, QObject *parent = nullptr);
    ~KisCanvasSurfaceColorSpaceManager();

    KisDisplayConfig displayConfig() const;

    QString colorManagementReport() const;

    void setProofingConfiguration(KisProofingConfigurationSP proofingConfig);

Q_SIGNALS:
    void sigDisplayConfigChanged(const KisDisplayConfig &config);

private:
    static KisSurfaceColorimetry::RenderIntent calculateConfigIntent(int intent, bool useBlackPointCompensation);
    static KisSurfaceColorimetry::RenderIntent calculateConfigIntent(const KisDisplayConfig &config);

private Q_SLOTS:
    void slotConfigChanged();
    void slotInterfaceReadyChanged(bool isReady);
    void slotInterfacePreferredDescriptionChanged();

private:
    void reinitializeSurfaceDescription();

private:
    QScopedPointer<KisSurfaceColorManagerInterface> m_interface;
    KisDisplayConfig m_currentConfig;
    std::optional<KisSurfaceColorimetry::RenderIntent> m_proofingIntentOverride;
};

#endif /* KISCANVASSURFACECOLORSPACEMANAGER_H */
