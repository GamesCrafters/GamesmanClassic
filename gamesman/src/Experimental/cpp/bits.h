
class BitPacker {
	private:
		struct SlotInfo {
			char	mask;
			char	shift; // positive=left, negative=right
			char	next;
			char	byte;
		};
		
		SlotInfo**	slots;
		int		slotCount;
	
	public:
		BitPacker ();
		~BitPacker ();
		
		void		addSlot (int slot, int start, int len);
		
		inline unsigned int get (unsigned char* data, int slot)
			{
				SlotInfo*	memo = slots[slot];
				int		i = 0;
				unsigned int	r = 0;
				
				do {
					int	shamt = memo[i].shift;
					
					if (shamt < 0)
						r |= (0xff & (data[memo[i].byte] & memo[i].mask)) >> -shamt;
					else
						r |= (0xff & (data[memo[i].byte] & memo[i].mask)) << shamt;
				} while (memo[i++].next);
				
				return r;
			}
		
		inline void put (unsigned char* data, int slot, unsigned int n)
			{
				SlotInfo*	memo = slots[slot];
				int		i = 0;
				
				do {
					int		shamt = (int) memo[i].shift;
					int		byte = (int) memo[i].byte;
					unsigned int	mask = (unsigned int) memo[i].mask;
					unsigned int	r;
					
					if (shamt < 0)
						r = n << -shamt;
					else
						r = n >> shamt;
					
					data[byte] = (data[byte] & ~mask) | (r & mask);
				} while (memo[i++].next);
			}
		
		void debug ();
};

class OctetPacker {
	private:
		BitPacker	packer;
		int		bits;
		int		fullBytes;
		int		leftOver;
		int		leftOverStart;
	
	public:
		OctetPacker (int bits);
		
		inline void get (unsigned char* octet, int slot, unsigned char* data)
			{
				int	i = 0;
				int	n = slot * fullBytes;
				
				while (i < fullBytes)
					data[i++] = octet[n++];
				
				if (leftOver)
					data[i] = (char) packer.get(&octet[leftOverStart], slot);
			}
		
		inline void put (unsigned char* octet, int slot, unsigned char* data)
			{
				int	i = 0;
				int	n = slot * fullBytes;
				
				while (i < fullBytes)
					octet[n++] = data[i++];
				
				if (leftOver)
					packer.put(&octet[leftOverStart], slot, data[i]);
			}
};
