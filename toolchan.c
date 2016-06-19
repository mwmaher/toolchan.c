/* ToolChan.C

   By Michael W. Maher


   Ver 1.0 06-14-90
       2.0 12-17-92  Now supports all printing devices with straight 80
		     column reports and no special printer controls.

   This program was written to provide the a Machining department with
   the ability to track and organize tool changes on their machines.

   A report indicates which machines and tools need to be changed based
   on tool change frequency and a counter range projection.
*/

/****************************** INCLUDE FILES ******************************/
#include<bios.h>
#include<conio.h>
#include<ctype.h>
#include<float.h>
#include<graph.h>
#include<io.h>
#include<math.h>
#include<search.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>

/*************** include files written by Michael W. Maher *****************/
#include<enhdkeys.h>                   /* IBM-PC enhanced keyboard codes */
#include"c:\c\pdbase\getvdstr.h"       /* get video string header file */
#include"c:\c\pdbase\getvdstr.lib"     /* get video string routines */

/********************************* DEFINE **********************************/
#define TRUE 1
#define FALSE 0
#define MAXASIZE 800
#define MAXLINES 25

#define BLACK 0L                       /* Long value for colors */
#define WHITE 7L
#define BLUE 1L
#define CYAN 3L
#define RED 4L
#define YELLOW 14L
#define GREEN 2L
#define BRIGHTWHITE 15

/***************************** TYPE DECLARATIONS ***************************/
typedef struct filerec{
		char toolnumb[26];     /* Tool Part# */
		char tooldesc[31];     /* Tool description */
		long freq;             /* Frequency of replacement */
		long beginning;        /* Beginning count */
		}filerec;

typedef struct indexrec{
		char linenum[16];      /* line number users label */
		char filename[11];     /* the filename used for the line */
		}indexrec;

typedef struct data_rec{
		int indexnum;          /* number of lines */
		int currentindex;      /* currently selected index     */
		int toolnum;           /* number of tools in line file */
		int currenttool;       /* currently selected tool      */
		int x, y;              /* Menu screen coordinates      */
		int locx, locy;        /* Menu item screen coordinates */
		FILE *output;
		} data_rec;

char *filename[MAXLINES] = {"Line01.Dat", "Line02.Dat", "Line03.Dat",
			    "Line04.Dat", "Line05.Dat", "Line06.Dat",
			    "Line07.Dat", "Line08.Dat", "Line09.Dat",
			    "Line10.Dat", "Line11.Dat", "Line12.Dat",
			    "Line13.Dat", "Line14.Dat", "Line15.Dat",
			    "Line16.Dat", "Line17.Dat", "Line18.Dat",
			    "Line19.Dat", "Line20.Dat", "Line21.Dat",
			    "Line22.Dat", "Line23.Dat", "Line24.Dat",
			    "Line25.Dat"};


int  main(int argc, char **argv);
void do_about(long seconds);
void loadindex(indexrec *lineindex, data_rec *mdata);
void DoMainMenu(filerec *data,
		filerec **dataptrs,
		indexrec *lineindex,
		data_rec *mdata);
void mainmenu_screen(void);
int  comparetool(const void *elem1, const void *elem2);
void closeindexfile(indexrec *lineindex, data_rec *mdata);
void EnterEdit(filerec *data,
	       filerec **dataptrs,
	       indexrec *lineindex,
	       data_rec *mdata);
int  selectline(indexrec *lineindex, data_rec *mdata);
unsigned getline(indexrec *lineindex, data_rec *mdata);
void addline(indexrec *lineindex, data_rec *mdata);
void deleteline(indexrec *linerec, data_rec *mdata);
void indextolocs(int *x, int *y, int index);
void loadnextfilename(indexrec *lineindex, data_rec *mdata);
void intindexmenu(indexrec *lineindex, data_rec *mdata);
void printrecord(filerec *record, char *linenum);
void updatescreen(unsigned key, indexrec *lineindex, data_rec *mdata);
void highlight(char *textline, int barlength);
void closetoolfile(char *currentfile,
		   filerec **dataptrs,
		   data_rec *mdata);
void loadlinetools(char *filename,
		   filerec *data,
		   filerec **dataptrs,
		   data_rec *mdata);
void editlinedataitem(filerec *data,
		      filerec **dataptrs,
		      char *linenum,
		      data_rec *mdata);
void updateeditscreen(unsigned key,
		      filerec *data,
		      filerec **dataptrs,
		      char *linenum,
		      data_rec *mdata);
void toolcounttoscreen(int count);
void tooltoscreen(filerec *tool, char *linenum);
int  edittool(filerec *tool);
void toolchanreport(indexrec *lineindex,
		    filerec *data,
		    filerec **dataptrs,
		    data_rec *mdata);
void toolreportheader(char *linenum,
		      int *page,
		      int *lines,
		      long *begin,
		      long *end,
		      data_rec *mdata);
