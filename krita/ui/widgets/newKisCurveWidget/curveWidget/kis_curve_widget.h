#ifndef KIS_CURVE_WIDGET_H
#define KIS_CURVE_WIDGET_H

#include <QtGui/QWidget>

class KisCurveWidgetBase;

class KisCurveWidget : public QWidget
{
    Q_OBJECT

public:
    KisCurveWidget(QWidget *parent = 0);
    ~KisCurveWidget();

public slots:
    void switchToFunction() {switchTo(m_functionLikeWidget);}
    void switchToCubic() {switchTo(m_cubicWidget);}
    void switchToLinear() {switchTo(m_linearWidget);}
    void switchToFreehand() {switchTo(m_freehandWidget);}
    void reset();

protected:
    void switchTo(KisCurveWidgetBase* newWidget);

private:
    KisCurveWidgetBase* m_currentCurve;
    KisCurveWidgetBase* m_functionLikeWidget;
    KisCurveWidgetBase* m_cubicWidget;
    KisCurveWidgetBase* m_linearWidget;
    KisCurveWidgetBase* m_freehandWidget;
};

#endif // KIS_CURVE_WIDGET_H
