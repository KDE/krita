require 'Krita'
require 'KritaFilter'

def scriptProcess( src, srcTopLeft, dst, dstTopLeft, size, config )
    puts "LOOOOOOOOOOOOOOOOOOOOOOOL"
    puts KritaFilter.category()
    puts src
    puts srcTopLeft
    puts dst
    puts dstTopLeft
    puts size
  for i in 0...size[1]
    itSrc = src.createHLineConstIterator( srcTopLeft[0], srcTopLeft[1] + i, size[0] )
    itDst = dst.createHLineIterator( dstTopLeft[0], dstTopLeft[1] + i, size[0] )
    while (not itDst.isDone())
      itDst.copyFrom(itSrc)
      itDst.invertColor()
      itSrc.next()
      itDst.next()
    end
  end
end

