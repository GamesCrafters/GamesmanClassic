package edu.berkeley.gamesman.parallel.gui;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.Set;

import javax.swing.ListModel;
import javax.swing.event.ListDataEvent;
import javax.swing.event.ListDataListener;

public class CFModel implements ListModel
{

	ArrayList<ListDataListener> ldl = new ArrayList<ListDataListener>();
	ArrayList<TierData> data = new ArrayList<TierData>();	
	Set<Long> tierSet = new HashSet<Long>();

	ArrayList<TierData> displayData = new ArrayList<TierData>();
	ArrayList<Mod> mods = new ArrayList<Mod>();
	
	TierDataPrinter tdp;
	
	public CFModel(TierDataPrinter tdp)
	{
		this.tdp = tdp;
	}
		
	public void addListDataListener(ListDataListener l)
	{
		ldl.add(l);	
	}

	public Object getElementAt(int index)
	{
		return tdp.printTD(displayData.get(index));
	}

	public int getSize()
	{
		return displayData.size();
	}

	public void removeListDataListener(ListDataListener l)
	{
		ldl.remove(l);
	}
	
	
	class TierData
	{
		public long tier;
		public double priority;
		public double timeTaken = 0.0;
		public boolean blacklist = false;
		public int coreOn;
		
		/**
		 * First initialization. We can glean the tier and priority
		 * @param tier
		 */
		public TierData(long tier, double pri)
		{
			this.tier = tier;
			this.priority = pri;
		}
	}
	interface TierDataPrinter
	{
		public String printTD(TierData td);
	}
	class Mod
	{
		public static final int ADD = 1;
		public static final int REMOVE = 2;
		public static final int CHANGE = 3;
		
		int type, index0, index1;
		public Mod(int type, int i0, int i1)
		{
			this.type = type;
			this.index0 = i0;
			this.index1 = i1;
		}
	}
	//Custom methods
	
	/*
	public void addTier(long tier)
	{
		addTier(tier, false, 0);
	}
	public void addTier(long tier, double priority)
	{
		addTier(tier, true, priority);
	}

	public synchronized void addTier(long tier, boolean pri, double priority)
	{
		if (tierSet.add(tier) == true)
		{
			TierData td = new TierData(tier);
			td.priority = priority;
			td.showPri = pri;
			data.add(td);
			
			//record modification
			Mod m = new Mod(Mod.ADD, data.size()-1, data.size()-1);
			mods.add(m);
		}
		else
			return;
		
	}
	
	public synchronized void addTiers(long[] tier)
	{		
		if (tier.length == 0)
			return;
		int start = data.size();
		for (int a=0;a<tier.length;a++)
		{
			if (tierSet.add(tier[a]) == true)
				data.add(new TierData(tier[a]));
		}
		int end = data.size()-1;
		
		Mod m = new Mod(Mod.ADD, start, end);
		mods.add(m);
		
	}*/
	
	public synchronized void addTiers(long[] tiers, double[] pris)
	{
		if (tiers.length == 0)
			return;
		int start = data.size();
		for (int a=0;a<tiers.length;a++)
		{
			if (tierSet.add(tiers[a]) == true)
				data.add(new TierData(tiers[a], pris[a]));
		}
		int end = data.size()-1;
		
		Mod m = new Mod(Mod.ADD, start, end);
		mods.add(m);
	}

	public synchronized TierData removeTier(long tier)
	{
		int ind = findTier(tier);
		if (ind >= 0)
		{
			TierData td = data.remove(ind);
			tierSet.remove(tier);
			Mod m = new Mod(Mod.REMOVE, ind, ind);
			mods.add(m);
			return td;
		}
		return null;
	}
	
	public synchronized void addTier(TierData td)
	{
		if (tierSet.add(td.tier) == true)
		{
			data.add(td);
			Mod m = new Mod(Mod.ADD, data.size()-1, data.size()-1);
			mods.add(m);
		}
	}
	
	/*
	public synchronized void removeTier(long tier)
	{
		int ind = findTier(tier);
		if (ind >= 0)
		{
			data.remove(ind);
			tierSet.remove(tier);
			Mod m = new Mod(Mod.REMOVE, ind, ind);
			mods.add(m);
			return;
		}
		
	}

	
	public synchronized void setPriority(long tier, double priority)
	{
		int ind = findTier(tier);
		if (ind >= 0)
		{
			data.get(ind).priority = priority;
			data.get(ind).showPri = true;
			Mod m = new Mod(Mod.CHANGE, ind, ind);
			mods.add(m);
		}
	}
	*/
	
	public synchronized void setBlacklist(long tier, boolean blacklist)
	{
		int ind = findTier(tier);
		if (ind >= 0)
		{
			if (data.get(ind).blacklist != blacklist)
			{
				data.get(ind).blacklist = blacklist;
				Mod m = new Mod(Mod.CHANGE, ind, ind);
				mods.add(m);
			}
		}
	}
	
	public int findTier(long tier)
	{
		if (tierSet.contains(tier) == false)
			return -1;
		for (int a=0;a<data.size();a++)
		{
			if (data.get(a).tier == tier)
			{
				return a;
			}
		}
		return -1;
	}
	
	

	public void doNotifications()
	{
		//sync the copy
		ArrayList<Mod> localMods = new ArrayList<Mod>(mods.size());
		synchronized(this)
		{
			//make a new copy
			displayData = (ArrayList<TierData>) data.clone();
			localMods.addAll(mods);
			mods.clear();
		}
		
		//now, run through the modifications
		for (Mod m : localMods)
		{
			for (int b=0;b<ldl.size();b++)
			{
				if (m.type == Mod.ADD)
				{
					ldl.get(b).intervalAdded(new ListDataEvent(
						this, ListDataEvent.INTERVAL_ADDED, m.index0, m.index1));
				}
				else if (m.type == Mod.REMOVE)
				{
					ldl.get(b).intervalRemoved(new ListDataEvent(
						this, ListDataEvent.INTERVAL_REMOVED, m.index0, m.index1));
				}
				else if (m.type == Mod.CHANGE)
				{
					ldl.get(b).contentsChanged(new ListDataEvent(
						this, ListDataEvent.CONTENTS_CHANGED, m.index0, m.index1));
				}
			}
		}
	}

	public long getTier(int index2)
	{
		try
		{
			return displayData.get(index2).tier;
		}
		catch (ArrayIndexOutOfBoundsException aobe)
		{
			return -1;
		}
	}

	public boolean getBLState(long tier)
	{
		
		int ind = findTier(tier);
		if (ind >= 0)
		{
			return data.get(ind).blacklist;
		}
		return false;
	}
}
