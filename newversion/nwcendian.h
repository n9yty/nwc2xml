
#ifndef	__NWCENDIAN_H__
#define	__NWCENDIAN_H__

#pragma once

// endian functions & macros : NWC files are little endian.
// Byte order detection uses the compiler-provided __BYTE_ORDER__ macro
// (GCC and Clang both define it), so no platform define is needed.

inline unsigned short SwapEndianS(unsigned short s)
{
	return (unsigned short)((s >> 8) | (s << 8));
}

inline unsigned long SwapEndianL(unsigned long l)
{
	// swaps the low 32 bits only; NWC data values are at most 32 bits
	return ((l >> 24) & 0x000000FFUL) |
	       ((l >>  8) & 0x0000FF00UL) |
	       ((l <<  8) & 0x00FF0000UL) |
	       ((l << 24) & 0xFF000000UL);
}

#if defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && \
    (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)

#define	BE2HostL(l)		(l)
#define	BE2HostS(s)		(s)
#define	Host2BEL(l)		(l)
#define	Host2BES(s)		(s)

#define	LE2HostL(l)		SwapEndianL(l)
#define	LE2HostS(s)		SwapEndianS(s)
#define	Host2LEL(l)		SwapEndianL(l)
#define	Host2LES(s)		SwapEndianS(s)

#else	// little endian host

#define	BE2HostL(l)		SwapEndianL(l)
#define	BE2HostS(s)		SwapEndianS(s)
#define	Host2BEL(l)		SwapEndianL(l)
#define	Host2BES(s)		SwapEndianS(s)

#define	LE2HostL(l)		(l)
#define	LE2HostS(s)		(s)
#define	Host2LEL(l)		(l)
#define	Host2LES(s)		(s)

#endif

#endif /* __NWCENDIAN_H__ */
