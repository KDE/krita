#ifndef KIS_COLOR_SELECTOR_SQUARE_H
#define KIS_COLOR_SELECTOR_SQUARE_H

class QColor;
typedef unsigned int QRgb;

#include "kis_color_selector_component.h"
#include "kis_color_selector.h"

class KisColorSelectorSimple : public KisColorSelectorComponent
{
Q_OBJECT
public:
    typedef KisColorSelector::Parameters Parameter;
    typedef KisColorSelector::Type Type;
    explicit KisColorSelectorSimple(KisColorSelectorBase *parent);
    void setConfiguration(Parameter param, Type type);

protected:
    virtual void selectColor(int x, int y);
    virtual void paint(QPainter*);
    QRgb colorAt(int x, int y);

private:
    Parameter m_parameter;
    Type m_type;
};

#endif // KIS_COLOR_SELECTOR_SQUARE_H
