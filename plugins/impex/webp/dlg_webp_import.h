/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef DLG_WEBP_IMPORT_H
#define DLG_WEBP_IMPORT_H

#include <QScopedPointer>

#include <KoDialog.h>
#include <kis_document_aware_spin_box_unit_manager.h>
#include <kis_properties_configuration.h>

#include "ui_dlg_webp_import.h"

class DlgWebPImport : public QWidget, public Ui::DlgWebPImport
{
    Q_OBJECT
public:
    DlgWebPImport(QWidget *parent)
        : QWidget(parent)
    {
        setupUi(this);
    }
};

class KisDlgWebPImport : public KoDialog
{
    Q_OBJECT

public:
    KisDlgWebPImport();
    void setConfiguration(const KisPropertiesConfigurationSP cfg);
    KisPropertiesConfigurationSP configuration() const;

private Q_SLOTS:
    void slotAspectChanged(bool keep);

    void slotWidthChanged(double v);
    void slotHeightChanged(double v);

private:
    QScopedPointer<DlgWebPImport> m_rawWidget;
    KisDocumentAwareSpinBoxUnitManager *m_heightUnitManager;
    KisDocumentAwareSpinBoxUnitManager *m_widthUnitManager;
    bool m_keepAspect;
    double m_aspectRatio, m_newWidth, m_newHeight;
};

#endif // DLG_WEBP_EXPORT_H
