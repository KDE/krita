KisView::KisView(KisDoc *doc, KisUndoAdapter *adapter, QWidget *parent)
    : super(doc, parent)
    , KXMLGUIBuilder( shell() )
    , m_panning( false )
    , m_popup( 0 )
{

    KisConfig cfg;

    setFocusPolicy( Qt::StrongFocus );

    connect(this, SIGNAL(autoScroll(const QPoint &)), SLOT(slotAutoScroll(const QPoint &)));

    setMouseTracking(true);
    resetMonitorProfile();

    setFocus();
}

KisView::~KisView()
{
    KisConfig cfg;
    cfg.setShowRulers( m_RulerAction->isChecked() );
}


void KisView::setupActions()
{
    m_RulerAction = new KToggleAction(i18n( "Show Rulers" ), actionCollection(), "view_ruler");
    m_RulerAction->setShortcut(Qt::CTRL+Qt::Key_R);
    connect(m_RulerAction, SIGNAL(triggered()), this, SLOT(showRuler()));
    m_RulerAction->setChecked(cfg.showRulers());
    m_RulerAction->setCheckedState(KGuiItem(i18n("Hide Rulers")));
    m_RulerAction->setWhatsThis( i18n("The rulers show the horizontal and vertical positions of the mouse on the image "
                                      "and can be used to position your mouse at the right place on the canvas. <p>Uncheck this to disable "
                                      "the rulers from being displayed." ) );

    // Add new palette
    action = new KAction(i18n("Add New Palette..."), actionCollection(), "add_palette");
    connect(action, SIGNAL(triggered()), this, SLOT(slotAddPalette()));

    action = new KAction(i18n("Edit Palette..."), actionCollection(), "edit_palette");
    connect(action, SIGNAL(triggered()), this, SLOT(slotEditPalette()));

}

void KisView::paletteChange(const QPalette& oldPalette)
{
    Q_UNUSED(oldPalette);
    refreshKisCanvas();
}

void KisView::updateOpenGLCanvas(const QRect& imageRect)
{
#ifdef HAVE_OPENGL
    KisImageSP img = currentImg();

    if (img && m_paintViewEnabled) {
        Q_ASSERT(!m_OpenGLImageContext.isNull());

        if (!m_OpenGLImageContext.isNull()) {
            m_OpenGLImageContext->update(imageRect);
        }
    }
#else
    Q_UNUSED(imageRect);
#endif
}

