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
public class AlgorithmSearchApplet extends SearchApplet
{
	private JComboBox algorithm;
	private JComboBox limit;
	private JComboBox songonce;
	private JComboBox sort;
	private JComboBox errors;
	private JComboBox gaps;

	/** 
	    Creates graphical elements in the interface. 
	*/
	public void init()
	{
		setup();

		limit = new JComboBox();
		songonce = new JComboBox();
		sort = new JComboBox();
		errors = new JComboBox();
		gaps = new JComboBox();

		errors.setEnabled(false); 
		gaps.setEnabled(false);

		sort.addItem(new ListItem("Errors and Transposition", "2"));
		sort.addItem(new ListItem("Splits", "3"));
		sort.addItem(new ListItem("Duration", "4"));
		sort.addItem(new ListItem("No Sorting", "0"));

		/* Create selections */
		algorithm = new JComboBox();
		algorithm.addActionListener(new ActionListener()
		{ 
			public void actionPerformed(ActionEvent e) 
			{ 
				String alg = ((ListItem) algorithm.getSelectedItem()).value;
				sort.setSelectedIndex(0);
				gaps.setEnabled(false); 
				errors.setEnabled(false); 

				if (alg == "geometric_p2" || alg == "lcts") 
				{ 
					errors.setEnabled(true); 
				}
				else if (alg == "geometric_p3")
				{
					sort.setSelectedIndex(2);
				}
				else if (alg == "splitting") 
				{ 
					errors.setEnabled(true); 
					gaps.setEnabled(true); 
					sort.setSelectedIndex(1);
				}
			} 
		} );

		algorithm.addItem(new ListItem("[1] P1", "geometric_p1"));
		algorithm.addItem(new ListItem("[2] P2", "geometric_p2"));
		algorithm.addItem(new ListItem("[3] P3", "geometric_p3"));
		algorithm.addItem(new ListItem("[4] MonoPoly", "monopoly"));
		algorithm.addItem(new ListItem("[5] IntervalMatching", "intervalmatching"));
		algorithm.addItem(new ListItem("[6] ShiftOrAnd", "shiftorand"));
		algorithm.addItem(new ListItem("[7] PolyCheck", "polycheck"));
		algorithm.addItem(new ListItem("[8] Splitting", "splitting"));
		algorithm.addItem(new ListItem("[9] LCTS", "lcts"));
		//algorithm.addItem(new ListItem("[10] Dynamic Programming", "dynprog"));

		limit.addItem(new ListItem("10", "10"));
		limit.addItem(new ListItem("20", "20"));
		limit.addItem(new ListItem("40", "40"));
		limit.addItem(new ListItem("80", "80"));
		limit.addItem(new ListItem("100", "100"));
		limit.addItem(new ListItem("150", "150"));
		limit.addItem(new ListItem("200", "200"));
		limit.addItem(new ListItem("400", "400"));
		limit.addItem(new ListItem("600", "600"));

		songonce.addItem(new ListItem("First", "1"));
		songonce.addItem(new ListItem("All", "0"));

		errors.addItem(new ListItem("0", "0"));
		errors.addItem(new ListItem("1", "1"));
		errors.addItem(new ListItem("2", "2"));
		errors.addItem(new ListItem("3", "3"));
		errors.addItem(new ListItem("4", "4"));
		errors.addItem(new ListItem("5", "5"));
		errors.addItem(new ListItem("6", "6"));
		errors.addItem(new ListItem("7", "7"));
		errors.addItem(new ListItem("8", "8"));
		errors.addItem(new ListItem("9", "9"));

		gaps.addItem(new ListItem("0", "0"));
		gaps.addItem(new ListItem("1", "1"));
		gaps.addItem(new ListItem("2", "2"));
		gaps.addItem(new ListItem("3", "3"));
		gaps.addItem(new ListItem("4", "4"));
		gaps.addItem(new ListItem("5", "5"));
		gaps.addItem(new ListItem("6", "6"));
		gaps.addItem(new ListItem("7", "7"));
		gaps.addItem(new ListItem("8", "8"));
		gaps.addItem(new ListItem("9", "9"));

		javax.swing.UIManager.put ("Label.foreground", Color.gray);

		selectionPanel.setLayout(new GridLayout(2,6));
		selectionPanel.add(new JLabel("Search method"));
		selectionPanel.add(new JLabel("Sort method"));
		selectionPanel.add(new JLabel("Max. Results"));
		selectionPanel.add(new JLabel("Matches inside song"));
		selectionPanel.add(new JLabel("Errors/Splits"));
		selectionPanel.add(new JLabel("Gaps"));
		selectionPanel.add(algorithm);
		selectionPanel.add(sort);
		selectionPanel.add(limit);
		selectionPanel.add(songonce);
		selectionPanel.add(errors);
		selectionPanel.add(gaps);
		selectionPanel.setMaximumSize(new Dimension(800, 46));
		selectionPanel.setPreferredSize(new Dimension(800, 46));
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

		textField.selectAll();

		/* Prepare a request */
		String urlString = serverAddress + 
			"?notepattern=" + notes +
			"&algorithm=" + ((ListItem) algorithm.getSelectedItem()).value +
			"&limit=" + ((ListItem) limit.getSelectedItem()).value +
			"&songonce=" + ((ListItem) songonce.getSelectedItem()).value +
			"&sort=" + ((ListItem) sort.getSelectedItem()).value +
			"&errors=" + ((ListItem) errors.getSelectedItem()).value +
			"&gap=" + ((ListItem) gaps.getSelectedItem()).value;

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
