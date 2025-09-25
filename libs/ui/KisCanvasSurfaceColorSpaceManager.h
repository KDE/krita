/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISCANVASSURFACECOLORSPACEMANAGER_H
#define KISCANVASSURFACECOLORSPACEMANAGER_H

#include <QObject>

#include <KisDisplayConfig.h>
#include <kis_types.h>
#include <kis_config.h>

class KoColorProfile;
class KisSurfaceColorManagerInterface;

class KRITAUI_EXPORT KisCanvasSurfaceColorSpaceManager : public QObject
{
    Q_OBJECT
public:
    KisCanvasSurfaceColorSpaceManager(KisSurfaceColorManagerInterface *interface, QObject *parent = nullptr);
    ~KisCanvasSurfaceColorSpaceManager();

    bool isReady() const;
    KisDisplayConfig displayConfig() const;

    QString colorManagementReport() const;

    void setProofingConfiguration(KisProofingConfigurationSP proofingConfig);

Q_SIGNALS:
    void sigDisplayConfigChanged(const KisDisplayConfig &config);

private Q_SLOTS:
    void slotConfigChanged();
    void slotInterfaceReadyChanged(bool isReady);
    void slotInterfacePreferredDescriptionChanged();

private:
    void reinitializeSurfaceDescription();

private:
    struct Private;
    QScopedPointer<Private> m_d;
};

#endif /* KISCANVASSURFACECOLORSPACEMANAGER_H */
