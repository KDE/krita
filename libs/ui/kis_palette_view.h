/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_PALETTE_VIEW_H
#define __KIS_PALETTE_VIEW_H

#include <QScopedPointer>
#include <KoTableView.h>
#include <KoColorSet.h>
#include "kritaui_export.h"

class KisPaletteModel;
class QWheelEvent;


class KRITAUI_EXPORT KisPaletteView : public KoTableView
{
    Q_OBJECT
public:
    KisPaletteView(QWidget *parent = 0);
    ~KisPaletteView() override;

    void setPaletteModel(KisPaletteModel *model);
    KisPaletteModel* paletteModel() const;

    /**
     * @brief updateRows
     * update the rows so they have the proper columnspanning.
     */
    void updateRows();

    void setCrossedKeyword(const QString &value);
Q_SIGNALS:
    /**
     * @brief entrySelected
     * signals when an entry is selected.
     * @param entry the selected entry.
     */
    void entrySelected(KoColorSetEntry entry);
protected:
    void wheelEvent(QWheelEvent *event) override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
private Q_SLOTS:
    /**
     * @brief entrySelection
     * the function that will emit entrySelected when the entry changes.
     */
    void entrySelection();
    /**
     * @brief modifyEntry
     * function for changing the entry at the given index.
     */
    void modifyEntry(QModelIndex index);
};

#endif /* __KIS_PALETTE_VIEW_H */