void toolreport(char *linenum,
		filerec *data,
		filerec **dataptrs,
		data_rec *mdata,
		long *begin, long *end);
unsigned getbeginend(long *begin, long *end);
void datafilereport(indexrec *lineindex,
		    filerec *data,
		    filerec **dataptrs,
		    data_rec *mdata);
void datareport(char *linenum,
		filerec *data,
		filerec **dataptrs,
		data_rec *mdata);
void datareportheader(char *linenum,
		      int *page,
		      int *lines,
		      data_rec *mdata);

int main(int argc, char **argv)
  {
  short    start_mode;
  data_rec mdata;
  filerec  *data,
	   **dataptrs;
  indexrec *lineindex;

  /* allocate data */
  if (((data = (filerec*) calloc(sizeof(filerec), MAXASIZE)) == NULL) ||
     ((dataptrs = (filerec**) calloc(sizeof(filerec*), MAXASIZE)) == NULL) ||
     ((lineindex = (indexrec*) calloc(sizeof(indexrec), MAXLINES)) == NULL))
    {
    printf("ERROR: Memory allocation error.\n");
    getch();
    }
  if ((argc == 2) && (strncmp(argv[1], "/H", 2) != 0))
    mdata.output = fopen(argv[1], "wa");
  else
    mdata.output = stdprn;
  start_mode = _setvideomode(_DEFAULTMODE);
  do_about(3L);
  loadindex(lineindex, &mdata);
  DoMainMenu(data, dataptrs, lineindex, &mdata);
  closeindexfile(lineindex, &mdata);
  free(data);                          /* release allocated memory */
  free(dataptrs);
  free(lineindex);
  do_about(1L);
  _setvideomode(start_mode);           /* reset mode */
  return(0);
  }


void do_about(long seconds)
  {
  time_t start,
	 stop;

  _settextwindow(1, 1, 25, 80);
  _setbkcolor(WHITE);
  _settextcolor(BLACK);
  _clearscreen(_GWINDOW);
  _settextcolor(BLACK);
  _settextposition(2, 19);
  _outtext("*---------TOOL REPLACEMENT PROGRAM---------*");
  _settextposition(10, 25);
  _outtext("   The Metalloy Corporation");
  _settextposition(11, 25);
  _outtext("    Version 2.0: 12/17/92");
  _settextposition(12, 25);
  _outtext("     By Michael W. Maher ");
  time(&start);
  while (time(&stop) <  (start + seconds));
  return;
  }


void loadindex(indexrec *lineindex, data_rec *mdata)
  {
  int  j = 0;
  FILE *indexptr;

  if ((indexptr = fopen("LinIndex.Dat", "r+b")) != NULL)
    {
    while ((fread(&lineindex[j], sizeof(indexrec), 1, indexptr) == 1) &&
	   (j < MAXLINES))
      {
      j++;
      }
    mdata->indexnum     = j;
    mdata->currentindex = 0;
    fclose(indexptr);
    }
  else if ((indexptr = fopen("LinIndex.Dat", "w+b")) != NULL)
    {
    mdata->indexnum     = 0;
    mdata->currentindex = 0;
    fclose(indexptr);
    }
  else
    {
    printf("ERROR: LinIndex.Dat not found could not be created.\n"
	   "Any key to continue...\n");
    getch();
    }
  return;
  }


void DoMainMenu(filerec *data, filerec **dataptrs,
		indexrec *lineindex, data_rec *mdata)
  {
  unsigned key;

  mainmenu_screen();
  do
    {
    key = _bios_keybrd(_KEYBRD_READ);
    switch (key)
      {
      case F1: EnterEdit(data, dataptrs, lineindex, mdata);
	       mainmenu_screen();
	       break;
      case F2: datafilereport(lineindex, data, dataptrs, mdata);
	       if (fflush(mdata->output) != 0)
		 {
		 printf("ERROR: Can not flush output stream.");
		 getch();
		 }
	       mainmenu_screen();
	       break;
      case F3: toolchanreport(lineindex, data, dataptrs, mdata);
	       mainmenu_screen();
	       break;
      default: break;
      }
    }
  while (key != ESCAPE);
  return;
  }


void closeindexfile(indexrec *lineindex, data_rec *mdata)
  {
  FILE *tempindexptr;
  int  i;

  if ((tempindexptr = fopen("TempIndx.Dat", "wb")) != NULL)
    {
    for (i = 0; i < mdata->indexnum; i++)
      fwrite(&lineindex[i], sizeof(indexrec), 1, tempindexptr);
    fclose(tempindexptr);
    remove("LinIndex.Dat");
    rename("TempIndx.Dat", "LinIndex.Dat");
    }
  else
    {
    printf("ERROR: Can not open TempIndx.Dat.\n");
    getch();
    }
  return;
  }


