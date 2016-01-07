#ifndef SIMPLEANNOTATIONWIDGET_H
#define SIMPLEANNOTATIONWIDGET_H

#include "ui_SimpleAnnotationWidget.h"
#include <QWidget>

class ReviewTool;

class SimpleAnnotationWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SimpleAnnotationWidget(ReviewTool *tool, QWidget *parent = 0);
    //virtual ~SimpleAnnotationWidget();

Q_SIGNALS:
    void doneWithFocus();

private:
    Ui::SimpleAnnotationWidget widget;
    ReviewTool *m_tool;

};

#endif // SIMPLEANNOTATIONWIDGET_H
