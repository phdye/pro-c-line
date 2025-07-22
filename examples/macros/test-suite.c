
/* Result Sets Interface */
#ifndef SQL_CRSR
#  define SQL_CRSR
  struct sql_cursor
  {
    unsigned int curocn;
    void *ptr1;
    void *ptr2;
    unsigned int magic;
  };
  typedef struct sql_cursor sql_cursor;
  typedef struct sql_cursor SQL_CURSOR;
#endif /* SQL_CRSR */

/* Thread Safety */
typedef void * sql_context;
typedef void * SQL_CONTEXT;

/* Object support */
struct sqltvn
{
  unsigned char *tvnvsn; 
  unsigned short tvnvsnl; 
  unsigned char *tvnnm;
  unsigned short tvnnml; 
  unsigned char *tvnsnm;
  unsigned short tvnsnml;
};
typedef struct sqltvn sqltvn;

struct sqladts
{
  unsigned int adtvsn; 
  unsigned short adtmode; 
  unsigned short adtnum;  
  sqltvn adttvn[1];       
};
typedef struct sqladts sqladts;

static struct sqladts sqladt = {
  1,1,0,
};

/* Binding to PL/SQL Records */
struct sqltdss
{
  unsigned int tdsvsn; 
  unsigned short tdsnum; 
  unsigned char *tdsval[1]; 
};
typedef struct sqltdss sqltdss;
static struct sqltdss sqltds =
{
  1,
  0,
};

/* File name & Package Name */
struct sqlcxp
{
  unsigned short fillen;
           char  filnam[14];
};
static const struct sqlcxp sqlfpn =
{
    13,
    "test-suite.pc"
};


static unsigned int sqlctx = 645603;


static struct sqlexd {
   unsigned int   sqlvsn;
   unsigned int   arrsiz;
   unsigned int   iters;
   unsigned int   offset;
   unsigned short selerr;
   unsigned short sqlety;
   unsigned int   occurs;
      const short *cud;
   unsigned char  *sqlest;
      const char  *stmt;
   sqladts *sqladtp;
   sqltdss *sqltdsp;
            void  **sqphsv;
   unsigned int   *sqphsl;
            int   *sqphss;
            void  **sqpind;
            int   *sqpins;
   unsigned int   *sqparm;
   unsigned int   **sqparc;
   unsigned short  *sqpadto;
   unsigned short  *sqptdso;
   unsigned int   sqlcmax;
   unsigned int   sqlcmin;
   unsigned int   sqlcincr;
   unsigned int   sqlctimeout;
   unsigned int   sqlcnowait;
              int   sqfoff;
   unsigned int   sqcmod;
   unsigned int   sqfmod;
   unsigned int   sqlpfmem;
            void  *sqhstv[1];
   unsigned int   sqhstl[1];
            int   sqhsts[1];
            void  *sqindv[1];
            int   sqinds[1];
   unsigned int   sqharm[1];
   unsigned int   *sqharc[1];
   unsigned short  sqadto[1];
   unsigned short  sqtdso[1];
} sqlstm = {13,1};

/* SQLLIB Prototypes */
extern void sqlcxt (void **, unsigned int *,
                    struct sqlexd *, const struct sqlcxp *);
extern void sqlcx2t(void **, unsigned int *,
                    struct sqlexd *, const struct sqlcxp *);
extern void sqlbuft(void **, char *);
extern void sqlgs2t(void **, char *);
extern void sqlorat(void **, unsigned int *, void *);

/* Forms Interface */
static const int IAPSUCC = 0;
static const int IAPFAIL = 1403;
static const int IAPFTL  = 535;
extern void sqliem(unsigned char *, signed int *);

typedef struct { unsigned short len; unsigned char arr[1]; } VARCHAR;
typedef struct { unsigned short len; unsigned char arr[1]; } varchar;

/* cud (compilation unit data) array */
static const short sqlcud0[] =
{13,4130,178,0,0,
};


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sqlca.h>
#include "varchar-macros.h"

#define VERSION "varchar-macros-test 0.1"

/* EXEC SQL BEGIN DECLARE SECTION; */ 

/* VARCHAR v1[50]; */ 
struct { unsigned short len; unsigned char arr[50]; } v1;

/* VARCHAR v2[50]; */ 
struct { unsigned short len; unsigned char arr[50]; } v2;

