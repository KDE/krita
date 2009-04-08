#  Copyright (c) 2007-2008 Cyrille Berger <cberger@cberger.net>
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

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
