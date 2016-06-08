:: Copyright (c) 2011-2012 KO GmbH.  All rights reserved.
:: Copyright (c) 2011-2012 Stuart Dickson <stuartmd@kogmbh.com>
::
:: The use and distribution terms for this software are covered by the
:: Common Public License 1.0 (http://opensource.org/licenses/cpl1.0.php)
:: which can be found in the file CPL.TXT at the root of this distribution.
:: By using this software in any fashion, you are agreeing to be bound by
:: the terms of this license.
::  
:: You must not remove this notice, or any other, from this software.
:: ------------------------------------------------------------------------
"m:\package\wix36\heat.exe" dir "%C2WINSTALL_INPUT%" -cg CalligraBaseFiles -gg -scom -sreg -sfrag -srd -dr CALLIGRADIR -var env.C2WINSTALL_INPUT -out "%~dp0HeatFragment.wxs"
copy HeatFragment.wxs PreviousHeatFragment.xml /Y
