/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "TextTool.h"
#include "ChangeTracker.h"
#include "PluginHelperAction.h"
#include "dialogs/SimpleStyleWidget.h"
#include "dialogs/StylesWidget.h"
#include "dialogs/ParagraphSettingsDialog.h"
#include "dialogs/StyleManagerDialog.h"
#include "dialogs/InsertCharacter.h"
#include "dialogs/FontDia.h"
#include "commands/TextCommandBase.h"
#include "commands/ChangeListCommand.h"

#include <KoAction.h>
#include <KoExecutePolicy.h>
#include <KoCanvasBase.h>
#include <KoSelection.h>
#include <KoShapeManager.h>
#include <KoPointerEvent.h>
#include <KoCanvasResourceProvider.h>
#include <KoColor.h>
#include <KoColorSetAction.h>
#include <KoColorBackground.h>

#include <KoCharacterStyle.h>
#include <KoTextDocumentLayout.h>
#include <KoInlineNote.h>
#include <KoParagraphStyle.h>
#include <KoTextEditingPlugin.h>
#include <KoTextEditingRegistry.h>
#include <KoTextEditingFactory.h>
#include <KoInlineTextObjectManager.h>
#include <KoListStyle.h>
#include <KoStyleManager.h>
#include <KoTextOdfSaveHelper.h>
#include <KoDrag.h>
#include <KoOdf.h>
#include <KoTextPaste.h>

#include <kdebug.h>
#include <KStandardShortcut>
#include <KFontSizeAction>
#include <KFontChooser>
#include <KFontAction>
#include <KAction>
#include <KStandardAction>
#include <KMimeType>
#include <KMessageBox>
#include <KRun>
#include <QAbstractTextDocumentLayout>
#include <QAction>
#include <QBuffer>
#include <QClipboard>
#include <QKeyEvent>
#include <QPointer>
#include <QTabWidget>
#include <QTextBlock>
#include <QUndoCommand>
#include <KoGenStyles.h>
#include <KoEmbeddedDocumentSaver.h>
#include <KoShapeSavingContext.h>

static bool hit(const QKeySequence &input, KStandardShortcut::StandardShortcut shortcut)
{
    foreach(QKeySequence ks, KStandardShortcut::shortcut(shortcut).toList()) {
        if(input == ks)
            return true;
    }
    return false;
}

static bool isRightToLeft(const QString &text)
{
    int ltr = 0, rtl = 0;

    QString::const_iterator iter = text.begin();
    while(iter != text.end()) {
        switch(QChar::direction((*iter).unicode()))
        {
        case QChar::DirL:
        case QChar::DirLRO:
        case QChar::DirLRE:
            ltr++;
            break;
        case QChar::DirR:
        case QChar::DirAL:
        case QChar::DirRLO:
        case QChar::DirRLE:
            rtl++;
        default:
            break;
        }
        ++iter;
    }
    return ltr < rtl;
}

