# C-Brahms Engine for Musical Information Retrieval
# University of Helsinki, Department of Computer Science
#
# Version 0.3.3, August 30th, 2013
#
# Copyright Mika Turkia

require 'zlib'
require 'sinatra'
require 'daemons'

require_relative './lib/note.rb'
require_relative './lib/chord.rb'
require_relative './lib/client.rb'
require_relative './lib/midiutils.rb'

set :environment, :production
set :server, %w[puma]
set :app_file, __FILE__
set :root, File.dirname(__FILE__)
set :bind, '127.0.0.1'
set :port, 8080
set :default_encoding, 'utf-8'

enable :run, :protection, :threaded, :static
disable :logging, :raise_errors, :sessions, :views, :show_exceptions, :dump_errors

before do content_type 'text/html' end

# Error message to be sent when syntax of the query pattern is incorrect.
Syntax_error = "Error: the syntax of the pattern is not correct for this algorithm. Refer to the instruction page."

# Error message to be sent when length of the query pattern is incorrect.
Length_error = "Error: pattern size must be between 2 and 30."

# Error message to be sent when error parameter is too small.
Errors_error = "Error: number of allowed errors must be two less than pattern size.<br>(Note that not all algorithms support matching with errors.)"

# Error message to be sent when gap parameter is too small or too large.
Gap_error = "Error: maximum gap between notes must be between zero and 10."

# MIDI division.
Quarternotelength = 960

Headers = "<?xml version='1.0' encoding='UTF-8'?><!DOCTYPE html PUBLIC '-//W3C//DTD XHTML 1.0 Strict//EN' 'DTD/xhtml1-strict.dtd'>" << 
	  "<html xmlns='http://www.w3.org/1999/xhtml' xml:lang='en' lang='en'>" <<
	  "<head><meta id='Generator' content='C-Brahms Melody Search Engine'></meta>" <<
	  "<title>C-Brahms Melody Search Results</title>" <<
	  "<link rel='stylesheet' type='text/css' href='styles_xml.css' " <<
	  "title='stylesheet'></link></head><body>"

Footer = "<br /><br /></body></html>"


def with_headers(s); "#{Headers}\n#{s}\n#{Footer}"; end

def with_exception_wrapper
	begin
		yield
	rescue => e
		with_headers "Error: #{e.message}\n#{e.backtrace.to_s}"
	end
end

def with_time
	start = Time.new
	[yield, Time.new - start]
end


client = MIR::Client.new


get '/matches' do
	with_exception_wrapper {
		temp = with_time {	
			client.search(params[:algorithm], params[:notepattern], params[:limit].to_i, 
				      params[:songonce].to_i, params[:sort].to_i, params[:errors].to_i, 
				      params[:gap].to_i, params[:textpattern])
		}
		results = temp[0]

		# search results are converted to html by server. results[3] contains a result table in html form.
		# here we add just headers and statistics.
		if results then 
			with_headers("Results: #{results[2]} matches in " +
			"#{results[3]} songs.<br>#{results[4]}<br />" +
			"Pattern: #{params[:notepattern]}<br>Algorithm: #{params[:algorithm]}<br>" +
			"Search time: #{temp[1]} (Total elapsed time on server side)")
		else 
			with_headers "Server error."
		end
	}
end

get '/similarities' do
	with_exception_wrapper {
		with_headers "Similarities for file #{params[:filepath]}:<br /><br />#{client.get_similarities(params[:filepath])}"
	}
end

get '/histogram' do
	with_exception_wrapper {
		with_headers "Histograms for file #{params[:filepath]}:<br /><br />#{client.get_histograms(params[:filepath])}"
	}
end

get '/midi' do
	with_exception_wrapper {
		content_type 'audio/midi'
		client.get_midi(params[:filepath], params[:firstchord], params[:lastchord])
	}
end

Daemons.daemonize({:app_name => "melodysearch_webserver"})
