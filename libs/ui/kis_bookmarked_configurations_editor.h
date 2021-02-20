/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_BOOKMARKED_CONFIGURATIONS_EDITOR_H_
#define _KIS_BOOKMARKED_CONFIGURATIONS_EDITOR_H_

#include <QDialog>
#include <kritaui_export.h>

#include <kis_serializable_configuration.h>

class KisBookmarkedConfigurationsModel;
class QItemSelection;

/**
 * This dialog is used to edit the content of a bookmark manager
 */
class KRITAUI_EXPORT KisBookmarkedConfigurationsEditor : public QDialog
{
    Q_OBJECT
public:
    /**
     * @param parent
     * @param manager the model representing the bookmark manager
     * @param currentConfig is used if the user choose to create a new configuration
     *                      entry or to replace an existing entry
     */
    KisBookmarkedConfigurationsEditor(QWidget* parent, KisBookmarkedConfigurationsModel* manager, const KisSerializableConfigurationSP currentConfig);
    ~KisBookmarkedConfigurationsEditor() override;
private Q_SLOTS:
    void currentConfigChanged(const QItemSelection& selected, const QItemSelection&);
    void addCurrentConfiguration();
    void deleteConfiguration();
private:
    struct Private;
    Private* const d;
};

#endif
