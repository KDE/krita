#ifndef DATEFORMATWIDGET_H
#define DATEFORMATWIDGET_H

#include <QWidget>
class Ui_TimeDateFormatWidgetPrototype;

class DateFormatWidget : public QWidget
{
    Q_OBJECT

public:
    DateFormatWidget( QWidget* parent );
    ~DateFormatWidget();
    QString resultString();
    int correctValue();

public slots:
    void updateLabel();
    void comboActivated();
    void slotPersonalizeChanged(bool b);
    void slotDefaultValueChanged(const QString & );
    void slotOffsetChanged(int);

private:
    Ui_TimeDateFormatWidgetPrototype* m_ui;
};

#endif // DATEFORMATWIDGET_H
