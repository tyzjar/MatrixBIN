#ifndef _MDB_H_INCLUDED
#define _MDB_H_INCLUDED

#define HOLD_REGS	64		// numbered 0--63
#define OUT_REGS	1024	// =0x400 1st address - 0040(mdb)

#define BYTES(buf)		(buf)[5]

#define FUNC(buf)		(buf)[7]
#define REG_0Hi(buf)	(buf)[8]
#define REG_0Lo(buf)	(buf)[9]
#define REGS_Hi(buf)	(buf)[10]
#define REGS_Lo(buf)	(buf)[11]

// reply 0x04
#define REP04_BYTES		8	// position for bytes count
#define REP04_REG0		9	// data starting position

// request 0x10
#define REQ10_REG0		13	// data starting position

typedef struct{
  int r;
  char name[33];
} REG_MAP;

#endif

