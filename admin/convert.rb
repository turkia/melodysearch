#!/usr/bin/env ruby

# C-Brahms Engine for Musical Information Retrieval
# University of Helsinki, Department of Computer Science
#
# Version 0.2.4, May 15th, 2003
#
# Copyright Mika Turkia
#
# This script takes a directory of MIDI files as an argument and creates a .songs file,
# that contains musical data from the MIDI files in a converted form.
#
# Usage: convert.rb <mididir>

require_relative '../lib/songcollection'

# This script takes a directory of MIDI files as an argument and creates a songs file. 
if ARGV.size != 1 then puts "Usage: convert.rb <mididir>"
else 
	mididir = ARGV[0]
	if File.directory?(mididir) then
		r = MIR::SongCollection.new
		start = Time.new
		r.convert_midifiles(mididir)
		elapsed = Time.new - start
		# save to current directory
		r.save(mididir.sub('.*\/([\w\d_\-]+)$','\1'))
		puts "Conversion time: #{elapsed}"
	else
		puts "Note: to convert files, you must select a directory containing MIDI files, not a single file."
	end
end
