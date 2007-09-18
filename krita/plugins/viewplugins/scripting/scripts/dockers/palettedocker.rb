# require 'Krita'
require 'KritaDockFactory'
if(require 'Qt')

    def createDockWidget
        GC.disable
        wdg = Qt::DockWidget.new(Qt::Object::tr("Palette"))
        label = Qt::Label.new( "Hello world",wdg)
        wdg.setWidget(label)
        voidptr = Qt::Internal.smoke2kross(wdg)
        ko = Kross::Object::fromVoidPtr(voidptr)
        return ko
    end

end
