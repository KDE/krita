/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_PROOFING_OPTIONS_WIDGET_H
#define KIS_PROOFING_OPTIONS_WIDGET_H

#include <QWidget>
#include <QScopedPointer>

#include <kis_types.h>
#include <KisDisplayConfig.h>

/**
 * @brief A widget that allows to select a combination of auto levels parameters
 */
class KRITAUI_EXPORT KisProofingOptionsWidget : public QWidget
{
    Q_OBJECT

public:
    KisProofingOptionsWidget(QWidget *parent);
    ~KisProofingOptionsWidget() override;

    KisProofingConfigurationSP currentProofingConfig() const;

    void setProofingConfig(KisProofingConfigurationSP config);
    void setDisplayConfigOptions(const KisDisplayConfig::Options &options);

    void stopPendingUpdates();

Q_SIGNALS:
    void sigProofingConfigChanged(KisProofingConfigurationSP config);

private Q_SLOTS:
    void slotProofingConfigChanged();

private:
    struct Private;
    QScopedPointer<Private> m_d;
};

#endif /* KIS_PROOFING_OPTIONS_WIDGET_H */