TextTool::TextTool(KoCanvasBase *canvas)
: KoTool(canvas),
    m_textShape(0),
    m_textShapeData(0),
    m_changeTracker(0),
    m_allowActions(true),
    m_allowAddUndoCommand(true),
    m_trackChanges(false),
    m_allowResourceProviderUpdates(true),
    m_needSpellChecking(true),
    m_processingKeyPress(false),
    m_prevCursorPosition(-1),
    m_spellcheckPlugin(0),
    m_currentCommand(0),
    m_currentCommandHasChildren(false),
    m_specialCharacterDocker(0)
{
    m_actionFormatBold  = new QAction(KIcon("format-text-bold"), i18n("Bold"), this);
    addAction("format_bold", m_actionFormatBold );
    m_actionFormatBold->setShortcut(Qt::CTRL + Qt::Key_B);
    m_actionFormatBold->setCheckable(true);
    connect( m_actionFormatBold, SIGNAL(triggered(bool)), &m_selectionHandler, SLOT(bold(bool)) );

    m_actionFormatItalic  = new QAction(KIcon("format-text-italic"), i18n("Italic"), this);
    addAction("format_italic", m_actionFormatItalic );
    m_actionFormatItalic->setShortcut( Qt::CTRL + Qt::Key_I);
    m_actionFormatItalic->setCheckable(true);
    connect( m_actionFormatItalic, SIGNAL(triggered(bool)), &m_selectionHandler, SLOT(italic(bool)) );

    m_actionFormatUnderline  = new QAction(KIcon("format-text-underline"), i18nc("Text formatting", "Underline"), this);
    addAction("format_underline", m_actionFormatUnderline );
    m_actionFormatUnderline->setShortcut(Qt::CTRL + Qt::Key_U);
    m_actionFormatUnderline->setCheckable(true);
    connect( m_actionFormatUnderline, SIGNAL(triggered(bool)), &m_selectionHandler, SLOT(underline(bool)) );

    m_actionFormatStrikeOut  = new QAction(KIcon("format-text-strikethrough"), i18n("Strike Out"), this);
    addAction("format_strike", m_actionFormatStrikeOut );
    m_actionFormatStrikeOut->setCheckable(true);
    connect( m_actionFormatStrikeOut, SIGNAL(triggered(bool)), &m_selectionHandler, SLOT(strikeOut(bool)) );

    QActionGroup *alignmentGroup = new QActionGroup(this);
    m_actionAlignLeft  = new QAction(KIcon("format-justify-left"), i18n("Align Left"), this);
    addAction("format_alignleft", m_actionAlignLeft );
    m_actionAlignLeft->setShortcut(Qt::CTRL + Qt::Key_L);
    m_actionAlignLeft->setCheckable(true);
    alignmentGroup->addAction(m_actionAlignLeft);
    connect(m_actionAlignLeft, SIGNAL(triggered(bool)), this, SLOT(alignLeft()));

    m_actionAlignRight  = new QAction(KIcon("format-justify-right"), i18n("Align Right"), this);
    addAction("format_alignright", m_actionAlignRight );
    m_actionAlignRight->setShortcut(Qt::CTRL + Qt::ALT + Qt::Key_R);
    m_actionAlignRight->setCheckable(true);
    alignmentGroup->addAction(m_actionAlignRight);
    connect(m_actionAlignRight, SIGNAL(triggered(bool)), this, SLOT(alignRight()));

    m_actionAlignCenter  = new QAction(KIcon("format-justify-center"), i18n("Align Center"), this);
    addAction("format_aligncenter", m_actionAlignCenter );
    m_actionAlignCenter->setShortcut(Qt::CTRL + Qt::ALT + Qt::Key_C);
    m_actionAlignCenter->setCheckable(true);
    alignmentGroup->addAction(m_actionAlignCenter);
    connect(m_actionAlignCenter, SIGNAL(triggered(bool)), this, SLOT(alignCenter()));

    m_actionAlignBlock  = new QAction(KIcon("format-justify-fill"), i18n("Align Block"), this);
    addAction("format_alignblock", m_actionAlignBlock );
    m_actionAlignBlock->setShortcut(Qt::CTRL + Qt::ALT + Qt::Key_R);
    m_actionAlignBlock->setCheckable(true);
    alignmentGroup->addAction(m_actionAlignBlock);
    connect(m_actionAlignBlock, SIGNAL(triggered(bool)), this, SLOT(alignBlock()));

    m_actionFormatSuper = new QAction(KIcon("format-text-superscript"), i18n("Superscript"), this);
    addAction("format_super", m_actionFormatSuper );
    m_actionFormatSuper->setCheckable(true);
    connect(m_actionFormatSuper, SIGNAL(triggered(bool)), this, SLOT(superScript(bool)));

    m_actionFormatSub = new QAction(KIcon("format-text-subscript"), i18n("Subscript"), this);
    addAction("format_sub", m_actionFormatSub );
    m_actionFormatSub->setCheckable(true);
    connect(m_actionFormatSub, SIGNAL(triggered(bool)), this, SLOT(subScript(bool)));

    QAction *action = new QAction(
            KIcon(QApplication::isRightToLeft() ? "format-indent-less" : "format-indent-more"),
            i18n("Increase Indent"), this);
    addAction("format_increaseindent", action );
    connect(action, SIGNAL(triggered()), this, SLOT(increaseIndent()));

    m_actionFormatDecreaseIndent = new QAction(
            KIcon(QApplication::isRightToLeft() ? "format-indent-more" :"format-indent-less"),
            i18n("Decrease Indent"), this);
    addAction("format_decreaseindent", m_actionFormatDecreaseIndent );
    connect(m_actionFormatDecreaseIndent, SIGNAL(triggered()), this, SLOT(decreaseIndent()));

    action = new QAction(i18n("Increase Font Size"), this);
    action->setShortcut(Qt::CTRL + Qt::Key_Greater);
    addAction("fontsizeup", action);
    connect(action, SIGNAL(triggered()), &m_selectionHandler, SLOT(increaseFontSize()));

    action = new QAction(i18n("Decrease Font Size"), this);
    action->setShortcut(Qt::CTRL + Qt::Key_Less);
    addAction("fontsizedown", action);
    connect(action, SIGNAL(triggered()), &m_selectionHandler, SLOT(decreaseFontSize()));

    m_actionFormatFontFamily = new KFontAction( KFontChooser::SmoothScalableFonts, this);
    addAction( "format_fontfamily", m_actionFormatFontFamily );
    connect( m_actionFormatFontFamily, SIGNAL( triggered( const QString & ) ),
            &m_selectionHandler, SLOT( setFontFamily( const QString & ) ) );

/*
    m_actionFormatStyleMenu  = new KActionMenu(i18n("Style"), this);
    addAction("format_stylemenu", m_actionFormatStyleMenu );
    m_actionFormatStyle  = new KSelectAction(i18n("Style"), this);
    addAction("format_style", m_actionFormatStyle );
    connect( m_actionFormatStyle, SIGNAL( activated( int ) ),
            this, SLOT( textStyleSelected( int ) ) );
    updateStyleList();

    // ----------------------- More format actions, for the toolbar only
    QActionGroup* spacingActionGroup = new QActionGroup( this );
    spacingActionGroup->setExclusive( true );
    m_actionFormatSpacingSingle = new KToggleAction( i18n( "Line Spacing 1" ), "format-line-spacing-simple", Qt::CTRL + Qt::Key_1,
            this, SLOT( textSpacingSingle() ),
            actionCollection(), "format_spacingsingle" );
    m_actionFormatSpacingSingle->setActionGroup( spacingActionGroup );
    m_actionFormatSpacingOneAndHalf = new KToggleAction( i18n( "Line Spacing 1.5" ), "format-line-spacing-double", Qt::CTRL + Qt::Key_5,
            this, SLOT( textSpacingOneAndHalf() ),
            actionCollection(), "format_spacing15" );
    m_actionFormatSpacingOneAndHalf->setActionGroup( spacingActionGroup );
    m_actionFormatSpacingDouble = new KToggleAction( i18n( "Line Spacing 2" ), "format-line-spacing-triple", Qt::CTRL + Qt::Key_2,
            this, SLOT( textSpacingDouble() ),
            actionCollection(), "format_spacingdouble" );
    m_actionFormatSpacingDouble->setActionGroup( spacingActionGroup );

    m_actionFormatColor = new TKSelectColorAction( i18n( "Text Color..." ), TKSelectColorAction::TextColor,
            this, SLOT( textColor() ),
            actionCollection(), "format_color", true );
    m_actionFormatColor->setDefaultColor(QColor());


    m_actionFormatNumber  = new KActionMenu(KIcon( "format-list-ordered" ), i18n("Number"), this);
    addAction("format_number", m_actionFormatNumber );
    m_actionFormatNumber->setDelayed( false );
    m_actionFormatBullet  = new KActionMenu(KIcon( "format-list-unordered" ), i18n("Bullet"), this);
    addAction("format_bullet", m_actionFormatBullet );
    m_actionFormatBullet->setDelayed( false );
    QActionGroup* counterStyleActionGroup = new QActionGroup( this );
    counterStyleActionGroup->setExclusive( true );
    Q3PtrList<KoCounterStyleWidget::StyleRepresenter> stylesList;
    KoCounterStyleWidget::makeCounterRepresenterList( stylesList );
    Q3PtrListIterator<KoCounterStyleWidget::StyleRepresenter> styleIt( stylesList );
    for ( ; styleIt.current() ; ++styleIt ) {
        // Dynamically create toggle-actions for each list style.
        // This approach allows to edit toolbars and extract separate actions from this menu
        KToggleAction* act = new KToggleAction( styleIt.current()->name(), // TODO icon
                actionCollection(),
                QString("counterstyle_%1").arg( styleIt.current()->style() ) );
        connect( act, SIGNAL( triggered(bool) ), this, SLOT( slotCounterStyleSelected() ) );
        act->setActionGroup( counterStyleActionGroup );
        // Add to the right menu: both for "none", bullet for bullets, numbers otherwise
        if ( styleIt.current()->style() == KoParagCounter::STYLE_NONE ) {
            m_actionFormatBullet->insert( act );
            m_actionFormatNumber->insert( act );
        } else if ( styleIt.current()->isBullet() )
            m_actionFormatBullet->insert( act );
        else
            m_actionFormatNumber->insert( act );
    }
*/


    // ------------------- Actions with a key binding and no GUI item
    action  = new QAction(i18n("Insert Non-Breaking Space"), this);
    addAction("nonbreaking_space", action );
    action->setShortcut( Qt::CTRL+Qt::Key_Space);
    connect(action, SIGNAL(triggered()), this, SLOT( nonbreakingSpace() ));

    action  = new QAction(i18n("Insert Non-Breaking Hyphen"), this);
    addAction("nonbreaking_hyphen", action );
    action->setShortcut( Qt::CTRL+Qt::SHIFT+Qt::Key_Minus);
    connect(action, SIGNAL(triggered()), this, SLOT( nonbreakingHyphen() ));

    action  = new QAction(i18n("Insert Index"), this);
action->setShortcut( Qt::CTRL+ Qt::Key_T);
    addAction("insert_index", action );
    connect(action, SIGNAL(triggered()), this, SLOT( insertIndexMarker() ));

    action  = new QAction(i18n("Insert Soft Hyphen"), this);
    addAction("soft_hyphen", action );
    //action->setShortcut( Qt::CTRL+Qt::Key_Minus); // TODO this one is also used for the kde-global zoom-out :(
    connect(action, SIGNAL(triggered()), this, SLOT( softHyphen() ));

    action  = new QAction(i18n("Line Break"), this);
    addAction("line_break", action );
    action->setShortcut( Qt::SHIFT+Qt::Key_Return);
    connect(action, SIGNAL(triggered()), this, SLOT( lineBreak() ));

    action  = new QAction(i18n("Font..."), this);
    addAction("format_font", action );
    action->setShortcut( Qt::ALT + Qt::CTRL + Qt::Key_F);
    action->setToolTip( i18n( "Change character size, font, boldface, italics etc." ) );
    action->setWhatsThis( i18n( "Change the attributes of the currently selected characters." ) );
    connect(action, SIGNAL(triggered()), this, SLOT( selectFont() ));

    m_actionFormatFontSize = new KFontSizeAction( i18n( "Font Size" ), this);
    addAction("format_fontsize", m_actionFormatFontSize );
    connect( m_actionFormatFontSize, SIGNAL(fontSizeChanged( int )), &m_selectionHandler, SLOT(setFontSize(int)) );

    m_actionFormatTextColor = new KoColorSetAction(this);
    m_actionFormatTextColor->setIcon(KIcon("textcolor"));
    m_actionFormatTextColor->setToolTip(i18n("Text Color..."));
    addAction("format_textcolor", m_actionFormatTextColor);
    connect(m_actionFormatTextColor, SIGNAL(colorChanged(const KoColor &)), this, SLOT(setTextColor(const KoColor &)) );

    m_actionFormatBackgroundColor = new KoColorSetAction(this);
    m_actionFormatBackgroundColor->setIcon(KIcon("format-fill-color"));
    m_actionFormatBackgroundColor->setToolTip(i18n("Background Color..."));
    m_actionFormatBackgroundColor->setText( i18n( "Background" ) );
    addAction("format_backgroundcolor", m_actionFormatBackgroundColor);
    connect(m_actionFormatBackgroundColor, SIGNAL(colorChanged(const KoColor &)), this, SLOT(setBackgroundColor(const KoColor &)) );

    action = new QAction(i18n("Default Format"), this);
    addAction("text_default", action);
    action->setToolTip( i18n( "Change font and paragraph attributes to their default values" ) );
    connect(action, SIGNAL(triggered()), this, SLOT( textDefaultFormat() ));

    foreach(QString key, KoTextEditingRegistry::instance()->keys()) {
        KoTextEditingFactory *factory =  KoTextEditingRegistry::instance()->value(key);
        Q_ASSERT(factory);
        if(m_textEditingPlugins.contains(factory->id())) {
            kWarning(32500) << "Duplicate id for textEditingPlugin, ignoring one! (" << factory->id() << ")\n";
            continue;
        }
        QString factoryId = factory->id();
        KoTextEditingPlugin *plugin = factory->create();
        if (factoryId == "spellcheck") {
            kDebug(32500) << "KOffice SpellCheck plugin found";
            m_spellcheckPlugin = plugin;
            connect(m_canvas->resourceProvider(), SIGNAL(resourceChanged(int, const QVariant &)),
                m_spellcheckPlugin, SLOT(resourceChanged(int, const QVariant &)));
        }
        m_textEditingPlugins.insert(factory->id(), plugin);
    }

    foreach (KoTextEditingPlugin* plugin, m_textEditingPlugins.values()) {
        connect(plugin, SIGNAL(startMacro(const QString &)), this, SLOT(startMacro(const QString &)));
        connect(plugin, SIGNAL(stopMacro()), this, SLOT(stopMacro()));
        QHash<QString, QAction*> actions = plugin->actions();
        QHash<QString, QAction*>::iterator i = actions.begin();
        while (i != actions.end()) {
            addAction(i.key(), i.value());
            i++;
        }
    }

    action = new QAction(i18n("Paragraph..."), this);
    addAction("format_paragraph", action);
    action->setShortcut(Qt::ALT + Qt::CTRL + Qt::Key_P);
    action->setToolTip( i18n( "Change paragraph margins, text flow, borders, bullets, numbering etc." ) );
    action->setWhatsThis( i18n( "Change paragraph margins, text flow, borders, bullets, numbering etc.<p>Select text in multiple paragraphs to change the formatting of all selected paragraphs.<p>If no text is selected, the paragraph where the cursor is located will be changed.</p>" ) );
    connect(action, SIGNAL(triggered()), this, SLOT(formatParagraph()));

    action = new QAction(i18n("Record"), this);
    action->setCheckable(true);
    addAction("edit_record_changes", action);
    connect(action, SIGNAL(triggered(bool)), this, SLOT(toggleTrackChanges(bool)));

    action = new QAction(i18n("Style Manager"), this);
    action->setShortcut( Qt::ALT + Qt::CTRL + Qt::Key_S);
    action->setToolTip( i18n( "Change attributes of styles" ) );
    action->setWhatsThis( i18n( "Change font and paragraph attributes of styles.<p>Multiple styles can be changed using the dialog box." ) );
    addAction("format_stylist", action);
    connect(action, SIGNAL(triggered()), this, SLOT(showStyleManager()));

    action = KStandardAction::selectAll(this, SLOT(selectAll()), this);
    addAction("edit_selectall", action);

    action = new QAction(i18n( "Special Character..." ), this);
    action->setIcon(KIcon("character-set"));
    action->setShortcut(Qt::ALT + Qt::SHIFT + Qt::Key_C);
    addAction("insert_specialchar", action);
    action->setToolTip( i18n( "Insert one or more symbols or characters not found on the keyboard" ) );
    action->setWhatsThis( i18n( "Insert one or more symbols or characters not found on the keyboard." ) );
    connect(action, SIGNAL(triggered()), this, SLOT( insertSpecialCharacter() ));

    m_updateParagDirection.action = new KoAction(this);
    m_updateParagDirection.action->setExecutePolicy(KoExecutePolicy::onlyLastPolicy);
    connect(m_updateParagDirection.action, SIGNAL(triggered(const QVariant &)),
            this, SLOT(updateParagraphDirection(const QVariant&)), Qt::DirectConnection);
    connect(m_updateParagDirection.action, SIGNAL(updateUi(const QVariant &)),
            this, SLOT(updateParagraphDirectionUi()));


    // setup the context list.
    QList<QAction*> list;
    list.append(this->action("text_default"));
    list.append(this->action("format_font"));
    foreach(QString key, KoTextEditingRegistry::instance()->keys()) {
        KoTextEditingFactory *factory =  KoTextEditingRegistry::instance()->value(key);
        if(factory->showInMenu())
            list.append(new PluginHelperAction(factory->title(), this, factory->id()));
    }
    setPopupActionList(list);

    connect(&m_selectionHandler, SIGNAL(startMacro(const QString&)), this, SLOT(startMacro(const QString&)));
    connect(&m_selectionHandler, SIGNAL(stopMacro()), this, SLOT(stopMacro()));
    connect(m_canvas->shapeManager()->selection(), SIGNAL(selectionChanged()), this, SLOT(shapeAddedToCanvas()));
}