void closetoolfile(char *currentfile,
		   filerec **dataptrs,
		   data_rec *mdata)
  {
  FILE *temptoolptr;
  int  i;

  qsort(dataptrs,
	mdata->toolnum,
	sizeof(filerec*),
	comparetool);
  if ((temptoolptr = fopen("Temptool.Dat", "wb")) != NULL)
    {
    for (i = 0; i < mdata->toolnum; i++)
      {
      if ((strlen(dataptrs[i]->toolnumb) > 0) &&
	  (dataptrs[i]->toolnumb[0] != ' '))
	fwrite(dataptrs[i], sizeof(filerec), 1, temptoolptr);
      }
    fclose(temptoolptr);
    remove(currentfile);
    rename("Temptool.Dat", currentfile);
    }
  else
    {
    printf("ERROR: Can not open Temptool.Dat.\n");
    getch();
    }
  return;
  }


void mainmenu_screen(void)
  {
  unsigned x = 26,
	   y = 10;

  _settextwindow(y, x, y + 8, x + 29);
  _setbkcolor(BLUE);
  _settextcolor(BRIGHTWHITE);
  _outtext("******************************");
  _outtext("* --------MAIN MENU--------- *");
  _outtext("*                            *");
  _outtext("*    F1 - Enter/Edit         *");
  _outtext("*    F2 - Data File Report   *");
  _outtext("*    F3 - Tool Change Report *");
  _outtext("* <Esc> - Exit To DOS        *");
  _outtext("******************************");
  return;
  }


void EnterEdit(filerec *data,
	       filerec **dataptrs,
	       indexrec *lineindex,
	       data_rec *mdata)
  {
  int escflag;

  do
    {
    escflag = selectline(lineindex, mdata);
    if (escflag != -1)
      {
      loadlinetools(lineindex[mdata->currentindex].filename,
		    data, dataptrs, mdata);
      editlinedataitem(data, dataptrs,
		       lineindex[mdata->currentindex].linenum, mdata);
      closetoolfile(lineindex[mdata->currentindex].filename,
		    dataptrs,
		    mdata);
      }
    }
  while (escflag != -1);
  return;
  }


void loadlinetools(char *filename,
		   filerec *data,
		   filerec **dataptrs,
		   data_rec *mdata)
  {
  FILE *linefileptr;
  int  j = 0;

  if ((linefileptr = fopen(filename, "r+b")) != NULL)
    {
    while((j < MAXASIZE) &&
	  (fread(&data[j], sizeof(filerec), 1, linefileptr) == 1))
      {
      dataptrs[j] = &data[j];
      j++;
      }
    mdata->currenttool = 0;
    mdata->toolnum     = j;
    fclose(linefileptr);
    }
  else if ((linefileptr = fopen(filename, "w+b")) != NULL)
    {
    mdata->toolnum      = 0;
    mdata->currenttool  = 0;
    memset(data[mdata->toolnum].toolnumb, '\0', 26);
    strcpy(data[mdata->toolnum].toolnumb, "?");
    memset(data[mdata->toolnum].tooldesc, '\0', 31);
    data[mdata->toolnum].freq      = 0L;
    data[mdata->toolnum].beginning = 0L;
    dataptrs[mdata->toolnum]       = &data[mdata->toolnum];
    fclose(linefileptr);
    }
  else
    {
    _outtext("ERROR:[");
    _outtext(filename);
    _outtext("] not found; can not create.\n   ANY KEY TO CONTINUE.");
    getch();
    }
  return;
  }


void editlinedataitem(filerec *data, filerec **dataptrs,
		      char *linenum, data_rec *mdata)
  {
  int      winx = 72,
	   winy = 17;
  unsigned key = 0;

  mdata->x = 7; mdata->y = 6;
  _settextwindow(mdata->y, mdata->x, mdata->y + winy, mdata->x + winx);
  _setbkcolor(BLUE);
  _settextcolor(BRIGHTWHITE);          /* set text in window */
  _clearscreen(_GWINDOW);              /* clear text window */
  _outtext(" *----Enter/Edit Tool Data----* \n");
  _settextposition(5,4);
  _outtext("Current Line[               ]");
  _settextcolor(WHITE);
  _settextposition(5,17);
  _outtext(linenum);
  _settextcolor(BRIGHTWHITE);
  _settextposition(7,4);
  _outtext("Tool Number [                         ]");
  _settextposition(9,4);
  _outtext("Description [                              ]");
  _settextposition(11,4);
  _outtext("Frequency   [         ]   0 - 999999999");
  _settextposition(13,4);
  _outtext("Beginning   [         ]   0 - 999999999");
  _settextposition(mdata->y + winy, 0);
  _outtext(" F1-Edit   F2-Add   HOME/END   F10-Delete   PgUp/PgDn-Next   <ESC>-Back");
  toolcounttoscreen(mdata->currenttool + 1);
  tooltoscreen(&data[mdata->currenttool], linenum);
  do
    {
    updateeditscreen(key, data, dataptrs, linenum, mdata);
    key = _bios_keybrd(_KEYBRD_READ);
    }
  while (key != ESCAPE);
  _setbkcolor(WHITE);
  _clearscreen(_GWINDOW);
  return;
  }


