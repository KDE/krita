#include "SimpleAnnotationWidget.h"

#include "../ReviewTool.h"
#include <QAction>
#include <QDebug>
SimpleAnnotationWidget::SimpleAnnotationWidget(ReviewTool *tool, QWidget *parent) 
    : QWidget(parent)
    , m_tool(tool)
{
    widget.setupUi(this);
    widget.insertAnnotation->setDefaultAction(m_tool->action("insert_annotation"));
    widget.removeAnnotation->setDefaultAction(m_tool->action("remove_annotation"));
}
