require 'Krita'
require 'KritaFilter'

def scriptProcess( src, srcTopLeft, dst, dstTopLeft, size, config )
  for i in 0...size[1]
    itDst = dst.createHLineIterator( dstTopLeft[0], dstTopLeft[1] + i, size[0] )
    while (not itDst.isDone())
      for k in 0...itDst.channelCount
        itDst.setChannel(k, (255*rand()).to_i)
      end
      itDst.next()
    end
  end
end

