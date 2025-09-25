/* This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "SvgTextEditor.h"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QBuffer>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFontComboBox>
#include <QFontDatabase>
#include <QFormLayout>
#include <QLineEdit>
#include <QListView>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QPalette>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QSvgGenerator>
#include <QTabWidget>
#include <QTextEdit>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidgetAction>
#include <QScreen>
#include <QScreen>

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include <kactioncollection.h>
#include <kxmlguifactory.h>
#include <ktoolbar.h>
#include <ktoggleaction.h>
#include <kguiitem.h>
#include <kstandardguiitem.h>

#include <KoDialog.h>
#include <KoResourcePaths.h>
#include <KoSvgTextShape.h>
#include <KoSvgTextShapeMarkupConverter.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorPopupAction.h>
#include <svg/SvgUtil.h>
#include <KisPortingUtils.h>

#include <KisSpinBoxI18nHelper.h>
#include <KisScreenColorSampler.h>
#include <kis_icon.h>
#include <kis_config.h>
#include <kis_file_name_requester.h>
#include <kis_action_registry.h>

#include "kis_signals_blocker.h"

class SvgTextEditor::Private
{
public:

    Private()
    {
    }

private:

};


SvgTextEditor::SvgTextEditor(QWidget *parent, Qt::WindowFlags flags)
    : KXmlGuiWindow(parent, flags)
    , m_page(new QWidget(this))
    , d(new Private())
{
    m_textEditorWidget.setupUi(m_page);
    setCentralWidget(m_page);

    KGuiItem::assign(m_textEditorWidget.buttons->button(QDialogButtonBox::Save), KStandardGuiItem::save());
    KGuiItem::assign(m_textEditorWidget.buttons->button(QDialogButtonBox::Cancel), KStandardGuiItem::cancel());
    connect(m_textEditorWidget.buttons, SIGNAL(accepted()), this, SLOT(save()));
    connect(m_textEditorWidget.buttons, SIGNAL(rejected()), this, SLOT(slotCloseEditor()));
    connect(m_textEditorWidget.buttons, SIGNAL(clicked(QAbstractButton*)), this, SLOT(dialogButtonClicked(QAbstractButton*)));

    KConfigGroup cg(KSharedConfig::openConfig(), "SvgTextTool");
    actionCollection()->setConfigGroup("SvgTextTool");
    actionCollection()->setComponentName("svgtexttool");
    actionCollection()->setComponentDisplayName(i18n("Text Tool"));

    if (cg.hasKey("WindowState")) {
        QByteArray state = cg.readEntry("State", QByteArray());
        // One day will need to load the version number, but for now, assume 0
        restoreState(QByteArray::fromBase64(state));
    }
    if (cg.hasKey("Geometry")) {
        QByteArray ba = cg.readEntry("Geometry", QByteArray());
        restoreGeometry(QByteArray::fromBase64(ba));
    }
    else {
        const int scnum = KisPortingUtils::getScreenNumberForWidget(QApplication::activeWindow());
        QRect desk = QGuiApplication::screens().at(scnum)->availableGeometry();

        quint32 x = desk.x();
        quint32 y = desk.y();
        quint32 w = 0;
        quint32 h = 0;
        const int deskWidth = desk.width();
        w = (deskWidth / 3) * 2;
        h = (desk.height() / 3) * 2;
        x += (desk.width() - w) / 2;
        y += (desk.height() - h) / 2;

        move(x,y);
        setGeometry(geometry().x(), geometry().y(), w, h);

    }

    setAcceptDrops(true);
    //setStandardToolBarMenuEnabled(true);
#ifdef Q_OS_MACOS
    setUnifiedTitleAndToolBarOnMac(true);
#endif
    setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);

    m_syntaxHighlighter = new BasicXMLSyntaxHighlighter(m_textEditorWidget.svgTextEdit);
    m_textEditorWidget.svgTextEdit->setFont(QFontDatabase().systemFont(QFontDatabase::FixedFont));

    createActions();
    // If we have customized the toolbars, load that first
    setLocalXMLFile(KoResourcePaths::locateLocal("data", "svgtexttool.xmlgui"));
    setXMLFile(":/kxmlgui5/svgtexttool.xmlgui");

    guiFactory()->addClient(this);

    applySettings();

}

SvgTextEditor::~SvgTextEditor()
{
    KConfigGroup g(KSharedConfig::openConfig(), "SvgTextTool");
    QByteArray ba = saveState();
    g.writeEntry("windowState", ba.toBase64());
    ba = saveGeometry();
    g.writeEntry("Geometry", ba.toBase64());
}


void SvgTextEditor::setInitialShape(KoSvgTextShape *shape)
{
    m_shape = shape;
    if (m_shape) {
        KoSvgTextShapeMarkupConverter converter(m_shape);

        QString svg;
        QString styles;

        if (converter.convertToSvg(&svg, &styles)) {
            m_textEditorWidget.svgTextEdit->setPlainText(svg);
            m_textEditorWidget.svgStylesEdit->setPlainText(styles);
            m_textEditorWidget.svgTextEdit->document()->setModified(false);
        }
        else {
            QMessageBox::warning(this, i18n("Conversion failed"), "Could not get svg text from the shape:\n" + converter.errors().join('\n') + "\n" + converter.warnings().join('\n'));
        }
    }
}

void SvgTextEditor::save()
{
    if (m_shape) {
        Q_EMIT textUpdated(m_shape, m_textEditorWidget.svgTextEdit->document()->toPlainText(), m_textEditorWidget.svgStylesEdit->document()->toPlainText());
        m_textEditorWidget.svgTextEdit->document()->setModified(false);
    }
}

void SvgTextEditor::undo()
{
    m_currentEditor->undo();
}

void SvgTextEditor::redo()
{
    m_currentEditor->redo();
}

void SvgTextEditor::cut()
{
    m_currentEditor->cut();
}

void SvgTextEditor::copy()
{
    m_currentEditor->copy();
}

void SvgTextEditor::paste()
{
    m_currentEditor->paste();
}

void SvgTextEditor::selectAll()
{
    m_currentEditor->selectAll();
}

void SvgTextEditor::deselect()
{
    QTextCursor cursor(m_currentEditor->textCursor());
    cursor.clearSelection();
    m_currentEditor->setTextCursor(cursor);
}

void SvgTextEditor::find()
{
    QDialog findDialog;
    findDialog.setWindowTitle(i18n("Find Text"));
    QFormLayout *layout = new QFormLayout(&findDialog);
    QLineEdit *lnSearchKey = new QLineEdit();
    layout->addRow(i18n("Find:"), lnSearchKey);
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    layout->addWidget(buttons);

    KGuiItem::assign(buttons->button(QDialogButtonBox::Ok), KStandardGuiItem::ok());
    KGuiItem::assign(buttons->button(QDialogButtonBox::Cancel), KStandardGuiItem::cancel());

    connect(buttons, SIGNAL(accepted()), &findDialog, SLOT(accept()));
    connect(buttons, SIGNAL(rejected()), &findDialog, SLOT(reject()));

    if (findDialog.exec() == QDialog::Accepted) {
        m_searchKey = lnSearchKey->text();
        m_currentEditor->find(m_searchKey);
    }
}

void SvgTextEditor::findNext()
{
    if (!m_currentEditor->find(m_searchKey)) {
        QTextCursor cursor(m_currentEditor->textCursor());
        cursor.movePosition(QTextCursor::Start);
        m_currentEditor->setTextCursor(cursor);
        m_currentEditor->find(m_searchKey);
    }
}

void SvgTextEditor::findPrev()
{
    if (!m_currentEditor->find(m_searchKey,QTextDocument::FindBackward)) {
        QTextCursor cursor(m_currentEditor->textCursor());
        cursor.movePosition(QTextCursor::End);
        m_currentEditor->setTextCursor(cursor);
        m_currentEditor->find(m_searchKey,QTextDocument::FindBackward);
    }
}

void SvgTextEditor::replace()
{
    QDialog findDialog;
    findDialog.setWindowTitle(i18n("Find and Replace all"));
    QFormLayout *layout = new QFormLayout(&findDialog);
    QLineEdit *lnSearchKey = new QLineEdit();
    QLineEdit *lnReplaceKey = new QLineEdit();
    layout->addRow(i18n("Find:"), lnSearchKey);
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    layout->addRow(i18n("Replace:"), lnReplaceKey);
    layout->addWidget(buttons);

    KGuiItem::assign(buttons->button(QDialogButtonBox::Ok), KStandardGuiItem::ok());
    KGuiItem::assign(buttons->button(QDialogButtonBox::Cancel), KStandardGuiItem::cancel());

    connect(buttons, SIGNAL(accepted()), &findDialog, SLOT(accept()));
    connect(buttons, SIGNAL(rejected()), &findDialog, SLOT(reject()));

    if (findDialog.exec() == QDialog::Accepted) {
        QString search = lnSearchKey->text();
        QString replace = lnReplaceKey->text();
        QTextCursor cursor(m_currentEditor->textCursor());
        cursor.movePosition(QTextCursor::Start);
        m_currentEditor->setTextCursor(cursor);
        while(m_currentEditor->find(search)) {
            m_currentEditor->textCursor().removeSelectedText();
            m_currentEditor->textCursor().insertText(replace);
        }

    }
}


void SvgTextEditor::zoomOut()
{
    m_currentEditor->zoomOut();
}

void SvgTextEditor::zoomIn()
{
    m_currentEditor->zoomIn();
}

void SvgTextEditor::setSettings()
{
    KoDialog settingsDialog(this);
    Ui_WdgSvgTextSettings textSettings;
    QWidget *settingsPage = new QWidget(&settingsDialog);
    settingsDialog.setMainWidget(settingsPage);
    textSettings.setupUi(settingsPage);

    // get the settings and initialize the dialog
    KConfigGroup cfg(KSharedConfig::openConfig(), "SvgTextTool");

    QColor background = cfg.readEntry("colorEditorBackground", qApp->palette().window().color());
    textSettings.colorEditorBackground->setColor(background);
    textSettings.colorEditorForeground->setColor(cfg.readEntry("colorEditorForeground", qApp->palette().text().color()));

    textSettings.colorKeyword->setColor(cfg.readEntry("colorKeyword", QColor(background.value() < 100 ? Qt::cyan : Qt::blue)));
    textSettings.chkBoldKeyword->setChecked(cfg.readEntry("BoldKeyword", true));
    textSettings.chkItalicKeyword->setChecked(cfg.readEntry("ItalicKeyword", false));

    textSettings.colorElement->setColor(cfg.readEntry("colorElement", QColor(background.value() < 100 ? Qt::magenta : Qt::darkMagenta)));
    textSettings.chkBoldElement->setChecked(cfg.readEntry("BoldElement", true));
    textSettings.chkItalicElement->setChecked(cfg.readEntry("ItalicElement", false));

    textSettings.colorAttribute->setColor(cfg.readEntry("colorAttribute", QColor(background.value() < 100 ? Qt::green : Qt::darkGreen)));
    textSettings.chkBoldAttribute->setChecked(cfg.readEntry("BoldAttribute", true));
    textSettings.chkItalicAttribute->setChecked(cfg.readEntry("ItalicAttribute", true));

    textSettings.colorValue->setColor(cfg.readEntry("colorValue", QColor(background.value() < 100 ? Qt::red: Qt::darkRed)));
    textSettings.chkBoldValue->setChecked(cfg.readEntry("BoldValue", true));
    textSettings.chkItalicValue->setChecked(cfg.readEntry("ItalicValue", false));

    textSettings.colorComment->setColor(cfg.readEntry("colorComment", QColor(background.value() < 100 ? Qt::lightGray : Qt::gray)));
    textSettings.chkBoldComment->setChecked(cfg.readEntry("BoldComment", false));
    textSettings.chkItalicComment->setChecked(cfg.readEntry("ItalicComment", false));

    settingsDialog.setButtons(KoDialog::Ok | KoDialog::Cancel);
    if (settingsDialog.exec() == QDialog::Accepted) {

        cfg.writeEntry("colorEditorBackground", textSettings.colorEditorBackground->color());
        cfg.writeEntry("colorEditorForeground", textSettings.colorEditorForeground->color());

        cfg.writeEntry("colorKeyword", textSettings.colorKeyword->color());
        cfg.writeEntry("BoldKeyword", textSettings.chkBoldKeyword->isChecked());
        cfg.writeEntry("ItalicKeyWord", textSettings.chkItalicKeyword->isChecked());

        cfg.writeEntry("colorElement", textSettings.colorElement->color());
        cfg.writeEntry("BoldElement", textSettings.chkBoldElement->isChecked());
        cfg.writeEntry("ItalicElement", textSettings.chkItalicElement->isChecked());

        cfg.writeEntry("colorAttribute", textSettings.colorAttribute->color());
        cfg.writeEntry("BoldAttribute", textSettings.chkBoldAttribute->isChecked());
        cfg.writeEntry("ItalicAttribute", textSettings.chkItalicAttribute->isChecked());

        cfg.writeEntry("colorValue", textSettings.colorValue->color());
        cfg.writeEntry("BoldValue", textSettings.chkBoldValue->isChecked());
        cfg.writeEntry("ItalicValue", textSettings.chkItalicValue->isChecked());

        cfg.writeEntry("colorComment", textSettings.colorComment->color());
        cfg.writeEntry("BoldComment", textSettings.chkBoldComment->isChecked());
        cfg.writeEntry("ItalicComment", textSettings.chkItalicComment->isChecked());

        applySettings();
    }
}


void SvgTextEditor::setModified(bool modified)
{
    if (modified) {
        m_textEditorWidget.buttons->setStandardButtons(QDialogButtonBox::Save | QDialogButtonBox::Discard);
        KGuiItem::assign(m_textEditorWidget.buttons->button(QDialogButtonBox::Save), KStandardGuiItem::save());
        KGuiItem::assign(m_textEditorWidget.buttons->button(QDialogButtonBox::Discard), KStandardGuiItem::discard());
    }
    else {
        m_textEditorWidget.buttons->setStandardButtons(QDialogButtonBox::Save | QDialogButtonBox::Close);
        KGuiItem::assign(m_textEditorWidget.buttons->button(QDialogButtonBox::Save), KStandardGuiItem::save());
        KGuiItem::assign(m_textEditorWidget.buttons->button(QDialogButtonBox::Close), KStandardGuiItem::close());
    }
}

void SvgTextEditor::dialogButtonClicked(QAbstractButton *button)
{
    if (m_textEditorWidget.buttons->standardButton(button) == QDialogButtonBox::Discard) {
        if (QMessageBox::warning(this, i18nc("@title:window", "Krita"), i18n("You have modified the text. Discard changes?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            close();
        }
    }
}

void SvgTextEditor::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        int numDegrees = event->angleDelta().y() / 8;
        int numSteps = numDegrees / 7;
        m_textEditorWidget.svgTextEdit->zoomOut(numSteps);
        event->accept();
    }
}

void SvgTextEditor::applySettings()
{
    KConfigGroup cfg(KSharedConfig::openConfig(), "SvgTextTool");

    m_page->setUpdatesEnabled(false);

    m_syntaxHighlighter->setFormats();

    QPalette palette = m_textEditorWidget.svgTextEdit->palette();

    QColor background = cfg.readEntry("colorEditorBackground", qApp->palette().window().color());
    palette.setBrush(QPalette::Active, QPalette::Window, QBrush(background));
    m_textEditorWidget.svgStylesEdit->setStyleSheet(QString("background-color:%1").arg(background.name()));
    m_textEditorWidget.svgTextEdit->setStyleSheet(QString("background-color:%1").arg(background.name()));

    QColor foreground = cfg.readEntry("colorEditorForeground", qApp->palette().text().color());
    palette.setBrush(QPalette::Active, QPalette::Text, QBrush(foreground));

    m_page->setUpdatesEnabled(true);
}

QAction *SvgTextEditor::createAction(const QString &name, const char *member)
{
    QAction *action = new QAction(this);
    KisActionRegistry *actionRegistry = KisActionRegistry::instance();
    actionRegistry->propertizeAction(name, action);

    actionCollection()->addAction(name, action);
    QObject::connect(action, SIGNAL(triggered(bool)), this, member);
    return action;
}


void SvgTextEditor::createActions()
{
    // File: new, open, save, save as, close
    KStandardAction::save(this, SLOT(save()), actionCollection());
    KStandardAction::close(this, SLOT(slotCloseEditor()), actionCollection());

    // Edit
    KStandardAction::undo(this, SLOT(undo()), actionCollection());
    KStandardAction::redo(this, SLOT(redo()), actionCollection());
    KStandardAction::cut(this, SLOT(cut()), actionCollection());
    KStandardAction::copy(this, SLOT(copy()), actionCollection());
    KStandardAction::paste(this, SLOT(paste()), actionCollection());
    KStandardAction::selectAll(this, SLOT(selectAll()), actionCollection());
    KStandardAction::deselect(this, SLOT(deselect()), actionCollection());
    KStandardAction::find(this, SLOT(find()), actionCollection());
    KStandardAction::findNext(this, SLOT(findNext()), actionCollection());
    KStandardAction::findPrev(this, SLOT(findPrev()), actionCollection());
    KStandardAction::replace(this, SLOT(replace()), actionCollection());

    // View
    // WISH: we cannot zoom-in/out in rech-text mode
    m_svgTextActions << KStandardAction::zoomOut(this, SLOT(zoomOut()), actionCollection());
    m_svgTextActions << KStandardAction::zoomIn(this, SLOT(zoomIn()), actionCollection());

    Q_FOREACH(QAction *action, m_svgTextActions) {
        action->setEnabled(true);
    }

    // Settings
    // do not add settings action to m_richTextActions list,
    // it should always be active, regardless of which editor mode is used.
    // otherwise we can lock the user out of being able to change
    // editor mode, if user changes to SVG only mode.
    createAction("svg_settings", SLOT(setSettings()));
}
void SvgTextEditor::slotCloseEditor()
{
    close();
    Q_EMIT textEditorClosed();
}

bool SvgTextEditor::eventFilter(QObject *const watched, QEvent *const event)
{
    if (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease) {
        QKeyEvent *const keyEvent = static_cast<QKeyEvent *>(event);
        if ((keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return)
                && (keyEvent->modifiers() & Qt::ShiftModifier)) {
            // Disable soft line breaks
            return true;
        }
    }
    return false;
    return KXmlGuiWindow::eventFilter(watched, event);
}
