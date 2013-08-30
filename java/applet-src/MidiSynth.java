/*
 * @(#)MidiSynth.java	1.15	99/12/03
 *
 * Copyright (c) 1999 Sun Microsystems, Inc. All Rights Reserved.
 *
 * Sun grants you ("Licensee") a non-exclusive, royalty free, license to use,
 * modify and redistribute this software in source and binary code form,
 * provided that i) this copyright notice and license appear on all copies of
 * the software; and ii) Licensee does not utilize the software in a manner
 * which is disparaging to Sun.
 *
 * This software is provided "AS IS," without a warranty of any kind. ALL
 * EXPRESS OR IMPLIED CONDITIONS, REPRESENTATIONS AND WARRANTIES, INCLUDING ANY
 * IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NON-INFRINGEMENT, ARE HEREBY EXCLUDED. SUN AND ITS LICENSORS SHALL NOT BE
 * LIABLE FOR ANY DAMAGES SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING
 * OR DISTRIBUTING THE SOFTWARE OR ITS DERIVATIVES. IN NO EVENT WILL SUN OR ITS
 * LICENSORS BE LIABLE FOR ANY LOST REVENUE, PROFIT OR DATA, OR FOR DIRECT,
 * INDIRECT, SPECIAL, CONSEQUENTIAL, INCIDENTAL OR PUNITIVE DAMAGES, HOWEVER
 * CAUSED AND REGARDLESS OF THE THEORY OF LIABILITY, ARISING OUT OF THE USE OF
 * OR INABILITY TO USE SOFTWARE, EVEN IF SUN HAS BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES.
 *
 * This software is not designed or intended for use in on-line control of
 * aircraft, air traffic, aircraft navigation or aircraft communications; or in
 * the design, construction, operation or maintenance of any nuclear
 * facility. Licensee represents and warrants that it will not use or
 * redistribute the Software for such purposes.
 */


import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.border.*;
import javax.swing.table.*;
import javax.swing.event.*;
import javax.sound.midi.*;
import java.util.Vector;
import java.util.StringTokenizer;
import java.io.File;
import java.io.IOException;



/**
   Handles sound synthesis and piano keyboard functions for SimpleSearchApplet.
   
   This class has been modified from Sun demonstration application
   JavaSoundDemo's MidiSynth class, version 1.15 99/12/03 by
   Brian Lichtenwalter.

   Most of the code in Piano and Keyboard classes is unchanged. 
   Some inner classes have been removed completely. 

   New functions include note string handling. 
   
   @author Mika Turkia 
 */
public class MidiSynth extends JPanel implements MetaEventListener
{
	final int NOTEON = 144;
	final int NOTEOFF = 128;
	final int END_OF_TRACK = 47;
	final int ON = 0, OFF = 1;

	final Color jfcBlue = new Color(204, 204, 255);
	final Color pink = new Color(255, 175, 175);

	/** Reference to applet instance. */
	SearchApplet applet;

	/** Synthetizer plays the keyboard events. */
	Synthesizer synthesizer;

	/** Sequencer plays the note string when play button is pressed, since
	unlike keyboard events, note string playing must be timed. */
	Sequencer sequencer;
	Sequence sequence;
	Track track;

	Instrument instruments[];
	ChannelData channels[];
	ChannelData cc;	// current channel

	Vector keys = new Vector();
	Vector whiteKeys = new Vector();
	Piano piano;


	/** Constructor creates piano keyboard. */
	public MidiSynth(SearchApplet a) 
	{

		this.applet = a;
		setLayout(new BorderLayout());

		JPanel p = new JPanel();
		p.setLayout(new BoxLayout(p, BoxLayout.Y_AXIS));
		JPanel pp = new JPanel(new BorderLayout());
		pp.add(piano = new Piano(this));
		p.add(pp);
		add(p);
	}


