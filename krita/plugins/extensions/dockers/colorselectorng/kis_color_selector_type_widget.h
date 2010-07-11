#ifndef KIS_COLOR_SELECTOR_TYPE_WIDGET_H
#define KIS_COLOR_SELECTOR_TYPE_WIDGET_H

class KisCanvas2;

#include <QComboBox>

class KisColorSelectorTypeWidget : public QComboBox
{
public:
    KisColorSelectorTypeWidget(QWidget* parent=0);
    ~KisColorSelectorTypeWidget();
    void hidePopup();
    void showPopup();
    void setCanvas(KisCanvas2* canvas);
protected:
    void paintEvent(QPaintEvent *e);
private:
    KisColorSelectorTypeWidget* m_popup;
};

#endif // KIS_COLOR_SELECTOR_TYPE_WIDGET_H
