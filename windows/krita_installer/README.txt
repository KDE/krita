c2winstaller README
-------------------

This includes instructions for preparing your environment to build an MSI installer for Calligra on Windows. 

Preparation
-----------

You will need to have a working KDE Windows environment which lets you build Calligra on the Windows platform.

The recommended folder structure is:
parent
+-c2winstaller
+-c2winstaller-input   (receives a packaged version of Calligra + KDE, output from package.bat)
+-c2winstaller-output  (intermediate and complete MSI written by build_msi.bat process)
+-c2winstaller-temp    (temp folder used by the package.bat process)
+-deps        (stores 3rd party dependencies - Visual C++ runtime components for the installer)
  +-vcredist
    +-DLLs
	+-MergeModules    (stores .msm files distrubuted with Visual Studio)
+-wix36       (install wix 3.6 here)

You will require the latest copy of the Wix toolset, 3.6 beta.
This may be downloaded from http://wixtoolset.org/

Checking the Environment
------------------------
You will need to ensure some paths are set prior to calling package.bat or build_msi.bat

To choose how/whether to distribute the VC++ runtime files, you must set C2WINSTALL_VC2010_DISTRIBUTE to point to the process you intend to use.
    DLL - informs wix that you want to distribute DLLs with it
	MSM - informs wix that you want to use the MergeModule
	(any other value is ignored and causes an error message to be displayed if it is not found, but a value must be set)
	
If wix has been installed anywhere other than ..\wix36, then you must set C2WINSTALL_WIX_BIN to point to it

You must set CALLIGRA_INST to point to the inst folder of the Calligra installation.
KDEROOT is set by kdeenv, so should not need to be explicitly declared.

C2WINSTALL_GITREV - the intention is for this to take a git revision identifier. This needs to be set in advance, and would normally be set by the build process which created the Calligra installation.
C2WINSTALL_VERSION - the version number of the package. This must consist of four sets of digits separate by periods (e.g. 2.3.87.1)

Currently the numbering convention for C2WINSTALL_VERSION uses the first three to identify the last main tagged release of calligra, and the fourth for subsequent builds from master.


Packaging Calligra
------------------
You will need to run the following within a working KDE for Windows environment.
From the c2winstaller directory, call:
env

This sets most of the required environment variables, using defaults unless you have overriden these previously.
You will need to set CALLIGRA_INST if you intend to package
You will need to set C2WINSTALL_VERSION 

Now you can run:
package

This will copy files from CALLIGRA_INST and KDEROOT in to C2WISNTALL_INPUT, packaging the Windows distribution of Calligra with the required KDE files.
The C2WINSTALL_TEMP folder contains some files that package touches, but it should clean this up on completion.
This should complete without errors (although you may need to prompt it to overwrite files ...may need to review this behaviour)

Once this has completed, you are ready to run:
build_msi

This will fun all the required steps:
  heat to read the C2WINSTALL_INPUT directory in order to generate HeatFragment.wxs
  candle to build the intermediate obj files fromm the xml sources
  light to build the msi file.
  
 
