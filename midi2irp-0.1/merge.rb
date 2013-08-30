#!/usr/bin/env ruby

# C-Brahms Engine for Musical Information Retrieval
# University of Helsinki, Department of Computer Science
#
# Version 0.2.4, May 15th, 2003
#
# Copyright Mika Turkia
#
# Contacts: turkia at cs helsinki fi
# 
# This script merges all MIDI files in a given directory to one MIDI file. 
# It is provided for experimenting and it is not needed for normal operation.
#
# Usage: merge.rb <midifiledir> [outfile]

require_relative 'lib/midiutils'

if ARGV.size == 0 or ARGV.size > 2 then puts 'Usage: merge.rb <midifiledir> [outfile]' 
else MIR::MidiUtils.merge(ARGV[0], ARGV[1] || "./merged.mid", 96) end
