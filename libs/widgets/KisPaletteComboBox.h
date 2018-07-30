#ifndef KISPALETTECOLORNAMELIST_H
#define KISPALETTECOLORNAMELIST_H

#include "kritawidgets_export.h"

#include <QComboBox>
#include <QCompleter>
#include <QPointer>
#include <QScopedPointer>
#include <QPixmap>
#include <QPair>
#include <QHash>

#include <KisPaletteModel.h>

class KRITAWIDGETS_EXPORT KisPaletteComboBox : public QComboBox
{
    Q_OBJECT
private /* typedef */:
    typedef KisSwatchGroup::SwatchInfo SwatchInfoType;
    typedef QPair<int, int> SwatchPosType; // first is column #, second is row #

public:
    explicit KisPaletteComboBox(QWidget *parent = Q_NULLPTR);
    ~KisPaletteComboBox();

Q_SIGNALS:
    void sigColorSelected(const KoColor &);

public /* methods */:
    void setPaletteModel(const KisPaletteModel *);

private Q_SLOTS:
    void slotPaletteChanged();
    void slotSwatchSelected(const QModelIndex &index);
    void slotColorSelected(int);

private /* methods */:
    QPixmap createColorSquare(const KisSwatch &swatch) const;
    static bool swatchInfoLess(const SwatchInfoType &, const SwatchInfoType &);

private /* member variables */:
    QScopedPointer<QCompleter> m_completer;
    QPointer<const KisPaletteModel> m_model;
    QHash<QPair<int, int>, int> m_posIdxMap; // map from pos in table to idx here
    QVector<KisSwatch> m_idxSwatchMap;
};

#endif // KISPALETTECOLORNAMELIST_H