TextTool::~TextTool()
{
    qDeleteAll(m_textEditingPlugins);
}

void TextTool::paint( QPainter &painter, const KoViewConverter &converter)
{
    QTextBlock block = m_textCursor.block();
    if(! block.layout()) // not layouted yet.  The Shape paint method will trigger a layout
        return;
    if (m_textShapeData == 0)
        return;

    int selectStart = m_textCursor.position();
    int selectEnd = m_textCursor.anchor();
    if (selectEnd < selectStart)
        qSwap(selectStart, selectEnd);
    QList<TextShape *> shapesToPaint;
    KoTextDocumentLayout *lay = dynamic_cast<KoTextDocumentLayout*> ( m_textShapeData->document()->documentLayout());
    if (lay) {
        foreach( KoShape *shape, lay->shapes()) {
            TextShape *ts = dynamic_cast<TextShape*> (shape);
            if (! ts)
                continue;
            KoTextShapeData *data = ts->textShapeData();
            // check if shape contains some of the selection, if not, skip
            if (! (data->endPosition() >= selectStart && data->position()  <= selectEnd
                    || data->position() <= selectStart && data->endPosition() >= selectEnd))
                continue;
            if (painter.hasClipping()) {
                QRect rect = converter.documentToView(ts->boundingRect()).toRect();
                if(painter.clipRegion().intersect( QRegion(rect) ).isEmpty())
                    continue;
            }
            shapesToPaint << ts;
        }
    }
    if (shapesToPaint.isEmpty()) // quite unlikely, though ;)
        return;

    double zoomX, zoomY;
    converter.zoom(&zoomX, &zoomY);
    painter.scale(zoomX, zoomY);

    QAbstractTextDocumentLayout::PaintContext pc;
    QAbstractTextDocumentLayout::Selection selection;
    selection.cursor = m_textCursor;
    selection.format.setBackground( m_canvas->canvasWidget()->palette().brush(QPalette::Highlight) );
    selection.format.setForeground( m_canvas->canvasWidget()->palette().brush(QPalette::HighlightedText) );
    pc.selections.append(selection);
    foreach (TextShape *ts, shapesToPaint) {
        KoTextShapeData *data = ts->textShapeData();
        Q_ASSERT(data);

        painter.save();
        painter.setMatrix( painter.matrix() * ts->absoluteTransformation(&converter) );
        painter.translate(0, -data->documentOffset());
        QRectF clip = textRect(qMax(data->position(), selectStart), qMin(data->endPosition(), selectEnd));
        painter.setClipRect(clip, Qt::IntersectClip);
        data->document()->documentLayout()->draw( &painter, pc);
        if (data == m_textShapeData) {
            // paint caret
            QPen caretPen(Qt::black);
            if(! m_textShape->hasTransparency()) {
                KoColorBackground * fill = dynamic_cast<KoColorBackground*>( m_textShape->background() );
                if( fill )
                {
                    QColor bg = fill->color();
                    QColor invert = QColor(255 - bg.red(), 255 - bg.green(), 255 - bg.blue());
                    caretPen.setColor(invert);
                }
            }
            painter.setPen(caretPen);
            const int posInParag = m_textCursor.position() - block.position();
            block.layout()->drawCursor(&painter, QPointF(0,0), posInParag);
        }

        painter.restore();
    }
}

void TextTool::mousePressEvent( KoPointerEvent *event )
{
    const bool canMoveCaret = !m_textCursor.hasSelection() || event->button() !=  Qt::RightButton;

    if(canMoveCaret && ! m_textShape->boundingRect().contains(event->point)) {
        QRectF area(event->point, QSizeF(1,1));
        repaintSelection();
        foreach(KoShape *shape, m_canvas->shapeManager()->shapesAt(area, true)) {
            TextShape *textShape = dynamic_cast<TextShape*> (shape);
            if(textShape) {
                m_textShape = textShape;
                KoTextShapeData *d = static_cast<KoTextShapeData*> (textShape->userData());
                if(d->document() == m_textShapeData->document())
                    break; // stop looking.
            }
        }
        setShapeData(static_cast<KoTextShapeData*> (m_textShape->userData()));
    }
    KoSelection *selection = m_canvas->shapeManager()->selection();
    if(!selection->isSelected(m_textShape)) {
        selection->deselectAll();
        selection->select(m_textShape);
    }

    if(canMoveCaret) {
        bool shiftPressed = event->modifiers() & Qt::ShiftModifier;
        if(m_textCursor.hasSelection() && !shiftPressed)
            repaintSelection(); // will erase selection
        else if(! m_textCursor.hasSelection())
            repaintCaret();
        int prevPosition = m_textCursor.position();
        int position = pointToPosition(event->point);
        m_textCursor.setPosition(position, shiftPressed ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor);
        if(shiftPressed) // altered selection.
            repaintSelection(prevPosition, m_textCursor.position());
        else
            repaintCaret();

        updateSelectionHandler();
        updateStyleManager();
    }
    updateActions();

    if(event->button() ==  Qt::MidButton) { // Paste
        QClipboard *clipboard = QApplication::clipboard();
        QString paste = clipboard->text(QClipboard::Selection);
        if(! paste.isEmpty()) {
            if (m_textCursor.hasSelection())
                m_selectionHandler.deleteInlineObjects();
            m_textCursor.insertText(paste);
            ensureCursorVisible();
            editingPluginEvents();
            emit blockChanged(m_textCursor.block());
        }
    } else {
        // Is there an anchor here ?
        if (m_textCursor.charFormat().isAnchor()) {
            QString anchor = m_textCursor.charFormat().anchorHref();
            bool isLocalLink = (anchor.indexOf("file:") == 0);
            QString type = KMimeType::findByUrl(anchor, 0, isLocalLink)->name();

            if ( KRun::isExecutableFile( anchor, type ) )
            {
                QString question = i18n("This link points to the program or script '%1'.\n"
                        "Malicious programs can harm your computer. "
                        "Are you sure that you want to run this program?", anchor);
                // this will also start local programs, so adding a "don't warn again"
                // checkbox will probably be too dangerous
                int choice = KMessageBox::warningYesNo(0, question, i18n("Open Link?"));
                if ( choice != KMessageBox::Yes )
                    return;
            }
            new KRun(m_textCursor.charFormat().anchorHref(), 0);
        }
        else
            event->ignore(); // allow the event to be used by another
    }
}

