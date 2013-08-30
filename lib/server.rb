# C-Brahms Engine for Musical Information Retrieval
# University of Helsinki, Department of Computer Science
#
# Version 0.2.4, May 15th, 2003
#
# Copyright Mika Turkia

require 'drb'
require 'daemons'

require_relative 'note'
require_relative 'chord'
require_relative 'song'
require_relative 'cserver/Server'
require_relative 'songcollection'
require_relative 'initdata'

module MIR

# This is a server class, whose instance creates a distributed object of itself. 
# Instances of Client class connect to this distributed object over DRb.
#
# As a rule, this version returns HTML to the client to avoid DRb traffic (e.g. sending large songs over DRb).
# Modifications are needed for possible new client applications that do not want HTML.
class Server

	# Starts a distributed server i.e. an instance of this class
	def initialize(dirname)

		@collections = []
		load_collections(dirname)

		# dynamically get algorithm names (public methods whose names begin with "scan_")
		@algorithms = MIR::Song.public_instance_methods.delete_if {|a| not a =~ /^scan_/ }
		# should also check that there is a corresponding init function for each scan function

		#Daemons.daemonize({:app_name => "melodysearch_drb_server"})
		DRb.start_service("druby://localhost:9822", self)
		DRb.thread.join
	end

	# Loads all .songs files in a given directory
	def load_collections(dirname)
		re = /.songs$/
		Dir.foreach(dirname) do |entry|
			if re.match(entry) then
				# note: loading from a client results in a crash
				s = MIR::SongCollection.new
				s.load(dirname + "/" + entry.sub(re, ''))
				if s then @collections.push(s); puts "loaded #{s.filepath}"
				else puts "error: skipping #{dirname + "/" + entry}" end
			end
		end
	end

	# Finds a song by URL of the MIDI file.
	def find_song(midiurl)
		@collections.each do |c|
			song = c.find_song(midiurl)
			return song if song
		end
		nil
	end

	# Returns a MIDI file containing requested chords. 
	def generate_midi(midiurl, firstchord, lastchord)
		song = find_song(midiurl)
		(song.nil? ? nil : song.generate_midi(firstchord, lastchord))
	end

	# Returns a HTML table containing simple pitch histogram similarity (dot product) values for song with given midiurl and all songs in loaded collections.
	def get_similarities(midiurl)

		song = find_song(midiurl)
		return nil if song.nil?

		# normalized pitch histogram similarities
		sim = []
		@collections.each do |c| sim += c.similarities(song, false, false) end
		sim.sort!
		sim.reverse!

		s =  "<b>Similarities based on dot products of normalized pitch histogram vectors:</b>"
		s << "<table><tr><th>Index</th><th>Rank</th>"
		s << "<th>MIDI</th><th>Composer</th><th>Title</th><th>Opus</th><th>Date</th><th>Style</th>"
		s << "<th>Instruments</th><th>Score</th><th>Metadata</th></tr>"

		sim.each_with_index do |si,i|
			s << "<tr><td>#{i}</td><td>#{si[0]}</td>#{si[1]}</tr>"; si = nil
		end
		s << "</table><br><br>"

		# pitch interval histogram similarities
		sim = []
		@collections.each do |c| sim += c.similarities(song, false, true) end
		sim.sort!
		sim.reverse!
		lower = 0; while sim[lower] == 0 do lower += 1 end
		upper = sim.size - 1; while sim[upper] == 0 do upper -= 1 end

		s << "<b>Similarities based on dot products of pitch interval histogram vectors:</b>"
		s << "<table><tr><th>Index</th><th>Rank</th>"
		s << "<th>MIDI</th><th>Composer</th><th>Title</th><th>Opus</th><th>Date</th><th>Style</th>"
		s << "<th>Instruments</th><th>Score</th><th>Metadata</th></tr>"

		for i in lower..upper do 
			s << "<tr><td>#{i}</td><td>#{sim[i][0]}</td>#{sim[i][1]}</tr>"; sim[i] = nil
		end

		sim = nil
		s << "</table><br><br>"
	end

	# Returns a HTML table containing pitch and pitch class histograms for a given song. 
	def get_histogram(midiurl)
		song = find_song(midiurl)
		return nil if song.nil?

		h = song.pitch_histogram
	
		# create pitch class histogram
		he = [0,0,0,0,0,0,0,0,0,0,0,0]
		h.each_with_index do |hi, i| he[i % 12] += hi end

		# pitch class histogram HTML representation
		nn = ["C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"]
		hist = "<b>Pitch Class Histogram:</b><table><tr><td>Note Class</td><td>Class Name</td><td>Frequency</td><td>Relative Frequency</td></tr>"
		he.each_with_index do |hi,i| hist << "<tr><td>#{i}</td><td>#{nn[i]}</td><td>#{hi}</td><td>#{(hi.to_f/song.num_notes * 100).to_i}</td></tr>" end
		hist << "</table><br /><br />"


		# pitch histogram mapped to the circle of fifths (calculated from pitch class histogram)
		hf = song.pitch_histogram_folded

		# folded histogram HTML representation
		hist << "<b>Folded Pitch Class Histogram:</b><table><tr><td>Note Class</td><td>Class Name</td><td>Frequency</td><td>Relative Frequency</td></tr>"
		hf.each_with_index do |hi,i| hist << "<tr><td>#{i}</td><td>#{nn[i]}</td><td>#{hi}</td><td>#{(hi.to_f/song.num_notes * 100).to_i}</td></tr>" end
		hist << "</table><br />"
		hist << "Pitch values are mapped to circle of fifths (folded). (See Tzanetakis, Ermolinskyi, Cook: "
		hist << "Pitch Histograms in Audio and Symbolic Music Information Retrieval. Proc. ISMIR 2002.)<br /><br /><br />"


		# pitch histogram HTML representation
		lower = 0; while h[lower] == 0 do lower += 1 end
		upper = h.size - 1; while h[upper] == 0 do upper -= 1 end
		hist << "<b>Pitch Histogram:</b><table><tr><td>Note Number</td><td>Note Name</td><td>Frequency</td><td>Relative Frequency</td></tr>"
		for i in lower..upper do 
			hist << "<tr><td>#{i}</td><td>#{nn[i % 12]}#{i / 12}</td><td>#{h[i]}</td><td>#{(h[i].to_f/song.num_notes * 100).to_i}</td></tr>" 
		end
		hist << "</table><br /><br />"


		# pitch interval histogram
		h = song.tracks_pitch_interval_histogram.shift

		# pitch histogram HTML representation
		lower = 0; while h[lower] == 0 do lower += 1 end
		upper = h.size - 1; while h[upper] == 0 do upper -= 1 end
		sum = 0
		h.each do |hi| sum += hi end
		first = -127
		hist << "<b>Pitch Interval Histogram:</b><table><tr><td>Interval</td><td>Frequency</td><td>Relative Frequency</td></tr>"
		for i in lower..upper do hist << "<tr><td>#{first + i}</td><td>#{h[i]}</td><td>#{(h[i].to_f/sum * 100).to_i}</td></tr>" end
		hist << "</table><br />"
		hist << "Intervals are calculated separately for each track, i.e. notes in different tracks are not compared. <br /><br /><br />"


		# duration class histogram
		h = song.duration_histogram

		# duration class histogram HTML representation
		names = ["Unknown", "Longer than full note", "Full note or shorter", "Half note or shorter", "Quarter note or shorter", "8th note or shorter", 
			"16th note or shorter", "32th note or shorter", "64th note or shorter", "128th note or shorter", "256th note or shorter"]
		sum = 0
		h.each do |hi| sum += hi end
		hist << "<b>Duration Class Histogram:</b><table><tr><td>Class</td><td>Frequency</td><td>Relative Frequency</td></tr>"
		h.each_with_index do |hi,i| hist << "<tr><td>#{names[i]}</td><td>#{hi}</td><td>#{(hi.to_f/sum * 100).to_i}</td></tr>" end
		hist << "</table><br /><br />"

		# and add also the metatexts here
		hist << "Metatexts from MIDI file:<br /><br />#{song.metatext.gsub(/\n/, "<br />")}<br /><br />"
	end

	# Searches from all loaded collections and returns results as a HTML string.
	def search(algorithm, pattern, limit, songonce, sort, errors, gap, textpattern)
		start = Time.new
		init_info = MIR::InitInfo.new(pattern)
		init_info.checkingfunction = 0
		init_info.matches = []
		init_info.errors = errors 
		init_info.gap = gap
		init_info.songonce = songonce
		init_info.textpattern = textpattern

		# get maximum number of notes in a song in all collections */
		# @collections.each do |c| m = c.notes; if m > init_info.maxnotes then init_info.maxnotes = m end end

		# this inconvenience should be solved
		if algorithm == "polycheck" then algorithm = "monopoly"; init_info.checkingfunction = 1 end

		# returns nil if server does not have the requested algorithm
	 	return nil if not @algorithms.include?("scan_#{algorithm}".to_sym)

		# dynamically calls an init method of the requested algorithm if such a method is defined
		if MIR::Song.singleton_methods.include?("init_#{ algorithm}".to_sym)
			MIR::Song.send("init_" + algorithm, init_info) 
		end

		inittime = Time.new - start
		start = Time.new
 
		# the actual search phase. matches are added to init_info.matches
		@collections.each do |c| c.search(algorithm, nil, init_info) end

		searchtime = Time.new - start

		# perform post-scan phase if such a function is defined
		MIR::Song.send("post_" + algorithm, init_info) if MIR::Song.singleton_methods.include?("post_" + algorithm)

		matches = init_info.matches

		if matches and matches.size > 0 then

			allmatches = matches.size
			counter = 0

			# match is array consisting of song, chordindex, lastchordindex, 
			# array of matched notes, transposition and octave shifts.
			# if requested, list the first match only
			# (is sorting working as expected, i.e. the best is the first?)
			if songonce == 1 then
				last = 0
				for i in 1...matches.size do
					if matches[i][0].filepath != matches[last][0].filepath
						last = i
					else
						if matches[i][5] < matches[last][5]
							matches[last] = matches[i]
						elsif matches[i][5] == matches[last][5] and
							matches[i][4].abs < matches[last][4] .abs then
							matches[last] = matches[i]
						end
						matches[i] = nil
					end
				end
				matches.compact!
				counter = matches.size
			elsif matches.size >= 1
				counter = 1
				for i in 1...matches.size do
					if matches[i][0].filepath != matches[i - 1][0].filepath
						counter += 1
					end
				end
			end

			# sort matches
			if sort == 0 then # do nothing
			elsif sort == 2 then
				# sort by allowed errors (k) (for lcts algorithm) and transposition
				matches.sort! { |a, b| 
					if a[5] < b[5] then -1
					elsif a[5] > b[5] then 1
					elsif a[4].abs == b[4].abs then a[4] <=> b[4]
					else a[4].abs <=> b[4].abs end
				}
			elsif sort == 3 then
				# sort by splits (for splitting algorithm) and transposition
				matches.sort! { |a, b| 
					if a[6].nil? or b[6].nil? then 
						if a[4].abs == b[4].abs then a[4] <=> b[4]
						else a[4].abs <=> b[4].abs end
					elsif a[6] < b[6] then -1
					elsif a[6] > b[6] then 1
					elsif a[4].abs == b[4].abs then a[4] <=> b[4]
					else a[4].abs <=> b[4].abs end
				}
			elsif sort == 4 then
				# sort by duration (for P3) and transposition
				matches.sort! { |a, b| 
					if a[6].nil? or b[6].nil? then 
						if a[4].abs == b[4].abs then a[4] <=> b[4]
						else a[4].abs <=> b[4].abs end
					elsif a[6] < b[6] then 1
					elsif a[6] > b[6] then -1
					elsif a[4].abs == b[4].abs then a[4] <=> b[4]
					else a[4].abs <=> b[4].abs end
				}
			else
				# sort by transposition
				matches.sort! { |a, b| 
					if a[4].abs == b[4].abs then a[4] <=> b[4]
					else a[4].abs <=> b[4].abs end
				}
			end

			# truncate match list to limit
			if matches.size > limit then matches = matches.slice!(0, limit) end

			# return matches to client
			[inittime, searchtime, allmatches, counter, matches_to_html(matches)]
			#[inittime, searchtime, allmatches, counter, "koetulokset"]
		else [inittime, searchtime, 0, 0, "No results found."] end
	end
end

end 	# module