void updateeditscreen(unsigned key,
		      filerec *data,
		      filerec **dataptrs,
		      char *linenum,
		      data_rec *mdata)
  {
  filerec tempdata;

  switch (key)
    {
    /* Edit */
    case  F1: tempdata = data[mdata->currenttool];
	      if (edittool(&tempdata) == TRUE)
		memmove(&data[mdata->currenttool], &tempdata, sizeof(filerec));
	      dataptrs[mdata->currenttool] = &data[mdata->currenttool];
	      tooltoscreen(&data[mdata->currenttool], linenum);
	      toolcounttoscreen(mdata->currenttool + 1);
	      break;
    /* Add */
    case  F2: memset(data[mdata->toolnum].toolnumb, '\0', 26);
	      strcpy(data[mdata->toolnum].toolnumb, "?");
	      memset(data[mdata->toolnum].tooldesc, '\0', 31);
	      data[mdata->toolnum].freq      = 0L;
	      data[mdata->toolnum].beginning = 0L;
	      tooltoscreen(&data[mdata->toolnum], linenum);
	      if (edittool(&data[mdata->toolnum]) == TRUE)
		{
		dataptrs[mdata->toolnum] = &data[mdata->toolnum];
		mdata->currenttool       = mdata->toolnum;
		mdata->toolnum += 1;
		}
	      tooltoscreen(&data[mdata->currenttool], linenum);
	      toolcounttoscreen(mdata->currenttool + 1);
	      break;
    /* search */
    case F3:  break;
    /* Delete */
    case F10: if (mdata->toolnum > 0)
		{
		memmove(&data[mdata->currenttool],
			&data[mdata->toolnum],
			sizeof(filerec));
		if (mdata->toolnum == mdata->currenttool)
		  mdata->currenttool -=1;
		mdata->toolnum -= 1;
		}
	      else
		{
		mdata->toolnum     = 0;
		mdata->currenttool = 0;
		memset(data[mdata->toolnum].toolnumb, '\0', 26);
		memset(data[mdata->toolnum].tooldesc, '\0', 31);
		data[mdata->toolnum].freq      = 0L;
		data[mdata->toolnum].beginning = 0L;
		}
	      tooltoscreen(&data[mdata->currenttool], linenum);
	      toolcounttoscreen(mdata->currenttool + 1);
	      break;
    case PG_UP:
    case U_ARROW:
	      mdata->currenttool -= 1;
	      if (mdata->currenttool < 0)
		mdata->currenttool = mdata->toolnum - 1;
	      tooltoscreen(&data[mdata->currenttool], linenum);
	      toolcounttoscreen(mdata->currenttool + 1);
	      break;
    case PG_DOWN:
    case D_ARROW:
	      mdata->currenttool += 1;
	      if (mdata->currenttool >= mdata->toolnum)
		mdata->currenttool = 0;
	      tooltoscreen(&data[mdata->currenttool], linenum);
	      toolcounttoscreen(mdata->currenttool + 1);
	      break;
    case HOME:mdata->currenttool = 0;
	      tooltoscreen(&data[mdata->currenttool], linenum);
	      toolcounttoscreen(mdata->currenttool + 1);
	      break;
    case END: mdata->currenttool = mdata->toolnum;
	      tooltoscreen(&data[mdata->currenttool], linenum);
	      toolcounttoscreen(mdata->currenttool + 1);
	      break;
    default:  break;
    }
  return;
  }


int edittool(filerec *tool)
  {
  char freq[10],
       beginning[10];
  int  okflag = TRUE;

  memset(beginning, '\0', 10);
  memset(freq, '\0', 10);
  ltoa(tool->freq, freq, 10);
  ltoa(tool->beginning, beginning, 10);
  _settextcolor(YELLOW);
  if (getvideostring(tool->toolnumb,
		     17, 7, 17, 25,
		     keyinexitlist, keyinalphanumnospacelist) == ESCAPE)
    okflag = FALSE;
  if ((okflag == FALSE) ||
      (getvideostring(tool->tooldesc,
		      17, 9, 17, 30,
		      keyinexitlist, keyinalphanumlist) == ESCAPE))
    okflag = FALSE;
  if ((okflag == FALSE) ||
      (getvideostring(freq,
		      17, 11, 17, 9,
		      keyinexitlist, keyinnumlist) == ESCAPE))
    okflag = FALSE;
  if ((okflag == FALSE) ||
      (getvideostring(beginning,
		      17, 13, 17, 9,
		      keyinexitlist, keyinnumlist) == ESCAPE))
    okflag = FALSE;
  if (okflag == TRUE)
    {
    tool->freq      = atol(freq);
    tool->beginning = atol(beginning);
    }
  return(okflag);
  }


