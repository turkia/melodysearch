# C-Brahms Engine for Musical Information Retrieval
# University of Helsinki, Department of Computer Science
#
# Version 0.2.6, August 5th, 2003
#
# Copyright Mika Turkia

require 'smf'
require 'zlib'

require_relative 'csong/Song'

require_relative 'midiconvert'
require_relative 'primes'

# This module contains classes for Musical Information Retrieval. 
# This file is part of C-Brahms Engine for Musical Information Retrieval.
#
# Copyright Mika Turkia 2002-2003.
# Released under the GPL license (see individual files for more information).
module MIR


# A class for song objects, that hold all information about one song. Instances are created by converting note data from MIDI files.
# It contains methods implemented in both Ruby and C.
# C implementations are in directory lib/csong.
class Song

	# A string containing byte data. Format: a sequence of chord headers and notes.
	# A chord header is 5 bytes, first of which is chord length, and the next 4 bytes is onset time for notes in this chord (strt).
	# Both are unsigned. Chord header length is defined in song.h.
	# Notes consist of 1 byte for pitch, 2 bytes for duration, and 1 byte for track (voic). Pitch is char, duration is unsigned short, voic is char.
	# Note length is defined in song.h; additional note parameters may be added by changing the length.
	# It should not affect existing algorithms.
	attr_reader :chords

	# Number of chords in this song. 
	attr_reader :num_chords

	# Number of notes in this song, excluding duplicates (notes having same pitch in the same chord). 
	attr_reader :num_notes

	# Number of notes in this song, including duplicates (notes having same pitch in the same chord). 
	attr_reader :num_notes_with_duplicates

	# Maximum polyphony (number of notes having the same onset time) in this song, excluding duplicates (notes having same pitch in the same chord).
	attr_reader :maxpoly

	# Maximum polyphony (number of notes having the same onset time) in this song, including duplicates (notes having same pitch in the same chord).
	attr_reader :maxpoly_with_duplicates

	# MIDI division that is used in this song in representing time. A given number of units represents the duration of one quarter note.
	attr_reader :quarternoteduration

	# Number of tracks in this song.
	attr_reader :num_tracks

	# An array containing lengths of the tracks. 
	attr_reader :tracklengths

	# A string containing preprocessed byte data for monopoly.
	# Data format: first 4 bytes containing offset from start of chords data for this chord.
	# Next 2 bytes containing all octave-equivalent intervals in this chord and previous chord.
	# Intervals are coded to bits so that if interval is present, then the corresponding bit has value 0, else it has value 1. 
	attr_reader :preprocessed

	# An array of primes for SIA(M)E1 algorithm's hash table. Array contains a prime to be used as hash table size for each pattern size. 
	# Note that SIA(M)E1 is not included in public package due to patent reasons. 
	attr_reader :primes

	# MIDI file path.
	attr_accessor :filepath

	# PS file path.
	attr_accessor :scoreurl

	# MIDI URL. 
	attr_accessor :midiurl

	# Metadata fields related to Mutopia collection. They depend on special metadata text files. Not used for other MIDI files.
	attr_accessor :composer, :title, :opus, :style, :instruments, :date

	# Vocabulary size for bit-parallel algorithms. Here it is the size of an octave (12 semitones).
	@@Vocsize = 12


	# Returns a song object, that holds converted note data, preprocessed data, metadata (e.g. composer, title), statistics about this song, etc. 
	def initialize(result)
		@chords = ""
		@num_chords = 0
		@tracklengths = []
		@max_tracklength = 0
		@num_notes = 0
		@num_notes_with_duplicates = 0
		@preprocessed = nil
		@maxpoly = 0
		@maxpoly_with_duplicates = 0
		@primes = []

		# MIDI's division parameter; used to calculate symbolic durations
		@quarternoteduration = result.division

		# for [strt, signature] pairs
		@timesignatures = result.timesignatures
		@keysignatures = result.keysignatures
		@num_tracks = result.num_tracks
		@division = result.division
		# for collecting all meta text in midi file
		@metatext = Zlib::Deflate.deflate(result.metatext)

		# initialize tracklength array
		@tracklengths = []
		@num_tracks.times do |i| @tracklengths[i + 1] = 0 end

		create_tracks_and_chords(result.notes)
		result = nil
		preprocess_monopoly
	end

	# MetaText events collected from MIDI file.
	def metatext
		if @metatext
			Zlib::Inflate.inflate(@metatext)
		else ''
		end
	end

	# Returns a normalized pitch histogram array. 
	def pitch_histogram_normalized
		h = pitch_histogram
		h.each_index do |i| h[i] = h[i].to_f / @num_notes end
	end

	# Creates a histogram of note intervals. Intervals are calculated for each track separately but added to same histogram. 
	# Intervals are calculated between notes in two chords.
	# Note that the definition of a chord in this software is
	# that notes with same onset time (strt) belong to same chord.
	#  
	# MIDI pitch values have a minimum value of 0 and maximum value of 127.
	# Therefore the minimum possible interval is 0 - 127 = -127 and
	# maximum interval is 127 - 0 = 127. Thus there are 127 + 1 + 127 = 255 
	# possible intervals.
	#
	# Returns an array containing count for each interval (-127-127) and number of calculated intervals.
	def tracks_pitch_interval_histogram
		h = []
		sum = 0
		255.times do |i| h[i] = 0 end
		@tracks.each do |t|
			if t then 
				i = 2
				# tracks contain real notes and dummy notes of value 255; idea is to skip dummy notes
				while i < t.size do 

					# first note must be real
					while t[i - 1].ord == 255 and i < t.size do i += 1 end
					break if i >= t.size - 1
					first = t[i - 1].ord

					# second note must be real
					while t[i] && t[i].ord == 255 and i < t.size do i += 1 end
					if i < t.size then sum += 1; h[127 + t[i].ord - first] += 1 end
					i += 1
				end 
			end
		end
		[h, sum]
	end

	# Returns a normalized pitch interval histogram array, i.e. histogram returned by tracks_pitch_interval_histogram 
	# where each value is divided by number of calculated intervals.
	def tracks_pitch_interval_histogram_normalized
		h, sum = tracks_pitch_interval_histogram
		h.each do |hi| hi = hi.to_f / sum end
	end

	# Returns metadata for this song as HTML table row. HTML table is constructed in SongCollection instance from rows returned by this method.
	def meta_to_html
		s = "<td><a href='#{@midiurl}'>#{@filepath.sub(/.*\/([\w\d_\-]+\.mid)$/,'\1')}</a></td>" <<
		"<td>#{@composer}</td><td>#{@title}</td><td>#{@opus}</td><td>#{@date}</td><td>#{@style}</td>"
		if scoreurl != "" then
			s << "<td>#{@instruments}</td><td><a href='#{@scoreurl.sub(/\.ps$/,'.pdf')}'>PDF</a></td>"
		else "<td>#{@instruments}</td><td></td>\n" end
		s << "<td><a href='histogram?filepath=#{@midiurl}'>Histogram</a></td>\n" 
		s << "<td><a href='similarities?filepath=#{@midiurl}'>Similarities</a></td>\n"
	end

	# Returns a MIDI file containing chords between and including given indexes. 
	# Problem: instrument data is discarded in conversion, because of which all instruments default to piano. 
	# Therefore e.g. drum track played with piano may make the actual melody unrecognizable.
	def generate_midi(firstchord, lastchord)
		# get note data
		chordarray = get_matched_chords(firstchord, lastchord)
		if chordarray == nil then error end

		# create a midi file from chord array
		tr = Track.new
		tr << TrackName.new(0, midiurl + ", chords " + firstchord.to_s + "-" + lastchord.to_s)
		ch = 0
                
		chordarray.each do |chord|
			chord.each do |note|
				# note array contains strt, ptch, dur in this order.
				tr << NoteOn.new(note[0], ch, note[1], 64)
				tr << NoteOff.new(note[0] + note[2], ch, note[1], 0)
			end     
		end     
                
		# midi division is included into sequence
		sq = Sequence.new(0, @quarternoteduration) << tr
		sq.encode
	end
 

	private

	# Creates an internal representation of note data to @chords string.
	# Gets notes array from MIDI conversion method in SMF::Sequence.\n
	# Data is sorted in lexical order. It means that chords are sorted in ascending order according to onset time (strt), 
	# and notes inside each chord are sorted in ascending order according to pitch.
	# This data does not contain duplicate notes, i.e. notes with same onset time and pitch. 
	# Maxpoly value refers to this data. 
	# However, this method also creates separate track strings to be used by some string matching algorithms (e.g. LCTS).
	# This data contains duplicate notes. 
	# There are counters for number of notes for both data formats. 
	def create_tracks_and_chords(notes)
		if notes.size == 0 then return nil end
		# puts notes.inspect

		# sort notes according to start time and pitch
		notes.sort! do |a, b|
			if a[0] == b[0] then a[1] <=> b[1]
			else a[0] <=> b[0] end
		end

		# create chord string. 
		# format: ( <chordlen:1><strt:4> (<ptch:1><dur:2><voic:1>)* )*
		@chords = ""

		# tracks array starts from index 1, and also pitch data on a track starts from index 1
		@tracks = []
		@num_tracks.times do |i| @tracks[i + 1] = " " end

		strt = notes[0][0]
		tempchord = [ notes[0].push(@num_chords) ]

		for i in 1..notes.size do

			if notes[i] and notes[i][0] == strt then
				# chord boundary was not crossed
				tempchord.push(notes[i].push(@num_chords))
			else
				# chord boundary was crossed
				@num_chords += 1

				# add chordlen to num_notes
				if tempchord.size > 255 then 
					puts "ERROR: maximum chord size exceeded. exiting."
					exit
				end
				@num_notes_with_duplicates += tempchord.size
				@maxpoly_with_duplicates = tempchord.size if tempchord.size > @maxpoly_with_duplicates

				prevnote = nil
				tempchord_noduplicates = []

				# reverse order: add highest note only to discard polyphonicity inside tracks
				tempchord.reverse_each do |note|
					# put pitch of the note to right track
					if @tracklengths[note[3] + 1] < @num_chords
						@tracks[note[3] + 1].concat(note[1])
						@tracklengths[note[3] + 1] += 1
					end

					# create copy of tempchord with no duplicates
					if prevnote.nil? or not prevnote[1] == note[1]
						tempchord_noduplicates.unshift(note)
						prevnote = note
					end
				end

				# put null symbols to other tracks
				@num_tracks.times do |j| if @tracklengths[j + 1] < @num_chords then @tracks[j + 1].concat(255); @tracklengths[j + 1] += 1 end end

				# append chord with no duplicates to chords string
				@maxpoly = tempchord_noduplicates.size if tempchord_noduplicates.size > @maxpoly
				@num_notes += tempchord_noduplicates.size
				@chords.concat([tempchord_noduplicates.size].pack("C") + [strt].pack("I"))
				begin
					tempchord_noduplicates.each do |note|
						dnote = note.dup
						dnote.shift # discard strt
						@chords.concat(dnote.pack("CSC"))
					end
				rescue
					# if duration is too large to pack into two bytes, skip the file,
					# since that is most likely an error in the file
					puts "Error: duration too large."
					false
				end

				# move to next chord if not at end
				if notes[i]
					strt = notes[i][0]
					tempchord = [ notes[i].push(@num_chords) ]
				end
			end
		end

		# Add pseudo infinity value to the end of @chords for geometric algorithms
		@chords.concat([1].pack("C") + [4294967295].pack("I"))	# chordlen and strt
		@chords.concat([127,65535,127].pack("CSC"))		# pitch, duration, track

		# For SIA(M)E1 hashtable: generate primes for all pattern sizes (note max. pattern size 32).
		# Uses GPL'd code from K.Kodama; see directory lib/poly-ruby.
		for i in 2...33 do @primes[i] = Number.nextPrime(i * @num_notes * 2 + 1) end

		# generate turning points arrays for geometric algorithm P3
		create_turningpoints(notes)

		notes = nil
	end


	# for clarity, create P3 turning points separately by doing another scan over the notes.
	# overlapping notes are merged. 
	# always must be so that prevnote.strt <= thisnote.strt. therefore 3 possible cases exist:
	#	1. --
	#          -
   	#	2. ---
	#	    -
	# 	3. - -
	# we must generate turning points from the original data, so that overlapping notes are preserved and can be merged together.
	def create_turningpoints(notes)

		# notes = [[0,0,10,1], [0,0,100,1], [0,0,20,1]]

		# array for tracking if note with this pitch is already on; if so, the overlap is removed.
		# items: [start point, end point, startchordindex, endchordindex]
		# (start and end points are MIDI onset times.)
		notes_on = []

		startpoints = []
		endpoints = []
		128.times do |ptch| notes_on[ptch] = nil end

		# process all notes returned from MIDI conversion.
		notes.each_with_index do |n, i|
			strt = n[0]
			ptch = n[1]
			endp  = n[0] + n[2]
			chordind = n[4]

			if notes_on[ptch].nil? then
				# if note with this pitch is not on, add to notes_on array.
				# endchordindex is the index of the chord where the note ends. initially it is the same as startchordindex, since
				# we cannot say where the note actually ends. only if there is overlap with a later note we update
				# end chord index to chord index of the last overlapping note.
				notes_on[ptch] = [strt, endp, chordind]
			elsif strt <= notes_on[ptch][1] then 
				# do this branch if strt of current note is smaller or equivalent to end point of last note;
				# i.e. if the current note overlaps with the note that is already on.
				# here we update endchordind.
				if endp > notes_on[ptch][1] then notes_on[ptch][1] = endp; notes_on[ptch][2] = chordind end
			else
				# do this branch if the last note ended before this note started.
				# save the turning points of the last note and add new note to the notes_on array.

				# both startpoints and endpoints get the end chord index, since what we want is to play the merged note in full 
				# and so we must play also the latter part which is in a later chord than startchord indicates (endchord).
				startpoints.push([notes_on[ptch][0], ptch, notes_on[ptch][2]])
				endpoints.push([notes_on[ptch][1], ptch, notes_on[ptch][2]])
				notes_on[ptch] = [strt, endp, chordind]
			end
		end

		# write notes that are open
		notes_on.each_with_index do |no,ptch| 
			if not no.nil? then startpoints.push([no[0], ptch, no[2]]); endpoints.push([no[1], ptch, no[2]]) end 
		end

		# sort
		startpoints.sort! do |a, b|
			if a[0] == b[0] then a[1] <=> b[1]
			else a[0] <=> b[0] end
		end
		endpoints.sort! do |a, b|
			if a[0] == b[0] then a[1] <=> b[1]
			else a[0] <=> b[0] end
		end

		# write C array		
		@preprocessed_p3_startpoints = ""
		@preprocessed_p3_endpoints = ""
		@preprocessed_p3_num_turningpoints = startpoints.size
		# also startpoints must be sorted because open notes are written at end.
		startpoints.each do |tp|
			# all values are unsigned ints
			@preprocessed_p3_startpoints.concat(tp.pack("III"))
			# puts "sp: #{tp[0]} #{tp[1]} #{tp[2]}"
		end
		endpoints.each do |tp|
			@preprocessed_p3_endpoints.concat(tp.pack("III"))
			# puts "ep: #{tp[0]} #{tp[1]} #{tp[2]}"
		end

		notes_on = nil
		startpoints = nil
		endpoints = nil
	end

end

end 	# module