	/** Opens synthesizer, creates sequencer, and gets a channel 
 	    for playing MIDI events. 
	*/
	public void open() 
	{
		try {
			if (synthesizer == null) 
			{
				if ((synthesizer = MidiSystem.getSynthesizer()) == null) 
				{
					System.out.println("getSynthesizer() failed!");
					return;
				}
			} 
			synthesizer.open();
			sequencer = MidiSystem.getSequencer();
			sequence = new Sequence(Sequence.PPQ, 10);
		} catch (Exception e) { e.printStackTrace(); return; }

		Soundbank sb = synthesizer.getDefaultSoundbank();
		if (sb != null) 
		{
			instruments = synthesizer.getDefaultSoundbank().getInstruments();
			synthesizer.loadInstrument(instruments[0]);
		}
		MidiChannel midiChannels[] = synthesizer.getChannels();
		channels = new ChannelData[midiChannels.length];
		for (int i = 0; i < channels.length; i++) 
		{
			channels[i] = new ChannelData(midiChannels[i], i);
		}
		cc = channels[0];
	}


	/** 
	    Closes synthetizer and sequencer. 
	*/
	public void close()
	{
		if (synthesizer != null) 
		{
			synthesizer.close();
		}

		if (sequencer != null) 
		{
			sequencer.close();
		}
		sequencer = null;
		synthesizer = null;
		instruments = null;
		channels = null;
	}


	/** 
	    This is the MetaEventListener callback. 
	    It closes the sequencer when END OF TRACK event is encountered. 
	*/
	public void meta(MetaMessage message)
	{
		if (message.getType() == END_OF_TRACK)
		{  
			this.applet.playButton.setEnabled(true);
			sequencer.close();
		}
	}


	/** 
	    Converts the note name string in the text field 
	    to MIDI events and plays them on sequencer. 

	    The implementation of this class is somewhat suboptimal;
	    it plays even some erroneous note strings. 
	*/
	public void playMidiString(String notes)
	{
		StringTokenizer n = new StringTokenizer(notes.toLowerCase());
		StringTokenizer c;
		String chord;
		String note;
		int midival, octave, timeInterval = 0, i = 0;
		Track track;
		long startTime = 0;
		ShortMessage message;
 
		sequencer.addMetaEventListener(this);
		try
		{
			sequence = new Sequence(Sequence.PPQ, 10);
		} 
		catch (Exception e) { e.printStackTrace(); }

		track = sequence.createTrack();


		/* Validate on the fly: play correct ones, ignore others. */
		/* should be done with a real parser */

		while (n.hasMoreTokens())
		{
			chord = n.nextToken();

			c = new StringTokenizer(chord, "+");

			while (c.hasMoreTokens())
			{
				note = c.nextToken();

				/* get duration */
				i = 0;
				timeInterval = 4;
				while (note.charAt(i) >= '0' && note.charAt(i) <= '9') i++;
				if (i > 0)
				{
					try
					{
						timeInterval = new Integer(note.substring(0, i)).intValue();
					}
					catch (Exception e) { }
				}

				/* get pitch */
				switch (note.charAt(i))
				{
					case 'c': midival = 0; break;
					case 'd': midival = 2; break;
					case 'e': midival = 4; break;
					case 'f': midival = 5; break;
					case 'g': midival = 7; break;
					case 'a': midival = 9; break;
					case 'b': midival = 11; break;
					default: midival = -2;
				}

				if (note.charAt(i + 1) == '#')
				{ 
					midival++;
					i++;
				}

				/* get octave */
				i++;
				try
				{
					octave = new Integer(note.substring(i, note.length())).intValue();
				}
				catch (Exception e)
				{
					octave = 5;
				}

				//System.err.println(timeInterval + " " + midival + " " + octave);

				/* basic validations */
				if (midival > 127 || midival < 0 || octave < 0 || octave > 9 || timeInterval < 0 || timeInterval > 128) continue;


				/* Add events to track. */
				try 
				{
					/* Add note on. cc.num is channel number of the current channel. Second argument is pitch. */ 
					message = new ShortMessage();
					message.setMessage(NOTEON + cc.num, midival + octave * 12, cc.velocity);
					track.add(new MidiEvent(message, startTime));

					/* Add note off. */ 
					message = new ShortMessage();
					message.setMessage(NOTEOFF + cc.num, midival + octave * 12, cc.velocity);
					track.add(new MidiEvent(message, startTime + sequence.getResolution() * 4 / timeInterval));

					//System.err.println("on: " + startTime + " off: " + (startTime + sequence.getResolution() * 4 / timeInterval) + 
					//	"res: " + sequence.getResolution());
				}
				catch (Exception e) { e.printStackTrace(); }
			}
			startTime += sequence.getResolution() * 4 / timeInterval;
	  	}

		try
		{
			sequencer.open();
			sequencer.setSequence(sequence);
		}
		catch (Exception e) { e.printStackTrace(); }

		this.applet.playButton.setEnabled(false);
		sequencer.start();
	}


