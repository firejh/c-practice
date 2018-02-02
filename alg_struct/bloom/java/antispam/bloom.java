package antispam;

class hash
{
	private static int fnv1(String str)
	{
		int idx = 0;
		int len = str.length();

		final int p = 16777619;
		int hash = (int)2166136261L;

		for(idx = 0; idx < len; idx++) {
			hash = (hash ^ str.charAt(idx)) * p;
		}

		hash += hash << 13;
		hash ^= hash >> 7;
		hash += hash << 3;
		hash ^= hash >> 17;
		hash += hash << 5;

		return (hash & 0x7FFFFFFF);
	}

	/* RS Hash Function */
	private static int rs(String str)
	{
		final int b = 378551;
		int a = 63689;

		int hash = 0;
		int idx  = 0;
		int len  = str.length();

		for (idx = 0; idx < len; idx++) {
			hash = hash * a + str.charAt(idx);
			a = a * b;
		}

		return (hash & 0x7FFFFFFF);
	}

	private static int js(String str)
	{
		int idx  = 0;
		int len  = str.length();
		int hash = 1315423911;

		for (idx = 0; idx < len; idx++) {
			hash ^= ((hash << 5) + str.charAt(idx) + (hash >> 2));
		}

		return (hash & 0x7FFFFFFF);
	}

	/* P. J. Weinberger Hash Function */
	private static int pjw(String str)
	{
		final int BitsInUnignedInt	= 32;
		int ThreeQuarters		= ((BitsInUnignedInt * 3) / 4);
		int OneEighth			= (BitsInUnignedInt / 8);

		int hash		= 0;
		int test		= 0;
		int idx			= 0;
		int HighBits	= (0xFFFFFFFF) << (BitsInUnignedInt - OneEighth);
		int len			= str.length();

		for (idx = 0; idx < len; idx++) {
			hash = (hash << OneEighth) + str.charAt(idx);
			if ((test = hash & HighBits) != 0)	{
				hash = ((hash ^ (test >> ThreeQuarters)) & (~HighBits));
			}
		}

		return (hash & 0x7FFFFFFF);
	}

	/* ELF Hash Function */
	private static int elf(String str)
	{
		int hash = 0;
		int x    = 0;
		int idx  = 0;
		int len = str.length();

		for (idx = 0; idx < len; idx++) {
			hash = (hash << 4) + str.charAt(idx);
			if ((x = hash & 0xF0000000) != 0)	{
				hash ^= (x >> 24);
				hash &= ~x;
			}
		}

		return (hash & 0x7FFFFFFF);
	}

	/* BKDR Hash Function */
	private static int bkdr(String str)
	{
		final int seed = 31; // 31 131 1313 13131 131313 etc..
		int hash = 0;
		int idx  = 0;
		int len = str.length();

		for (idx = 0; idx < len; idx++) {
			hash = hash * seed + str.charAt(idx);
		}

		return (hash & 0x7FFFFFFF);
	}

	/* SDBM Hash Function */
	private static int sdbm(String str)
	{
		int idx = 0;
		int len = str.length();
		int hash = 0;

		for (idx = 0; idx < len; idx++) {
			hash = str.charAt(idx) + (hash << 6) + (hash << 16) - hash;
		}

		return (hash & 0x7FFFFFFF);
	}

	/* DJB Hash Function */
	private static int djb(String str)
	{
		int idx  = 0;
		int len = str.length();
		int hash = 5381;

		for (idx = 0; idx < len; idx++) {
			hash += (hash << 5) + str.charAt(idx);
		}

		return (hash & 0x7FFFFFFF);
	}

	/* AP Hash Function */
	private static int ap(String str)
	{
		int idx = 0;
		int len = str.length();
		int hash = 0;

		for (idx = 0; idx < len; idx++) {
			if ((idx & 0x1) == 0) {
				hash ^= ((hash << 7) ^ str.charAt(idx) ^ (hash >> 3));
			} else {
				hash ^= (~((hash << 11) ^ str.charAt(idx) ^ (hash >> 5)));
			}
		}

		return (hash & 0x7FFFFFFF);
	}

	private static int dek(String str)
	{
		int idx = 0;
		int len = str.length();
		int hash = len;

		for (idx = 0; idx < len; idx++) {
			hash = ((hash << 5) ^ (hash >> 27)) ^ str.charAt(idx);
		}

		return (hash & 0x7FFFFFFF);
	}

	private static int bernstein(String str)
	{
		int hash = 0;
		int idx = 0;
		int len = str.length();

		for (idx=0; idx < len; idx++) {
			hash = 33 * hash + str.charAt(idx);
		}

		return (hash & 0x7FFFFFFF);
	}

	private static int rotate(String str)
	{
		int idx = 0;
		int len = str.length();
		int hash = len;

		for (idx = 0; idx < len; ++idx) {
			hash = (hash << 4) ^ (hash >> 28) ^ str.charAt(idx);
		}

		return ((hash ^ (hash >> 10) ^ (hash >> 20)) & 0x7FFFFFFF);
	}

	private static int mix(String str)
	{
		int hash = bkdr(str);
		hash += fnv1(str);

		return (hash & 0x7FFFFFFF);
	}

	public static int hash_calc(String str, int hash_idx)
	{
		switch (hash_idx) {
			case 0:
				return fnv1(str);

			case 1:
				return rs(str);

			case 2:
				return js(str);

			case 3:
				return pjw(str);

			case 4:
				return elf(str);

			case 5:
				return bkdr(str);

			case 6:
				return sdbm(str);

			case 7:
				return djb(str);

			case 8:
				return ap(str);

			case 9:
				return dek(str);

			case 10:
				return bernstein(str);

			case 11:
				return rotate(str);

			case 12:
				return mix(str);
		}

		return 0;
	}

	public static final int hash_func_num = 13;
}

public class bloom
{
	public bloom(int size)
	{
		size *= ratio_;
		size_ = (size >> 3) + (((size % 8) != 0) ? 1 : 0);

		bitset_ = new byte[size_];
		size_ <<= 3;
	}

	public bloom(byte[] dict)
	{
		size_ = dict.length;
		bitset_ = new byte[size_];
		System.arraycopy(dict, 0, bitset_, 0, size_);
		size_ <<= 3;
	}

	public void put(String str)
	{
		int idx = 0;
		int pos = 0;
		int hash_num = hash.hash_func_num;

		// System.out.println(str);
		for (idx = 0; idx < hash_num; idx++) {
			pos = Math.abs(hash.hash_calc(str, idx) % size_);
			bitset_[pos >> 3] |= ((byte)(0x01) << (pos % 8));
			// System.out.println("idx = " + idx + ", pos = " + pos);
		}
		// System.out.println("");
	}

	public boolean find(String str)
	{
		int idx = 0;
		int pos = 0;
		byte flag = 0;
		int hash_num = hash.hash_func_num;

		for (idx = 0; idx < hash_num; idx++) {
			pos = Math.abs(hash.hash_calc(str, idx) % size_);
			flag = (byte)(bitset_[pos >> 3] & ((byte)(0x01) << (pos % 8)));
			if (flag == (byte)(0x0)) {
				// System.out.println("idx = " + idx + ", pos = " + pos + ", flag = " + flag);
				return false;
			}
		}

		return true;
	}

	byte[] data()
	{
		return bitset_;
	}

	private byte []bitset_;
	private int size_;
	private final int ratio_ = 27; //error rate:27 1/500000; 25 1/100000; 20 1/10000

	public static final String BLOOM_VERSION = "cmppn_w112";  //"bloom_v001"
}

