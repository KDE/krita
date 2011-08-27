#include "TableOfContentsStyleConfigure.h"
#include "ui_TableOfContentsStyleConfigure.h"

#include "KoStyleManager.h"
#include "KoParagraphStyle.h"

#include <QStandardItemModel>

TableOfContentsStyleConfigure::TableOfContentsStyleConfigure(KoStyleManager *manager, QWidget *parent) :
    QDialog(parent),
    m_stylesTree(0),
    m_styleManager(manager),
    ui(new Ui::TableOfContentsStyleConfigure)
{
    ui->setupUi(this);

    Q_ASSERT(manager);
    ui->stylesAvailableLabel->setText(i18n("Styles available"));
    ui->tocLevels->setText(i18n("Levels"));

    ui->treeView->setHeaderHidden(true);
}

TableOfContentsStyleConfigure::~TableOfContentsStyleConfigure()
{
    delete ui;
}


void TableOfContentsStyleConfigure::initializeUi()
{

    QList<KoParagraphStyle*> paragraphStyleList= m_styleManager->paragraphStyles();

            foreach(KoParagraphStyle *style, paragraphStyleList) {
                ui->listWidget->addItem(style->name());
            }

    if(!m_stylesTree)
    {
        m_stylesTree= new QStandardItemModel(this);
    }
    m_stylesTree->clear();
    m_stylesTree->setColumnCount(1);
    m_stylesTree->setRowCount(10);

    for( int r=0; r<10; r++ )
    {
        QStandardItem *item = new QStandardItem( QString("Level %0").arg(r+1) );

        if(r==0 || r==1 || r==5) {
            for( int i=0; i<3; i++ )
            {
                QStandardItem *child = new QStandardItem( QString("Item %0").arg(i) );
                child->setEditable( false );
                item->appendRow( child );
            }
        }

        m_stylesTree->setItem(r, 0, item);
    }

    ui->treeView->setModel(m_stylesTree);

    //expands the tree only if there are styles at that level
    for( int r=0; r<10; r++ )
    {
        if(m_stylesTree->hasChildren(m_stylesTree->index(r,0))) {
            ui->treeView->setExpanded(m_stylesTree->index(r,0),true);
        } else {
            ui->treeView->setExpanded(m_stylesTree->index(r,0),false);
        }

}

    this->setVisible(true);
}
