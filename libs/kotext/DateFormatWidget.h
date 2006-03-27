#ifndef DATEFORMATWIDGET_H
#define DATEFORMATWIDGET_H
#include "timedateformatwidget.h"

class DateFormatWidget : public TimeDateFormatWidgetPrototype
{
    Q_OBJECT

public:
    DateFormatWidget( QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = 0 );
    ~DateFormatWidget();
    QString resultString();
    int correctValue();
public slots:
    void updateLabel();
    void comboActivated();
    void slotPersonalizeChanged(bool b);
    void slotDefaultValueChanged(const QString & );
    void slotOffsetChanged(int);
};

#endif // DATEFORMATWIDGET_H
