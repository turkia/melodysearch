# C-Brahms Engine for Musical Information Retrieval
# University of Helsinki, Department of Computer Science
#
# Version 0.2.4, May 15th, 2003
#
# Copyright Mika Turkia
#
# A holder class for algorithm parameters. 
# Objects of this class are given as arguments to search functions.

module MIR

# A holder class for algorithm parameters. 
# Objects of this class are given as arguments to search functions.
class InitInfo 

	# Number of chords in the pattern.
	attr_reader :pattern_size

	# Maximum polyphony of the pattern (excluding duplicate notes).
	attr_reader :maxpoly

	# Number of notes in the pattern (excluding duplicate notes).
	attr_reader :pattern_notes
	
	# Vector and string form patterns.
	# To make algorithm implementations a bit simpler, we create many versions of the pattern.
	attr_reader :pattern_monophonic_vector, :pattern_polyphonic_vector, :pattern_pitch_string

	# Holders for parameters used by string matching algorithms.
	attr_accessor :e, :em, :mask, :t

	# Array for matches. 
	attr_accessor :matches

	# Parameters passed by the client.
	attr_accessor :limit, :sort, :songonce, :checkingfunction, :errors, :gap, :textpattern

	# Converts given pattern to monophonic and polyphonic vectors.
	# Also converts pattern to a (monophonic) string containing pitches only.
	# Infinity values are added to end of vector form patterns.
	# Writable fields must be modified directly. 
	def initialize(pattern)
		@pattern_size = pattern.size

		i = 0
		strt = ""
		@maxpoly = 0
		@pattern_pitch_string = " "
		@pattern_monophonic_vector = ""
		@pattern_polyphonic_vector = ""
		@songonce = 0
		@gap = 0

		pattern.each do |chord|

			# ensure uniqueness and lexical order.
			# efficiency is not really needed here.
			chord.notes.uniq!
			chord.notes.sort!
			@maxpoly = chord.notes.size if chord.notes.size > @maxpoly

			strt = [chord.strt].pack("I")

			@pattern_pitch_string.concat(chord.notes[0].ptch)
			@pattern_monophonic_vector.concat(strt + [chord.notes[0].ptch.ord, chord.notes[0].dur, chord.notes[0].voic].pack("CSC"))

			chord.notes.each do |note|
				@pattern_polyphonic_vector.concat(strt + [note.ptch.ord, note.dur, note.voic].pack("CSC"))
				i += 1
			end
		end

		# Add pseudo infinity value to the end of pattern for geometric algorithms
		@pattern_monophonic_vector.concat([4294967295].pack("I") + [127, 65535, 127].pack("CSC"))
		@pattern_polyphonic_vector.concat([4294967295].pack("I") + [127, 65535, 127].pack("CSC"))
		@pattern_notes = i
	end
end

end
