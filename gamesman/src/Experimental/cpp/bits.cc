
#include <iostream>

#include "bits.h"

using namespace std;

BitPacker::BitPacker ()
{
	slotCount = 0;
	slots = (SlotInfo**) 0;
}

BitPacker::~BitPacker ()
{
	if (slots)
		delete slots;
}

void BitPacker::addSlot (int slot, int start, int len)
{
	int	firstBit, lastBit;
	int	firstByte, lastByte;
	int	i, n;
	
	if (slot >= slotCount) {
		SlotInfo**	newSlots = new SlotInfo*[slot + 1];
		
		for (i = 0; i < slotCount; i++)
			newSlots[i] = slots[i];
		
		slotCount = slot + 1;
		for (; i < slotCount; i++)
			newSlots[i] = 0;
		
		slots = newSlots;
	}
	
	firstBit = start;
	lastBit = start + len - 1;
	firstByte = firstBit >> 3;
	lastByte = lastBit >> 3;
	
	slots[slot] = new SlotInfo[lastByte - firstByte + 1];
	
	if (firstByte == lastByte) {
		slots[slot][0].byte = firstByte;
		slots[slot][0].next = 0;
		slots[slot][0].mask = ((1 << len) - 1) << (firstBit & 7);
		slots[slot][0].shift = -(firstBit & 7);
		
		return;
	}
	
	slots[slot][0].byte = firstByte;
	slots[slot][0].next = 1;
	slots[slot][0].mask = (0xff << (firstBit & 7)) & 0xff;
	slots[slot][0].shift = -(firstBit & 7);
	
	for (i = 1, n = firstByte + 1; n < lastByte; i++, n++) {
		slots[slot][i].byte = n;
		slots[slot][i].next = 1;
		slots[slot][i].mask = 0xff;
		slots[slot][i].shift = (n << 3) - start;
	}
	
	slots[slot][i].byte = n;
	slots[slot][i].next = 0;
	slots[slot][i].mask = (1 << (1 + lastBit - (n << 3))) - 1;
	slots[slot][i].shift = (n << 3) - start;
}


OctetPacker::OctetPacker (int bits)
:	bits(bits)
{
	leftOver = bits & 7;
	fullBytes = bits >> 3;
	leftOverStart = fullBytes * 8;
	
	if (leftOver) {
		for (int i = 7; i >= 0; i--)
			packer.addSlot(i, i * leftOver, leftOver);
	}
}

static void printBinary (unsigned long n, int bits = (8 * sizeof(long)))
{
	for (unsigned long mask = 1 << (bits - 1); mask; mask >>= 1)
		cout << (mask & n ? "1" : "0");
}

void BitPacker::debug ()
{
	for (int i = 0; i < slotCount; i++) {
		if (!slots[i])
			continue;
		
		int t = 0;
		
		cout << "Slot " << i << endl;
		do {
			if (t)
				cout << "---------------" << endl;
			cout << "  mask = ";
			printBinary(slots[i][t].mask, 8);
			cout << endl << "  byte = " << ((int)slots[i][t].byte) << endl;
			cout << "  shamt = " << ((int) slots[i][t].shift) << endl;
			cout << "  next = " << ((int) slots[i][t].next) << endl;
		} while (slots[i][t++].next);
	}
}
