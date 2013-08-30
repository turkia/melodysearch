# C-Brahms Engine for Musical Information Retrieval
# University of Helsinki, Department of Computer Science
#
# Version 0.2.4, May 15th, 2003
#
# Copyright Mika Turkia

require 'cgi'
require_relative 'song'
require_relative 'initdata'
require_relative 'midiconvert'


module MIR

class SongCollection

	# Path to file containing songs in this collection. 
	attr_reader :filepath
 
	# Returns an empty song collection. Songs must be loaded with load method. 
	def initialize
		@songs = []
		@filepath = ""
		@maxpoly = nil
		@maxpoly_with_duplicates = nil
		@chords = nil
		@notes = nil
		@notes_with_duplicates = nil
	end

	# Returns number of songs in this collection. 
	def songs 
		@songs.size
	end

	# Returns a song object with index i.
	def get_song(i)
		@songs[i]
	end

	# Returns similarity index between two songs with indexes i and j. 
	# Similarity index is a dot product of normalized note pitch histograms. 
	def similarity(i, j)
		hi = @songs[i].pitch_histogram_normalized
		hj = @songs[j].pitch_histogram_normalized
		sum = 0.0; hi.each do |h| sum += h * h end
		hi_len = Math::sqrt(sum)
		sum = 0.0; hj.each do |h| sum += h * h end
		hj_len = Math::sqrt(sum)
		dotproduct = 0.0; hi.each_with_index do |h, k| dotproduct += h * hj[k] end
		dotproduct / (hi_len * hj_len)
	end

	# Returns an array of similarity indexes between a song given as a parameter and all songs in the collection. 
	# Similarity indexes are dot products of normalized note pitch histograms. 
	def similarities(song, oe, intervals)
		similarities = []
		#if intervals then hi = song.pitch_histogram_folded
		if intervals then hi = song.tracks_pitch_interval_histogram_normalized
		else hi = song.pitch_histogram_normalized end

		if oe then
			he = [0,0,0,0,0,0,0,0,0,0,0,0]
			hi.each do |i| he[i % 12] += i end
			hi = he
		end
		sum = 0.0; hi.each do |h| sum += h * h end
		hi_len = Math::sqrt(sum)

		@songs.each_with_index do |s, i| 

			#if intervals then hj = s.pitch_histogram_folded
			if intervals then hj = s.tracks_pitch_interval_histogram_normalized
			else hj = s.pitch_histogram_normalized end

			if oe then
				he = [0,0,0,0,0,0,0,0,0,0,0,0]
				hj.each do |h| he[h % 12] += h end
				hj = he
			end
			sum = 0.0; hj.each do |h| sum += h * h end
			hj_len = Math::sqrt(sum)
			dotproduct = 0.0; hi.each_with_index do |h, k| dotproduct += h * hj[k] end
			dotproduct /= (hi_len * hj_len)
			similarities[i] = [dotproduct, s.meta_to_html]
		end
		similarities
	end

	# Returns an array with LCTS edit distances for each song pair. 
	# Returned distance is minimum distance of comparisons between each track.
	# Array items are <song, song2, min_distance> triples. 
	# Sorted in descending order of edit distance.
	#
	# Note: this method is computationally very demanding and time requirements for most nontrivial 
	# songs are too large for this to be practically useful.
	def lcts_distances
		table = []
		min = 0
		@songs.each_with_index do |song, i|
			@songs.each do |song2|
				puts "Comparing: #{song.filepath}, #{song2.filepath}"
				min = song.lcts_distances(song2).min
				puts "Result:    #{min}:\t#{song.filepath}, #{song2.filepath}"
				table.push([min, song, song2]) if song != song2
			end
		end
		table.sort! { |a, b| a[0] <=> b[0] }
		table
	end

	# Returns number of notes in this collection. 
	def notes
		if not @notes or not @notes_with_duplicates then
			@notes = 0
			@notes_with_duplicates = 0
			@songs.each do |song| 
				@notes += song.num_notes 
				@notes_with_duplicates += song.num_notes_with_duplicates 
			end
		end
		[@notes, @notes_with_duplicates]
	end

	# Returns number of chords in this collection. 
	def chords 
		if not @chords then
			@chords = 0
			@songs.each do |song| @chords += song.num_chords end
		end
		@chords	
	end

	# Returns maximum length of a song track in this collection. 
	def max_tracklength
		len = 0
		@songs.each do |song|
			if song.tracklengths.max > len then len = song.tracklengths.max end 
		end
		len	
	end

	# Returns maximum polyphony (maximum number of notes with same onset time) in this collection. 
	def maxpoly
		if not @maxpoly or not @maxpoly_with_duplicates then
		 	@maxpoly = 0
		 	@maxpoly_with_duplicates = 0
			@songs.each do |song| 
				@maxpoly = song.maxpoly if song.maxpoly > @maxpoly 
				@maxpoly_with_duplicates = song.maxpoly_with_duplicates if song.maxpoly_with_duplicates > @maxpoly_with_duplicates 
			end
		end
		[@maxpoly, @maxpoly_with_duplicates]
	end

	# Returns average polyphony (average number of notes with same onset time) in this collection. 
	def avgpoly
		[@notes.to_f / @chords.to_f, @notes_with_duplicates.to_f / @chords.to_f]
	end


	# Loads songs in a given file into this collection. 
	def load(filename)
		File.open(filename + ".songs", "r") do |file|
			@songs = Marshal.load(file)
		end
		@filepath = filename + ".songs"
	end

	# Saves the songs in this collection into a given file.
	def save(filename)
		File.open(filename + ".songs", "w") do |file|
			Marshal.dump(@songs, file)
		end
	end


	# Creates a preprocessed data format that is used by monopoly algorithm for each song in this collection.
	# Actual processing is done in Song class. 
	def monopoly_preprocess
		@songs.each do |song| song.monopoly_preprocess end
	end

	# Converts MIDI files in a given directory to Song objects. 
	# This method merely just calls MIDI conversion method SMF::Sequence class and a metadata conversion method. 
	def convert_midifiles(path)
		#if path then path = path.gsub(/\/\//, '/') end

		if File.directory?(path) and not path =~ /\.$/
			Dir.foreach(path) do |entry| convert_midifiles(path + '/' + entry) end
			#puts "Conversion of directory #{path} finished."
		elsif path =~ /\.mid$/
			puts "Converting: #{path}"
			begin
				sq = SMF::Sequence.decodefile(path)
				s = sq.convert
				if not s then return nil end

				s.filepath = path

				# metadata handling specific to mutopia collection
				if path =~ /mutopia/ then set_metadata(s)
				else
					s.title = path.gsub(/public\//, '')
					s.midiurl = CGI.escape(s.filepath.gsub(/public\//, '')).gsub(/%2F/, "/")
					s.scoreurl = s.composer = s.opus = s.date = s.style = s.instruments = ""
				end

				# add to collection
				@songs.push(s)

			rescue => e	# SMF::Sequence::ReadError
				puts "Error: skipping file #{path}.\e#{e.to_s}"
				return nil
			end
		end
		maxpoly
	end


	# Returns song with given MIDI url. 
	def find_song(midiurl)
		@songs.find { |song| song.midiurl == midiurl} 
	end

	# Searches songs in this collection for a pattern given in init_info, with given algorithm and parameters given in init_info. 
	# Returns a list of matches. 
	def search(algorithm, matchlist, init_info)
		if (init_info.pattern_size > 31) then puts "Error: pattern too long\n"; return nil end

		# select if search is from previous results or all songs in the collection
		if matchlist then s = songs_from_matchlist(matchlist) else s = @songs end

		method = "scan_#{algorithm}".to_sym

		if init_info.textpattern.size == 0 then s.each do |song| song.send(method, init_info) end
		else 
			textpattern = Regexp.new(init_info.textpattern, Regexp::IGNORECASE)
			s.each do |song|
				song.send(method, init_info) if textpattern =~ song.title or textpattern =~ song.composer or textpattern =~ song.filepath or textpattern =~ song.opus or textpattern =~ song.style or textpattern =~ song.instruments or textpattern =~ song.date
			end
		end

		init_info.matches
	end

	# Returns string representations of songs in this collection. 
	def to_s
		@songs.each do |song|
			puts song.filepath + ":\n" + song.to_s
		end
	end

	# Returns a HTML page containing metadata for songs in this collection.
	def meta_to_html
		r = ""
		@songs.each_with_index do |s,i|
			r << "<tr><td>#{i + 1}</td>#{s.meta_to_html}</tr>"
		end

		"<?xml version='1.0' encoding='UTF-8'?><!DOCTYPE html PUBLIC '-//W3C//DTD XHTML 1.0 Strict//EN'" <<
		"'DTD/xhtml1-strict.dtd'><html xmlns='http://www.w3.org/1999/xhtml' xml:lang='en' lang='en'>" <<
		"<head><meta id='Generator' content='mt200802'></meta><title>C-Brahms Melody Search Engine</title>" <<
		"<link rel='stylesheet' type='text/css' href='styles_xml.css' title='stylesheet'></link></head>" <<
		"<body><h4>Melody Search Database</h4>" <<
		"<table>" <<
		"<tr><th>Database</th><th>Songs</th><th>Chords</th><th>Notes</th><th>Max. Polyphony</th>" <<
		"<th>Avg. Polyphony</th></tr><tr><td>#{@filepath}</td><td>#{songs}</td><td>#{chords}</td>" <<
		"<td>#{notes[0]}/#{notes[1]}</td><td>#{maxpoly[0]}/#{maxpoly[1]}</td>" <<
		"<td>#{"%4.2f" % avgpoly[0]}/#{"%4.2f" % avgpoly[1]}</td></tr></table>" <<
		"For fields with two values, the first value is without duplicate notes (notes with same pitch in same chord), " <<
		"and the second value is with duplicates.<br /><br />" <<
		"<table><tr><th>#</th><th>MIDI</th><th>Composer</th><th>Title</th><th>Opus</th><th>Date</th><th>Style</th>" <<
		"<th>Instruments</th><th>Score</th><th>Histogram</th><th>Similarities</th></tr>" <<
		"#{r}</table></body></html>"
	end

	private

	# Returns a list of songs included in a list of matches.
	def songs_from_matchlist(matchlist)
		songs = []
		matchlist.each do |match| songs.push(match.song) end
		songs.uniq!
		if !songs then @songs else songs end
	end

	# Sets metadata for Mutopia collection only. 
	def set_metadata(s)
		# set urls (hack; to be redone)
		path = s.filepath.sub(/public\//, '')
		s.midiurl = path
		s.scoreurl = path.sub(/.mid$/, '-a4.ps')

		# read metadata file (.dat) (specific to mutopia collection)
		# original dat files must be renamed to meta.dat with clean.rb script
		# not very good since same files may be read multiple times but
		# format is going to change to xml anyway and performance is not really important here
		begin
			datfile = IO.readlines(s.filepath.sub(/\/([\-\w]+).mid$/,'/meta.dat'), "r").join
			datfile.gsub!(/\r\n/m, "\n") or datfile.gsub!(/\r/m, "\n")
			datfile = datfile.split("\n")

			if datfile[4] then s.title = CGI.unescape(datfile[4]) end
			if datfile[5] then s.composer = CGI.unescape(datfile[5]) end
			if datfile[6] then s.opus = datfile[6].to_s end
			if datfile[9] then s.date = datfile[9].to_s end
			if datfile[8] then s.instruments = datfile[8].to_s end
			if datfile[10] then s.style = datfile[10].to_s end
		rescue
			puts "Error: could not read #{s.filepath.sub(/\/([\-\w]+).mid$/,'/meta.dat')}"
		end
	end
end

end 	# module
