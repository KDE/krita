#include "KisTagLabel.h"

#include <QPainter>
#include <QPainterPath>
#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>

TagLabel::TagLabel(QString string, QWidget *parent) :
    QWidget(parent)
{
    m_string = string;

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 0, 8, 0);
    layout->setSpacing(2);

    QLabel *label = new QLabel(parent);
    label->setText(m_string);
    label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    layout->addWidget(label);

    setLayout(layout);
}

TagLabel::~TagLabel()
{
}

void TagLabel::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    QColor backGroundColor = qApp->palette().light().color();
    QColor foregroundColor = qApp->palette().windowText().color();

    QWidget::paintEvent(event);
    painter.setRenderHint(QPainter::Antialiasing);
    QPainterPath path;
    path.addRoundedRect(this->rect(), 6, 6);

    // good color:
    painter.fillPath(path, qApp->palette().light());
}

QString TagLabel::getText()
{
    return m_string;
}