void TextTool::setShapeData(KoTextShapeData *data)
{
    bool docChanged = data == 0 || m_textShapeData == 0 || m_textShapeData->document() != data->document();
    if(m_textShapeData && docChanged)
        disconnect(m_textShapeData->document(), SIGNAL(undoCommandAdded()), this, SLOT(addUndoCommand()));
    m_textShapeData = data;
    if (m_textShapeData == 0)
        return;
    if(docChanged) {
        connect(m_textShapeData->document(), SIGNAL(undoCommandAdded()), this, SLOT(addUndoCommand()));
        m_textCursor = QTextCursor(m_textShapeData->document());

        if(m_textShape->demoText()) {
            m_textShapeData->document()->setUndoRedoEnabled(false); // removes undo history
            m_textShape->setDemoText(false); // remove demo text
        }
        m_textShapeData->document()->setUndoRedoEnabled(true); // allow undo history
    }
    if(m_trackChanges) {
        if(m_changeTracker == 0)
            m_changeTracker = new ChangeTracker(this);
        m_changeTracker->setDocument(m_textShapeData->document());
    }
    if(m_spellcheckPlugin)
        m_spellcheckPlugin->checkSection(m_textShapeData->document(), 0, 0);
}

void TextTool::updateSelectionHandler()
{
    m_selectionHandler.setShape(m_textShape);
    m_selectionHandler.setShapeData(m_textShapeData);
    m_selectionHandler.setCaret(&m_textCursor);
    emit selectionChanged(m_textCursor.hasSelection());

    if(m_textCursor.hasSelection()) {
        QClipboard *clipboard = QApplication::clipboard();
        if(clipboard->supportsSelection())
            clipboard->setText(m_textCursor.selectedText(), QClipboard::Selection);
    }
    KoCanvasResourceProvider *p = m_canvas->resourceProvider();
    m_allowResourceProviderUpdates = false;
    if(m_textShapeData) {
        p->setResource(KoText::CurrentTextPosition, m_textCursor.position());
        p->setResource(KoText::CurrentTextAnchor, m_textCursor.anchor());
        QVariant variant;
        variant.setValue<void*>(m_textShapeData->document());
        p->setResource(KoText::CurrentTextDocument, variant);
    }
    else {
        p->clearResource(KoText::CurrentTextPosition);
        p->clearResource(KoText::CurrentTextAnchor);
        p->clearResource(KoText::CurrentTextDocument);
    }
    m_allowResourceProviderUpdates = true;
}

void TextTool::copy() const
{
    if(m_textShapeData == 0 || !m_textCursor.hasSelection()) return;
    int from = m_textCursor.position();
    int to = m_textCursor.anchor();
    KoTextOdfSaveHelper saveHelper( m_textShapeData, from, to );
    KoDrag drag;
    drag.setOdf( KoOdf::mimeType( KoOdf::Text ), saveHelper );
    // TODO add also a text version to the clipboard
    drag.addToClipboard();
}

void TextTool::deleteSelection()
{
    if ( !m_selectionHandler.deleteInlineObjects( false ) || m_textCursor.hasSelection() ) {
        m_textCursor.deleteChar();
    }
    editingPluginEvents();
}

bool TextTool::paste()
{
    if (m_textCursor.hasSelection()) {
        m_selectionHandler.deleteInlineObjects();
    }

    // check for mime type
    const QMimeData *data = QApplication::clipboard()->mimeData();

    if(data->hasFormat("application/vnd.oasis.opendocument.text")) {
        startMacro( "Paste" );
        KoTextPaste paste( m_textShapeData, m_textCursor, m_canvas );
        paste.paste( KoOdf::Text, data );
        stopMacro();
    }
    else if(data->hasHtml()) {
        startMacro( "Paste" );
        m_textCursor.insertHtml(data->html());
        stopMacro();
    }
    else if(data->hasText()) {
        startMacro( "Paste" );
        m_textCursor.insertText(data->text());
        stopMacro();
    }
    else
        return false;

    ensureCursorVisible();
    editingPluginEvents();
    emit blockChanged(m_textCursor.block());
    return true;
}

QStringList TextTool::supportedPasteMimeTypes() const
{
    QStringList list;
    list << "text/plain" << "text/html" << "application/vnd.oasis.opendocument.text";
    return list;
}

int TextTool::pointToPosition(const QPointF & point) const
{
    QPointF p = m_textShape->convertScreenPos(point);
    int caretPos = m_textCursor.block().document()->documentLayout()->hitTest(p, Qt::FuzzyHit);
    caretPos = qMax(caretPos, m_textShapeData->position());
    if(m_textShapeData->endPosition() == -1) {
        kWarning(32500) << "Clicking in not fully laid-out textframe\n";
        m_textShapeData->fireResizeEvent(); // requests a layout run ;)
    }
    caretPos = qMin(caretPos, m_textShapeData->endPosition());
    return caretPos;
}

void TextTool::mouseDoubleClickEvent( KoPointerEvent *event )
{
    if(m_canvas->shapeManager()->shapeAt(event->point) != m_textShape) {
        event->ignore(); // allow the event to be used by another
        return;
    }
    m_textCursor.clearSelection();
    int pos = m_textCursor.position();
    m_textCursor.movePosition(QTextCursor::WordLeft);
    m_textCursor.movePosition(QTextCursor::WordRight, QTextCursor::KeepAnchor);
    if(qAbs(pos - m_textCursor.position()) <= 1) // clicked between two words
        m_textCursor.movePosition(QTextCursor::WordRight, QTextCursor::KeepAnchor);

    repaintSelection();
    updateSelectionHandler();
}

void TextTool::mouseMoveEvent( KoPointerEvent *event )
{
    useCursor(Qt::IBeamCursor);
    int position = pointToPosition(event->point);

    if(event->buttons() == Qt::NoButton) {
        QTextCursor cursor(m_textCursor);
        cursor.setPosition(position);

        if (cursor.charFormat().isAnchor())
            useCursor(Qt::PointingHandCursor);
        else
            useCursor(Qt::IBeamCursor);

        return;
    }

    if(position == m_textCursor.position()) return;
    if(position >= 0) {
        repaintCaret();
        int prevPos = m_textCursor.position();
        m_textCursor.setPosition(position, QTextCursor::KeepAnchor);
        repaintSelection(prevPos, m_textCursor.position());
    }

    updateSelectionHandler();
}

void TextTool::mouseReleaseEvent( KoPointerEvent *event )
{
    event->ignore();
    editingPluginEvents();
}

