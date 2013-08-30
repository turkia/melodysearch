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
# Usage: admin/start_server.rb

require_relative '../lib/server'

MIR::Server.new('songs')
