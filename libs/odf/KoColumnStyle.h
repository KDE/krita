#ifndef KOCOLUMNSTYLE_H
#define KOCOLUMNSTYLE_H

#include "KoStyle.h"

class KoColumnStyle : public KoStyle
{
public:
    KoColumnStyle();
    ~KoColumnStyle();

    void setBreakBefore(bool breakBefore);
    bool breakBefore() const;

    void setBreakAfter(bool breakAfter);
    bool breakAfter() const;

    enum WidthType{
        MinimumWidth,
        ExactWidth,
        OptimalWidth
    };
    void setWidth(qreal width);
    void setWidthType(WidthType type);
    qreal width() const;

private:
    bool m_breakAfter;
    bool m_breakBefore;
    qreal m_width;
    WidthType m_widthType;
};

#endif
