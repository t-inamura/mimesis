/*
 * Test program for Extrapolation & Interpolation of HMM
 *
 * Last modified on 2007 Dec 26th by Tetsunari Inamura 
 *
 */

#include <sys/types.h>
#include <unistd.h>

#include <mimesis.h>
#include <JHMM.h>
#include <Behavior.h>
#include <Mds.h>
#include <MotionDB.h>
#include <WorkSpace.h>
#include <HTKUnit.h>


using namespace std;

char		hmm_file[10][MAX_STRING];
double		weight[10];
WorkSpace	*work = NULL;
int		hmm_index=0;

int print_usage (void)
{
  fprintf (stderr, "usage : synthesis [--debug] [--input-hmm] hmm_file_1 hmm_file_2 ....\n");
  fprintf (stderr, "                  [--weight] 1.0  1.5  ....\n");
  return 0;
}


int get_args (int argc, char **argv)
{
  int	i, weight_index=0;

  for (i=1; i<argc;)
    {
      if (tl_strmember (argv[i], "--debug"))
	{
	  Common_SetDebug (TRUE);
	  i++;
	}
      else if (tl_strmember (argv[i], "--input-hmm"))
	{
	  i++;
	  while(1)
	    {
	      strcpy (hmm_file[hmm_index], argv[i]);
	      tl_message ("<%s>", hmm_file[hmm_index]);
	      i++; hmm_index++;
	      if (i==argc || tl_strmember(argv[i], "--")) break;
	    }
	}
      else if (tl_strmember (argv[i], "--weight"))
	{
	  i++;
	  while(1)
	    {
	      weight[weight_index] = atof(argv[i]);
	      tl_message ("<%g>", weight[weight_index]);
	      i++; weight_index++;
	      if (i==argc || tl_strmember(argv[i], "--")) break;
	    }
	}
      else 
	{ print_usage(); exit(0);}
    }
  if (hmm_index==0)
    { print_usage(); exit(0);}
  return TRUE;
}




int main(int argc, char **argv)
{
  srand (getpid());
  Common_SetDebug (FALSE);

  get_args(argc, argv);

  tl_message ("start");
  JHMM		*synthesis_hmm = NULL, *hmm[10];
  Behavior	*beh = NULL;

  tl_message ("Loading hmm factor for synthesis");
  for (int i=0; i<hmm_index; i++)
    {
      hmm[i] = new JHMM;
      hmm[i]->Load (hmm_file[i]);
      hmm[i]->Show();
      tl_message ("%g x %s", weight[i], hmm_file[i]);
    }
  synthesis_hmm = new JHMM;
  tl_message ("Now generate a general synthesised HMM");
  synthesis_hmm->InterpolationAny (hmm, weight, hmm_index);
  synthesis_hmm->Show();
  beh = synthesis_hmm->GenerateBehavior(100, 30);
  beh->FileOut("synthesis.beh");

  delete beh;
  for (int i=0; i<hmm_index; i++)
    delete hmm[i];
  delete synthesis_hmm;
  return TRUE;
  return TRUE;
}
