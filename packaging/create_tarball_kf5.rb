#!/usr/bin/ruby

#
# Ruby script for generating tarball releases of KDE repositories (git or SVN).
# This script can create tarballs with source code, translations and documentation
# for the given KF5-based project(s). For KDE4-based projects, use create_tarball.
#
# (c) 2005 Mark Kretschmann <markey@web.de>
# (c) 2006-2008 Tom Albers <tomalbers@kde.nl>
# (c) 2007 Angelo Naselli <anaselli@linux.it> (command line parameters)
# Some parts of this code taken from cvs2dist
SPDX-License-Identifier: GPL-2.0-only

require 'optparse'
require 'ostruct'
require 'find'
require 'fileutils'

# check command line parameters
options = OpenStruct.new
options.help  = false
options.https = false
options.ask   = true
options.translations = true

opts = OptionParser.new do |opts|
  opts.on("-u", "--user USERNAME", "svn account") do |u|
    options.username = u
  end
  opts.on("-w", "--https", "Using https instead of svn+ssh") do |w|
    options.https = w
  end
  opts.on("-n", "--noaccount", "Using svn://anonsvn.kde.org/ instead of svn+ssh") do |n|
    options.anonsvn = n
  end
  opts.on("-a", "--application APPL", "Application name (all for all, kde_release for apps that have kde_release=yes)") do |a|
    options.application = a
    options.ask = false
  end
  opts.on("-v", "--version VER", "Overwrite package version set in config.ini") do |v|
    options.ver = v
  end
  opts.on_tail("-h", "--help", "Show this usage statement") do |h|
    options.help = true
  end
  opts.on("-r", "--revision REV", "Use a specific revision of the repository") do |r|
    options.rev = r
  end
  opts.on("-t", "--no-translations", "Don't include translations") do |t|
    options.translations = false
  end
end

begin
  opts.parse!(ARGV)
rescue Exception => e
  puts e, "", opts
  puts
  exit
end

if (options.username)
  username = options.username + "@"
end

if (options.application)
  apps = Array.new
  apps << options.application
end

if (options.https)
  if (username)
    svnbase    = "https://#{username}svn.kde.org/home/kde"
  else
    puts opts
    puts
    puts "Username is mandatory with https"
    exit
  end
else
  svnbase    = "svn+ssh://#{username}svn.kde.org/home/kde"
end

if (options.anonsvn)
  if (options.https)
     puts opts
     puts
     puts "https or anonsvn please, not both"
     exit
   end
   svnbase    = "svn://anonsvn.kde.org/home/kde"
end

if (options.help)
  puts opts
  exit
end

############# START #############

kde_version  = `svn ls svn://anonsvn.kde.org/home/kde/tags/KDE | sort | tail -n1 | cut -d "/" -f1`.chomp

#----------------------------------------------------------------
# initiate.
#----------------------------------------------------------------

f = File.new("config.ini")
app = Array.new
begin
    while (line = f.readline)
        aline = line.chomp
        if aline[0,1] == "["
            app << aline[1,(aline.length-2)]
        end
    end
rescue EOFError
    f.close
end

puts "Last KDE version found: " + kde_version
if (options.ask)
    puts "Which apps (multiple sep by space, possibilities: all kde_release " + app.join(" ") + ")?"
    apps = gets.split(" ")
end

kde_release = false;
if apps[0] == "all"
    apps = app
elsif apps[0] == "kde_release"
    apps = app
    kde_release = true;
end

puts "-> Considering " + apps.join(" & ")
if kde_release
    puts " -> Only applications which have kde_release = yes in config "
end
puts

#----------------------------------------------------------------
# retrieve apps.
#----------------------------------------------------------------

