# C-Brahms Engine for Musical Information Retrieval
# University of Helsinki, Department of Computer Science
#
# Version 0.2.4, May 15th, 2003
#
# Copyright Mika Turkia
#
# Contacts: turkia at cs helsinki fi
#
# Handles reading and conversion of MIDI files.
# Based on smf2text.rb by Tadayoshi Funaba 1999-2001.

require 'smf'
include SMF

# Handles reading and conversion of MIDI files.
# Based on smf2text.rb by Tadayoshi Funaba 1999-2001.
module SMF

# Handles reading and conversion of MIDI files.
# Based on smf2text.rb by Tadayoshi Funaba 1999-2001.
class Sequence

	# Callback functions for MIDI events. 
	# Notes are acquired in the order of note off event times, which is different from order of note on event times.
	# Therefore they are stored in an array. This array is sorted and processed further in Song class constructor. 
	class MIDI2Chords < XSCallback

		# Number of tracks.
		attr_reader :num_tracks

		# Number of notes.
		attr_reader :notes

		# MIDI division.
		attr_reader :division

		# An array for time signatures.
		attr_reader :timesignatures

		# An array for key signatures.
		attr_reader :keysignatures

		# A string for textual events. 
		attr_reader :metatext

		# MIDI File Header event handler.
		def header(format, ntrks, division, tc = nil)
			@format = format
			@division = division
			@notes_on = {}
			@notes = []
			@currenttrack = -1
			@timesignatures = []
			@keysignatures = []
			@num_tracks = ntrks
			@metatext = ""
			@s = format("Sequence %d %d %d\n", format, ntrks, division)
		end

		# MIDI Start of Track event handler.
		def track_start()
			@offset = 0
			@currenttrack += 1
		end

		# Necessary but unclear what it does.
		def delta(delta)
			@offset += delta
		end

		# MIDI Note Off event handler.
		def noteoff(ch, ptch, vel)
			begin
				strt = @notes_on.fetch([ch, ptch])

				# start time, pitch, duration in milliseconds, channel (voice)
				@notes.push([strt, ptch, @offset - strt, @currenttrack])

			rescue IndexError
				puts "ignoring note off without note on"
				# note off events without a corresponding note on event are ignored
				# (as are also note on events without note off)
			end
		end

		# MIDI Note On event handler.
		def noteon(ch, ptch, vel)
			if vel == 0 then noteoff(ch, ptch, vel)
			else @notes_on.store([ch, ptch], @offset) end
		end

		# MIDI Time Signature event handler. Time signatures are stored in an array.
		def timesignature(nn, dd, cc, bb)
			found = false
			@timesignatures.each do |ts| if ts[0] == @offset then found = true end end
			@timesignatures.push([@offset, nn, dd, cc, bb]) if not found
		end

		# MIDI Key Signature event handler. Key signatures are stored in an array.
		def keysignature(sf, mi)
			@keysignatures.push([@offset, sf, mi])
		end

		# Event handler for various textual MIDI events. All texts are stored in a string that can be returned to a client.
		def text(name, text)
			@metatext << format("%d %s %s\n", @offset, name, text.inspect)
		end

		private :text

		def generalpurposetext(text) text('GeneralPurposeText', text) end
		def copyrightnotice(text) text('CopyrightNotice', text) end
		def trackname(text) text('TrackName', text) end
		def instrumentname(text) text('InstrumentName', text) end
		def lyric(text) text('Lyric', text) end
		def marker(text) text('Marker', text) end
		def cuepoint(text) text('CuePoint', text) end
		def programname(text) text('ProgramName', text) end
		def devicename(text) text('DeviceName', text) end
		def text0a(text) text('Text0A', text) end
		def text0b(text) text('Text0B', text) end
		def text0c(text) text('Text0C', text) end
		def text0d(text) text('Text0D', text) end
		def text0e(text) text('Text0E', text) end
		def text0f(text) text('Text0F', text) end

		# Handles end of file.
		# Sends instance of this class including converted data to the constructor of Song class as a parameter.
		def result() if @notes.size == 0 then nil else self end end
	end

	# Converts a MIDI file to a two-dimensional array. First dimension consists of chords, and second dimension of notes in chords.
	# Also notes are represented as an array, which adds a third dimension.
	def convert
		WS.new(self, MIDI2Chords.new).read
	end
end

end # module SMF

