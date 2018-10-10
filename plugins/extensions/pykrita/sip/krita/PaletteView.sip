class PaletteView : QWidget
{
%TypeHeaderCode
#include "PaletteView.h"
%End
PaletteView(const PaletteView & __0);
public:
    PaletteView(QWidget *parent = 0);
    ~PaletteView();
public Q_SLOTS:
    void setPalette(Palette *palette);
    bool addEntryWithDialog(ManagedColor *color);
    bool addGroupWithDialog();
    bool removeSelectedEntryWithDialog();
    void trySelectClosestColor(ManagedColor *color);
Q_SIGNALS:
    void entrySelectedForeGround(KisSwatch entry);
    void entrySelectedBackGround(KisSwatch entry);
private:
};
