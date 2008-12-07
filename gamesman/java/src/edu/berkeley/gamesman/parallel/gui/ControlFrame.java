package edu.berkeley.gamesman.parallel.gui;

import java.awt.BorderLayout;
import java.util.Iterator;
import java.util.Map;

import javax.swing.JPanel;
import javax.swing.JFrame;

import edu.berkeley.gamesman.parallel.TierEventListener;
import edu.berkeley.gamesman.parallel.TierTreeManager;
import edu.berkeley.gamesman.parallel.TierTreeNode;
import edu.berkeley.gamesman.parallel.gui.CFModel.TierData;
import edu.berkeley.gamesman.parallel.gui.CFModel.TierDataPrinter;

import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.JList;
import javax.swing.JOptionPane;
import javax.swing.JScrollPane;
import javax.swing.JButton;
import javax.swing.JTabbedPane;
import javax.swing.Timer;
import java.awt.GridBagLayout;
import java.awt.GridBagConstraints;

/**
 * This needs a view for tiers to solve, tiers solved, tiers solving
 * And a few buttons to control stuff.
 * So one JFrame on the center with 3 colums, and another on the south
 * that has the control buttons.
 * @author John
 *
 */
public class ControlFrame extends JFrame implements TierEventListener {


	private static final long serialVersionUID = 1L;
	private JPanel jContentPane = null;
	private JList jlSolved = null;
	private JList jlToSolve = null;
	private CFModel cfModelSolving;  //  @jve:decl-index=0:
	private CFModel cfModelToSolve;  //  @jve:decl-index=0:
	private CFModel cfModelReady;
	private CFModel cfModelSolved;  //  @jve:decl-index=0:
	private JScrollPane jspSolved = null;
	private JScrollPane jspSolving = null;
	private JList jlSolving = null;
	private JScrollPane jspToSolve = null;
	private JButton jButton = null;
	private JTabbedPane jTabbedPane = null;
	private JPanel jPanel1 = null;
	private JPanel jPanel2 = null;
	private JPanel jPanel3 = null;
	private JPanel jPanel4 = null;
	private JScrollPane jspReady = null;
	private JList jlReady = null;
	private Timer t = new Timer(150, new ActionListener() {
		public void actionPerformed(ActionEvent e)
		{
			//update all 4 cfs
			CFModel[] cfs = new CFModel[]{cfModelSolving, cfModelToSolve,
					cfModelReady, cfModelSolved};
			
			for (CFModel cf : cfs)
			{
				if (cf == null)
					continue;
				cf.doNotifications();
			}
		}
	});
	private JPanel jpBottom = null;
	private JButton jbUnblacklist = null;
	private TierTreeManager ttm;
	/**
	 * This is the default constructor
	 * @param ttm 
	 */
	public ControlFrame(TierTreeManager ttm)
	{
		super();
		this.ttm = ttm;
		initialize();
	}

	/**
	 * This method initializes this
	 * 
	 * @return void
	 */
	private void initialize()
	{
		this.setSize(317, 340);
		this.setContentPane(getJContentPane());
		this.setTitle("JFrame");
		t.start();
	}

	/**
	 * This method initializes jContentPane
	 * 
	 * @return javax.swing.JPanel
	 */
	private JPanel getJContentPane()
	{
		if (jContentPane == null)
		{
			jContentPane = new JPanel();
			jContentPane.setLayout(new BorderLayout());
			jContentPane.add(getJTabbedPane(), BorderLayout.CENTER);
			jContentPane.add(getJpBottom(), BorderLayout.SOUTH);
		}
		return jContentPane;
	}

	public void done(boolean errors)
	{
		setVisible(false);
		this.dispose();	//have to do this, retarded
		t.stop();
	}

	Map<Long, TierTreeNode> fullMap;
	private JButton jbReduce = null;
	private JButton jbInc = null;
	