void TextTool::keyPressEvent(QKeyEvent *event)
{
    int destinationPosition = -1; // for those cases where the moveOperation is not relevant;
    QTextCursor::MoveOperation moveOperation = QTextCursor::NoMove;
    if(event->key() == Qt::Key_Backspace) {
        if(! m_textCursor.hasSelection() && m_textCursor.block().textList() && m_textCursor.block().length() == 1) {
            // backspace on numbered, empty parag, removes numbering.
            ChangeListCommand *clc = new ChangeListCommand(m_textCursor.block(), KoListStyle::NoItem);
            addCommand(clc);
        }
        else {
            if(!m_textCursor.hasSelection() && event->modifiers() & Qt::ControlModifier) // delete prev word.
                m_textCursor.movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor);
            // if the cursor position (no selection) has inline object, the character + inline object
            // is deleted by the InlineTextObjectManager
            if (!m_selectionHandler.deleteInlineObjects(true) || m_textCursor.hasSelection())
                m_textCursor.deletePreviousChar();
            editingPluginEvents();
        }
        ensureCursorVisible();
    }
    else if(event->key() == Qt::Key_Delete) {
        if(!m_textCursor.hasSelection() && event->modifiers() & Qt::ControlModifier) // delete next word.
            m_textCursor.movePosition(QTextCursor::NextWord, QTextCursor::KeepAnchor);
        // the event only gets through when the Del is not used in the app
        // if the app forwards Del then deleteSelection is used
        if (!m_selectionHandler.deleteInlineObjects(false) || m_textCursor.hasSelection())
            m_textCursor.deleteChar();
        editingPluginEvents();
    }
    else if((event->key() == Qt::Key_Left) && (event->modifiers() | Qt::ShiftModifier) == Qt::ShiftModifier)
        moveOperation = QTextCursor::Left;
    else if((event->key() == Qt::Key_Right) && (event->modifiers() | Qt::ShiftModifier) == Qt::ShiftModifier)
        moveOperation = QTextCursor::Right;
    else if((event->key() == Qt::Key_Up) && (event->modifiers() | Qt::ShiftModifier) == Qt::ShiftModifier)
        moveOperation = QTextCursor::Up;
    else if((event->key() == Qt::Key_Down) && (event->modifiers() | Qt::ShiftModifier) == Qt::ShiftModifier)
        moveOperation = QTextCursor::Down;
    else {
        // check for shortcuts.
        QKeySequence item(event->key() | ((Qt::ControlModifier | Qt::AltModifier) & event->modifiers()));
        if(hit(item, KStandardShortcut::Begin))
            // Goto beginning of the document. Default: Ctrl-Home
            destinationPosition = 0;
        else if(hit(item, KStandardShortcut::End)) {
            // Goto end of the document. Default: Ctrl-End
            QTextBlock last = m_textShapeData->document()->end().previous();
            destinationPosition = last.position() + last.length() -1;
        }
        else if(hit(item, KStandardShortcut::Prior)) // page up
            // Scroll up one page. Default: Prior
            moveOperation = QTextCursor::StartOfLine; // TODO
        else if(hit(item, KStandardShortcut::Next))
            // Scroll down one page. Default: Next
            moveOperation = QTextCursor::StartOfLine; // TODO
        else if(hit(item, KStandardShortcut::BeginningOfLine))
            // Goto beginning of current line. Default: Home
            moveOperation = QTextCursor::StartOfLine;
        else if(hit(item, KStandardShortcut::EndOfLine))
            // Goto end of current line. Default: End
            moveOperation = QTextCursor::EndOfLine;
        else if(hit(item, KStandardShortcut::BackwardWord))
            moveOperation = QTextCursor::WordLeft;
        else if(hit(item, KStandardShortcut::ForwardWord))
            moveOperation = QTextCursor::WordRight;
#ifndef NDEBUG
        else if(event->key() == Qt::Key_F12) {
            KoInlineNote *fn = new KoInlineNote(KoInlineNote::Footnote);
            fn->setText("Lorem ipsum dolor sit amet, XgXgectetuer adiXiscing elit, sed diam nonummy");
            fn->setLabel("1");
            KoTextDocumentLayout *lay = dynamic_cast<KoTextDocumentLayout*> (m_textShapeData->document()->documentLayout());
            Q_ASSERT(lay);
            lay->inlineObjectTextManager()->insertInlineObject(m_textCursor, fn);
        }
#endif
        else if((event->modifiers() & (Qt::ControlModifier | Qt::AltModifier)) || event->text().length() == 0) {
            event->ignore();
            return;
        }
        else if(event->text().at(0) == '\r') {
            startKeyPressMacro();
            if (m_textCursor.hasSelection())
                m_selectionHandler.deleteInlineObjects();
            QTextBlockFormat format = m_textCursor.blockFormat();
            m_selectionHandler.nextParagraph();

            QVariant direction = format.property(KoParagraphStyle::TextProgressionDirection);
            format = m_textCursor.blockFormat();
            if(m_textShapeData->pageDirection() != KoText::AutoDirection) { // inherit from shape
                KoText::Direction dir;
                switch(m_textShapeData->pageDirection()) {
                    case KoText::RightLeftTopBottom:
                        dir = KoText::PerhapsRightLeftTopBottom;
                        break;
                    case KoText::LeftRightTopBottom:
                    default:
                        dir = KoText::PerhapsLeftRightTopBottom;
                }
                format.setProperty(KoParagraphStyle::TextProgressionDirection, dir);
            }
            else if(! direction.isNull()) // then we inherit from the previous paragraph.
                format.setProperty(KoParagraphStyle::TextProgressionDirection, direction);
            m_textCursor.setBlockFormat(format);
            updateActions();
            editingPluginEvents();
            ensureCursorVisible();
        }
        else if(event->key() == Qt::Key_Tab || ! (event->text().length() == 1 && !event->text().at(0).isPrint())) { // insert the text
            startKeyPressMacro();
            if (m_textCursor.hasSelection())
                m_selectionHandler.deleteInlineObjects();
            m_prevCursorPosition = m_textCursor.position();
            ensureCursorVisible();
            m_textCursor.insertText(event->text());
            if(m_textShapeData->pageDirection() == KoText::AutoDirection)
                m_updateParagDirection.action->execute(m_prevCursorPosition);
            editingPluginEvents();
            emit blockChanged(m_textCursor.block());
        }
    }
    if(moveOperation != QTextCursor::NoMove || destinationPosition != -1) {
        useCursor(Qt::BlankCursor);
        bool shiftPressed = event->modifiers() & Qt::ShiftModifier;
        if(m_textCursor.hasSelection() && !shiftPressed)
            repaintSelection(); // will erase selection
        else if(! m_textCursor.hasSelection())
            repaintCaret();
        QTextBlockFormat format = m_textCursor.blockFormat();


        KoText::Direction dir = static_cast<KoText::Direction> (format.intProperty(KoParagraphStyle::TextProgressionDirection));
        bool isRtl;
        if(dir == KoText::AutoDirection)
            isRtl = m_textCursor.block().text().isRightToLeft();
        else
            isRtl =  dir == KoText::RightLeftTopBottom;

        if(isRtl) { // if RTL toggle direction of cursor movement.
            switch(moveOperation) {
            case QTextCursor::Left: moveOperation = QTextCursor::Right; break;
            case QTextCursor::Right: moveOperation = QTextCursor::Left; break;
            case QTextCursor::WordRight: moveOperation = QTextCursor::WordLeft; break;
            case QTextCursor::WordLeft: moveOperation = QTextCursor::WordRight; break;
            default: break;
            }
        }
        int prevPosition = m_textCursor.position();
        if(moveOperation != QTextCursor::NoMove)
            m_textCursor.movePosition(moveOperation,
                shiftPressed ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor);
        else
            m_textCursor.setPosition(destinationPosition,
                shiftPressed ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor);
        if(shiftPressed) // altered selection.
            repaintSelection(prevPosition, m_textCursor.position());
        else
            repaintCaret();
        updateActions();
        editingPluginEvents();
        ensureCursorVisible();
    }

    updateSelectionHandler();
}

QVariant TextTool::inputMethodQuery(Qt::InputMethodQuery query, const KoViewConverter &converter) const
{
    switch(query) {
    case Qt::ImMicroFocus: {
        // The rectangle covering the area of the input cursor in widget coordinates.
        QRectF rect = textRect(m_textCursor.position(), m_textCursor.position());
        rect.moveTop(rect.top() - m_textShapeData->documentOffset());
        rect = m_textShape->absoluteTransformation(&converter).mapRect(rect);
        return rect.toRect();
    }
    case Qt::ImFont:
        // The currently used font for text input.
        return m_textCursor.charFormat().font();
    case Qt::ImCursorPosition:
        // The logical position of the cursor within the text surrounding the input area (see ImSurroundingText).
        return m_textCursor.position() - m_textCursor.block().position();
    case Qt::ImSurroundingText:
        // The plain text around the input area, for example the current paragraph.
        return m_textCursor.block().text();
    case Qt::ImCurrentSelection:
        // The currently selected text.
        return m_textCursor.selectedText();
    }
    return QVariant();
}

void TextTool::inputMethodEvent (QInputMethodEvent * event)
{
    if (event->replacementLength() > 0) {
        m_textCursor.setPosition(m_textCursor.position() + event->replacementStart());
        for (int i = event->replacementLength(); i > 0; --i) {
            m_textCursor.deleteChar();
        }
    }
    if(! event->commitString().isEmpty()) {
        QKeyEvent ke(QEvent::KeyPress, -1, 0, event->commitString());
        keyPressEvent(&ke);
    }
    event->accept();
}

