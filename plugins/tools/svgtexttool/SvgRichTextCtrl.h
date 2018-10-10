#ifndef SVGRICHTEXTCTRL_H
#define SVGRICHTEXTCTRL_H

#include <QTextEdit>

class SvgRichTextCtrl : public QTextEdit
{
public:
    SvgRichTextCtrl(QWidget* parent = nullptr);

protected:
    void insertFromMimeData(const QMimeData* source) override;
};

#endif // SVGRICHTEXTCTRL_H
