Krita installer README
-------------------

This includes instructions for preparing your environment to build an MSI installer for Krita 3 on Windows. 

Preparation
-----------

You will need to have a working KDE Windows environment which lets you build Calligra on the Windows platform.

The default folder structure is:
parent ($PACKAGER_ROOT)
+-krita
+-installer-input   (receives a packaged version of Krita + KDE, output from package.ps1)
+-installer-output  (intermediate and complete MSI written by build_msi.ps1 process)
+-installer-temp    (temp folder used by the package.ps1 process)
+-deps        (stores 3rd party dependencies - Visual C++ runtime components for the installer)
  +-vcredist
    +-DLLs
	+-MergeModules    (stores .msm files distrubuted with Visual Studio)
+-wix310       (install wix 3.10 here)

You will require the Wix toolset. Currently tested with 3.10.
This may be downloaded from http://wixtoolset.org/
Extract the .zip into the installation folder 

Setting up the environment
------------------------

The following script sets the necessary shell variables.  

> .\env.ps1

You will need to customize the script to, at a minimum, update the version
number. You should also ensure that the variables inside match your directory
structure. Check whether it worked correctly with the command:

> dir variables:

To choose how/whether to distribute the VC++ runtime files, you must set
KRITA_VC2015_DISTRIBUTE to point to the process you intend to use.

  DLL - informs wix that you want to distribute DLLs with it
	MSM - informs wix that you want to use the MergeModule

Any other value is ignored and causes an error message to be displayed if it is
not found.
	
If wix has been installed anywhere other than ..\wix36, then you must set
KRITA_WIX_BIN to point to it.

KRITA_VERSION - the version number of the package. This must consist of four
sets of digits separate by periods (e.g. 2.3.87.1).  Currently the numbering
convention for KRITA_VERSION uses the first three to identify the last main
tagged release of calligra, and the fourth for subsequent builds from master.

KRITA_GITREV - the intention is for this to take a git revision identifier.
This needs to be set in advance, and would normally be set by the build process
which created the Calligra installation.

PACKAGER_ROOT - working path for package.ps1 and build_msi.ps1

You must set CALLIGRA_INST to point to the inst folder of the Calligra
installation. KDEROOT is set by kdeenv, so should not need to be explicitly
declared.



Packaging Calligra
------------------

Now you can run:
.\package.ps1

This will copy files from KRITA_INST and KDEROOT in to KRITA_INPUT,
packaging the Windows distribution of Calligra with the required KDE files. The
KRITA_TEMP folder contains some files that package touches, but it should clean
this up on completion. This should complete without errors (although you may
need to prompt it to overwrite files ...may need to review this behaviour)

Once this has completed, you are ready to run:
.\build_msi.ps1

This will run all the required steps:
  heat to read the KRITA_INPUT directory in order to generate HeatFragment.wxs
  candle to build the intermediate obj files fromm the xml sources
  light to build the msi file.
  
 
