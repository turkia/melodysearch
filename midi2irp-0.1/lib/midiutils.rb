# C-Brahms Engine for Musical Information Retrieval
# University of Helsinki, Department of Computer Science
#
# Version 0.2.4, May 15th, 2003
#
# Copyright Mika Turkia
#
# Contacts: turkia at cs helsinki fi

require 'smf'
include  SMF

module MIR

# Helper methods for handling MIDI related issues.
class MidiUtils

	@@names_en = ['C', 'C#', 'D', 'D#', 'E', 'F', 'F#', 'G', 'G#', 'A', 'A#', 'B']
	@@names_fi = ['c', 'cis', 'd', 'dis', 'e', 'f', 'fis', 'g', 'gis', 'a', 'ais', 'h']

	# Converts note name to MIDI pitch value (range 0..127). Parameter lang may be "en" or "fi", 
	# referring to English and Finnish note names.
	def MidiUtils.notename_to_midival(notename, lang)
		if not notename =~ '^([cCdDeEfFgGaAbBhH])(is|es|#){0,1}([0-9])$' then return nil end

		refs = Regexp.last_match
		notechar = refs[1].downcase
		raised = refs[2]
		octave = refs[3]

		if lang == 'fi' then
			@@names_fi.index(notechar)
		else
			case notechar
				when 'c' then midival = 0
				when 'd' then midival = 2
				when 'e' then midival = 4
				when 'f' then midival = 5
				when 'g' then midival = 7
				when 'a' then midival = 9
				when 'b' then midival = 11
				else return nil
			end
			if raised == '#' then midival += 1 end
		end

		midival += octave.to_i * 12 
	end

	# This script concatenates midi files in a given directory to one midi file given as second parameter.
	def MidiUtils.merge(ifile_dir, ofile, ofiledivision)

		if ofiledivision.nil? or ofiledivision < 1 then ofiledivision == 96 end
		maxtime = 2 ** 32 - 1

		nsq = Sequence.new(0, ofiledivision)
		16.times do |i| nsq[i] = Track.new end
		lastfileoffset = 0
		normalized = 0

		if File.directory?(ifile_dir) and not ifile_dir =~ /^\./ then
			Dir.foreach(ifile_dir) do |file|
				if file =~ /\.mid$/ then
					lastoffset = 0
					maxoffset = 0

					begin
					sq = Sequence.decodefile(ifile_dir + '/' + file)
					if sq.division > ofiledivision then 
						puts "ERROR: division = #{sq.division}. Starting over."
						merge(ifile_dir, ofile, sq.division)
						return 
					end

					puts "Next song: (#{lastfileoffset}; #{sq.division})\t#{file}"
					sq.each do |tr|
						i = 0
						tr.each do |e|
							case e
							when NoteOn;
								normalized = (e.offset.to_f * ofiledivision / sq.division).to_i
								e.offset = normalized + lastfileoffset
								if e.offset > maxtime then raise end
								nsq[i] << e
							when NoteOff;
								normalized = (e.offset.to_f * ofiledivision / sq.division).to_i
								e.offset = normalized + lastfileoffset
								if e.offset > maxtime then puts "ERROR: offset = #{e.offset} > #{maxtime}."; exit end
								nsq[i] << e
								e = nil
							else
							end
						end
						tr = nil
						if normalized > maxoffset then maxoffset = normalized end
						i += 1
					end

					sq = nil
					lastfileoffset += maxoffset
					rescue => detail
						puts detail
					end
				end
			end
			nsq.encodefile(ofile) if lastfileoffset > 0 	# at least one file was processed
		end
	end
end
 
end 	# module
