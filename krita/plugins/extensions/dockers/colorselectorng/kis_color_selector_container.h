#ifndef KIS_COLOR_SELECTOR_CONTAINER_H
#define KIS_COLOR_SELECTOR_CONTAINER_H

#include <QWidget>

class KisColorSelector;
class KisMyPaintShadeSelector;
class KisMinimalShadeSelector;

class KisColorSelectorContainer : public QWidget
{
Q_OBJECT
public:
    explicit KisColorSelectorContainer(QWidget *parent = 0);

    enum ShadeSelectorType{MyPaintSelector, MinimalSelector, NoSelector};

    void setShadeSelectorType(int type);
    void setShadeSelectorHideable(bool hideable);
    void setAllowHorizontalLayout(bool allow);

signals:
    void openSettings();

protected:
    void resizeEvent(QResizeEvent *);

private:
    KisColorSelector* m_colorSelector;
    KisMyPaintShadeSelector* m_myPaintShadeSelector;
    KisMinimalShadeSelector* m_minimalShadeSelector;
    QWidget* m_shadeSelector;

    bool m_shadeSelectorHideable;
    bool m_allowHorizontalLayout;

    QLayout* m_buttonLayout;

    enum Direction{Horizontal, Vertical};
    void setNewLayout(Direction direction);

};

#endif // KIS_COLOR_SELECTOR_CONTAINER_H
