# C-Brahms Engine for Musical Information Retrieval
# University of Helsinki, Department of Computer Science
#
# Version 0.2.4, May 15th, 2003
#
# Copyright Mika Turkia

module MIR

# This class is used for representing chords in patterns; it could also be replaced with an array.
class Chord

	# An array of notes in this chord.
	attr_reader :notes

	# Onset time of notes in this chord (same for all notes!)
	attr_reader :strt

	# Returns an empty chord with given onset time.
	def initialize(strt)
		@notes = []
		@strt = strt
	end

	# Adds a given note to this chord. Notes are sorted in ascending order according to note pitch.
	# Rejects duplicate notes to ensure the correctness of bit operations in algorithms.
	def add(note)
		if note and not @notes.include?(note)
			@notes.push(note)
			@notes.sort!
		end
		self
	end

	# Returns a string representation of this chord.
	def to_s
		s = "<#{strt}: "
		for i in 0...@notes.size - 1 do s << "#{@notes[i].to_s}, " end
		s << "#{@notes[@notes.size - 1].to_s}> "
	end
end

end 	# module
