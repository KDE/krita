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

class KisPaletteView;

class KRITAWIDGETS_EXPORT KisPaletteComboBox : public QComboBox
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
    void setPaletteModel(const KisPaletteModel *);
    void setCompanionView(KisPaletteView *);

private Q_SLOTS:
    void slotPaletteChanged();
    void slotSwatchSelected(const QModelIndex &index);
    void slotIndexSelected(int);

private /* methods */:
    QPixmap createColorSquare(const KisSwatch &swatch) const;
    static bool swatchInfoLess(const SwatchInfoType &, const SwatchInfoType &);

private /* member variables */:
    QScopedPointer<QCompleter> m_completer;
    QPointer<const KisPaletteModel> m_model;
    QHash<QString, PosIdxMapType> m_groupMapMap;
    QVector<KisSwatch> m_idxSwatchMap;
};

#endif // KISPALETTECOLORNAMELIST_H
