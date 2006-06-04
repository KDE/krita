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

#include <QComboBox>
#include <QVBoxLayout>

#include <kdebug.h>

#include "kis_palette_widget.h"
#include "kis_resource.h"
#include "kis_palette.h"
#include "kis_palette_view.h"

KisPaletteWidget::KisPaletteWidget( QWidget *parent, int minWidth, int cols)
    : QWidget( parent ), mMinWidth(minWidth), mCols(cols)
{
    init = false;

    m_currentPalette = 0;

    QVBoxLayout *layout = new QVBoxLayout( this );

    combo = new QComboBox( this );
    combo->setFocusPolicy( Qt::ClickFocus );
    layout->addWidget(combo);

    m_view = new KisPaletteView(this, 0, minWidth, cols);
    layout->addWidget( m_view );

    //setFixedSize(sizeHint());

    connect(combo, SIGNAL(activated(const QString &)),
            this, SLOT(slotSetPalette(const QString &)));
    connect(m_view, SIGNAL(colorSelected(const KoColor &)),
            this, SIGNAL(colorSelected(const KoColor &)));
    connect(m_view, SIGNAL(colorSelected(const QColor &)),
            this, SIGNAL(colorSelected(const QColor &)));
    connect(m_view, SIGNAL(colorDoubleClicked(const KoColor &, const QString &)),
            this, SIGNAL(colorDoubleClicked(const KoColor &, const QString &)));
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
        int i = combo->findText(paletteName);

        if (i >= 0) {
            combo->setCurrentIndex(i);
        } else {
            combo->addItem(paletteName);
            combo->setCurrentIndex(combo->count() - 1);
        }
    }

    m_view->setPalette(m_currentPalette);
}

void KisPaletteWidget::slotAddPalette(KisResource * palette)
{
    KisPalette * p = dynamic_cast<KisPalette*>(palette);

    if (p) {
        m_namedPaletteMap.insert(palette->name(), p);

        combo->addItem(palette->name());

        if (!init) {
            combo->setCurrentIndex(0);
            setPalette(combo ->currentText());
            init = true;
        }
    }
}


#include "kis_palette_widget.moc"