void toolcounttoscreen(int count)
  {
  char countstr[8];

  itoa(count, countstr, 10);
  _settextposition(2, 50);
  _outtext("Record #");
  _outtext(countstr);
  _outtext("  ");
  return;
  }


void tooltoscreen(filerec *tool, char *linenum)
  {
  char freq[10],
       beginning[10];

  _settextcolor(YELLOW);
  _settextposition(5, 17);  _outtext("               ");
  _settextposition(5, 17);
  _outtext(linenum);
  _settextposition(7, 17);  _outtext("                         ");
  _settextposition(7, 17);
  _outtext(tool->toolnumb);
  _settextposition(9, 17);  _outtext("                              ");
  _settextposition(9, 17);
  _outtext(tool->tooldesc);
  _settextposition(11, 17); _outtext("         ");
  _settextposition(11, 17);
  ltoa(tool->freq, freq, 10);
  _outtext(freq);
  _settextposition(13, 17); _outtext("         ");
  _settextposition(13, 17);
  ltoa(tool->beginning, beginning, 10);
  _outtext(beginning);
  _settextcolor(BRIGHTWHITE);
  return;
  }


int selectline(indexrec *lineindex, data_rec *mdata)
  {
  int i,
      winx = 60,
      winy = 16;
  unsigned key = 0;

  mdata->x = 5; mdata->y = 4;
  _settextwindow(mdata->y, mdata->x, mdata->y + winy, mdata->x + winx);
  _setbkcolor(BLUE);
  _settextcolor(BRIGHTWHITE);          /* set text in window */
  _clearscreen(_GWINDOW);              /* clear text window */
  _outtext(" *----Select Line----* \n");
  _settextposition(mdata->y + winy, 0);
  _outtext("  <ENTER>-Select   F2-Add   F10-Delete   <ESC>-Main Menu");
  intindexmenu(lineindex, mdata);
  do
    {
    key = _bios_keybrd(_KEYBRD_READ);
    updatescreen(key, lineindex, mdata);
    }
  while ((key != ESCAPE) && (key != ENTER));
  _setbkcolor(WHITE);
  _clearscreen(_GWINDOW);             /* Erase Delete Requester Window */
  if (key == ESCAPE)
    return(-1);
  else
    return(mdata->currentindex);
  }


void updatescreen(unsigned key, indexrec *lineindex, data_rec *mdata)
  {
  int leftcol  = 3,
      rightcol = 25,
      toprow   = 3,
      botrow   = 15;
  int tempx = mdata->locx,
      tempy = mdata->locy,
      tempindex = mdata->currentindex;

  switch (key)
    {
    case      F2: addline(lineindex, mdata);
		  break;
    case     F10: if (mdata->indexnum > 0)
		    deleteline(lineindex, mdata);
		  break;
    case U_ARROW: mdata->currentindex -= 1;
		  if (mdata->currentindex < 0)
		    mdata->currentindex = mdata->indexnum - 1;
		  break;
    case D_ARROW: mdata->currentindex += 1;
		  if (mdata->currentindex >= mdata->indexnum)
		    mdata->currentindex = 0;
		  break;
    case L_ARROW: if (mdata->currentindex > (botrow - toprow))
		    mdata->currentindex -= (botrow - toprow);
		  else
		    mdata->currentindex += (botrow - toprow - 1);
		  if (mdata->currentindex < 0)
		    mdata->currentindex = 0;
		  else if (mdata->currentindex > mdata->indexnum)
		    mdata->currentindex = mdata->indexnum;
		  break;
    case R_ARROW: if (mdata->currentindex > (botrow - toprow))
		    mdata->currentindex -= (botrow - toprow - 1);
		  else
		    mdata->currentindex += (botrow - toprow);
		  if (mdata->currentindex < 0)
		    mdata->currentindex = 0;
		  else if (mdata->currentindex > mdata->indexnum)
		    mdata->currentindex = mdata->indexnum;
		  break;
    default:      break;
    }
  if (key != F10)
    {
    indextolocs(&tempx, &tempy, tempindex);
    _settextposition(tempy, tempx);
    _outtext(lineindex[tempindex].linenum);
    }
  indextolocs(&mdata->locx, &mdata->locy, mdata->currentindex);
  _settextposition(mdata->locy, mdata->locx);
  highlight(lineindex[mdata->currentindex].linenum, 15);
  return;
  }


unsigned getline(indexrec *lineindex, data_rec *mdata)
  {
  unsigned key;
  short    lasttextcolor;
  long     lastbkcolor;

  _settextwindow(22,10, 22, 52);
  lastbkcolor   = _setbkcolor(RED);
  lasttextcolor = _settextcolor(BLUE);
  _clearscreen(_GWINDOW);
  _outtext(" Enter The Line Number [               ]");
  memset(lineindex[mdata->currentindex].linenum, '\0', 16);
  key = getvideostring(lineindex[mdata->currentindex].linenum,
		       25, 1, 25, 15,
		       keyinexitlist, keyinalphanumnospacelist);
  _setbkcolor(WHITE);
  _clearscreen(_GWINDOW);             /* Erase Line Requester Window */
  _settextwindow(mdata->y, mdata->x, mdata->y + 17, mdata->x + 72);
  _setbkcolor(lastbkcolor);
  _settextcolor(lasttextcolor);
  return(key);
  }


