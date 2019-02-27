/*
 *  Copyright (c) 2018 Michael Zhou <simeirxh@gmail.com>
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

#ifndef KISPALETTECOMBOBOX_H
#define KISPALETTECOMBOBOX_H

#include "kritawidgets_export.h"

#include <QComboBox>
#include <QPointer>
#include <QScopedPointer>
#include <QPixmap>
#include <QPair>
#include <QHash>
#include <KisSqueezedComboBox.h>
#include <KisPaletteModel.h>

class KisPaletteView;

/**
 * @brief The KisPaletteComboBox class
 * A combobox used with KisPaletteView
 *
 */
class KRITAWIDGETS_EXPORT KisPaletteComboBox : public KisSqueezedComboBox
{
    Q_OBJECT
private /* typedef */:
    typedef KisSwatchGroup::SwatchInfo SwatchInfoType;
    typedef QPair<int, int> SwatchPosType; // first is column #, second is row #
    typedef QHash<SwatchPosType, int> PosIdxMapType;

public:
    explicit KisPaletteComboBox(QWidget *parent = Q_NULLPTR);
    ~KisPaletteComboBox();

Q_SIGNALS:
    void sigColorSelected(const KoColor &);

public /* methods */:
    void setCompanionView(KisPaletteView *);

private Q_SLOTS:
    void setPaletteModel(const KisPaletteModel *);
    void slotPaletteChanged();
    void slotSwatchSelected(const QModelIndex &index);
    void slotIndexUpdated(int);

private /* methods */:
    QPixmap createColorSquare(const KisSwatch &swatch) const;
    static bool swatchInfoLess(const SwatchInfoType &, const SwatchInfoType &);

private /* member variables */:
    QPointer<const KisPaletteModel> m_model;
    QPointer<KisPaletteView> m_view;
    QHash<QString, PosIdxMapType> m_groupMapMap;
    QVector<KisSwatch> m_idxSwatchMap;
};

#endif // KISPALETTECOMBOBOX_H
