/* This file is part of the KDE project
   Copyright (C) 2006 Martin Pfeiffer <hubipete@gmx.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef FORMULARENDERER_H
#define FORMULARENDERER_H

/**
 * @file
 * This file is part of KFormula and defines the classe FormulaRenderer
 */

#include <QList>

/** The KFormula namespace contains all classes needed to FormulaShape in KOffice */
namespace KFormula {

class FormulaContainer;



/**
 * @short FormulaRenderer renders the content of a given FormulaContainer
 *
 * The FormulaRenderer class contains all code for rendering a formula. 
 *
 * @author Martin Pfeiffer <hubipete@gmx.net>
 * @since 2.0
 */
class FormulaRenderer
{
public:
	/**
	 * The constructor
	 * @param container The FormulaContainer that will be rendered
	 */
	FormulaRenderer( FormulaContainer* container = 0 );

	/**
	 * The constructor
	 * @param element The BasicElement the FormulaRenderer will render
	 */
	FormulaRenderer( BasicElement* element = 0 );
	
	/// The destructor
	~FormulaRenderer();

	/**
	 * Render the given formula content to a @p QPainter
	 * @param p The QPainter to render to
	 */
	void render( QPainter* p );
	const QSize& defaultSize() const;
	const QSize& contentSize() const;
	void setDecoration( bool decorate );

	void setFormulaContainer( FormulaContainer* container );
	void setElement( BasicElement* element );

protected:
	void paintDecoration();

private:
	QList<BasicElement*> m_shownElements;
	FormulaContainer* m_formulaContainer;
	BasicElement* m_element;
	bool m_paintDecoration;
};

} // namespace KFormula

#endif

