#ifndef KIS_COLOR_HISTORY_H
#define KIS_COLOR_HISTORY_H

#include "kis_color_patches.h"


class KisColorHistory : public KisColorPatches
{
    Q_OBJECT
public:
    explicit KisColorHistory(QWidget *parent = 0);
    void setCanvas(KisCanvas2 *canvas);

protected:
    KisColorSelectorBase* createPopup() const;

public slots:
    void commitColor(const KoColor& color);
    
private:
    QList<KoColor> m_colorHistory;

};

#endif // KIS_COLOR_HISTORY_H
