# C-Brahms Engine for Musical Information Retrieval
# University of Helsinki, Department of Computer Science
#
# Version 0.2.4, May 15th, 2003
#
# Copyright Mika Turkia


module MIR

# This class is used for representing notes in patterns; it could also be replaced with an array. 
class Note

	# MIDI value for note pitch; range 0..127.
	attr_reader :ptch

	# Duration for the note. When representing patterns, 960 units of duration amount to one quarter note.
	attr_reader :dur

	# MIDI track number; range 0..15.
	attr_reader :voic

	# Returns a note object initialized with given parameters. No checks for validity of parameters.
	def initialize(ptch, dur, voic)
		@ptch = ptch
		@dur = dur
		@voic = voic
	end

	# Compares pitches of notes.
	def eql?(note)
		# durations are not handled by algorithms
		@ptch == note.ptch
	end

	alias == eql?

	def <=>(n)
		@ptch <=> n.ptch
	end

	# Returns a string representation of this note.
	def to_s
		"(ptch:#{@ptch} dur:#{@dur} voic:#{@voic})"
	end
end

end 	# module
