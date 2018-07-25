#ifndef KISPALETTECOLORNAMELIST_H
#define KISPALETTECOLORNAMELIST_H

#include "kritawidgets_export.h"

#include <QComboBox>
#include <QCompleter>
#include <QPointer>
#include <QScopedPointer>
#include <QPixmap>

#include <KisPaletteModel.h>

class KRITAWIDGETS_EXPORT KisPaletteComboBox : public QComboBox
{
    Q_OBJECT
private /* typedef */:
    typedef KisSwatchGroup::SwatchInfo SwatchInfoType;

private /* struct */:
    struct SwatchPos {
        int row;
        int column;
    };

public:
    explicit KisPaletteComboBox(QWidget *parent = Q_NULLPTR);
    ~KisPaletteComboBox();

public /* methods */:
    void setPaletteModel(const KisPaletteModel *);

private Q_SLOTS:
    void slotPaletteChanged();
    void slotSwatchSelected(const KisSwatch &swatch) const;

private /* methods */:
    QPixmap createColorSquare(const KisSwatch &swatch) const;
    static bool swatchLess(const KisSwatch &, const KisSwatch &);

private /* member variables */:
    QScopedPointer<QCompleter> m_completer;
    QPointer<const KisPaletteModel> m_model;
};

#endif // KISPALETTECOLORNAMELIST_H
