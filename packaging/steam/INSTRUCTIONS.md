Steam is a little bit involved, but I have it pretty much down to a system now. :)

The entire process is documented on https://partner.steamgames.com/doc/sdk, but here's the rundown:

1. Set up a **[Steamworks](https://partner.steamgames.com/)** account, and download the [Steamworks SDK](https://partner.steamgames.com/doc/sdk) somewhere on your system. (Works on both Windows and Linux, and maybe MacOSX too, but I'm not sure about that.)

1. Download stable **Krita** binaries for all Steam platforms (right now that's just Linux and Windows); place the respective content into `windows/` and `linux/` subdirectories inside `sdk/tools/ContentBuilder/content/`.

   - ##### Linux

     For Steam we currently run Krita through a small `launch.sh` script. This is used to circumvent Steam's built-in "linux runtime", since we don't need it and it interferes with appimages. `launch.sh` expects the appimage to be named `krita.AppImage`, so we name it that.

   - ##### Windows

     On Windows, Steam is configured to launch Krita at the relative path `krita\bin\krita.exe` so I usually rename the folder that holds our portable build to just `krita`. This could be changed, but it works for me.

1. With the SDK downloaded and the content in the right place, we can //almost// build and push, but first we need to set up "**SteamPipe Build Scripts**" (https://partner.steamgames.com/doc/sdk/uploading#3).

   - This is a script that we feed to `tools\ContentBuilder\builder\steamcmd.exe` that contains, among other things, Krita's **`AppID`** (which you can find on Steamworks), the path where our content is located, the path where we want to build to, and the path to a separate build script for each of our **Depots**. 

   - A "depot" is basically what Steam calls a package. Each depot has a unique DepotID number. Through Steamworks' web interface we can create depots, and through the SteamworksSDK we can push our built content to them.
   
   For Krita we have 4 depots (Krita Common, Krita Windows, Krita Linux, and Krita MacOSX). However, right now we only really use the Windows and Linux ones.
   
   The other ones were made just in case we released on OSX or had some common data that we could share between all platforms. When I got involved with Krita on Steam I think we had 10-12 depots! Haha.

   - Each depot has a script that we referenced in our main App build script, and it's basically just used to tell the SDK where the content for that particular depot lives. (The windows depot, for example, points to the `sdk/tools/ContentBuilder/content/windows/` on my machine.)

   - This is the worst part of the entire thing, but it really only has to be set up once--make the depots on Steamworks website, and then write a build script for the App and for each of the 4 depots, keep them on your computer forever.

1. Then, run `sdk/tools/ContentBuilder/builder_linux/steamcmd.sh +login <STEAM_USERNAME> <STEAM_PASSWORD> +run_app_build_http -desc "Krita Desktop Build" <PATH_TO_APP_BUILD_SCRIPT>` (or the Windows equivalent), which will log you into Steamworks, build all of our depots and hopefully push them up to Steam.

1. At this point the newest build has been pushed to Steam--but it's not LIVE yet! In order to send it out to our Steam users we have to set the `default` branch to the latest build in the "SteamPipe" section of the Steamworks web interface.

   ##### Note

   We also have a `beta` branch that we can use for pushing beta builds if needed, as well as a `rollback` branch that I keep pointing to the last minor version build for Steam users that want the option of jumping back a version. (Both are opt-in through Steam's GUI.)

1. Finally, we should tell our users what's been updated and thank them for their support, and we can do that through the `Post/Manage Events & Announcements` section of the Steamworks web interface.