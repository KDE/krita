/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <stdio.h>
#include <stdlib.h>

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qdrawutil.h>
#include <qevent.h>
#include <qfile.h>
#include <qimage.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qvalidator.h>
#include <qpainter.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qtimer.h>
//Added by qt3to4:
#include <Q3VBoxLayout>

#include <kapplication.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <klistbox.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kseparator.h>
#include <kpalette.h>
#include <kimageeffect.h>

#include <kcolordialog.h>
#include <k3colordrag.h>
#include <config.h>
#include <kdebug.h>

#include <kis_meta_registry.h>
#include <kis_color.h>
#include <kis_factory.h>
#include <kis_colorspace_factory_registry.h>
#include "kis_palette_widget.h"
#include "kis_resource.h"
#include "kis_palette.h"
#include "kis_palette_view.h"

KisPaletteWidget::KisPaletteWidget( QWidget *parent, int minWidth, int cols)
    : QWidget( parent ), mMinWidth(minWidth), mCols(cols)
{
    init = false;

    m_currentPalette = 0;

    Q3VBoxLayout *layout = new Q3VBoxLayout( this );

    combo = new QComboBox( false, this );
    combo->setFocusPolicy( Qt::ClickFocus );
    layout->addWidget(combo);

    m_view = new KisPaletteView(this, 0, minWidth, cols);
    layout->addWidget( m_view );

    //setFixedSize(sizeHint());

    connect(combo, SIGNAL(activated(const QString &)),
            this, SLOT(slotSetPalette(const QString &)));
    connect(m_view, SIGNAL(colorSelected(const KisColor &)),
            this, SIGNAL(colorSelected(const KisColor &)));
    connect(m_view, SIGNAL(colorSelected(const QColor &)),
            this, SIGNAL(colorSelected(const QColor &)));
    connect(m_view, SIGNAL(colorDoubleClicked(const KisColor &, const QString &)),
            this, SIGNAL(colorDoubleClicked(const KisColor &, const QString &)));
}

KisPaletteWidget::~KisPaletteWidget()
{
}

QString KisPaletteWidget::palette() const
{
    return combo->currentText();
}


// 2000-02-12 Espen Sand
// Set the color in two steps. The setPalette() slot will not emit a signal
// with the current color setting. The reason is that setPalette() is used
// by the color selector dialog on startup. In the color selector dialog
// we normally want to display a startup color which we specify
// when the dialog is started. The slotSetPalette() slot below will
// set the palette and then use the information to emit a signal with the
// new color setting. It is only used by the combobox widget.
//
void KisPaletteWidget::slotSetPalette( const QString &_paletteName )
{
    setPalette( _paletteName );
    m_view->slotColorCellSelected(0); // FIXME: We need to save the current value!!
}


void KisPaletteWidget::setPalette( const QString &_paletteName )
{
    QString paletteName( _paletteName);

    m_currentPalette = m_namedPaletteMap[paletteName];

    if (combo->currentText() != paletteName)
    {
        bool found = false;
        for(int i = 0; i < combo->count(); i++)
        {
            if (combo->text(i) == paletteName)
            {
                combo->setCurrentItem(i);
                found = true;
                break;
            }
        }
        if (!found)
        {
            combo->insertItem(paletteName);
            combo->setCurrentItem(combo->count()-1);
        }
    }

    m_view->setPalette(m_currentPalette);
}

void KisPaletteWidget::slotAddPalette(KisResource * palette)
{
    KisPalette * p = dynamic_cast<KisPalette*>(palette);

    m_namedPaletteMap.insert(palette->name(), p);

    combo->insertItem(palette->name());

    if (!init) {
        combo->setCurrentItem(0);
        setPalette(combo ->currentText());
        init = true;
    }
}


#include "kis_palette_widget.moc"

