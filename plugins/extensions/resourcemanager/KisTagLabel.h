#ifndef TAGLABEL_H
#define TAGLABEL_H

#include <QWidget>

class TagLabel : public QWidget
{
    Q_OBJECT

public:
    explicit TagLabel(QString string, QWidget *parent = nullptr);
    ~TagLabel();

public:
    QString getText();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QString m_string;
};

#endif // TAGLABEL_H
