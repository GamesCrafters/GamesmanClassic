
#include <iostream>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>

#include "bits.h"

using namespace std;

static const int	testCount = 1000;

int* generateSlots (int count)
{
	static int	slotSizes[4];
	int		i;
	
	do {
		for (i = 0; i < 4; i++)
			slotSizes[i] = 0;
		
		for (i = 0; i < count; i++)
			++slotSizes[rand() & 3];
		
		for (i = 0; i < 4; i++)
			if (slotSizes[i] < 1 || slotSizes[i] > 32)
				break;
	} while (i != 4);
	
	return slotSizes;
}

void printBinary (unsigned int n, int bits)
{
	unsigned int	p = 1 << (bits - 1);
	
	while (p) {
		if (n & p)
			cout << "1";
		else
			cout << "0";
		
		p >>= 1;
	}
}

bool check (BitPacker& bp, unsigned char* data, unsigned int* n, int size)
{
	int	i, t;
	
	for (i = 0; i < 4; i++) {
		if (n[i] != bp.get(data, i)) {
			cout << "Failed!" << endl << "Dump: " << endl;
			for (t = 0; t < size; t++) {
				printBinary(data[t], 8);
				cout << " ";
			}
			
			cout << endl;
			
			for (t = 0; t < 4; t++) {
				printBinary(bp.get(data, t), 32);
				cout << " ";
			}
			
			cout << endl;
			bp.debug();
			
			return false;
		}
	}
	
	return true;
}

bool bpGetTest (int size)
{
	int*		slots = generateSlots(size * 8);
	unsigned char	data[size];
	int		bits[size * 8];
	int		i, t, q;
	unsigned int	n[4] = {0, 0, 0, 0};
	BitPacker	bp;
	
	for (i = 0, q = 0; i < size; i++) {
		data[i] = rand() & 0xff;
		for (t = 0; t < 8; t++, q++)
			bits[q] = (data[i] & (1 << t));
	}
	
	for (i = 0, q = 0; i < 4; i++) {
		bp.addSlot(i, q, slots[i]);
		n[i] = 0;
		for (t = 0; t < slots[i]; t++, q++) {
			if (bits[q])
				n[i] |= 1 << t;
		}
	}
	
	return check(bp, data, n, size);
}

bool bpPutTest (int size)
{
	int*		slots = generateSlots(size * 8);
	unsigned char	data[size];
	unsigned int	n[4] = {0, 0, 0, 0};
	int		i, q;
	BitPacker	bp;
	
	memset(data, 0, size);
	for (i = q = 0; i < 4; i++) {
		bp.addSlot(i, q, slots[i]);
		q += slots[i];
	}
	
	for (i = 0; i < 100; i++) {
		int	idx = rand() & 3;
		int	x = rand() & ((1 << slots[idx]) - 1);
		
		n[idx] = x;
		bp.put(data, idx, x);
	}
	
	return check(bp, data, n, size);
}

bool opTest (int size)
{
	unsigned char	octet[size];
	unsigned char	data[4];
	unsigned int	n[8];
	BitPacker	bp;
	OctetPacker	op(size);
	unsigned int	mask = (1 << size) - 1;
	
	bp.addSlot(0, 0, size);
	
	for (int i = 0; i < 100; i++) {
		unsigned int	v = rand() & mask;
		int		idx = rand() & 7;
		
		n[idx] = v;
		bp.put(data, 0, v);
		op.put(octet, idx, data);
	}
	
	for (int i = 0; i < 8; i++) {
		int	t;
		
		op.get(octet, i, data);
		if (n[i] != bp.get(data, 0)) {
			cout << "Failed!" << endl << "Dump: " << endl;
			cout << "Numbers: ";
			
			for (t = 0; t < 8; t++) {
				printBinary(n[t], size);
				cout << " ";
			}
			
			cout << endl;
			cout << "Octet:   ";
			
			for (t = 0; t < size; t++) {
				printBinary(octet[t], 8);
				cout << " ";
			}
			
			cout << endl;
			cout << "Output:  ";
			
			for (t = 0; t < 8; t++) {
				op.get(octet, t, data);
				printBinary(bp.get(data, 0), size);
			}
			
			cout << endl;
			return false;
		}
	}
	
	return true;
}

int main (int argc, char** argv)
{
	int	i;
	int	seed;
	int	n;
	
	if (argc == 2)
		seed = atoi(argv[1]);
	else
		seed = time(NULL);
	
	srand(seed);
	
	cout << "Seed: " << seed << endl;
	cout << "Running BitPacker::get() tests.." << endl;
	
	for (n = 3; n < 13; n += 2) {
		cout << "Trying " << testCount << " tests on " << n << " bytes: ";
		for (i = 0; i < testCount && bpGetTest(n); i++) {}
		
		if (i < testCount) {
			cout << "Failed on test #" << (i + 1) << endl;
			return 1;
		} else
			cout << "Passed!" << endl;
	}
	
	cout << "Running BitPacker::put() tests.." << endl;
	for (n = 3; n < 13; n += 2) {
		cout << "Trying " << testCount << " tests on " << n << " bytes: ";
		for (i = 0; i < testCount && bpPutTest(n); i++) {}
		
		if (i < testCount) {
			cout << "Failed on test #" << (i + 1) << endl;
			return 1;
		} else
			cout << "Passed!" << endl;
	}
	
	cout << "Running OctetPacker tests.." << endl;
	for (n = 1; n <= 32; n++) {
		cout << "Trying " << testCount << " tests on " << n << "-bit octets: ";
		for (i = 0; i < testCount && opTest(n); i++) {}
		
		if (i < testCount) {
			cout << "Failed on test #" << (i + 1) << endl;
			return 1;
		} else
			cout << "Passed!" << endl;
	}
	
	return 0;
}
