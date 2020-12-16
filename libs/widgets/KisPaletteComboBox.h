/*
 *  SPDX-FileCopyrightText: 2018 Michael Zhou <simeirxh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
    explicit KisPaletteComboBox(QWidget *parent = 0);
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
