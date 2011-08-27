#include<KoTextEditor.h>

#include "TableOfContentsConfigure.h"
#include "TableOfContentsStyleConfigure.h"
#include "KoTableOfContentsGeneratorInfo.h"
#include "KoTextDocument.h"

#include "KoParagraphStyle.h"

#include <QListWidgetItem>

TableOfContentsConfigure::TableOfContentsConfigure(KoTextEditor *editor, QWidget *parent) :
    QDialog(parent),
    m_textEditor(editor),
    document(0),
    m_tocStyleConfigure(0)
{
    ui.setupUi(this);

    ui.lineEditTitle->setText(i18n("Table Title"));
    ui.useOutline->setText(i18n("Use outline"));
    ui.configureStyles->setText(i18n("Configure"));

    connect(this,SIGNAL(accepted()),this,SLOT(save()));
    connect(ui.tocList,SIGNAL(currentRowChanged(int)),this,SLOT(tocListIndexChanged(int)));
    connect(ui.configureStyles,SIGNAL(clicked(bool)),this,SLOT(showStyleConfiguration(bool)));
    setDisplay();

    this->setVisible(true);
}

TableOfContentsConfigure::~TableOfContentsConfigure()
{
}

void TableOfContentsConfigure::setDisplay()
{
    document=m_textEditor->document();
    int i=0;
    for (QTextBlock it = document->begin(); it != document->end(); it = it.next(), i++)
    {
        if (it.blockFormat().hasProperty(KoParagraphStyle::TableOfContentsDocument)) {
            KoTableOfContentsGeneratorInfo *info = it.blockFormat().property(KoParagraphStyle::TableOfContentsData).value<KoTableOfContentsGeneratorInfo*>();
            QListWidgetItem *tocItem= new QListWidgetItem(info->m_name, ui.tocList);
            tocItem->setData(Qt::UserRole+1, QVariant::fromValue<KoTableOfContentsGeneratorInfo*>(info));
            tocItem->setData(Qt::UserRole+2, QVariant::fromValue<QTextBlock>(it));

            ui.tocList->addItem(tocItem);
        }
    }

    ui.tocList->setCurrentRow(0);
}

void TableOfContentsConfigure::save()
{

    KoTableOfContentsGeneratorInfo *info=ui.tocList->item(0)->data(Qt::UserRole+1).value<KoTableOfContentsGeneratorInfo*>();
    info->m_name=ui.lineEditTitle->text();
    info->m_indexTitleTemplate.text=ui.lineEditTitle->text();
    info->m_useOutlineLevel=ui.useOutline->checkState()==Qt::Checked?true:false;

    m_textEditor->updateTableOfContents(info,ui.tocList->item(0)->data(Qt::UserRole+2).value<QTextBlock>());
}

void TableOfContentsConfigure::tocListIndexChanged(int index)
{
    KoTableOfContentsGeneratorInfo *info = ui.tocList->item(index)->data(Qt::UserRole+1).value<KoTableOfContentsGeneratorInfo*>();
    ui.lineEditTitle->setText(info->m_indexTitleTemplate.text);
    ui.useOutline->setCheckState(info->m_useOutlineLevel?Qt::Checked:Qt::Unchecked);
}

void TableOfContentsConfigure::showStyleConfiguration(bool show)
{
    Q_ASSERT(document);
    if(!m_tocStyleConfigure) {
        m_tocStyleConfigure=new TableOfContentsStyleConfigure(KoTextDocument(document).styleManager(), this);
    }
m_tocStyleConfigure->initializeUi();

}
