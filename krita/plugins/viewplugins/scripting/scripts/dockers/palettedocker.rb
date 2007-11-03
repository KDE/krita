# require 'Krita'
require 'KritaDockFactory'
if(require 'Qt')
#     class Kikoo < Qt::Object
#         slots 'timeout()'
#         def initialize()
#             super()
#             @timer = Qt::Timer.new
#             connect(@timer, SIGNAL('timeout()'), SLOT('timeout()'))
#             @timer.start(100) 
#         end
#         def timeout
#             while(true)
#                 puts "kikoo"
#                 Qt::Application.instance().processEvents
#                 sleep(0.1)
#             end
#         end
#     end 
    class PaletteWidget < Qt::Widget
        slots 'slotLinkActivated(QString)'
        def initialize(parent)
            super(parent)
            @count = 5
            @label = Qt::Label.new("<a href=\"myLink\">Link</a>", self)
            connect(@label, SIGNAL('linkActivated(QString)'), self, SLOT('slotLinkActivated(QString)'))
        end
        def slotLinkActivated(link)
            puts "slotLinkActivated " + link
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
        #$kikoo = Kikoo.new
        voidptr = Qt::Internal.smoke2kross($wdg)
        ko = Kross::Object::fromVoidPtr(voidptr)
        return ko
    end

end