void KisView::paintOpenGLView(const QRect& canvasRect)
{
#ifdef HAVE_OPENGL
    if (!m_canvas->updatesEnabled()) {
        return;
    }

    m_canvas->OpenGLWidget()->makeCurrent();

    glDrawBuffer(GL_BACK);

    QColor widgetBackgroundColor = palette().color(QPalette::Mid);

    glClearColor(widgetBackgroundColor.red() / 255.0, widgetBackgroundColor.green() / 255.0, widgetBackgroundColor.blue() / 255.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    KisImageSP img = currentImg();

    if (img && m_paintViewEnabled) {

        QRect vr = canvasRect;
        vr &= QRect(0, 0, m_canvas->width(), m_canvas->height());

        if (!vr.isNull()) {

            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glViewport(0, 0, m_canvas->width(), m_canvas->height());
            glOrtho(0, m_canvas->width(), m_canvas->height(), 0, -1, 1);

            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();

            glBindTexture(GL_TEXTURE_2D, m_OpenGLImageContext->backgroundTexture());

            glTranslatef(m_canvasXOffset, m_canvasYOffset, 0.0);

            glEnable(GL_TEXTURE_2D);
            glBegin(GL_QUADS);

            glTexCoord2f(0.0, 0.0);
            glVertex2f(0.0, 0.0);

            glTexCoord2f((img->width() * zoom()) / KisOpenGLImageContext::BACKGROUND_TEXTURE_WIDTH, 0.0);
            glVertex2f(img->width() * zoom(), 0.0);

            glTexCoord2f((img->width() * zoom()) / KisOpenGLImageContext::BACKGROUND_TEXTURE_WIDTH,
                         (img->height() * zoom()) / KisOpenGLImageContext::BACKGROUND_TEXTURE_HEIGHT);
            glVertex2f(img->width() * zoom(), img->height() * zoom());

            glTexCoord2f(0.0, (img->height() * zoom()) / KisOpenGLImageContext::BACKGROUND_TEXTURE_HEIGHT);
            glVertex2f(0.0, img->height() * zoom());

            glEnd();

            glTranslatef(-m_canvasXOffset, -m_canvasYOffset, 0.0);

            glTranslatef(-horzValue(), -vertValue(), 0.0);
            glScalef(zoomFactor(), zoomFactor(), 1.0);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            QRect wr = viewToWindow(QRect(0, 0, m_canvas->width(), m_canvas->height()));
            wr &= QRect(0, 0, img->width(), img->height());

            m_OpenGLImageContext->setHDRExposure(HDRExposure());

            m_canvas->OpenGLWidget()->makeCurrent();

            for (int x = (wr.left() / m_OpenGLImageContext->imageTextureTileWidth()) * m_OpenGLImageContext->imageTextureTileWidth();
                  x <= wr.right();
                  x += m_OpenGLImageContext->imageTextureTileWidth()) {
                for (int y = (wr.top() / m_OpenGLImageContext->imageTextureTileHeight()) * m_OpenGLImageContext->imageTextureTileHeight();
                      y <= wr.bottom();
                      y += m_OpenGLImageContext->imageTextureTileHeight()) {

                    glBindTexture(GL_TEXTURE_2D, m_OpenGLImageContext->imageTextureTile(x, y));

                    glBegin(GL_QUADS);

                    glTexCoord2f(0.0, 0.0);
                    glVertex2f(x, y);

                    glTexCoord2f(1.0, 0.0);
                    glVertex2f(x + m_OpenGLImageContext->imageTextureTileWidth(), y);

                    glTexCoord2f(1.0, 1.0);
                    glVertex2f(x + m_OpenGLImageContext->imageTextureTileWidth(), y + m_OpenGLImageContext->imageTextureTileHeight());

                    glTexCoord2f(0.0, 1.0);
                    glVertex2f(x, y + m_OpenGLImageContext->imageTextureTileHeight());

                    glEnd();
                }
            }

            glDisable(GL_TEXTURE_2D);
            glDisable(GL_BLEND);

            m_gridManager->drawGrid(wr, 0, true);
            m_perspectiveGridManager->drawGrid( wr, 0, true );

            // Unbind the texture otherwise the ATI driver crashes when the canvas context is
            // made current after the textures are deleted following an image resize.
            glBindTexture(GL_TEXTURE_2D, 0);

            //paintGuides();
        }
    }

    m_canvas->OpenGLWidget()->swapBuffers();

    paintToolOverlay(QRegion(canvasRect));

#else
    Q_UNUSED(canvasRect);
#endif
}

void KisView::setInputDevice(KoInputDevice inputDevice)
{
    if (inputDevice != m_inputDevice) {
        m_inputDevice = inputDevice;

        m_toolManager->setToolForInputDevice(m_inputDevice, inputDevice);

        if (m_toolManager->currentTool() == 0) {
            m_toolManager->setCurrentTool(m_toolManager->findTool("tool_brush", m_inputDevice));
        }
        else {
            m_toolManager->setCurrentTool(m_toolManager->currentTool());
        }
        m_toolManager->activateCurrentTool();

        emit sigInputDeviceChanged(inputDevice);
    }

}

void KisView::selectionDisplayToggled(bool displaySelection)
{
#ifdef HAVE_OPENGL
    if (m_canvas->isOpenGLCanvas()) {
        if (m_OpenGLImageContext) {
            m_OpenGLImageContext->setSelectionDisplayEnabled(displaySelection);
        }
    }
#else
    Q_UNUSED(displaySelection);
#endif
    updateCanvas();
}

void KisView::slotAddPalette()
{
    KDialog *base = new KDialog(this);
    base->setCaption( i18n("Add Palette") );
    base->setButtons( KDialog::Ok | KDialog::Cancel);
    base->setDefaultButton( KDialog::Ok );
    KisCustomPalette *p = new KisCustomPalette(base, "add palette", i18n("Add Palette"), this);
    base->setMainWidget(p);
    base->show();
}

void KisView::slotEditPalette()
{
    KisPaletteChooser chooser(this);
    KisResourceServerBase* srv = KisResourceServerRegistry::instance()->get("PaletteServer");
    if (!srv) {
        return;
    }
    QList<KisResource*> resources = srv->resources();
    QList<KisPalette*> palettes;

    foreach (KisResource *resource, resources) {
        KisPalette* palette = dynamic_cast<KisPalette*>(resource);
        if (!palette) continue;

        chooser.paletteList->addItem(palette->name());
        palettes.append(palette);
    }

    if (chooser.exec() != QDialog::Accepted ) {
        return;
    }

    int index = chooser.paletteList->currentRow();
    if (index < 0) {
        KMessageBox::error(this, i18n("No palette selected."), i18n("Palette"));
        return;
    }

    KDialog* base = new KDialog(this );
    base->setCaption(  i18n("Edit Palette") );
    base->setButtons( KDialog::Ok);
    base->setDefaultButton( KDialog::Ok);
    KisCustomPalette* cp = new KisCustomPalette(base, "edit palette",
            i18n("Edit Palette"), this);
    cp->setEditMode(true);
    cp->setPalette(palettes.at(index));
    base->setMainWidget(cp);
    base->show();
}

void KisView::canvasGotKeyPressEvent(QKeyEvent *event)
{
    if (!m_toolManager->currentTool()) {
        event->ignore();
        return;
    }

    if (event->key() == Qt::Key_Space) {
        if (!m_panning) {
            // Set tool temporarily to pan
            m_panning = true;
            m_oldTool = m_toolManager->currentTool();
            m_toolManager->setCurrentTool( "tool_pan" );
        }
        else {
            // Unset panning
            m_panning = false;
            m_toolManager->setCurrentTool( m_oldTool );
            m_oldTool = 0;
        }
    }
    if (m_toolManager->currentTool())
        m_toolManager->currentTool()->keyPress(event);
}


void KisView::imgUpdated(QRect rc)
{
    updateCanvas(rc);
}

void KisView::slotOpenGLImageUpdated(QRect rc)
{
    paintOpenGLView(windowToView(rc));
}

void KisView::setCurrentImage(KisImageSP image)
{
    if(!image) return;

    disconnectCurrentImg();
    m_image = image;

    KisConfig cfg;

#ifdef HAVE_OPENGL
    if (cfg.useOpenGL()) {
        m_OpenGLImageContext = KisOpenGLImageContext::getImageContext(image, monitorProfile());
        m_canvas->createOpenGLCanvas(m_OpenGLImageContext->sharedContextWidget());
    }
#endif
    connectCurrentImg();
    m_layerBox->setImage(currentImg());

    zoomAroundPoint(0, 0, 1.0);

    if (!currentImg())
        layersUpdated();

    imgUpdateGUI();

    image->blockSignals(false);
}


QCursor KisView::setCanvasCursor(const QCursor & cursor)
{
    QCursor oldCursor = m_canvas->cursor();
    QCursor newCursor;

    KisConfig cfg;

    switch (cfg.cursorStyle()) {
    case CURSOR_STYLE_TOOLICON:
        newCursor = cursor;
        break;
    case CURSOR_STYLE_CROSSHAIR:
        newCursor = KisCursor::crossCursor();
        break;
    case CURSOR_STYLE_POINTER:
        newCursor = KisCursor::arrowCursor();
        break;
    case CURSOR_STYLE_OUTLINE:
        newCursor = cursor;
        break;
    default:
        newCursor = cursor;
    }

    m_canvas->setCursor(newCursor);
    return oldCursor;
}

void KisView::createDockers()
{

    m_colorchooser = new KoUniColorChooser(0);
    m_colorchooser->setWindowTitle(i18n("Color by values"));

    connect(m_colorchooser, SIGNAL(sigColorChanged(const KoColor &)), this, SLOT(slotSetFGColor(const KoColor &)));
    connect(this, SIGNAL(sigFGColorChanged(const KoColor &)), m_colorchooser, SLOT(setColor(const KoColor &)));
    createDock(i18n("Color by values"), m_colorchooser);

    //make sure the color chooser get right default values
    emit sigFGColorChanged(m_fg);

    m_palettewidget = new KisPaletteWidget(0);
    m_palettewidget->setWindowTitle(i18n("Palettes"));
    connect(m_palettewidget, SIGNAL(colorSelected(const QColor &)),
            this, SLOT(slotSetFGQColor(const QColor &)));
    // No BGColor or reverse slotFGChanged->palette connections, since that's not useful here

    KisResourceServerBase* rServer;
    rServer = KisResourceServerRegistry::instance()->get("PaletteServer");
    QList<KisResource*> resources = rServer->resources();

    foreach (KisResource *resource, resources) {
        m_palettewidget->slotAddPalette(resource);
    }
    connect(m_palettewidget, SIGNAL(colorSelected(const KoColor &)), this, SLOT(slotSetFGColor(const KoColor &)));
    createDock(i18n("Palettes"), m_palettewidget);
}

