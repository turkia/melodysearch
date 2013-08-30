import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.sound.midi.*;
import java.applet.*;
import java.net.URL;
import java.net.URLEncoder;
import java.net.MalformedURLException;


/**
    An applet that contains piano keyboard and controls for making a search with
    C-Brahms Melody Search system.
   
    The applet can be used instead of a simple HTML form. 

    Interface contains combo boxes to select algorith, sorting method, 
    maximum number of results and to choose whether to show best match 
    or all matches for each song. 

    There is also a text field with a note string containing all notes
    corresponding to the pressed piano keys, buttons for playing or clearing
    the note string, and for starting a search.
 
    @author Mika Turkia 
*/
public class SearchApplet extends JApplet implements ActionListener
{
	protected static String serverAddress = "http://localhost:4567/matches";

	/** 
	    Text field for note names. Public since it is accessed from Piano instance. 
	*/
	public JTextField textField;
	public JButton playButton;

	AppletContext appletContext;
	MidiSynth midiSynth;
	JButton clearButton;
	JButton searchButton;
	JPanel selectionPanel;
	JPanel buttonPanel; 
	JPanel searchPanel; 

	/** 
	    Creates graphical elements in the interface. 
	*/
	public void init()
	{
	}

	public void setup()
	{
		setBackground(Color.white);
		this.appletContext = getAppletContext();
		Container contentPane = getContentPane();
		contentPane.setBackground(Color.white);
		contentPane.setLayout(new FlowLayout());

		/* Create search options panel */
		selectionPanel = new JPanel();
		selectionPanel.setBackground(Color.white);
		selectionPanel.setLayout(new FlowLayout(FlowLayout.CENTER, 0, 0));

		/* Create field for note pattern input */
		textField = new JTextField("", 36);
		textField.addActionListener(this);

		/* Create buttons. */
		playButton = new JButton("Play");
		playButton.addActionListener(new ActionListener() { public void actionPerformed(ActionEvent e) { midiSynth.playMidiString(textField.getText()); } } );
		clearButton = new JButton("Clear");
		clearButton.addActionListener(new ActionListener() { public void actionPerformed(ActionEvent e) { textField.setText(""); } } );
		searchButton = new JButton("Search");
		searchButton.addActionListener(this);

		/* Create button panel */
		buttonPanel = new JPanel();
		buttonPanel.setLayout(new GridLayout(1,3));
		buttonPanel.add(playButton);
		buttonPanel.add(clearButton);
		buttonPanel.add(searchButton);

		/* Create search panel */
		searchPanel = new JPanel();
		searchPanel.setBackground(Color.white);
		searchPanel.setLayout(new BorderLayout());
		searchPanel.add(textField, BorderLayout.CENTER);
		searchPanel.add(buttonPanel, BorderLayout.EAST);
		searchPanel.setMaximumSize(new Dimension(800, 22));
		searchPanel.setPreferredSize(new Dimension(800, 22));

		midiSynth = new MidiSynth(this);

		contentPane.add(selectionPanel);
		contentPane.add(searchPanel);
		contentPane.add(midiSynth); 
	}


	/** 
	    Must be overridden in subclasses.
	*/
	public void actionPerformed(ActionEvent e) { }
 

	/** 
	    Opens the synthetizer. 
	    It is open all the time when the applet
	    is visible. This is a problem, if the operating system does not
	    support sound mixing, like Linux with FVWM2 window manager. 
	*/
	public void start()
	{
		midiSynth.open();
	}


	/** 
	    Closes the synthetizer. 
	*/
	public void stop()
	{
		midiSynth.close();
	}


	/**
	    Inner class for selection list (JComboBox) items. 
	*/
	protected class ListItem
	{
		public String name;
		public String value;

		/** 
		    Stores the text to be displayed in the user 
		    interface and the value to be associated with 
		    this name, which is sent to the search server. 
		*/
		public ListItem(String name, String value)
		{
			this.name = name;
			this.value = value;
		}

		public String toString()
		{
			return this.name;
		}
	}
}


