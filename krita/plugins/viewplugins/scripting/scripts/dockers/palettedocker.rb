GC.disable
# GC.disable
# require 'Krita'
require 'KritaDockFactory'
if(require 'korundum4')
    class Kikoo < Qt::Object
        slots 'timeout()'
        def initialize()
            super()
            @timer = Qt::Timer.new
            connect(@timer, SIGNAL('timeout()'), SLOT('timeout()'))
            @timer.start(100)
        end
        def timeout
            while(true)
                puts "kikoo #{Qt::EventLoop::AllEvents | Qt::EventLoop::WaitForMoreEvents | Qt::EventLoop::DeferredDeletion}"
                Qt::Application.instance().processEvents(Qt::EventLoop::AllEvents | Qt::EventLoop::WaitForMoreEvents | Qt::EventLoop::DeferredDeletion)
            end
        end
    end 
    class PaletteWidget < Qt::Widget
        def initialize(parent)
            super(parent)
            @count = 5
        end
#         def paintEvent( event )
#             painter = Qt::Painter.new( self )
#             width = self.width
#             height = self.height
#             s = width / @count
#             s = height if s > height
#             for i in 0...@count
#                 painter.drawEllipse( i * s, 0, s, s )
#             end
#         end
#         def heightForWidth ( w )
#             return w / @count + 1
#         end
    end
    def createDockWidget
        $wdg = Qt::DockWidget.new(Qt::Object::tr("Palette"))
        $label = PaletteWidget.new($wdg)
        $wdg.setWidget($label)
        voidptr = Qt::Internal.smoke2kross($wdg)
        ko = Kross::Object::fromVoidPtr(voidptr)
#         $kikoo = Kikoo.new
        return ko
    end

end