	public void setup(Map<Long, TierTreeNode> fullMap, Map<Long, TierTreeNode> toSolveMap)
	{
		this.fullMap = fullMap;
		long[] tiersFull = new long[fullMap.size()];
		int a = 0;
		for (Iterator<Long> i = fullMap.keySet().iterator();i.hasNext();)
		{
			tiersFull[a++] = i.next();
		}
		
		long[] tiersToSolve = new long[toSolveMap.size()];
		double[] tiersToSolvePri = new double[toSolveMap.size()];
		a = 0;
		for (Iterator<Long> i = toSolveMap.keySet().iterator();i.hasNext();)
		{
			tiersToSolve[a] = i.next();
			tiersToSolvePri[a] = toSolveMap.get(tiersToSolve[a]).priority;
			a++;
		}
		
		//iterate thru tiersFull, and add to tiersSolved if not in tiersToSolve
		long[] tiersSolved = new long[fullMap.size() - toSolveMap.size()];
		double[] tiersSolvedPri = new double[fullMap.size() - toSolveMap.size()];
		a = 0;
		for (int b=0;b<tiersFull.length;b++)
		{
			if (toSolveMap.keySet().contains(tiersFull[b]) == false)
			{
				tiersSolved[a] = tiersFull[b];
				tiersSolvedPri[a] = fullMap.get(tiersFull[b]).priority;
				a++;
			}
		}
		
		//set up the lists
		cfModelSolved.addTiers(tiersSolved, tiersSolvedPri);
		cfModelToSolve.addTiers(tiersToSolve, tiersToSolvePri);	
	}
	
	public void start()
	{
		//enable the buttons
		jButton.setEnabled(true);
		jbUnblacklist.setEnabled(true);
		jbReduce.setEnabled(true);
		jbInc.setEnabled(true);
	}
	
	interface modifyTD
	{
		public void modify(TierData td);
	}

	private void moveTier(CFModel cfmSrc, CFModel cfmDest, long tier, modifyTD modifier)
	{
		TierData td = cfmSrc.removeTier(tier);
		if (td != null)
		{
			modifier.modify(td);	//do whatever you need to do to it.
			cfmDest.addTier(td);
		}
	}
	
	public void tierMoveToReady(long tier)
	{
		//Transfer, don;t need to do anything to the tier.
		moveTier(cfModelToSolve, cfModelReady, tier, new modifyTD() {
			public void modify(TierData td){}});
	}

	public void tierFinishedSolve(long tier, boolean bad, final double seconds)
	{
		moveTier(cfModelSolving, cfModelSolved, tier, new modifyTD() {
			public void modify(TierData td)
			{
				td.timeTaken = seconds;
			}
		});
	}

	public void tierStartSolve(long tier, final int coreOn)
	{
		moveTier(cfModelReady, cfModelSolving, tier, new modifyTD() {
			public void modify(TierData td)
			{
				td.coreOn = coreOn;
			}
		});
	}

	/**
	 * This method initializes jlSolved	
	 * 	
	 * @return javax.swing.JList	
	 */
	private JList getJlSolved()
	{
		if (jlSolved == null)
		{
			jlSolved = new JList();
			this.cfModelSolved = new CFModel(new TierDataPrinter() {
				public String printTD(TierData td)
				{
					//For solved list, we print out tier num and time to solve
					return td.tier + " - " + td.timeTaken;
				}
			});
			jlSolved.setModel(this.cfModelSolved);
		}
		return jlSolved;
	}

	/**
	 * This method initializes jlToSolve	
	 * 	
	 * @return javax.swing.JList	
	 */
	private JList getJlToSolve()
	{
		if (jlToSolve == null)
		{
			jlToSolve = new JList();
			this.cfModelToSolve = new CFModel(new TierDataPrinter() {
				public String printTD(TierData td)
				{
					//For toSolve list, we just print out tier num, and if blacklist
					return (td.blacklist?"!":"") + td.tier;
				}
			});
			jlToSolve.setModel(this.cfModelToSolve);
		}
		return jlToSolve;
	}

	/**
	 * This method initializes jspSolved	
	 * 	
	 * @return javax.swing.JScrollPane	
	 */
	private JScrollPane getJspSolved()
	{
		if (jspSolved == null)
		{
			jspSolved = new JScrollPane();
			jspSolved.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
			jspSolved.setViewportView(getJlSolved());
		}
		return jspSolved;
	}

	/**
	 * This method initializes jspSolving	
	 * 	
	 * @return javax.swing.JScrollPane	
	 */
	private JScrollPane getJspSolving()
	{
		if (jspSolving == null)
		{
			jspSolving = new JScrollPane();
			jspSolving.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
			jspSolving.setViewportView(getJlSolving());
		}
		return jspSolving;
	}

