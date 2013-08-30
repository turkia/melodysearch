#!/usr/bin/env ruby

# C-Brahms Engine for Musical Information Retrieval
# University of Helsinki, Department of Computer Science
#
# Version 0.2.4, May 15th, 2003
#
# Copyright Mika Turkia
#
# Contacts: turkia at cs helsinki fi
#
# This script takes a filepath of a .songs file as an argument and creates a HTML file containing metadata of the songs.
#
# Usage: create_metahtml.rb <songsfile>

require_relative '../lib/songcollection'

if ARGV.size != 1 then puts "usage: create_metahtml.rb <songsfile>"
else 
	name = ARGV[0].dup

	File.open("public/meta_#{name}.html", "w") do |file|
		s = MIR::SongCollection.new
		s.load(name.sub(/\.songs$/, ''))
		file.puts s.meta_to_html
	end
end
