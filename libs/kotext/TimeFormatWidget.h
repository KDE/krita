#ifndef TIMEFORMATWIDGET_H
#define TIMEFORMATWIDGET_H
#include "timedateformatwidget.h"

class TimeFormatWidget : public TimeDateFormatWidgetPrototype
{
    Q_OBJECT

public:
    TimeFormatWidget( QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = 0 );
    ~TimeFormatWidget();
    QString resultString();
    int correctValue();
public slots:
    void updateLabel();
    void comboActivated();
    void slotPersonalizeChanged(bool b);
    void slotDefaultValueChanged(const QString & );
    void slotOffsetChanged(int);
};

#endif // TIMEFORMATWIDGET_H