	/**
	 * This method initializes jlSolving	
	 * 	
	 * @return javax.swing.JList	
	 */
	private JList getJlSolving()
	{
		if (jlSolving == null)
		{
			jlSolving = new JList();
			this.cfModelSolving = new CFModel(new TierDataPrinter() {
				public String printTD(TierData td)
				{
					//For solving list, we print out tier num and the core it is on
					return td.tier + " on core " + td.coreOn;
				}
			});
			jlSolving.setModel(this.cfModelSolving);
		}
		return jlSolving;
	}

	/**
	 * This method initializes jspToSolve	
	 * 	
	 * @return javax.swing.JScrollPane	
	 */
	private JScrollPane getJspToSolve()
	{
		if (jspToSolve == null)
		{
			jspToSolve = new JScrollPane();
			jspToSolve.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
			jspToSolve.setViewportView(getJlToSolve());
		}
		return jspToSolve;
	}

	/**
	 * This method initializes jButton	
	 * 	
	 * @return javax.swing.JButton	
	 */
	private JButton getJButton()
	{
		if (jButton == null)
		{
			jButton = new JButton();
			jButton.setText("Blacklist Tiers");
			jButton.setEnabled(false);
			jButton.addActionListener(new java.awt.event.ActionListener() {
				public void actionPerformed(java.awt.event.ActionEvent e)
				{
					ttm.blacklistTiers(getTiersSelectedForBL(true));
				}
			});
		}
		return jButton;
	}
	
	private long[] getTiersSelectedForBL(boolean blacklist)
	{
		//check out whats selected in the toSolve tier.
		int tab = jTabbedPane.getSelectedIndex();
		long[] tiersToBL;
		int[] indicies = new int[0];
		CFModel cfm = null;
		if (tab == 0 || tab == 1)
		{
			JOptionPane.showMessageDialog(null, "Cannot blacklist solved or solving threads.");
		}
		else if (tab == 2)
		{
			indicies = jlReady.getSelectedIndices();
			cfm = cfModelReady;
		}
		else if (tab == 3)
		{
			indicies = jlToSolve.getSelectedIndices();
			cfm = cfModelToSolve;
		}
		else
		{
			JOptionPane.showMessageDialog(null, "Invalid tab selected - " + tab + ".");
		}
		if (indicies.length == 0)
			return new long[0];
		tiersToBL = new long[indicies.length];
		for (int a=0;a<indicies.length;a++)
		{
			tiersToBL[a] = cfm.getTier(indicies[a]);
		}
		for (int a=0;a<tiersToBL.length;a++)
		{
			cfm.setBlacklist(tiersToBL[a], blacklist);
		}
		return tiersToBL;
	}

	/**
	 * This method initializes jTabbedPane	
	 * 	
	 * @return javax.swing.JTabbedPane	
	 */
	private JTabbedPane getJTabbedPane()
	{
		if (jTabbedPane == null)
		{
			jTabbedPane = new JTabbedPane();
			jTabbedPane.addTab("Solved", null, getJPanel1(), "List of tiers that are solved already");
			jTabbedPane.addTab("Solving", null, getJPanel2(), "List of tiers that are currently solving");
			jTabbedPane.addTab("Ready", null, getJPanel3(), "List of tiers that are currently ready to solve (all dependencies are done)");
			jTabbedPane.addTab("To Solve", null, getJPanel4(), "List of tiers that still have to be solved, excluding ready tiers");
		}
		return jTabbedPane;
	}

	/**
	 * This method initializes jPanel1	
	 * 	
	 * @return javax.swing.JPanel	
	 */
	private JPanel getJPanel1()
	{
		if (jPanel1 == null)
		{
			GridLayout gridLayout1 = new GridLayout();
			gridLayout1.setRows(1);
			gridLayout1.setColumns(1);
			jPanel1 = new JPanel();
			jPanel1.setLayout(gridLayout1);
			jPanel1.setName("");
			jPanel1.add(getJspSolved(), null);
		}
		return jPanel1;
	}

	/**
	 * This method initializes jPanel2	
	 * 	
	 * @return javax.swing.JPanel	
	 */
	private JPanel getJPanel2()
	{
		if (jPanel2 == null)
		{
			GridLayout gridLayout2 = new GridLayout();
			gridLayout2.setRows(1);
			gridLayout2.setColumns(1);
			jPanel2 = new JPanel();
			jPanel2.setLayout(gridLayout2);
			jPanel2.add(getJspSolving(), null);
		}
		return jPanel2;
	}

