#ifndef _PROT_H
#define _PROT_H

#pragma pack(push,1)

typedef union {
  struct {
	char type;
	char err;	// 0 - OK, otherwise error code >0
  } q;
  unsigned short d;
} data_qu_t; // data quality
/*	Note to the "type" field
	------------------------
	For weight:  type=0,1,2 (norm/underweight/overweight)
	For car ID and seal ID:  type=0,1 (auto/manual)
*/

typedef struct{
  unsigned short num;	// seal number
  data_qu_t qu;			// data quality
  unsigned short regs;	// acual number of regs to store id
  char id[32];			// NULL-terminated string
} seal_t;

typedef struct{
  data_qu_t qu;			// data quality
  unsigned short regs;	// acual number of regs to store id
  char id[32];			// NULL-terminated string
} car_t;

typedef union{
  struct{
	data_qu_t qu;		// data quality
	char stb;			// stability bit (formatted as short for good alignement)
	char spare;			// zero field
	unsigned short w[3];// 0,1,2 = brutto,netto,tare
  } wgt;
  unsigned short d[5];
} wgt_t;
#pragma pack(pop)


#endif

