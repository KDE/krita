#ifndef CITATIONBIBLIOGRAPHYWIDGET_H
#define CITATIONBIBLIOGRAPHYWIDGET_H

#include "ui_CitationBibliographyWidget.h"
#include <kdialog.h>
#include <KoListStyle.h>

#include <QTextBlock>


class TextTool;
class KoStyleManager;

class CitationBibliographyWidget : public KDialog
{
    Q_OBJECT
public:
    explicit CitationBibliographyWidget(QTextDocument* document,QWidget *parent = 0);

public slots:
    void setStyleManager(KoStyleManager *sm);
    void insertCitation();

signals:
    void doneWithFocus();

private:
    Ui::CitationBibliographyWidget widget;
    KoStyleManager *m_styleManager;
    bool m_blockSignals;
    bool m_comboboxHasBidiItems;
    QTextBlock m_currentBlock;
    QTextDocument *document;
};

#endif // CITATIONBIBLIOGRAPHYWIDGET_H