void TextTool::ensureCursorVisible()
{
    if(m_textShapeData->endPosition() < m_textCursor.position() || m_textShapeData->position() > m_textCursor.position()) {
        KoTextDocumentLayout *lay = dynamic_cast<KoTextDocumentLayout*> (m_textShapeData->document()->documentLayout());
        Q_ASSERT(lay);
        foreach(KoShape* shape, lay->shapes()) {
            TextShape *textShape = dynamic_cast<TextShape*> (shape);
            Q_ASSERT(textShape);
            KoTextShapeData *d = static_cast<KoTextShapeData*> (textShape->userData());
            if(m_textCursor.position() >= d->position() && m_textCursor.position() <= d->endPosition()) {
                m_textShapeData = d;
                m_textShape = textShape;
                break;
            }
        }
    }

    QRectF cursorPos = textRect(m_textCursor.position(), m_textCursor.position());
    if(! cursorPos.isValid()) { // paragraph is not yet layouted.
        // The number one usecase for this is when the user pressed enter.
        // So take bottom of last paragraph.
        QTextBlock block = m_textCursor.block().previous();
        if(block.isValid()) {
            double y = block.layout()->boundingRect().bottom();
            cursorPos = QRectF(0, y, 1, 10);
        }
    }
    cursorPos.moveTop(cursorPos.top() - m_textShapeData->documentOffset());
    m_canvas->ensureVisible(m_textShape->absoluteTransformation(0).mapRect(cursorPos));
}

void TextTool::keyReleaseEvent(QKeyEvent *event)
{
    event->ignore();
}

void TextTool::updateActions()
{
    m_allowActions = false;
    QTextCharFormat cf = m_textCursor.charFormat();
    m_actionFormatBold->setChecked(cf.fontWeight() > QFont::Normal);
    m_actionFormatItalic->setChecked(cf.fontItalic());
    m_actionFormatUnderline->setChecked(cf.intProperty(KoCharacterStyle::UnderlineType) != KoCharacterStyle::NoLineType);
    m_actionFormatStrikeOut->setChecked(cf.intProperty(KoCharacterStyle::StrikeOutType) != KoCharacterStyle::NoLineType);
    bool super=false, sub=false;
    switch(cf.verticalAlignment()) {
        case QTextCharFormat::AlignSuperScript: super = true; break;
        case QTextCharFormat::AlignSubScript: sub = true; break;
        default:;
    }
    m_actionFormatSuper->setChecked(super);
    m_actionFormatSub->setChecked(sub);
    m_actionFormatFontSize->setFontSize(qRound(cf.fontPointSize()));
    m_actionFormatFontFamily->setFont(cf.font().family());

    QTextBlockFormat bf = m_textCursor.blockFormat();
    if(bf.alignment() == Qt::AlignLeading || bf.alignment() == Qt::AlignTrailing) {
        bool revert = (m_textCursor.block().layout()->textOption().textDirection() == Qt::LeftToRight) != QApplication::isLeftToRight();
        if(bf.alignment() == (Qt::AlignLeading ^ revert))
            m_actionAlignLeft->setChecked(true);
        else
            m_actionAlignRight->setChecked(true);
    }
    else if(bf.alignment() == Qt::AlignHCenter)
        m_actionAlignCenter->setChecked(true);
    if(bf.alignment() == Qt::AlignJustify)
        m_actionAlignBlock->setChecked(true);
    else if(bf.alignment() == (Qt::AlignLeft | Qt::AlignAbsolute))
        m_actionAlignLeft->setChecked(true);
    else if(bf.alignment() == (Qt::AlignRight | Qt::AlignAbsolute))
        m_actionAlignRight->setChecked(true);

    m_actionFormatDecreaseIndent->setEnabled(m_textCursor.blockFormat().leftMargin() > 0.);
    m_allowActions = true;

    //action("text_default")->setEnabled(m_textCursor.hasSelection());

    emit charFormatChanged(cf);
    emit blockFormatChanged(bf);
    emit blockChanged(m_textCursor.block());
}

void TextTool::updateStyleManager()
{
    Q_ASSERT(m_textShapeData);
    KoTextDocumentLayout *lay = dynamic_cast<KoTextDocumentLayout*> (m_textShapeData->document()->documentLayout());
    if(lay)
        emit styleManagerChanged(lay->styleManager());
    else {
        emit styleManagerChanged(0);
        kWarning(32500) << "Shape does not have a KoTextDocumentLayout\n";
    }
}

void TextTool::activate (bool temporary)
{
    Q_UNUSED(temporary);
    KoSelection *selection = m_canvas->shapeManager()->selection();
    foreach(KoShape *shape, selection->selectedShapes()) {
        m_textShape = dynamic_cast<TextShape*> (shape);
        if(m_textShape)
            break;
    }
    if(m_textShape == 0) { // none found
        emit done();
        return;
    }
    foreach(KoShape *shape, selection->selectedShapes()) {
        // deselect others.
        if(m_textShape == shape) continue;
        selection->deselect(shape);
    }
    setShapeData(static_cast<KoTextShapeData*> (m_textShape->userData()));
    useCursor(Qt::IBeamCursor, true);

    // restore the selection from a previous time we edited this document.
    for(int i=0; i < m_previousSelections.count(); i++ ) {
        TextSelection selection = m_previousSelections.at(i);
        if(selection.document == m_textShapeData->document()) {
            m_textCursor.setPosition(selection.anchor);
            m_textCursor.setPosition(selection.position, QTextCursor::KeepAnchor);
            m_previousSelections.removeAt(i);
            break;
        }
    }

    if (m_needSpellChecking && m_spellcheckPlugin) {
        foreach (KoShape *shape, m_canvas->shapeManager()->shapes()) {
            TextShape *textShape = dynamic_cast<TextShape*>(shape);
            if (textShape) {
                KoTextShapeData *textShapeData = static_cast<KoTextShapeData*>(textShape->userData());
                m_spellcheckPlugin->checkSection(textShapeData->document(), 0, -1);
            }
        }
        m_needSpellChecking = false;
    }

    m_textShape->update();

    updateSelectionHandler();
    updateActions();
    updateStyleManager();
    if(m_specialCharacterDocker)
        m_specialCharacterDocker->setEnabled(true);
}

void TextTool::deactivate()
{
    m_textShape = 0;
    if(m_textShapeData) {
        m_textShapeData->document()->setUndoRedoEnabled(false); // erase undo history.
        TextSelection selection;
        selection.document = m_textShapeData->document();
        selection.position = m_textCursor.position();
        selection.anchor = m_textCursor.anchor();
        m_previousSelections.append(selection);
    }
    setShapeData(0);
    if(m_previousSelections.count() > 20) // don't let it grow indefinitely
        m_previousSelections.removeAt(0);

    updateSelectionHandler();
    if(m_specialCharacterDocker) {
        m_specialCharacterDocker->setEnabled(false);
        m_specialCharacterDocker->setVisible(false);
    }
}

void TextTool::repaintDecorations()
{
    if(m_textShapeData)
        repaintSelection();
}

void TextTool::repaintCaret()
{
    QTextBlock block = m_textCursor.block();
    if(block.isValid()) {
        QTextLine tl = block.layout()->lineForTextPosition(m_textCursor.position() - block.position());
        QRectF repaintRect;
        if(tl.isValid()) {
            repaintRect = tl.rect();
            repaintRect.setX(tl.cursorToX(m_textCursor.position() - block.position()) - 2);
            repaintRect.setWidth(6);
        }
        repaintRect.moveTop(repaintRect.y() - m_textShapeData->documentOffset());
        repaintRect = m_textShape->absoluteTransformation(0).mapRect(repaintRect);
        m_canvas->updateCanvas(repaintRect);
    }
}

void TextTool::repaintSelection()
{
    repaintSelection(m_textCursor.position(), m_textCursor.anchor());
}

void TextTool::repaintSelection(int startPosition, int endPosition)
{
    QList<TextShape *> shapes;
    if(m_textShapeData->position() > startPosition || m_textShapeData->endPosition() < endPosition) {
        KoTextDocumentLayout *lay = dynamic_cast<KoTextDocumentLayout*> (m_textShapeData->document()->documentLayout());
        Q_ASSERT(lay);
        foreach(KoShape* shape, lay->shapes()) {
            TextShape *textShape = dynamic_cast<TextShape*> (shape);
            Q_ASSERT(textShape);
            if(textShape->textShapeData()->position() >= startPosition &&
                    textShape->textShapeData()->endPosition() <= endPosition)
                shapes.append(textShape);
        }
    }
    else // the simple case; the full selection is inside the current shape.
        shapes.append(m_textShape);

    // loop over all shapes that contain the text and update per shape.
    QRectF repaintRect = textRect(startPosition, endPosition);
    foreach(TextShape *ts, shapes) {
        QRectF rect = repaintRect;
        rect.moveTop(rect.y() - ts->textShapeData()->documentOffset());
        rect = ts->absoluteTransformation(0).mapRect(rect);
        m_canvas->updateCanvas(ts->boundingRect().intersected(rect));
    }
}