apps.each do |app|
    puts
    puts "-> Processing " + app

    found = false;
    appdata = Hash.new
    f = File.new("config.ini")
    begin
        while (line = f.readline)
            aline = line.chomp
            if aline == "[" + app + "]"
                found = true;
            elsif aline.length > 0 && found
                data = aline.split("=");
                temp = { data[0].strip => data[1].strip }
                appdata = appdata.merge(temp)
            else
                found = false
            end
        end
        rescue EOFError
        f.close
    end

    if !found
        puts " -> Application '#{app}' not found."
        next
    end

    if (kde_release && appdata["kde_release"] != "yes")
      puts "  -> Skipping because kde_release is not set in the config.ini"
      next
    end

    if (options.ver)
      temp = { "version" => options.ver }
      appdata = appdata.merge(temp)
    else
      if !appdata["version"]
        temp = { "version" => kde_version }
        appdata = appdata.merge(temp)
      else
        if kde_release
           temp = { "version" => appdata["version"] + "-kde" + kde_version }
           appdata = appdata.merge(temp)
        end
      end
    end

    if !appdata["name"]
        temp = { "name" => app }
        appdata = appdata.merge(temp)
    end

    if appdata["folder"]
        app = appdata["folder"]
    end

    if !appdata["folder"] || appdata["name"]
        temp = { "folder" => appdata["name"] + "-" + appdata["version"] }
    else
        temp = { "folder" => appdata["folder"] + "-" + appdata["version"] }
    end
    appdata = appdata.merge(temp)

    if appdata["addPo"] && appdata["addPo"].length > 0
        temp = { "addPo" =>  (appdata["addPo"]+" "+app).split(" ") }
    else
        temp = { "addPo" =>  [app] }
    end
    appdata = appdata.merge(temp)

    if appdata["addDocs"] && appdata["addDocs"].length > 0
        temp = { "addDocs" =>  (appdata["addDocs"]+" "+app).split(" ") }
    else
        temp = { "addDocs" =>  [app] }
    end
    appdata = appdata.merge(temp)

    tmpl10nmodule = appdata["l10nmodule"]

    if appdata["submodule"] && appdata["submodule"].length > 0
        if appdata["mainmodule"] == "extragear" || appdata["mainmodule"] == "playground"
          temp = { "submodulepath" => appdata["submodule"] + "/", "l10nmodule" => appdata["mainmodule"] + "-" + appdata["submodule"] }
        else
          temp = { "submodulepath" => appdata["submodule"] + "/", "l10nmodule" => appdata["mainmodule"] }
        end
    else
        temp = { "submodulepath" => "", "l10nmodule" => appdata["mainmodule"] }
    end
    appdata = appdata.merge(temp)

    # if l10nmodule is specified in the config file, then use it
    if tmpl10nmodule
      temp = { "l10nmodule" => tmpl10nmodule }
      appdata = appdata.merge(temp)
    end

    if !appdata["customlang"]
        temp = { "customlang" => [] }
    end
    appdata = appdata.merge(temp)

    # Preparing
    rev = ""
    revString = ""
    if (options.rev)
      rev = "-r " + options.rev
      revString = " Rev " + options.rev
    end

    if appdata["gitModule"]
        if !appdata["category"]
            appdata["category"] = "kde"
        end
        if !appdata["gitTag"]
            appData["gitTag"] = "master"
        end
        puts "-> Fetching https://invent.kde.org/#{appdata["category"]}/#{app}/-/archive/#{appdata["gitTag"]}/#{app}-#{appdata["gitTag"]}.tar.gz"
    else
        puts "-> Fetching " + appdata["mainmodule"] + "/" + appdata["submodulepath"] + app + revString + " into " + appdata["folder"] + "..."
    end
    # Remove old folder, if exists
    `rm -rf #{appdata["folder"]} 2> /dev/null`
    `rm -f #{appdata["folder"]}.tar.gz 2> /dev/null`
    Dir.mkdir( appdata["folder"] )
    Dir.chdir( appdata["folder"] )

    if appdata["mainmodule"][0,5] == "trunk" || appdata["mainmodule"][0,8] == "branches"
        svnroot = "#{svnbase}/"
        if !appdata["l10npath"]
            appdata["l10npath"] = appdata["mainmodule"]
        end
    else
        #trunk is assumed for all mainmodules that don't start with "trunk" or "branches"
        svnroot = "#{svnbase}/trunk/"

        if !appdata["l10npath"]
            appdata["l10npath"] = ""
        end
    end

    # Do the main checkouts.
    if appdata["gitModule"]
        `curl "https://invent.kde.org/#{appdata["category"]}/#{app}/-/archive/#{appdata["gitTag"]}/#{app}-#{appdata["gitTag"]}.tar.gz" | tar xz --strip-components=1`
    else
        if appdata["wholeModule"]
            `svn co #{svnroot}/#{appdata["mainmodule"]}/#{appdata["submodulepath"]} #{rev} #{app}-tmp`
        else
            `svn co #{svnroot}/#{appdata["mainmodule"]}/#{appdata["submodulepath"]}#{app} #{rev} #{app}-tmp`
        end
        Dir.chdir( app + "-tmp" )

        if appdata["docs"] != "no"
            if !appdata["docpath"]
                temp = { "docpath" => "doc/#{app}" }
                appdata = appdata.merge(temp)
            end
            `svn co #{svnroot}/#{appdata["mainmodule"]}/#{appdata["submodulepath"]}/#{appdata["docpath"]} #{rev} doc`
        end

        # Move them to the toplevel
        `/bin/mv * ..`
        Dir.chdir( ".." )

        `find -name ".svn" | xargs rm -rf`
        `rm -rf #{app}-tmp`
    end

    # translations
    if appdata["translations"] != "no" && options.translations
        puts "-> Fetching l10n docs for #{appdata["submodulepath"]}#{app} #{revString} from '#{svnroot}/#{appdata["l10npath"]}/l10n-kf5/subdirs #{rev}'..."

        i18nlangs = `svn cat #{svnroot}/#{appdata["l10npath"]}/l10n-kf5/subdirs #{rev}`.split
        i18nlangsCleaned = []
        for lang in i18nlangs
            l = lang.chomp
            if (l != "x-test") && (appdata["customlang"].empty? || appdata["customlang"].include?(l))
                i18nlangsCleaned += [l];
            end
        end
        i18nlangs = i18nlangsCleaned

        Dir.mkdir( "po" )

        Dir.mkdir( "l10n" )
        Dir.chdir( "l10n" )

        # docs
        for lang in i18nlangs
            lang.chomp!

            for dg in appdata["addDocs"]
                dg.chomp!
                `rm -rf #{dg}`
                docdirname = "#{appdata["l10npath"]}/l10n-kf5/#{lang}/docs/#{appdata["l10nmodule"]}/#{dg}"
                if ( appdata["docs"] != "no" )
                    puts "  -> Checking if #{lang} has translated documentation...\n"
                    if dg.include? "/"
                        `svn co -q #{rev} #{svnroot}/#{docdirname} #{dg} > /dev/null 2>&1`
                    else
                        `svn co -q #{rev} #{svnroot}/#{docdirname} > /dev/null 2>&1`
                    end
                end
                next unless FileTest.exists?( dg + '/index.docbook' )

                print "    -> Copying #{lang}'s #{dg} documentation over...  "
                if dg.include? "/"
                    FileUtils.mkdir_p( "../po/#{lang}/docs/#{dg}" )
                    `cp -R #{dg}/* ../po/#{lang}/docs/#{dg}`
                else
                    FileUtils.mkdir_p( "../po/#{lang}/docs/#{dg}/" )
                    `cp -R #{dg}/ ../po/#{lang}/docs`
                end

                puts( "done.\n" )
            end
        end

        # app translations
        puts "-> Fetching l10n po for #{appdata["submodulepath"]}#{app} from '#{svnroot}/#{appdata["l10npath"]}/l10n-kf5/'..."

        Dir.chdir( ".." ) # in submodule now

        $subdirs = false

        for lang in i18nlangs
            lang.chomp!
            dest = "po/#{lang}"

            for dg in appdata["addPo"]
                dg.chomp!
                if appdata["wholeModule"]
                    pofolder = "#{appdata["l10npath"]}/l10n-kf5/#{lang}/messages/#{appdata["l10nmodule"]}"
                    print "  -> Copying #{lang}'s over ..#{pofolder}\n"
                    `svn co #{svnroot}/#{pofolder} #{dest}`
                    next if !FileTest.exist?( dest )

                elsif appdata["custompo"]
                    valid = false
                    for sp in appdata["custompo"].split(/,/)
                        pofilename = "#{appdata["l10npath"]}/l10n-kf5/#{lang}/messages/#{appdata["l10nmodule"]}/#{sp}.po"
                        `svn cat #{svnroot}/#{pofilename} #{rev} 2> /dev/null | tee l10n/#{sp}.po`
                        if not FileTest.size( "l10n/#{sp}.po" ) == 0
                            valid=true
                            if !FileTest.exist?( dest )
                                Dir.mkdir( dest )
                            end
                            print "\n  -> Copying #{lang}'s #{sp}.po over ..  "
                            `mv l10n/#{sp}.po #{dest}`
                        end
                    end
                    next if not valid
                else
                    pofilename = "#{appdata["l10npath"]}/l10n-kf5/#{lang}/messages/#{appdata["l10nmodule"]}/#{dg}.po"
                    `svn cat #{svnroot}/#{pofilename} #{rev} 2> /dev/null | tee l10n/#{dg}.po`
                    next if FileTest.size( "l10n/#{dg}.po" ) == 0

                    if !FileTest.exist?( dest )
                        Dir.mkdir( dest )
                    end

                    print "  -> Copying #{lang}'s #{dg}.po over ..  "
                    `mv l10n/#{dg}.po #{dest}`
                    puts( "done.\n" )
                end
            end
        end

        `rm -rf l10n`

        if appdata["docs"] != "no"
            # add docs to compilation.
            `echo "find_package(KF5DocTools CONFIG REQUIRED)" >> CMakeLists.txt`
            `echo "kdoctools_install(po)" >> CMakeLists.txt`
        end
    end

    datafolder = appdata["l10ndata"]
    if datafolder
        if !FileTest.exists?( "l10ndata_temp" )
            puts "-> Fetching l10n data from #{datafolder} #{revString}..."

            i18nlangs = `svn cat #{svnroot}/#{appdata["l10npath"]}/l10n-kf5/subdirs #{rev}`.split
            i18nlangsCleaned = []
            for lang in i18nlangs
                l = lang.chomp
                if (l != "x-test") && (appdata["customlang"].empty? || appdata["customlang"].include?(l))
                    i18nlangsCleaned += [l];
                end
            end
            i18nlangs = i18nlangsCleaned

            Dir.mkdir( "l10ndata" )
            topmakefile = File.new( "l10ndata/CMakeLists.txt", File::CREAT | File::RDWR | File::TRUNC )

            # data
            for lang in i18nlangs
                lang.chomp!

                docdirname = "#{appdata["l10npath"]}/l10n-kf5/#{lang}/data/#{datafolder}"
                puts "  -> Checking if #{lang} has localized data...\n"
                `rm -rf l10ndata_temp`
                `svn co -q #{rev} #{svnroot}/#{docdirname} l10ndata_temp 2> /dev/null 2>&1`
                next unless FileTest.exists?( 'l10ndata_temp/CMakeLists.txt' )

                topmakefile << "add_subdirectory( #{lang} )\n"

                print "  -> Copying #{lang}'s data over ..  "
                `mv l10ndata_temp l10ndata/#{lang}`

                puts( "done.\n" )
            end
            topmakefile.close()

            # add data to compilation.
            `echo "add_subdirectory( l10ndata )" >> CMakeLists.txt`
        else
            puts "l10ndata_temp folder exists in source, could not add l10ndata"
        end
    end

    # add doc generation to compilation
    if (appdata["docs"] != "no") && (!appdata["gitModule"])
        `echo "add_subdirectory( doc )" >> CMakeLists.txt`
    end

    # Remove cruft
    `find -name ".svn" | xargs rm -rf`
    if ( appdata["remove"] != "")
        `/bin/rm -rf #{appdata["remove"]}`
    end

    print "-> Compressing ..  "
    Dir.chdir( ".." ) # root folder
    `tar -czf #{appdata["folder"]}.tar.gz --format=gnu --group=root --owner=root  #{appdata["folder"]}`
    `tar -cJf #{appdata["folder"]}.tar.xz --format=gnu --group=root --owner=root  #{appdata["folder"]}`
    #`rm -rf #{appdata["folder"]}`
    puts " done."
    puts ""
    print "md5sum: ", `md5sum #{appdata["folder"]}.tar.gz`
    print "sha256sum: ", `sha256sum #{appdata["folder"]}.tar.gz`
end

