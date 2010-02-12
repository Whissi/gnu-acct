/* pacct_rd.c
 *
 * routines that read acct/pacct files
 *
 */

#include "config.h"

#include <stdio.h>
#include <math.h>

#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#include "common.h"
#include "files.h"
#include "file_rd.h"
#include "pacct_rd.h"

/* globals */

static struct file_rd_info *pacct_info = NULL;

extern int ahz;

#define BUFFERED_RECS 256

#ifdef LINUX_MULTIFORMAT
static int convert_acct_record_read (struct acct *rec_in, struct acct *rec_out);
#endif

/* Set up pacct handling routines */

void pacct_init(int backwards)
{
#ifdef LINUX_MULTIFORMAT
  pacct_info = file_reader_init (sizeof (struct acct_v0), BUFFERED_RECS,
#else
  pacct_info = file_reader_init (sizeof (struct acct), BUFFERED_RECS,
#endif
                                 backwards);
}

/* Add a pacct/acct file to the list of files to process */

void add_pacct_file(char *name)
{
  file_reader_add_file (pacct_info, name);
}

/* Do a buffered read of the file and return the next record in REC.
   Return 0 if no more entries. */

struct acct *pacct_get_entry(void)
{
#ifdef LINUX_MULTIFORMAT
  struct acct *entry;
  static struct acct buf;

  /* Convert acct file formats on-the-fly.
     A non-zero return value from convert_acct_record_read() indicates that
     this entry doesn't contain decodable information and is to
     be skipped. */
  do
    {
      entry = (struct acct *) file_reader_get_entry (pacct_info);
      if (entry == (struct acct *) 0)
        return entry;
    }
  while (convert_acct_record_read(entry, &buf) != 0);
  return &buf;
#else
  return (struct acct *) file_reader_get_entry (pacct_info);
#endif
}


void print_pacct_record(struct acct *rec, FILE *out)
{
  time_t btime = (time_t)rec->ac_btime;

  (void)fprintf(out, "%-*.*s|", COMM_LEN, COMM_LEN, rec->ac_comm);

#define NUM_FORMAT "%9.2f"

#ifdef LINUX_MULTIFORMAT
  (void)fprintf(out, "v%1d|", rec->ac_version & 0x7f);
#endif

#ifdef HAVE_ACUTIME
  (void)fprintf(out, NUM_FORMAT "|", ACUTIME_2_DOUBLE(rec->ac_utime));
#endif

#ifdef HAVE_ACSTIME
  (void)fprintf(out, NUM_FORMAT "|", ACSTIME_2_DOUBLE(rec->ac_stime));
#endif

#ifdef HAVE_ACETIME
  (void)fprintf(out, NUM_FORMAT "|", ACETIME_2_DOUBLE(rec->ac_etime));
#endif

  (void)fprintf(out, "%6lu|%6lu|",
                (unsigned long)rec->ac_uid, (unsigned long)rec->ac_gid);

#ifdef HAVE_ACMEM
  (void)fprintf(out, NUM_FORMAT "|", ACMEM_2_DOUBLE(rec->ac_mem));
#endif

#ifdef HAVE_ACIO
  (void)fprintf(out, NUM_FORMAT "|", ACIO_2_DOUBLE(rec->ac_io));
#endif

#ifdef LINUX_MULTIFORMAT
  (void)fprintf(out, "%8d %8d|", rec->ac_pid, rec->ac_ppid);
#endif

  (void)fprintf(out, "%s", ctime (&btime));
}


void pacct_print_file_and_line(FILE *out)
{
  file_reader_print_file_and_line (out, pacct_info);
}


#ifdef HAVE_COMP_T

/* convert a comp_t to a double */

double comp_t_2_double(comp_t c_num)
{
  unsigned long in = 0;		/* the number part */

  /* comp_t are strange numbers -- of 16 bits, the first three are are
     the exponent and the last 13 are the number.  The exp is base 8
     (yuck)! */

  in = c_num & 017777;
  c_num >>= 13;

  while (c_num)
    {
      c_num--;
      in <<= 3;
    }

  /* printf ("%x -> %d %d -> %f\n",
     (int) c_num, (int) ex, (int) in, fl); */

  return (double) in;
}

#endif /* HAVE_COMP_T */

#ifdef LINUX_MULTIFORMAT

/* convert a comp2_t to a double */

#define MANTSIZE2 20                       /* 20 bit mantissa. */
#define MAXFRACT2 ((1ul << MANTSIZE2) - 1) /* Maximum fractional value. */

double comp2_t_2_double(comp2_t val)
{
  unsigned long tmp = val & (MAXFRACT2 >> 1),
                      exp = val >> (MANTSIZE2 - 1),
                            shift = 1;

  if (exp != 0)
    {
      tmp |= 1 << (MANTSIZE2 - 1);
      shift <<= (exp - 1);
    }

  return (double) tmp * shift;
}

/*
 *  encode the integer part of a double into a comp_t
 *
 *  The encoding is a 13-bit fraction with a 3-bit (base 8) exponent.
 */

#define MANTSIZE	13			/* 13 bit mantissa. */
#define EXPSIZE		3			/* base 3 bit exponent. */
#define EXPBASE		3			/* base 8 (3 bit) exponent. */
#define MAXMANT		((1 << MANTSIZE) - 1)	/* maximum mantissa value. */
#define MAXEXP		((1 << EXPSIZE) - 1)	/* maximum exponent value. */
#define MAXCOMP_T	(MAXMANT | (MAXEXP << MANTSIZE))
/* maximum comp_t value */

static comp_t double_2_comp_t(double value)
{
  double mantissa;
  int exponent, exp;
  comp_t mant;

  mantissa = frexp (value, &exponent);
  if (exponent < MANTSIZE)
    {
      /* value fits into mantissa without shifting */
      exp = 0;
    }
  else
    {
      /* (exponent-MANTSIZE) bits dont fit.
                   * convert that into base 8 exponent (rounding up) */
      exp = (exponent - MANTSIZE + (EXPBASE-1)) / EXPBASE;
    }
  mant = (comp_t) (ldexp (mantissa, exponent - exp*EXPBASE) + 0.5);
  if (mant > MAXMANT)
    {
      /* overflow of the mantissa because of rounding */
      mant >>= EXPBASE;
      exp++;
    }
  if (exp > MAXEXP)
    {
      /* overflow. return largest representable number instead */
      return MAXCOMP_T;
    }
  return mant | (exp << MANTSIZE);
}

/*
 * encode the integer part of a double into a comp2_t (24 bits)
 *
 * Format: 5 bit base 2 exponent, 20 bits mantissa.
 * The leading bit of the mantissa is not stored, but implied for
 * non-zero exponents.
 * Largest encodable value is 50 bits.
 */

#define MANTSIZE2	20			/* 20 bit mantissa. */
#define EXPSIZE2	5			/* 5 bit base 2 exponent. */
#define EXPBASE2	1			/* base 2 (2^1) exponent. */
#define MAXMANT2	((1ul << MANTSIZE2) - 1)/* maximum mantissa value. */
#define MAXEXP2		((1ul << EXPSIZE2) - 1)	/* maximum exponent value. */
#define MAXCOMP2_T	(MAXMANT2 | (MAXEXP2 << (MANTSIZE2-1)))
/* maximum comp2_t value */

static comp2_t double_2_comp2_t(double value)
{
  double mantissa;
  int exponent, exp;
  comp2_t mant;

  mantissa = frexp (value, &exponent);
  if (exponent <= MANTSIZE2)
    {
      /* value fits into mantissa without shifting */
      exp = 0;
    }
  else
    {
      /* (exponent-MANTSIZE2) bits dont fit */
      exp = exponent - MANTSIZE2;
    }
  mant = (comp2_t) (ldexp (mantissa, exponent - exp) + 0.5);
  if (mant > MAXMANT2)
    {
      /* overflow of the mantissa because of rounding */
      mant >>= EXPBASE2;
      exp++;
    }
  if (exp >= MAXEXP2)
    {
      /* overflow. return largest representable number instead */
      return MAXCOMP2_T;
    }
  if (mant > (MAXMANT2 / 2))
    {
      /* discard leading 1 */
      mant &= (MAXMANT2 >> 1);
      exp++;
    }
  /* shift exponent into place */
  return (mant | (exp << (MANTSIZE2 - 1)));
}


static signed char permute[76][5] =
{
  /* primary fields: always contain valid data */
  { 0,  0,   0,  0,  0},		/* char		ac_flag; */
  { 1,  1,   1,  1,  1},		/* char		ac_version; */
  { 2,  3,   6,  6,  2},		/* __u16	ac_tty; */
  { 3,  2,   7,  7,  3},
  { 4,  7,  30, 32,  4},		/* __u32	ac_exitcode; */
  { 5,  6,  31, 33,  5},
  { 6,  5,  32, 34,  6},
  { 7,  4,  33, 35,  7},
  { 8, 11,  54, 56,  8},		/* __u32	ac_uid;	*/
  { 9, 10,  55, 57,  9},
  {10,  9,  56, 58, 10},
  {11,  8,  57, 59, 11},
  {12, 15,  58, 60, 12},		/* __u32	ac_gid;	*/
  {13, 14,  59, 61, 13},
  {14, 13,  60, 62, 14},
  {15, 12,  61, 63, 15},
  {16, 19,  -1, -1, 16},		/* __u32	ac_pid;	*/
  {17, 18,  -1, -1, 17},
  {18, 17,  -1, -1, 18},
  {19, 16,  -1, -1, 19},
  {20, 23,  -1, -1, 20},		/* __u32	ac_ppid; */
  {21, 22,  -1, -1, 21},
  {22, 21,  -1, -1, 22},
  {23, 20,  -1, -1, 23},
  {24, 27,   8,  8, 24},		/* __u32	ac_btime; */
  {25, 26,   9,  9, 25},
  {26, 25,  10, 10, 26},
  {27, 24,  11, 11, 27},
  {28, 31,  -1, -1, 28},		/* float	ac_etime; */
  {29, 30,  -1, -1, 29},
  {30, 29,  -1, -1, 30},
  {31, 28,  -1, -1, 31},
  {32, 33,  12, 12, 32},		/* comp_t	ac_utime; */
  {33, 32,  13, 13, 33},
  {34, 35,  14, 14, 34},		/* comp_t	ac_stime; */
  {35, 34,  15, 15, 35},
  {36, 37,  18, 18, 36},		/* comp_t	ac_mem;	*/
  {37, 36,  19, 19, 37},
  {38, 39,  20, 20, 38},		/* comp_t	ac_io; */
  {39, 38,  21, 21, 39},
  {40, 41,  22, 22, 40},		/* comp_t	ac_rw; */
  {41, 40,  23, 23, 41},
  {42, 43,  24, 24, 42},		/* comp_t	ac_minflt; */
  {43, 42,  25, 25, 43},
  {44, 45,  26, 26, 44},		/* comp_t	ac_majflt; */
  {45, 44,  27, 27, 45},
  {46, 47,  28, 28, 46},		/* comp_t	ac_swaps; */
  {47, 46,  29, 29, 47},
  {48, 48,  34, 36, 48},		/* char		ac_comm[ACCT_COMM]; */
  {49, 49,  35, 37, 49},
  {50, 50,  36, 38, 50},
  {51, 51,  37, 39, 51},
  {52, 52,  38, 40, 52},
  {53, 53,  39, 41, 53},
  {54, 54,  40, 42, 54},
  {55, 55,  41, 43, 55},
  {56, 56,  42, 44, 56},
  {57, 57,  43, 45, 57},
  {58, 58,  44, 46, 58},
  {59, 59,  45, 47, 59},
  {60, 60,  46, 48, 60},
  {61, 61,  47, 49, 61},
  {62, 62,  48, 50, 62},
  {63, 63,  49, 51, 63},
  {64, 65,  -1, 30, -1},		/* __u16	ac_ahz; */
  {65, 64,  -1, 31, -1},
  /* secondary fields: may contain derived or invalid data! */
  {66, 67,   2,  2, -1},		/* __u16	ac_uid16; */
  {67, 66,   3,  3, -1},
  {68, 69,   4,  4, -1},		/* __u16	ac_gid16; */
  {69, 68,   5,  5, -1},
  {70, 71,  16, 16, -1},		/* __u16	ac_etime_16; */
  {71, 70,  17, 17, -1},
  {72, 73,  52, 54, -1},		/* __u16	ac_etime_lo; */
  {73, 72,  53, 55, -1},
  {74, 74,  51, 53, -1},		/* __u8		ac_etime_hi; */
  {-1, -1,  -1, -1, -1}
};	/* __u8		ac_pad; */

/* convert various formats to internal format */

static int convert_acct_record_read(struct acct *rec_in, struct acct *rec_out)
{
  int byteswap = 0;
  int perm;
  unsigned int i;
  int j;

  if (rec_in->ac_version != 0
      && ((rec_in->ac_version ^ BYTEORDER) & 0x80) !=0)
    byteswap = 1;

  switch (rec_in->ac_version & 0x7f)
    {
    case 0:	/* old 2.0-2.4 format */
      /* TODO: support old M68K acct files */
      perm = 3;
      break;
    case 1:	/* extended M68K binary compatible format */
      perm = 2;
      break;
    case 2:	/* new binary compatible format */
      perm = 3;
      break;
    case 3:	/* new incompatible 64 byte format */
    case 4:	/* new incompatible 128 byte format, first half */
      perm = 4;
      break;
    case 5:	/* new incompatible 128 byte format, second half */
      return 1;
    default:
      fprintf(stderr, "ERROR: unknown acct file format\n");
      return 2;
    }

  for (i=0; i < sizeof (struct acct); i++)
    {
      j = permute[i][byteswap];
      j = permute[j][perm];
      if (j>=0)
        {
          ((char *) rec_out)[i] = ((char *) rec_in)[j];
        }
      else
        {
          ((char *) rec_out)[i] = 0;
        }
    }

  if (rec_out->ac_ahz == 0)
    {
      if ((rec_in->ac_version & 0x7f)==3)
        /* v3 file format always uses AHZ=100, non-overridable */
        rec_out->ac_ahz = AHZ;
      else
        rec_out->ac_ahz = ahz;
    }

  switch (rec_out->ac_version & 0x7f)
    {
    case 0:
      if (rec_out->ac_uid16 != (__u16) rec_out->ac_uid
          || rec_out->ac_gid16 != (__u16) rec_out->ac_gid)
        {
          /* 32 bit fields incorrect */
          rec_out->ac_uid = rec_out->ac_uid16;
          rec_out->ac_gid = rec_out->ac_gid16;
        }
      /* no break here */
    case 1:
      rec_out->ac_etime = (float) comp_t_2_double (rec_out->ac_etime_16);
      break;
    case 2:
      rec_out->ac_etime = (float) comp2_t_2_double (rec_out->ac_etime_lo
                          | rec_out->ac_etime_hi << 16);
      break;
    }

  return 0;
}


/* convert internal format to the format denoted by "version" and "byteswap"
   BEWARE! *rec_in might be changed according to output format! */

size_t convert_acct_record_write(struct acct *rec_in, struct acct *rec_out,
                                 int version, int byteswap)
{
  int perm;
  static int warned = 0;
  unsigned int i;
  int j;
  size_t size = 64;
  comp2_t etime;
  double factor;

  if (version < 0)
    version = rec_in->ac_version & 0x7f;

  switch (version)
    {
    case 0:	/* old 2.0-2.4 format */
      /* same as v2 except of M68k */
      /* TODO: support output of old M68K acct files */
      perm = 3;
      break;
    case 1:	/* extended M68K binary compatible format */
      perm = 2;
      size = 62;
      break;
    case 2:	/* new binary compatible format */
      perm = 3;
      break;
    case 3:	/* new incompatible 64 byte format */
      perm = 4;
      break;
    case 4:	/* new incompatible 128 byte format, first half */
      if (!warned)
        {
          fprintf (stderr, "WARNING: v4 file format not supported for writing. "
                   "Using v3 file format instead.\n");
          warned = 1;
        }
      perm = 4;
      version = 3;
      break;
    case 5:	/* new incompatible 128 byte format, second half */
    default:
      if (!warned)
        {
          fprintf (stderr, "WARNING: unknown file format requested. "
                   "Using v2 file format instead.\n");
          warned = 1;
        }
      perm = 3;
      version = 2;
    }

  if (version == 0)
    {
      /* Check whether uid/gid fit into 16 bits */
      if ((rec_in->ac_uid & 0xffff0000) != 0)
        fprintf (stderr, "WARNING: uid %lu truncated to %u\n",
                 (unsigned long) (rec_in->ac_uid),
                 (unsigned int) (rec_in->ac_uid & 0xffff));
      if ((rec_in->ac_gid & 0xffff0000) != 0)
        fprintf (stderr, "WARNING: gid %lu truncated to %u\n",
                 (unsigned long) (rec_in->ac_gid),
                 (unsigned int) (rec_in->ac_gid & 0xffff));
    }

  if (((version==1 || version==3)  && rec_in->ac_ahz != AHZ)
      || ((version==0 || version==2) && rec_in->ac_ahz != ahz))
    {
      /* these formats don't store ac_ahz but assume AHZ==100,
         so convert times to that unit.
         for versions 0 and 2 AHZ==100 may be overridden */
      if (version==0 || version==2)
        factor = ahz / (double) rec_in->ac_ahz;
      else
        factor = AHZ / (double) rec_in->ac_ahz;
      rec_in->ac_utime = double_2_comp_t (factor * comp_t_2_double (rec_in->ac_utime));
      rec_in->ac_stime = double_2_comp_t (factor * comp_t_2_double (rec_in->ac_stime));
      rec_in->ac_etime *= factor;
      rec_in->ac_ahz = AHZ;
    }

  if (version==0 || version==1 || version==2)
    {
      /* these store etime as comp_t or comp2_t */
      rec_in->ac_etime_16 = double_2_comp_t (rec_in->ac_etime);
      etime = double_2_comp2_t (rec_in->ac_etime);
      rec_in->ac_etime_hi = etime >> 16;
      rec_in->ac_etime_lo = (__u16) etime;
      rec_in->ac_uid16 = (__u16) rec_in->ac_uid;
      rec_in->ac_gid16 = (__u16) rec_in->ac_gid;
    }

  /* mset (rec_out, 0, sizeof (struct acct_v0)) */
  for (i=0; i < size; i++)
    ((char *)rec_out)[i] = 0;

  for (i=0; i < sizeof (struct acct); i++)
    {
      j = permute[i][byteswap];
      j = permute[j][perm];
      if (j >= 0)
        ((char *) rec_out)[j] = ((char *) rec_in)[i];
    }

  if (byteswap != 0)
    byteswap = 0x80;

  rec_out->ac_version = version | (BYTEORDER ^ byteswap);

  return size;
}
#endif /* LINUX_MULTIFORMAT */
