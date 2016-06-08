# -*- coding: utf-8 -*-
import info

class subinfo( info.infoclass ):
    def setTargets( self ):
        self.targets[ '3.2.2' ] = 'http://www.fftw.org/fftw-3.2.2.tar.gz'
        self.targetDigests[ '3.2.2' ] = 'd43b799eedfb9408f62f9f056f5e8a645618467b'
        self.targetInstSrc[ '3.2.2' ] = "fftw-3.2.2"
        self.patchToApply[ '3.2.2' ] = [ ( 'fftw-3.2.2-20111221.diff', 1 ),
                                         ( 'fftw-3.2.2-20130818.diff', 1 ) ]
        self.shortDescription = "a C subroutine library for computing the discrete Fourier transform (DFT)"

        self.defaultTarget = '3.2.2'

    def setDependencies( self ):
        self.buildDependencies[ 'virtual/base' ] = 'default'

from Package.CMakePackageBase import *

class Package( CMakePackageBase ):
    def __init__( self ):
        CMakePackageBase.__init__( self )
        self.supportsNinja = False
        self.subinfo.options.configure.defines = "-DFFTW_SINGLE=ON -DFFTW_DOUBLE=OFF -DBUILD_BENCHMARKS=OFF"

