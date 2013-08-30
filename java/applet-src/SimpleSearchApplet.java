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
public class SimpleSearchApplet extends SearchApplet
{
	private JComboBox limit;
	private JComboBox errors;

	/** 
	    Creates graphical elements in the interface. 
	*/
	public void init()
	{
		setup();

		/* Add additional UI components here. */
	}


	/** 
	    Sends the search request when enter is pressed. 
	*/
	public void actionPerformed(ActionEvent e)
	{
		String notes = "";
		textField.selectAll();

		try
		{
			notes = URLEncoder.encode(textField.getText(), "UTF-8");
		}
		catch(Exception ex) {} 

		/* Prepare a request */
		String urlString = serverAddress + 
			"?notepattern=" + notes +
			"&algorithm=geometric_p2" +
			"&limit=100" + 
			"&songonce=1" +    // show first matches only
			"&sort=2" +  // sort by errors
			"&errors=0" + 
			"&gap=0";

		URL url = null;
		try
		{
			url = new URL(urlString);
		}
		catch (MalformedURLException mue) { return; }

		/* Send request and show results in result frame */
		this.appletContext.showDocument(url, "result");
	}
}