void addline(indexrec *lineindex, data_rec *mdata)
  {
  int  leftcol  = 3,
       rightcol = 25,
       toprow   = 3,
       botrow   = 15;
  int tempindex = mdata->currentindex;
  struct rccoord lastpos;
  FILE *indexptr;

  if (mdata->currentindex >= MAXLINES)
    {
    lastpos = _settextposition(0, 25);
    _outtext("No more lines allowed! Delete old lines to make room.");
    _settextposition(lastpos.row, lastpos.col);
    }
  else
    {
    if ((indexptr = fopen("LinIndex.Dat", "r+b")) == NULL)
      {
      if ((indexptr = fopen("LinIndex.Dat", "w+b")) == NULL)
	{
	printf("ERROR: Unable to create LinIndex.Dat file.\n"
	       "Any key to continue...\n");
	getch();
	}
      }
    mdata->currentindex = mdata->indexnum;
    if (getline(lineindex , mdata) != ESCAPE)
      {
      mdata->indexnum += 1;
      loadnextfilename(lineindex, mdata);
      fseek(indexptr, 0L, SEEK_END);
      fwrite(&lineindex[mdata->currentindex], sizeof(indexrec), 1, indexptr);
      }
    else
      mdata->currentindex = tempindex;
    fclose(indexptr);
    }
  return;
  }


void loadnextfilename(indexrec *lineindex, data_rec *mdata)
  {
  int useit = TRUE,
      x,
      j = 0;

  do
    {
    for (useit = TRUE, x = 0; x <= mdata->indexnum; x++, j++)
      {
      if (strncmp(lineindex[x].filename, filename[j], 10) == 0)
	useit = FALSE;
      }
    }
  while ((useit == FALSE) && (j < MAXLINES));
  strcpy(lineindex[mdata->currentindex].filename, filename[j]);
  return;
  }


void indextolocs(int *x, int *y, int index)
  {
  /* this converts the arrayvalue (index) into its location on the menu */
  int leftcol  = 3,
      rightcol = 25,
      toprow   = 3,
      botrow   = 15;

  if (index < (botrow - toprow))
    {
    *x = leftcol;
    *y = index + toprow;
    }
  else
    {
    *x = rightcol;
    *y = index - (botrow - toprow) + toprow;
    }
  return;
  }


void deleteline(indexrec *lineindex, data_rec *mdata)
  {
  indexrec temprec;
  unsigned key;
  int      x,
	   y;
  short    lasttextcolor;
  long     lastbkcolor;

  _settextwindow(22,10, 22, 52);
  lastbkcolor   = _setbkcolor(BLUE);
  lasttextcolor = _settextcolor(BRIGHTWHITE);
  _clearscreen(_GWINDOW);
  _outtext(" Delete [");
  _settextcolor(RED);
  _outtext(lineindex[mdata->currentindex].linenum);
  _settextcolor(BRIGHTWHITE);
  _outtext("]??? (Y/N)");
  key = _bios_keybrd(_KEYBRD_READ);
  _setbkcolor(WHITE);
  _clearscreen(_GWINDOW);             /* Erase Delete Requester Window */
  _settextwindow(mdata->y, mdata->x, mdata->y + 17, mdata->x + 72);
  _setbkcolor(lastbkcolor);
  _settextcolor(lasttextcolor);
  if ((key == YKEY) || (key == yKEY))
    {
    remove(lineindex[mdata->currentindex].filename);
    lineindex[mdata->currentindex] = lineindex[mdata->indexnum];
    mdata->indexnum -= 1;
    if (mdata->indexnum < 0)
      mdata->indexnum = 0;
    mdata->currentindex = 0;
    intindexmenu(lineindex, mdata);
    }
  return;
  }


void intindexmenu(indexrec *lineindex, data_rec *mdata)
  {
  int leftcol  = 3,
      rightcol = 25,
      toprow   = 3,
      botrow   = 15;
  int i,
      tx = leftcol,
      ty =  toprow;

  for(i = 0; i <= mdata->indexnum; i++)
    {
    if (i == mdata->currentindex)
      {
      _settextposition(ty, tx);
      mdata->locx = tx;
      mdata->locy = ty;
      highlight(lineindex[i].linenum, 15);
      }
    else
      {
      _settextposition(ty, tx);
      _outtext(lineindex[i].linenum);
      }
    if (ty <= (botrow - toprow + 1))
      ty++;
    else
      {
      ty = toprow;
      tx = rightcol;
      }
    }
  indextolocs(&tx, &ty, mdata->indexnum);
  _settextposition(ty, tx);
  _outtext("End Of File    ");
  _settextposition(ty + 1, tx);
  _outtext("               ");
  _settextposition(mdata->locy, mdata->locx);
  return;
  }


