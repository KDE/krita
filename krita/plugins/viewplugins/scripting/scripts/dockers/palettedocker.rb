require 'KritaDockFactory'
require 'korundum4'
class PaletteWidget < Qt::Widget
    def initialize(parent)
        super(parent)
        @count = 5
        @painter = Qt::Painter.new()
    end
    def paintEvent( event )
        @painter.begin(self)
        width = self.width
        height = self.height
        s = width / @count
        s = height if s > height
        for i in 0...@count
            @painter.drawEllipse( i * s, 0, s, s )
        end
        @painter.end()
    end
    def heightForWidth ( w )
        return w / @count + 1
    end
end
def createDockWidget
    wdg = Qt::DockWidget.new(Qt::Object::tr("Ruby Palette"))
    label = PaletteWidget.new(wdg)
    wdg.setWidget(label)
    return wdg
end