QRectF TextTool::textRect(int startPosition, int endPosition) const
{
    if(startPosition > endPosition)
        qSwap(startPosition, endPosition);
    QTextBlock block = m_textShapeData->document()->findBlock(startPosition);
    QTextLine line1 = block.layout()->lineForTextPosition(startPosition - block.position());
    if(! line1.isValid())
        return QRectF();
    double startX = line1.cursorToX(startPosition - block.position());
    if(startPosition == endPosition)
        return QRectF(startX, line1.y(), 1, line1.height());

    QTextBlock block2 = m_textShapeData->document()->findBlock(endPosition);
    QTextLine line2 = block2.layout()->lineForTextPosition(endPosition - block2.position());
    if(! line2.isValid())
        return QRectF();
    double endX = line2.cursorToX(endPosition - block2.position());

    if(line1.textStart() + block.position() == line2.textStart()+ block2.position() )
        return QRectF(qMin(startX, endX), line1.y(), qAbs(startX - endX), line1.height());
    return QRectF(0, line1.y(), 10E6, line2.y() + line2.height() - line1.y());
}

KoToolSelection* TextTool::selection()
{
    return &m_selectionHandler;
}

QWidget *TextTool::createOptionWidget()
{
    QTabWidget *widget = new QTabWidget();
    SimpleStyleWidget *ssw = new SimpleStyleWidget(this, widget);
    widget->addTab(ssw, i18n("Abc")); // XXX: Replace with icon
    StylesWidget *paragTab = new StylesWidget(StylesWidget::ParagraphStyle, widget);
    widget->addTab(paragTab, i18n("¶")); // XXX: Replace with icon
    StylesWidget *charTab =new StylesWidget(StylesWidget::CharacterStyle, widget);
    widget->addTab(charTab, i18n("T")); // XXX: Replace with icon

    connect(this, SIGNAL(styleManagerChanged(KoStyleManager *)), ssw, SLOT(setStyleManager(KoStyleManager *)));
    connect(this, SIGNAL(blockChanged(const QTextBlock&)), ssw, SLOT(setCurrentBlock(const QTextBlock&)));
    connect(this, SIGNAL(charFormatChanged(const QTextCharFormat &)), ssw, SLOT(setCurrentFormat(const QTextCharFormat &)));

    connect(this, SIGNAL(styleManagerChanged(KoStyleManager *)), paragTab, SLOT(setStyleManager(KoStyleManager *)));
    connect(this, SIGNAL(styleManagerChanged(KoStyleManager *)), charTab, SLOT(setStyleManager(KoStyleManager *)));
    connect(this, SIGNAL(charFormatChanged(const QTextCharFormat &)),
            paragTab, SLOT(setCurrentFormat(const QTextCharFormat &)));
    connect(this, SIGNAL(blockFormatChanged(const QTextBlockFormat &)),
            paragTab, SLOT(setCurrentFormat(const QTextBlockFormat &)));
    connect(this, SIGNAL(charFormatChanged(const QTextCharFormat &)),
            charTab, SLOT(setCurrentFormat(const QTextCharFormat &)));

    connect(paragTab, SIGNAL(paragraphStyleSelected(KoParagraphStyle *)),
            &m_selectionHandler, SLOT(setStyle(KoParagraphStyle*)));
    connect(charTab, SIGNAL(characterStyleSelected(KoCharacterStyle *)),
            &m_selectionHandler, SLOT(setStyle(KoCharacterStyle*)));

    updateStyleManager();
    if(m_textShape)
        updateActions();
    return widget;
}

void TextTool::addUndoCommand()
{
    if(! m_allowAddUndoCommand) return;
    class UndoTextCommand : public QUndoCommand {
      public:
        UndoTextCommand(QTextDocument *document, TextTool *tool, QUndoCommand *parent = 0)
            : QUndoCommand(i18n("Text"), parent),
            m_document(document),
            m_tool(tool)
        {
        }

        void undo () {
            if(m_document.isNull())
                return;
            if(! m_tool.isNull()) {
                m_tool->stopMacro();
                m_tool->m_allowAddUndoCommand = false;
               if(m_tool->m_changeTracker && !m_tool->m_canvas->resourceProvider()->boolResource(KoCanvasResource::DocumentIsLoading))
                   m_tool->m_changeTracker->notifyForUndo();
                m_document->undo(&m_tool->m_textCursor);
            }
            else
                m_document->undo();
            if(! m_tool.isNull())
                m_tool->m_allowAddUndoCommand = true;
        }

        void redo () {
            if(m_document.isNull())
                return;

            if(! m_tool.isNull()) {
                m_tool->m_allowAddUndoCommand = false;
                m_document->redo(&m_tool->m_textCursor);
            }
            else
                m_document->redo();
            if(! m_tool.isNull())
                m_tool->m_allowAddUndoCommand = true;
        }

        QPointer<QTextDocument> m_document;
        QPointer<TextTool> m_tool;
    };
    if(m_currentCommand) {
        new UndoTextCommand(m_textShapeData->document(), this, m_currentCommand);
        if (! m_currentCommandHasChildren)
            m_canvas->addCommand(m_currentCommand);
        m_currentCommandHasChildren = true;
    }
    else
        m_canvas->addCommand(new UndoTextCommand(m_textShapeData->document(), this));
}

void TextTool::addCommand(QUndoCommand *command)
{
    m_currentCommand = command;
    TextCommandBase *cmd = dynamic_cast<TextCommandBase*> (command);
    if(cmd)
        cmd->setTool(this);
    m_canvas->addCommand(command); // will execute it.
    m_currentCommand = 0;
}

void TextTool::nonbreakingSpace()
{
    if(! m_allowActions) return;
    m_selectionHandler.insert(QString(QChar(Qt::Key_nobreakspace)));
}

void TextTool::nonbreakingHyphen()
{
    if(! m_allowActions) return;
    m_selectionHandler.insert(QString(QChar(0x2013)));
}

void TextTool::softHyphen()
{
    if(! m_allowActions) return;
    m_selectionHandler.insert(QString(QChar(Qt::Key_hyphen)));
}

void TextTool::lineBreak()
{
    if(! m_allowActions) return;
    m_selectionHandler.insert(QString(QChar(0x2028)));
}

void TextTool::alignLeft()
{
    if(! m_allowActions) return;
    Qt::Alignment align = Qt::AlignLeading;
    if(m_textCursor.block().layout()->textOption().textDirection() != Qt::LeftToRight)
        align |= Qt::AlignTrailing;
    m_selectionHandler.setHorizontalTextAlignment(align);
}

void TextTool::alignRight()
{
    if(! m_allowActions) return;
    Qt::Alignment align = Qt::AlignTrailing;
    if(m_textCursor.block().layout()->textOption().textDirection() == Qt::RightToLeft)
        align = Qt::AlignLeading;
    m_selectionHandler.setHorizontalTextAlignment(align);
}

void TextTool::alignCenter()
{
    if(! m_allowActions) return;
    m_selectionHandler.setHorizontalTextAlignment(Qt::AlignHCenter);
}

void TextTool::alignBlock()
{
    if(! m_allowActions) return;
    m_selectionHandler.setHorizontalTextAlignment(Qt::AlignJustify);
}

void TextTool::superScript(bool on)
{
    if(! m_allowActions) return;
    if(on)
        m_actionFormatSub->setChecked(false);
    m_selectionHandler.setVerticalTextAlignment(on ? Qt::AlignTop : Qt::AlignVCenter);
}

void TextTool::subScript(bool on)
{
    if(! m_allowActions) return;
    if(on)
        m_actionFormatSuper->setChecked(false);
    m_selectionHandler.setVerticalTextAlignment(on ? Qt::AlignBottom : Qt::AlignVCenter);
}

void TextTool::increaseIndent()
{
    if(! m_allowActions) return;
    m_selectionHandler.increaseIndent();
    m_actionFormatDecreaseIndent->setEnabled(m_textCursor.blockFormat().leftMargin() > 0.);
}

void TextTool::decreaseIndent()
{
    if(! m_allowActions) return;
    m_selectionHandler.decreaseIndent();
    m_actionFormatDecreaseIndent->setEnabled(m_textCursor.blockFormat().leftMargin() > 0.);
}

void TextTool::textDefaultFormat()
{
    // TODO
    kDebug(32500) <<"TextTool::textDefaultFormat";
}

void TextTool::insertIndexMarker()
{
    // TODO handle result when we figure out how to report errors from a tool.
    m_selectionHandler.insertIndexMarker();
}

void TextTool::formatParagraph()
{
    ParagraphSettingsDialog *dia = new ParagraphSettingsDialog(m_canvas->canvasWidget(), this);
    dia->open(m_textCursor);
    dia->setUnit(m_canvas->unit());
    connect(dia, SIGNAL(startMacro(const QString&)), this, SLOT(startMacro(const QString&)));
    connect(dia, SIGNAL(stopMacro()), this, SLOT(stopMacro()));

    dia->show();
}

void TextTool::toggleTrackChanges(bool on)
{
    m_trackChanges = on;
    if(m_textShapeData && on){
        if(m_changeTracker == 0)
            m_changeTracker = new ChangeTracker(this);
        if(m_changeTracker)
            m_changeTracker->setDocument(m_textShapeData->document());
    }
    else if(m_changeTracker)
        m_changeTracker->setDocument(0);
}

