/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
