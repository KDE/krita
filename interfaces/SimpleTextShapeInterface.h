#ifndef SIMPLETEXT_H
#define SIMPLETEXT_H

#include <KoShape.h>

/**
 * Interface for the SimpleTextShape plugin, originally written for Karbon
 * 
 * Use this pure virtual class instead of using SimpleTextShape directly
 * to avoid unnecessary dependencies of your code, as all plugins are optional.
 */
class SimpleTextShapeInterface : public KoShape
{
public:

    /// Sets the text to display
    virtual void setText( const QString & text ) = 0;

    /// Returns the text content
    virtual QString text() const = 0;
    
    virtual ~SimpleTextShapeInterface() {};

    /**
     * Sets the font used for drawing
     * Note that it is expected that the font has its point size set
     * in postscript points.
     */
    virtual void setFont( const QFont & font ) = 0;

    /// Returns the font
    virtual QFont font() const = 0;
};

#endif

