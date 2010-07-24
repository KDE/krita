#ifndef KIS_COLOR_HISTORY_H
#define KIS_COLOR_HISTORY_H

#include "kis_color_patches.h"

class KisColorHistory : public KisColorPatches
{
    Q_OBJECT
public:
    explicit KisColorHistory(QWidget *parent = 0);

signals:

public slots:
    void commitColor(const QColor& color);
    
private:
    QList<QColor> m_colorHistory;

};

#endif // KIS_COLOR_HISTORY_H