	/**
	    Converts MIDI pitch number to English note name and octave number. 
	*/
	public String midiValueToNoteName(int midival)
	{
		String name = null;

		switch (midival % 12)
		{
			case 0: name = "C"; break;
			case 1: name = "C#"; break;
			case 2: name = "D"; break;
			case 3: name = "D#"; break;
			case 4: name = "E"; break;
			case 5: name = "F"; break;
			case 6: name = "F#"; break;
			case 7: name = "G"; break;
			case 8: name = "G#"; break;
			case 9: name = "A"; break;
			case 10: name = "A#"; break;
			case 11: name = "B"; break;
		}
		return name + new Integer(midival / 12).toString();
	}


	/**
	    This class implements black and white keys or notes on the piano.
	    Piano class calls on and off methods, which set NOTE ON and NOTE OFF
	    events on the current channel of the synthetizer. 
	    NOTE ON also appends the note name to the text field. 
	*/
	class Key extends Rectangle 
	{
		/** 
		    Indicates if the key is pressed or not. 
		*/
		int noteState = OFF;

		/** 
		    The pitch associated with this key. 
		*/
		int kNum;

		/** 
		    Reference to the synthetizer for playing notes. 
		*/
		private MidiSynth midiSynth;

		/**
		   Constructs a Key object that contains information 
		   on the pitch associated with this key. 
		*/
		public Key(int x, int y, int width, int height, int num, MidiSynth ms) 
		{
			super(x, y, width, height);
			midiSynth = ms;
			kNum = num;
		}


		/** 
		    Returns true if the key is pressed, false otherwise. 
		*/
		public boolean isNoteOn() 
		{
			return noteState == ON;
		}


		/**
		    Puts a NOTE ON event on the current channel.
		    Also adds note name to the text field.
		    Time interval for all notes is initially a quarter note.
		    Called from Piano's noteOn method.
		*/
		public void on() 
		{
			setNoteState(ON);
			cc.channel.noteOn(kNum, cc.velocity);
			/* all notes are initially quarter notes*/
			this.midiSynth.applet.textField.setText(this.midiSynth.applet.textField.getText() + " 4" + midiValueToNoteName(kNum));
		}


		/**
		    Puts a NOTE OFF event on the current channel.
		    Called from Piano's noteOn method. 
		*/
		public void off() 
		{
			setNoteState(OFF);
			cc.channel.noteOff(kNum, cc.velocity);
		}


		/** Sets the state of key (up = false, down/pressed = true). */
		public void setNoteState(int state) 
		{
			noteState = state;
		}
	} // End class Key



	/**
	    Piano class renders black and white keys and plays the notes for a MIDI channel.  
	    It handles keyboard events and calls the on and off methods of Key class. 
	*/
	class Piano extends JPanel implements MouseListener 
	{

		Vector blackKeys = new Vector();
		Key prevKey;
		final int kw = 16, kh = 80;
		private MidiSynth midiSynth;