void highlight(char *textline, int barlength)
  {
  int i;
  struct rccoord curpos;

  curpos = _gettextposition();
  _settextcolor(YELLOW);
  _outtext(textline);
  for(i = 1; i < barlength - strlen(textline); i++)
    _outtext(" ");
  _settextcolor(BRIGHTWHITE);
  _settextposition(curpos.row, curpos.col);
  return;
  }


void toolchanreport(indexrec *lineindex, filerec *data,
		    filerec **dataptrs, data_rec *mdata)
  {
  int      i;
  long     begin,
	   end;
  unsigned key;
  short    lasttextcolor;
  long     lastbkcolor;

  do
    {
    _settextwindow(12,10, 22, 62);
    _setbkcolor(BLUE);
    _settextcolor(BRIGHTWHITE);
    _clearscreen(_GWINDOW);
    _outtext(" *----Tool Change Report Menu----*");
    _settextposition(4, 5);
    _outtext("F1 - Print report for a particular line-number.");
    _settextposition(6, 5);
    _outtext("F2 - Print report for all active line-numbers.");
    key = _bios_keybrd(_KEYBRD_READ);
    switch (key)
      {
      /* report by line: select line */
      case F1: if (selectline(lineindex, mdata) != -1)
		 {
		 loadlinetools(lineindex[mdata->currentindex].filename,
			       data,
			       dataptrs,
			       mdata);
		 if (getbeginend(&begin, &end) != ESCAPE)
		   toolreport(lineindex[mdata->currentindex].linenum,
			      data,
			      dataptrs,
			      mdata,
			      &begin,
			      &end);
		 }
	       break;
      /* report for all line numbers */
      case F2: for (i = 0; i < mdata->indexnum; i++)
		  {
		  loadlinetools(lineindex[i].filename,
				data,
				dataptrs,
				mdata);
		  if (getbeginend(&begin, &end) != ESCAPE)
		    toolreport(lineindex[i].linenum,
			       data,
			       dataptrs,
			       mdata,
			       &begin,
			       &end);
		  }
	       break;
      default: break;
      }
    }
  while (key != ESCAPE);
  _setbkcolor(WHITE);
  _clearscreen(_GWINDOW);             /* Erase Delete Requester Window */
  _settextwindow(10, 26, 10 + 8, 26 + 29);
  _setbkcolor(lastbkcolor);
  _settextcolor(lasttextcolor);
  return;
  }


unsigned getbeginend(long *begin, long *end)
  {
  unsigned key;
  char     beginstr[10],
	   endstr[10];

  do
    {
    memset(beginstr, '\0', 10);
    memset(endstr, '\0', 10);
    _settextwindow(14,14, 20, 65);
    _setbkcolor(BLUE);
    _settextcolor(BRIGHTWHITE);
    _clearscreen(_GWINDOW);
    _outtext(" *----Beginning/Ending Count Values----*");
    _settextposition(5, 4);
    _outtext("Enter Beginning Count =");
    key = getvideostring(beginstr,
			  25 + 4, 5, 25 + 4, 9,
			  keyinexitlist, keyinnumlist);
    _settextposition(7, 4);
    _outtext("Enter Ending Count    =");
    key = getvideostring(endstr,
			  25 + 4, 7, 25 + 4, 9,
			  keyinexitlist, keyinnumlist);
    *begin = atol(beginstr);
    *end = atol(endstr);
    }
  while (*end < *begin);
  if (key == ESCAPE)
    {
    *begin = 0;
    *end   = 1;
    }
  _setbkcolor(WHITE);
  _clearscreen(_GWINDOW);             /* Erase Delete Requester Window */
  _settextwindow(12,10, 22, 62);
  _setbkcolor(BLUE);
  _settextcolor(BRIGHTWHITE);
  return(key);
  }


void toolreport(char *linenum,
		filerec *data,
		filerec **dataptrs,
		data_rec *mdata,
		long *begin,
		long *end)
  {
  int    i,
	 lines = 0,
	 page  = 1;
  long   j;
  double numb,
	 freq;

  toolreportheader(linenum, &page, &lines, begin, end, mdata);
  qsort(dataptrs,
	mdata->toolnum,
	sizeof(filerec*),
	comparetool);
  _outtext("Printing....");
  for (j = *begin; j <= *end; j += 10)
    {
    for (i = 0; i < mdata->toolnum; i++)
      {
      freq = (double) dataptrs[i]->freq;
      numb = fabs(((double) j - (double) dataptrs[i]->beginning) / freq);
      if ((numb - floor(numb)) <= (1.0f/(freq + 1.0f)))
	{
	fprintf(mdata->output, "%-9ld %-25s %-29s ________ _____\n\n",
		j, dataptrs[i]->toolnumb, dataptrs[i]->tooldesc);
	lines += 2;
	if (lines >= 58)
	  toolreportheader(linenum, &page, &lines, begin, end, mdata);
	}
      }
    }
  fprintf(mdata->output, "\f");
  return;
  }