	/**
	 * This method initializes jPanel3	
	 * 	
	 * @return javax.swing.JPanel	
	 */
	private JPanel getJPanel3()
	{
		if (jPanel3 == null)
		{
			GridLayout gridLayout = new GridLayout();
			gridLayout.setRows(1);
			gridLayout.setColumns(1);
			jPanel3 = new JPanel();
			jPanel3.setLayout(gridLayout);
			jPanel3.add(getJspReady(), null);
		}
		return jPanel3;
	}

	/**
	 * This method initializes jPanel4	
	 * 	
	 * @return javax.swing.JPanel	
	 */
	private JPanel getJPanel4()
	{
		if (jPanel4 == null)
		{
			GridLayout gridLayout3 = new GridLayout();
			gridLayout3.setRows(1);
			gridLayout3.setColumns(1);
			jPanel4 = new JPanel();
			jPanel4.setLayout(gridLayout3);
			jPanel4.add(getJspToSolve(), null);
		}
		return jPanel4;
	}

	/**
	 * This method initializes jspReady	
	 * 	
	 * @return javax.swing.JScrollPane	
	 */
	private JScrollPane getJspReady()
	{
		if (jspReady == null)
		{
			jspReady = new JScrollPane();
			jspReady.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
			jspReady.setViewportView(getJlReady());
		}
		return jspReady;
	}

	/**
	 * This method initializes jlReady	
	 * 	
	 * @return javax.swing.JList	
	 */
	private JList getJlReady()
	{
		if (jlReady == null)
		{
			jlReady = new JList();
			this.cfModelReady = new CFModel(new TierDataPrinter() {
				public String printTD(TierData td)
				{
					//For ready list, we print out tier num and priority/bl
					return (td.blacklist?"!":"") + td.tier + " - " + td.priority;
				}
			});
			jlReady.setModel(this.cfModelReady);
		}
		return jlReady;
	}

	/**
	 * This method initializes jpBottom	
	 * 	
	 * @return javax.swing.JPanel	
	 */
	private JPanel getJpBottom()
	{
		if (jpBottom == null)
		{
			GridBagConstraints gridBagConstraints1 = new GridBagConstraints();
			gridBagConstraints1.gridx = 1;
			gridBagConstraints1.gridy = 1;
			GridBagConstraints gridBagConstraints = new GridBagConstraints();
			gridBagConstraints.gridx = 0;
			gridBagConstraints.gridy = 1;
			jpBottom = new JPanel();
			jpBottom.setLayout(new GridBagLayout());
			jpBottom.add(getJButton(), new GridBagConstraints());
			jpBottom.add(getJbUnblacklist(), new GridBagConstraints());
			jpBottom.add(getJbReduce(), gridBagConstraints);
			jpBottom.add(getJbInc(), gridBagConstraints1);
		}
		return jpBottom;
	}

	/**
	 * This method initializes jbUnblacklist	
	 * 	
	 * @return javax.swing.JButton	
	 */
	private JButton getJbUnblacklist()
	{
		if (jbUnblacklist == null)
		{
			jbUnblacklist = new JButton();
			jbUnblacklist.setText("Unblacklist Tiers");
			jbUnblacklist.setEnabled(false);
			jbUnblacklist.addActionListener(new java.awt.event.ActionListener() {
				public void actionPerformed(java.awt.event.ActionEvent e)
				{
					ttm.unblacklistTiers(getTiersSelectedForBL(false));
				}
			});
		}
		return jbUnblacklist;
	}

	/**
	 * This method initializes jbReduce	
	 * 	
	 * @return javax.swing.JButton	
	 */
	private JButton getJbReduce()
	{
		if (jbReduce == null)
		{
			jbReduce = new JButton();
			jbReduce.setText("Reduce Cores");
			jbReduce.setEnabled(false);
			jbReduce.addActionListener(new java.awt.event.ActionListener() {
				public void actionPerformed(java.awt.event.ActionEvent e)
				{
					ttm.decreaseCoreCount();
				}
			});
		}
		return jbReduce;
	}

	/**
	 * This method initializes jbInc	
	 * 	
	 * @return javax.swing.JButton	
	 */
	private JButton getJbInc()
	{
		if (jbInc == null)
		{
			jbInc = new JButton();
			jbInc.setText("Increase Cores");
			jbInc.setEnabled(false);
			jbInc.addActionListener(new java.awt.event.ActionListener() {
				public void actionPerformed(java.awt.event.ActionEvent e)
				{
					ttm.increaseCoreCount();
				}
			});
		}
		return jbInc;
	}

}  //  @jve:decl-index=0:visual-constraint="10,10"