		public Piano(MidiSynth midisynth) 
		{
			this.midiSynth = midisynth;

			setLayout(new BorderLayout());
			setPreferredSize(new Dimension(42*kw+1, kh+1));
			int transpose = 24;  
			int whiteIDs[] = { 0, 2, 4, 5, 7, 9, 11 }; 
		
			for (int i = 0, x = 0; i < 6; i++) 
			{
				for (int j = 0; j < 7; j++, x += kw) 
				{
					int keyNum = i * 12 + whiteIDs[j] + transpose;
					whiteKeys.add(new Key(x, 0, kw, kh, keyNum, midiSynth));
				}
			}

			for (int i = 0, x = 0; i < 6; i++, x += kw) 
			{
				int keyNum = i * 12 + transpose;
				blackKeys.add(new Key((x += kw)-4, 0, kw/2, kh/2, keyNum+1, midiSynth));
				blackKeys.add(new Key((x += kw)-4, 0, kw/2, kh/2, keyNum+3, midiSynth));
				x += kw;
				blackKeys.add(new Key((x += kw)-4, 0, kw/2, kh/2, keyNum+6, midiSynth));
				blackKeys.add(new Key((x += kw)-4, 0, kw/2, kh/2, keyNum+8, midiSynth));
				blackKeys.add(new Key((x += kw)-4, 0, kw/2, kh/2, keyNum+10, midiSynth));
			}
			keys.addAll(blackKeys);
			keys.addAll(whiteKeys);

			addMouseListener(this);
		}


		public void mousePressed(MouseEvent e) 
		{ 
			prevKey = getKey(e.getPoint());
			if (prevKey != null)
			{
				prevKey.on();
				repaint();
			}
		}


		public void mouseReleased(MouseEvent e) 
		{ 
			if (prevKey != null)
			{
				prevKey.off();
				repaint();
			}
		}


		public void mouseExited(MouseEvent e) 
		{ 
			if (prevKey != null)
			{
				prevKey.off();
				repaint();
				prevKey = null;
			}
		}


		public void mouseClicked(MouseEvent e) { }


		public void mouseEntered(MouseEvent e) { }


		public Key getKey(Point point) 
		{
			for (int i = 0; i < keys.size(); i++)
			{ 
				if (((Key) keys.get(i)).contains(point))
				{
					return (Key) keys.get(i);
				}
			}
			return null;
		}


		public void paint(Graphics g)
		{
			Graphics2D g2 = (Graphics2D) g;
			Dimension d = getSize();

			g2.setBackground(getBackground());
			g2.clearRect(0, 0, d.width, d.height);

			g2.setColor(Color.white);
			g2.fillRect(0, 0, 42*kw, kh);

			for (int i = 0; i < whiteKeys.size(); i++)
			{
				Key key = (Key) whiteKeys.get(i);
				if (key.isNoteOn()) 
				{
					g2.setColor(jfcBlue);
					g2.fill(key);
				}
				g2.setColor(Color.black);
				g2.draw(key);
			}

			for (int i = 0; i < blackKeys.size(); i++) 
			{
				Key key = (Key) blackKeys.get(i);
				if (key.isNoteOn()) 
				{
					g2.setColor(jfcBlue);
					g2.fill(key);
					g2.setColor(Color.black);
					g2.draw(key);
				} 
				else 
				{
					g2.setColor(Color.black);
					g2.fill(key);
				}
			}
		}
	} // End class Piano


	/**
	    Stores MidiChannel information.
	    This class could be removed since we use only one channel
 	    (it could be stored in an instance variable).
	*/
	class ChannelData
	{
		MidiChannel channel;
		int velocity, pressure, bend, reverb;
		int num;

		/** Stores channel info. */
		public ChannelData(MidiChannel channel, int num) 
		{
			this.channel = channel;
			this.num = num;
			velocity = pressure = bend = reverb = 64;
		}
	} // End class ChannelData
} 


