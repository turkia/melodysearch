C-Brahms Engine for Musical Information Retrieval
=================================================

University of Helsinki, Department of Computer Science
Version 0.3.3, August 30th, 2013

Introduction
------------

This software contains very performant algorithms for melody-based searching of MIDI files.
It was written as a part of an algorithm research project in 2002-2003
for Ruby 1.6 and updated to Ruby 2.0 in August 2013.

At the moment this software is unmaintained. 
Bug fixes are welcome, but don't send feature requests or bug reports without a fix. 

Some algorithms are not included in this version. 

Usage
-----

0. Install Ruby 2.0.0, SMF 0.15.12 and other dependencies:
       
        gem install smf daemons sinatra puma

1. Compile C extensions: 

        rake build

2. Download MIDI package and unpack it to public folder:

3. Start the server:

	admin/start_server.rb
	admin/start_webserver.rb

4. Navigate to http://localhost:8080/ with your browser. 


Changelog
---------

Version 0.3.3, August 30th, 2013:

- ported to Ruby 2.0.0
- dependency from an Apache server replaced by Sinatra
- minor refactoring of other Ruby code
- Java applet for piano keyboard based searches compiles but has not been tested as Java is nowadays usually disabled in browsers. It is excluded from this version. 
- porting from 32 bit to 64 bit hardware has introduced bugs in some algorithms (P3, LCTS, Splitting). They are not included in this version. Debugging help is welcome.
- publications are not included in the package

Version 0.3.0, August 25th, 2012:

- ported from Ruby 1.6 to Ruby 1.9.3 and SMF 0.15.12
- piano roll image generation library could not be found; this functionality has been removed. 
- geometric P3 crashes
- mostly untested

Version 0.2.7, August 25th, 2003:

- New algorithms: geometric P3
- Piano roll images of the music pieces (access trough song list page)
- Similarities calculation (experimental, access through song list page)
- New compiler (gcc 3.2.2) improved performance quite a lot for P2 (and SIA(M)E2).

Version 0.2.6:

- New algorithms: Dynamic Programming, LCTS, Splitting.
- Optional text pattern to limit the search by e.g. composer or title. This speeds up searches since only a part of database is scanned.
- Refactorings in the code; some documentation added. 

Version 0.2.4:

- New algorithms: P2
- Improved implementation of P1
- Tester class that samples test patterns from the database.
- Various histograms can be calculated from note data
- Lots of refactoring in the code
- More documentation added
- Publications included in the package
- Metatexts are packed using zlib
- Prototypes of P1 and P2 are provided for learning and testing.
- Attempts to calculate similarities between songs implemented but not available in the client interface by default because of incoherent results. 
- Also two patented algorithms SIA(M)E1 and SIA(M)E2 were implemented; these are not included in the public distribution package.

Copyright notice for Mutopia Project MutopiaBSD licensed files:
Permission is hereby granted, without written agreement and without license or royalty fees, 
to use, copy, modify, distribute, perform and record this music and to distribute, perform 
and record modified versions of this music for any purpose, provided that the above copyright notice, 
this paragraph and the following disclaimer appear with all copies of this music, 
whatever the format of the copy, printed, audio or otherwise.
THIS MUSIC IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
PARTICULAR PURPOSE AND NONINFRINGEMENT.


Overview (original version of 2003):

     ----------- 
    |  applet   |----
     -----------     |  http    -------------------    drb    ------------ 
                     |---------| get*.rb/client.rb |---------|  server.rb |
     -----------     |          -------------------           ------------ 
    | HTML form |----                                               | 1..1
     -----------                                                    |
                                                                    | 0..*
                                                             ----------------  
                                                            | SongCollection |
                                                             ---------------- 
                                                                    | 1..1
                                                                    |
                                                                    | 0..*
                                                                  ------
                                                                 | Song |
                                                                  ------

Bugs (2003)
-----------

- Splitting algorithm crashes on some data. 
- Dynprog and lcts return different results; it's unclear if this is correct or not. Possibly a bug in dynprog.
- Overlapping note segments problem in C version of P3 is fixed, but incorrect chord indexes were returned; this may or may not be corrected.
- Bar calculation may not be working correctly in all cases due to MIDI limitations. 
- Matched notes don't give an idea of durations; also simultaneous notes in polyphonic patterns are presented incorrectly as a sequence.

To do (2003)
------------

- Matched notes should contain duration information (maybe duration class since deriving actual symbolic duration is difficult). 
- Source scanning could be implemented more cleanly in some algorithms.
- If only best match for each song is requested, other matches should be discarded already in the algorithm, if possible. 
- Pattern processing is still a bit clumsy, e.g. Note and Chord classes could be replaced with arrays. 