/* VARCHAR temp[50]; */ 
struct { unsigned short len; unsigned char arr[50]; } temp;

char cbuf[100];
/* EXEC SQL END DECLARE SECTION; */ 


int failures = 0;
int verbose = 0;

// Pytest-style assert with verbosity
#define CHECK(name, expr) do { \
    if (!(expr)) { \
        printf("\nFAIL: %s\n", name); \
        failures++; \
    } else if (verbose) { \
        printf("PASS: %s\n", name); \
    } else { \
        fputc('.', stdout); \
        fflush(stdout); \
    } \
} while (0)

void test_vset_and_vtoc() {
    VSET(v1, "hello");
    VTOC(v1, cbuf);
    CHECK("VSET+VTOC round-trip", strcmp(cbuf, "hello") == 0);
    CHECK("VSET sets correct length", v1.len == 5);
}

void test_vappend() {
    VSET(v1, "foo");
    VSET(v2, "bar");
    VAPPEND(v1, v2);
    VTOC(v1, cbuf);
    CHECK("VAPPEND concatenates correctly", strcmp(cbuf, "foobar") == 0);
}

void test_vcmp() {
    VSET(v1, "abc");
    VSET(v2, "abc");
    CHECK("VCMP equal", VCMP(v1, v2) == 0);

    VSET(v2, "abd");
    CHECK("VCMP less-than", VCMP(v1, v2) < 0);

    VSET(v2, "aaa");
    CHECK("VCMP greater-than", VCMP(v1, v2) > 0);
}

void test_case_mods() {
    VSET(v1, "AbC");
    VUPPER(v1);
    VTOC(v1, cbuf);
    CHECK("VUPPER", strcmp(cbuf, "ABC") == 0);

    VLOWER(v1);
    VTOC(v1, cbuf);
    CHECK("VLOWER", strcmp(cbuf, "abc") == 0);
}

void test_trim() {
    VSET(v1, "   spaced  ");
    VTRIM(v1);
    VTOC(v1, cbuf);
    CHECK("VTRIM trims whitespace", strcmp(cbuf, "spaced") == 0);
}

void test_fill() {
    VFILL(v1, 'X', 5);
    VTOC(v1, cbuf);
    CHECK("VFILL sets to X", strncmp(cbuf, "XXXXX", 5) == 0);
    CHECK("VFILL length", v1.len == 5);
}

void test_copy_zero() {
    VSET(v1, "CopyMe");
    VCOPY(v2, v1);
    VTOC(v2, cbuf);
    CHECK("VCOPY round-trip", strcmp(cbuf, "CopyMe") == 0);

    VZERO(v2);
    int cleared = 1;
    for (size_t i = 0; i < sizeof(v2.arr); i++) if (v2.arr[i] != 0) cleared = 0;
    CHECK("VZERO clears array", cleared);
}

void test_clr() {
    VSET(v1, "erase");
    VCLR(v1);
    CHECK("VCLR sets len=0", v1.len == 0);
    int cleared = 1;
    for (size_t i = 0; i < sizeof(v1.arr); i++) if (v1.arr[i] != 0) cleared = 0;
    CHECK("VCLR clears arr", cleared);
}

void print_usage(const char *prog) {
    printf("Usage:\n");
    printf("  %s [--verbose | -v] [--help] [--version]\n\n", prog);
    printf("Options:\n");
    printf("  -v --verbose     Show individual test passes\n");
    printf("  --help           Show this screen\n");
    printf("  --version        Show version\n");
}

int main(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--help")) {
            print_usage(argv[0]);
            return 0;
        } else if (!strcmp(argv[i], "--version")) {
            printf("%s\n", VERSION);
            return 0;
        } else if (!strcmp(argv[i], "--verbose") || !strcmp(argv[i], "-v")) {
            verbose = 1;
        }
    }

    if (verbose)
        printf("Running varchar macro test suite...\n");

    test_vset_and_vtoc();
    test_vappend();
    test_vcmp();
    test_case_mods();
    test_trim();
    test_fill();
    test_copy_zero();
    test_clr();

    if (failures == 0) {
        printf(verbose ? "\nAll tests passed.\n" : ".");
    } else {
        printf("\n%d test(s) failed.\n", failures);
    }

    return failures;
}
