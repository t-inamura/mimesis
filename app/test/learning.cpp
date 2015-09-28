/*
 * Test code for HMM learning 
 *
 * Last Modified on 2015 Sep 28th by Tetsunari Inamura 
 *
 */


#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "mimesis.h"
#include "JHMM.h"
#include "Behavior.h"
#include "Mds.h"
#include "MotionDB.h"
#include "WorkSpace.h"
#include "HTKUnit.h"


char script_file[MAX_STRING];


int print_usage (void)
{
  fprintf (stderr, "usage : learninge --rc [script file] [--debug]\n");
  fprintf (stderr, "        (example # learninge --rc script/learning_punch\n");
  fprintf (stderr, "        --debug  :  print debug message\n");
  return 0;
}


int get_args (int argc, char **argv)
{
  int	i;

  script_file[0] = 0;

  for (i=1; i<argc;)
    {
      if (tl_strmember (argv[i], "--rc")) 
	{
	  strcpy (script_file, argv[++i]);
	  tl_message ("script file is : %s", script_file);
	  i++;
	}
      else if (tl_strmember (argv[i], "--debug"))
	{
	  Common_SetDebug (TRUE);
	  i++;
	}
      else 
	{ print_usage(); exit(0);}
    }
  if (script_file[0]==0)
    { print_usage(); exit(0);}
  return TRUE;
}



int main(int argc, char **argv)
{
  WorkSpace *work    = NULL;

  get_args(argc, argv);
  
  srand (getpid());

  work = new WorkSpace();
  work->LoadMotionDB (script_file);
  work->ExecLearning ();
  
  delete work;

  return 0;
}
