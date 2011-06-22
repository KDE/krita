#include "CitationBibliographyWidget.h"
#include "ui_CitationBibliographyWidget.h"
#include "TextTool.h"

#include <KAction>
#include <KDebug>
#include "KoInlineCite.h"
#include "KoTextEditor.h"

#include <QWidget>
#include <QMessageBox>

CitationBibliographyWidget::CitationBibliographyWidget(QTextDocument *doc,QWidget *parent) :
    KDialog(parent),
    m_blockSignals(false),
    document(doc)
{
    widget.setupUi(this);
    setCaption(i18n("Insert Citations/Bibliography"));
    setMainWidget(parent);
    setModal(true);
    connect(widget.buttonBox,SIGNAL(accepted()),this,SLOT(insertCitation()));
    connect(widget.buttonBox,SIGNAL(accepted()),this,SLOT(accept()));
}

void CitationBibliographyWidget::insertCitation()
{
    KoInlineCite *cite = KoTextEditor(document).insertCitation();

    cite->setAddress(widget.address->text());
    cite->setAnnotation(widget.annotation->text());
    cite->setAuthor(widget.author->text());
    cite->setBibliographyType(widget.sourceType->currentText());
    cite->setBookTitle(widget.booktitle->text());
    cite->setChapter(widget.chapter->text());
    cite->setCustom1(widget.ud1->text());
    cite->setCustom2(widget.ud2->text());
    cite->setCustom2(widget.ud3->text());
    cite->setCustom2(widget.ud4->text());
    cite->setCustom2(widget.ud5->text());
    cite->setEdition(widget.edition->text());
    cite->setEditor(widget.editor->text());
    cite->setIdentifier(widget.shortName->text());
    cite->setInstitution(widget.institution->text());
    cite->setISBN(widget.isbn->text());
    cite->setJournal(widget.journal->text());
    cite->setMonth(widget.month->text());
    cite->setNote(widget.note->text());
    cite->setNumber(widget.number->text());
    cite->setOrganisation(widget.organisation->text());
    cite->setPages(widget.pages->text());
    cite->setPublicationType(widget.publication->text());
    cite->setPublisher(widget.publisher->text());
    cite->setReportType(widget.reporttype->text());
    cite->setSchool(widget.school->text());
    cite->setSeries(widget.series->text());
    cite->setTitle(widget.title->text());
    cite->setURL(widget.url->text());
    cite->setVolume(widget.volume->text());
    cite->setYear(widget.year->text());

    //QTextCursor cursor(cite->textFrame()->lastCursorPosition());


    //cursor.insertText(QString("[%1]").arg(widget.shortName->text()));

    //cite->
    //KoInlineNote *note = new KoInlineNote(KoInlineNote::Footnote);
    //note->setMotherFrame(KoTextDocument(document).footNotesFrame());
    //note->setLabel(widget.tagName->text());

    //KoInlineTextObjectManager *manager = KoTextDocument(document).inlineTextObjectManager();

    /*KoTextEditor *handler = qobject_cast<KoTextEditor*> (m_view->canvasBase()->toolProxy()->selection());
    Q_ASSERT(handler);
    QTextCursor *cursor = const_cast<QTextCursor*>(handler->cursor());*/
    //QMessageBox::critical(0,QString("in insertCitation"),QString("in insertCitation"),QMessageBox::Ok);
}

void CitationBibliographyWidget::setStyleManager(KoStyleManager *sm)
{
    m_styleManager = sm;
}
