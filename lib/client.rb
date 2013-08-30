# C-Brahms Engine for Musical Information Retrieval
# University of Helsinki, Department of Computer Science
#
# Version 0.3.0, August 27th, 2013
# Version 0.2.4, May 15th, 2003
#
# Copyright Mika Turkia

require 'drb'

require_relative './note.rb'
require_relative './chord.rb'
require_relative './midiutils.rb'

module MIR

# Error message to be sent when syntax of the query pattern is incorrect.
Syntax_error = "the syntax of the pattern is not correct for this algorithm. Read instruction page.</a>."

# Error message to be sent when length of the query pattern is incorrect.
Length_error = "pattern size must be between 2 and 30."

# Error message to be sent when error parameter is too small.
Errors_error = "number of allowed errors must be two less than pattern size.<br>(Note that not all algorithms support matching with errors.)"

# Error message to be sent when gap parameter is too small or too large.
Gap_error = "maximum gap between notes must be between zero and 10."

# MIDI division.
Quarternotelength = 960


# Client class that implements methods used by http_server.rb.
# As a rule, server returns a HTML string (2013 note: impractical but refactoring this is too much work since html conversion is implemented in the C extension.)
class Client

	# Creates a new client.
	def initialize
		begin
			DRb.start_service()
			@server = DRbObject.new(nil, 'druby://localhost:9822')
		rescue => e
			"server is not running. Try 'admin/start_server.rb' and 'admin/start_server.rb'.<br /></body></html>"
		end
	end

	# Default error message.
	Error = "Error: Could not find MIDI file."

	# Main search method. Checks parameter values and converts note pattern string to an array of Chord objects that contain Note objects.
	def search(algorithm, notepattern, limit, songonce = 0, sort = 2, errors = 0, gap = 0, textpattern = '')

		algorithm = 'monopoly' if not (3..30).include?(algorithm.size)
		textpattern ||= ''
		notechords = notepattern.split(' ')

		raise Length_error if not (2..30).include?(notechords.size) and not (0..50).include?(textpattern.size)
		raise Errors_error if not (0..(notechords.size - 2)).include?(errors)
		raise Gap_error if not (0..10).include?(gap)

		if limit <= 0 or limit > 1000 then limit = 1000 end
		if sort < 0 or sort > 10 then sort = 0 end
		songonce = 1 if not [0, 1].include?(songonce) 

		# check that the pattern syntax is ok,
		# convert notes to a chord array containing midi values
		# and make a pattern
		pattern = []
		strt = 0
		duration = 0
		notechords.each_with_index do |notechord, i|
			pattern[i] = MIR::Chord.new(strt)
			notechord.split('+').each_with_index do |note, j|
				note =~ /(\d{0,2})(\w[#]{0,1}\d)/
				ptch = $2
				durcode = $1
				midival = MIR::MidiUtils.notename_to_midival(ptch, nil)
				if not midival then
					raise Syntax_error
					exit
				else
					if durcode.size == 0 then 
						duration = Quarternotelength
						pattern[i].add(MIR::Note.new(midival, Quarternotelength, 0))
					else
						duration = 4.0 / durcode.to_f * Quarternotelength
						pattern[i].add(MIR::Note.new(midival, duration.to_i, 0))
					end
				end
			end
			# duration of the last note in the chord is used in incrementing strt. 
			# i.e. when you have a chord with seminote and quarternote (1/2 and 1/4),
			# if you put seminote first, the time gap to next note is 1/2. if you put quarternote last, it is 1/4. 
			# NOTE: at the moment the implementation ignores other than the lowest note in each chord.
			strt += duration
		end

		# search results are converted to html by server. results[3] contains a result table in html form. 
		@server.search(algorithm, pattern, limit, songonce, sort, errors, gap, textpattern)
	end

	# Returns MIDI file containing matched part of a song. File may then be played on client computer.
	# Matched part is defined by cgi parameters firstchord and lastchord.
	def get_midi(filepath, firstchord, lastchord)
		_check_filepath_param(filepath)
		raise Error if firstchord.nil? or lastchord.nil?

		# validate chord numbers
		firstchord = firstchord.to_i
		lastchord = lastchord.to_i
		if firstchord < 0 or lastchord <= 0 or lastchord - firstchord <= 0 or firstchord > 999999 or lastchord > 999999 then error end

		# get reference to song instance 
		midi = @server.generate_midi(filepath, firstchord, lastchord)
		raise Error if midi.nil?

		# return midi file
		midi
	end

	# Returns HTML string containing various histograms of song's note data.
	def get_histograms(filepath)
		_check_filepath_param(filepath)
		hist = @server.get_histogram(filepath)
		raise 'Histogram generation failed.' if hist.nil?
		hist
	end

	# Returns HTML string containing various similarity measures based on song's note data.
	def get_similarities(filepath)
		raise Error if filepath.nil? or filepath.size > 250
		# get similarities array: [rank, song metadata] 
		s = @server.get_similarities(filepath)
		raise Error if s.nil?
		s
	end

	private

	def _check_filepath_param(filepath)
		raise Error if filepath.nil? or filepath.size > 250
	end
end

end	# module
