require 'rake/clean'

EXT_CONF = 'lib/csong/extconf.rb'
MAKEFILE = 'lib/csong/Makefile'
MODULE = 'lib/csong/Song.bundle'  # .bundle for OS X, .so for Linux
SRC = Dir.glob('lib/csong/*.c')
SRC << MAKEFILE

EXT_CONF2 = 'lib/cserver/extconf.rb'
MAKEFILE2 = 'lib/cserver/Makefile'
MODULE2 = 'lib/cserver/Server.bundle'
SRC2 = Dir.glob('lib/cserver/*.c')
SRC2 << MAKEFILE2

CLEAN.include [ 'lib/c*/*.o', 'lib/c*/depend', MODULE, MODULE2]
CLOBBER.include [ 'config.save', 'lib/cs*/mkmf.log', 'lib/cs*/extconf.h', MAKEFILE, MAKEFILE2 ]

file MAKEFILE => EXT_CONF do |t|
	Dir::chdir(File::dirname(EXT_CONF)) do
			unless sh "ruby #{File::basename(EXT_CONF)}"
			$stderr.puts "Failed to run extconf"
			break
		end
	end
end

file MODULE => SRC do |t|
	Dir::chdir(File::dirname(EXT_CONF)) do
		unless sh "make"
			$stderr.puts "make failed"
			break
		end
	end
end

file MAKEFILE2 => EXT_CONF2 do |t|
	Dir::chdir(File::dirname(EXT_CONF2)) do
			unless sh "ruby #{File::basename(EXT_CONF2)}"
			$stderr.puts "Failed to run extconf"
			break
		end
	end
end

file MODULE2 => SRC2 do |t|
	Dir::chdir(File::dirname(EXT_CONF2)) do
		unless sh "make"
			$stderr.puts "make failed"
			break
		end
	end
end

desc "Build the native algorithm library"
task :build_song => MODULE

desc "Build the native server library"
task :build_server => MODULE2

desc "Build all native modules"
task :build => [:build_song, :build_server]
