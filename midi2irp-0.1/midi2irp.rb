#!/usr/bin/env ruby

# C-Brahms Engine for Musical Information Retrieval
# University of Helsinki, Department of Computer Science
#
# Version 0.2.4, May 12th, 2003
#
# Copyright Mika Turkia
#
# Merges all tracks and prints IRP format to a file. 
# (Note that e.g. Semex prints tracks separately.)
# First line printed is MIDI division.
# IRP format is <strt, dur, symbdur, IOI, ptch, int, dyn, voic, acc, deg, typ, txtstrt, txtend>. 
# (voic field is MIDI track number.)
#
# Usage: midi2irp.rb <mididir>

require_relative 'lib/midiconvert'

def print_irp(filename, s)

	notes = s.notes

	if notes.size == 0 then return nil end

	# sort notes according to start time and pitch
	notes.sort! do |a, b|
		if a[0] == b[0] then a[1] <=> b[1]
		else a[0] <=> b[0] end
	end

	# loop through chords
	strt = notes[0][0]
	prevnote = nil
	tempchord = [ notes[0] ]

	File.open(filename, "w") do |file|

		file.puts "#{s.division}\n"

		for i in 1..notes.size do

			if notes[i] and notes[i][0] == strt then
				# chord boundary was not crossed
				tempchord.push(notes[i])
			else
				# chord boundary was crossed: process chord
				tempchord.each do |note|
					# <strt,dur,symbdur,IOI,ptch,int,dyn,voic,acc,deg,typ,txtstrt,txtend> 
					if prevnote then file.puts "<#{note[0]},#{note[2]},-,-,#{note[1]},#{note[1]-prevnote[1]},-,#{note[3]},isk,deg,typ,n:k>"
					else file.puts "<#{note[0]},#{note[2]},-,-,#{note[1]},#{note[1]},-,#{note[3]},isk,deg,typ,n:k>" end
					prevnote = note
				end

				# move to next chord if not at the end of source
				if notes[i]
					strt = notes[i][0]
					tempchord = [ notes[i] ]
				end
			end
		end
        end
	notes = nil
end


# Converts MIDI files in a given directory to IRP. 
# This method merely just calls MIDI conversion method SMF::Sequence class and an IRP printing method. 
def convert_midifiles(path)
	#if path then path.gsub!('//', '/') end
	re = /\.mid$/

	if File.directory?(path) and path !~ /\.$/
		Dir.foreach(path) do |entry| convert_midifiles(path + '/' + entry) end
	elsif re.match(path)
		puts "Converting #{path}..."
		begin
			sq = SMF::Sequence.decodefile(path)
			s = sq.convert
			if not s then return nil end

			print_irp(path.sub(re, '.irp'), s)

		rescue	# SMF::Sequence::ReadError
			puts "Error: skipping file #{path}."
			return nil
		end
	end
end


# This script takes a directory of MIDI files as an argument and creates a songs file. 
if ARGV.size != 1 then puts "Usage: midi2irp.rb <mididir>"
else 
	mididir = ARGV[0]
	if File.directory?(mididir) then convert_midifiles(mididir)
	else puts "Note: to convert files, you must select a directory containing MIDI files, not a single file." end
end