void toolreportheader(char *linenum,
		      int *page,
		      int *lines,
		      long *begin,
		      long *end,
		      data_rec *mdata)
  {
  char datebuff[9],
       timebuff[9];

  _strdate(datebuff);
  _strtime(timebuff);
  if (*page > 1)
    fprintf(mdata->output, "\f");
  fprintf(mdata->output,"\n%*s page %-3i\n", 70, " ", *page);
  fprintf(mdata->output, "                             The Metalloy Corporation\n");
  fprintf(mdata->output, "                                Tool-Change Report\n");
  fprintf(mdata->output, "                              Line Number - %-s\n", linenum);
  fprintf(mdata->output, "                                %s  %s\n",
	  datebuff, timebuff);
  fprintf(mdata->output, "                             Count=%-ld to %-ld\n\n", *begin, *end);
  fprintf(mdata->output, "COUNT     TOOL NUMBER               DESCRIPTION                   CHANGE  INTIAL\n\n");
  *lines = 8;
  *page += 1;
  return;
  }


void datafilereport(indexrec *lineindex, filerec *data,
		    filerec **dataptrs, data_rec *mdata)
  {
  int i;
  long begin, end;
  unsigned key;
  short lasttextcolor;
  long  lastbkcolor;

  do
    {
    _settextwindow(12,10, 22, 62);
    _setbkcolor(BLUE);
    _settextcolor(BRIGHTWHITE);
    _clearscreen(_GWINDOW);
    _outtext(" *----Data File Report Menu----*");
    _settextposition(4, 5);
    _outtext("F1 - Print report for a particular line-number.");
    _settextposition(6, 5);
    _outtext("F2 - Print report for all active line-numbers.");
    key = _bios_keybrd(_KEYBRD_READ);
    if (key == F1)
      {                                /* report by line: select line */
      if (selectline(lineindex, mdata) != -1)
	{
	loadlinetools(lineindex[mdata->currentindex].filename, data,
		      dataptrs, mdata);
	datareport(lineindex[mdata->currentindex].linenum, data,
		     dataptrs, mdata);
	}
      }
    else if (key == F2)                /* report for all line numbers */
      for (i = 0; i < mdata->indexnum; i++)
	 {
	 loadlinetools(lineindex[i].filename, data,
		       dataptrs, mdata);
	 datareport(lineindex[i].linenum, data,
		    dataptrs, mdata);
	 }
    }
  while (key != ESCAPE);
  _setbkcolor(WHITE);
  _clearscreen(_GWINDOW);             /* Erase Delete Requester Window */
  _settextwindow(10, 26, 10 + 8, 26 + 29);
  _setbkcolor(lastbkcolor);
  _settextcolor(lasttextcolor);
  return;
  }


void datareport(char *linenum, filerec *data,
		filerec **dataptrs, data_rec *mdata)
   {
   int lines = 0,
       page  = 1,
       i;

   datareportheader(linenum, &page, &lines, mdata);
   qsort(dataptrs,
	 mdata->toolnum,
	 sizeof(filerec*),
	 comparetool);
   for (i = 0; i < mdata->toolnum; i++)
     {
     lines += 1;
     fprintf(mdata->output, "%3d %-25s %-30s %9ld %9ld\n",
	     i+1,
	     dataptrs[i]->toolnumb,
	     dataptrs[i]->tooldesc,
	     dataptrs[i]->freq,
	     dataptrs[i]->beginning);
     if (page >= 55)
       datareportheader(linenum, &page, &lines, mdata);
     }
   fprintf(mdata->output, "\f");
   return;
   }


void datareportheader(char *linenum,
		      int *page,
		      int *lines,
		      data_rec *mdata)
  {
  char datebuff[9],
       timebuff[9];

  _strdate(datebuff);
  _strtime(timebuff);
  if (*page < 1)
    fprintf(mdata->output, "\f");
  fprintf(mdata->output,"\n%*s page %i\n", 70, " ", *page);
  fprintf(mdata->output, "                             The Metalloy Corporation\n");
  fprintf(mdata->output, "                             Tooling-Data File Report\n");
  fprintf(mdata->output, "                              Line Number - %-s\n", linenum);
  fprintf(mdata->output, "                                %s  %s\n\n",
	  datebuff, timebuff);
  fprintf(mdata->output, "REC TOOL NUMBER               DESCRIPTION                    FREQUENCY BEGINNING\n\n");
  *lines = 8;
  *page += 1;
  return;
  }


int comparetool(filerec **arg1, filerec **arg2)
  {
  return(strcmp(((filerec *)(*arg1))->toolnumb,
		((filerec *)(*arg2))->toolnumb));
  }