void TextTool::selectAll()
{
    if(m_textShapeData == 0) return;
    const int selectionLength = qAbs(m_textCursor.position() - m_textCursor.anchor());
    QTextBlock lastBlock = m_textShapeData->document()->end().previous();
    m_textCursor.setPosition(lastBlock.position() + lastBlock.length() - 1);
    m_textCursor.setPosition(0, QTextCursor::KeepAnchor);
    repaintSelection(0, m_textCursor.anchor());
    if (selectionLength != qAbs(m_textCursor.position() - m_textCursor.anchor())) // it actually changed
        emit selectionChanged(true);
}

void TextTool::startMacro(const QString &title)
{
    if(m_currentCommand) return;
    class MacroCommand : public QUndoCommand {
      public:
        MacroCommand(const QString &title) : QUndoCommand(title), m_first(true) {}
        virtual void redo() {
            if(! m_first)
                QUndoCommand::redo();
            m_first = false;
        }
        virtual bool mergeWith (const QUndoCommand *) { return false; }
        bool m_first;
    };
    m_currentCommand = new MacroCommand(title);
    m_currentCommandHasChildren = false;
    m_processingKeyPress = false;
}

void TextTool::stopMacro()
{
    if(m_currentCommand == 0) return;
    if (! m_currentCommandHasChildren)
        delete m_currentCommand;
    m_currentCommand = 0;
    m_processingKeyPress = false;
}

void TextTool::showStyleManager()
{
    KoTextDocumentLayout *lay = dynamic_cast<KoTextDocumentLayout*> (m_textShapeData->document()->documentLayout());
    if(lay) {
        Q_ASSERT( lay->styleManager() );
        if( ! lay->styleManager() ) return; //don't crash
        StyleManagerDialog *dia = new StyleManagerDialog(m_canvas->canvasWidget());
        dia->setStyleManager(lay->styleManager());
        dia->setUnit(m_canvas->unit());
        dia->show();
    }
}

void TextTool::startTextEditingPlugin(const QString &pluginId)
{
    KoTextEditingPlugin *plugin = m_textEditingPlugins.value(pluginId);
    if(plugin) {
        if(m_textCursor.hasSelection()) {
            int from = m_textCursor.position();
            int to = m_textCursor.anchor();
            if(from > to) // make sure we call the plugin consistently
                qSwap(from, to);
            plugin->checkSection(m_textShapeData->document(), from, to);
        }
        else
            plugin->finishedWord(m_textShapeData->document(), m_textCursor.position());
    }
}

bool TextTool::isBidiDocument() const
{
    if(m_canvas->resourceProvider())
        return m_canvas->resourceProvider()->boolResource(KoText::BidiDocument);
    return false;
}

void TextTool::updateParagraphDirection(const QVariant &variant)
{
    int position = variant.toInt();
    KoTextShapeData *data = m_textShapeData;
    if(data == 0) // tool is deactivated already
        return;
    m_updateParagDirection.block = data->document()->findBlock(position);
    m_updateParagDirection.direction = KoText::AutoDirection;
    if(! m_updateParagDirection.block.isValid())
        return;
    QTextBlockFormat format = m_updateParagDirection.block.blockFormat();
    
    KoText::Direction dir =
        static_cast<KoText::Direction> (format.intProperty( KoParagraphStyle::TextProgressionDirection));
        
    if(dir == KoText::AutoDirection || dir == KoText::PerhapsLeftRightTopBottom ||
            dir == KoText::PerhapsRightLeftTopBottom) {
        bool rtl = isRightToLeft(m_updateParagDirection.block.text());
        if(rtl && (dir != KoText::AutoDirection || QApplication::isLeftToRight()))
            m_updateParagDirection.direction = KoText::PerhapsRightLeftTopBottom;
        else if(!rtl && (dir != KoText::AutoDirection || QApplication::isRightToLeft())) // remove previously set one if needed.
            m_updateParagDirection.direction = KoText::PerhapsLeftRightTopBottom;
    }
    else
        m_updateParagDirection.block = QTextBlock();
}

void TextTool::updateParagraphDirectionUi()
{
    if(! m_updateParagDirection.block.isValid())
        return;
    QTextCursor cursor(m_updateParagDirection.block);
    QTextBlockFormat format = cursor.blockFormat();
    if(format.property(KoParagraphStyle::TextProgressionDirection).toInt() != m_updateParagDirection.direction) {
        format.setProperty(KoParagraphStyle::TextProgressionDirection, m_updateParagDirection.direction);
        cursor.setBlockFormat(format); // note that setting this causes a re-layout.
    }

    if(m_canvas->resourceProvider() && ! isBidiDocument()) {
        if((QApplication::isLeftToRight() &&
                (m_updateParagDirection.direction == KoText::RightLeftTopBottom ||
                 m_updateParagDirection.direction == KoText::PerhapsRightLeftTopBottom)) ||
                (QApplication::isRightToLeft() &&
                 (m_updateParagDirection.direction == KoText::LeftRightTopBottom ||
                  m_updateParagDirection.direction == KoText::PerhapsLeftRightTopBottom))) {
            m_canvas->resourceProvider()->setResource(KoText::BidiDocument, true);

            emit blockChanged(m_textCursor.block()); // make sure that the dialogs follow this change
        }
    }
    updateActions();
}

void TextTool::resourceChanged(int key, const QVariant &var)
{
    if(m_allowResourceProviderUpdates == false)
        return;
    if(key == KoText::CurrentTextPosition) {
        repaintSelection();
        m_textCursor.setPosition(var.toInt());
        ensureCursorVisible();
    }
    else if(key == KoText::CurrentTextAnchor) {
        repaintSelection();
        int pos = m_textCursor.position();
        m_textCursor.setPosition(var.toInt());
        m_textCursor.setPosition(pos, QTextCursor::KeepAnchor);
    }
    else return;

    repaintSelection();
}

void TextTool::insertSpecialCharacter()
{
    if(m_specialCharacterDocker == 0) {
        m_specialCharacterDocker = new InsertCharacter(m_canvas->canvasWidget());
        connect(m_specialCharacterDocker, SIGNAL(insertCharacter(const QString&)),
                &m_selectionHandler, SLOT(insert(const QString&)));
    }

    m_specialCharacterDocker->show();
}

void TextTool::selectFont()
{
    FontDia *fontDlg = new FontDia( m_textCursor.charFormat());
    fontDlg->exec();
    fontDlg->style().applyStyle(&m_textCursor);
    delete fontDlg;
}

void TextTool::shapeAddedToCanvas()
{
    if (m_textShape) {
        KoSelection *selection = m_canvas->shapeManager()->selection();
        KoShape *shape = selection->firstSelectedShape();
        if (shape != m_textShape) {
            // this situation applies when someone, not us, changed the selection by selecting another
            // text shape. Possibly by adding one.
            // Deselect the new shape again, so we can keep editing what we were already editing
            selection->select(m_textShape);
            selection->deselect(shape);
        }
    }
}

// ---------- editing plugins methods.
void TextTool::editingPluginEvents()
{
    if(m_prevCursorPosition == -1 || m_prevCursorPosition == m_textCursor.position())
        return;

    QTextBlock block = m_textCursor.block();
    if(! block.contains(m_prevCursorPosition)) {
        finishedWord();
        finishedParagraph();
        m_prevCursorPosition = -1;
    }
    else {
        int from = m_prevCursorPosition;
        int to = m_textCursor.position();
        if(from > to)
            qSwap(from, to);
        QString section = block.text().mid(from - block.position(), to - from);
        if(section.contains(' ')) {
            finishedWord();
            m_prevCursorPosition = -1;
        }
    }
}

void TextTool::finishedWord()
{
    foreach(KoTextEditingPlugin* plugin, m_textEditingPlugins.values())
        plugin->finishedWord(m_textShapeData->document(), m_prevCursorPosition);
}

void TextTool::finishedParagraph()
{
    foreach(KoTextEditingPlugin* plugin, m_textEditingPlugins.values())
        plugin->finishedParagraph(m_textShapeData->document(), m_prevCursorPosition);
}

void TextTool::setTextColor(const KoColor &color)
{
    m_selectionHandler.setTextColor(color.toQColor());
}

void TextTool::setBackgroundColor(const KoColor &color)
{
    m_selectionHandler.setTextBackgroundColor(color.toQColor());
}

void TextTool::startKeyPressMacro()
{
/* we have a little state machine here;
    As soon as the user presses a key (i.e. we enter the keyPressEvent) this method should be
    called and we make sure that all commands from that point on are combined into one so things like
    spell checking will not create extra undo states :)  [state a]
    As soon as the user does a different action, like executing a menu option, we create new undo states
    again, separating them into different user-undoable actions.  [state b]

    Then there is the 'macro' function that plugins etc can start;  which is [state c].
*/

    if (m_currentCommand) {
        if (m_processingKeyPress) // already have a key-press macro
            return;
        stopMacro();
    }

    startMacro(i18n("Key press"));
    m_processingKeyPress = true;
}

#include "TextTool.moc"
