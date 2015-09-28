/*
 * Test code to create proto-symbol space
 *
 * Last Modified on 2015 Sep 28th by Tetsunari Inamura
 *
 */

#include <string.h>

#include <mimesis.h>
#include <JHMM.h>
#include <Behavior.h>
#include <Mds.h>
#include <MotionDB.h>
#include <WorkSpace.h>
#include <HTKUnit.h>
#include <unistd.h>

#define	SAMPLE_FILE	"../../script/learning_scriptfile_3"

char	script_file[MAX_STRING];
int	Dim = 0;
DistanceType	D_type = HELLINGER;

int print_usage (void)
{
  fprintf (stderr, "usage : space_create --rc [script file] [--debug]\n");
  fprintf (stderr, "        (example # space_create --rc script/learning_punch\n");
  fprintf (stderr, "        --dim N  :  set number of dimension\n");
  fprintf (stderr, "        --use-kld    :  use KL divergence as a distance measurement\n");
  fprintf (stderr, "        --use-hd     :  use Hellinger distance as a distance measurement\n");
  fprintf (stderr, "        --debug      :  print debug message\n");
  return 0;
}


int get_args (int argc, char **argv)
{
  int	i;
  strcpy (script_file, SAMPLE_FILE);
  for (i=1; i<argc;)
    {
      if (tl_strmember (argv[i], "--rc")) 
	{
	  strcpy (script_file, argv[++i]);
	  i++;
	}
      else if (tl_strmember (argv[i], "--dim"))
	{
	  Dim = atoi(argv[++i]);
	  i++;
	}
      else if (tl_strmember (argv[i], "--debug"))
	{
	  Common_SetDebug (TRUE);
	  tl_message ("debug on ");
	  i++;
	}
      else if (tl_strmember (argv[i], "--use-kld"))
	{
	  tl_message ("Use Kullback-Leibler Divergence!");
	  D_type = KL_DIVERGENCE;
	  i++;
	}
      else if (tl_strmember (argv[i], "--use-hd"))
	{
	  tl_message ("Use Hellinger Distance!");
	  D_type = HELLINGER;
	  i++;
	}
      else 
	{ print_usage(); exit(0);}
    }
  tl_message ("script file is : %s", script_file);  
  if (Dim==0)
    {
      print_usage(); exit(0);
    }
  return TRUE;
}



int main(int argc, char **argv)
{
  srand(getpid());
  get_args(argc, argv);

  WorkSpace *work = NULL;
  work = new WorkSpace(D_type);

  // ファイルからロードする(mdbまで)
  work->LoadMotionDB(script_file);

  // RecUnitをセットする(mdbを参考に)
  cerr << "SetHTKUnits." << endl;
  work->SetHTKUnitsFromMotionDB();

  // 認識の準備
  work->SetLabelFromHTKUnit();
  work->BeforeRecognize();

  // DisVectorの計算
  cerr << "Calculate Distances....." << endl;

  if (D_type == KL_DIVERGENCE)
    {
      //work->SetDisVector(); delete on 2008-07-17
      work->CalcKLDivergenceMatrix();
      cerr << "Symmetrize....." << endl;
      work->SymmentrizeDisVector();
    }
  else if (D_type == HELLINGER)
    {
      work->CalcHellingerMatrix();
    }
  else {tl_warning ("something wrong!"); exit(0);}
  cerr << "File out....." << endl;
  work->DistanceFileOut("../../script/symbol_data/tmp_distance_vector");
  // work->DisVectorLoad   ("../../script/tmp_distance_vector");

  // Spaceの作成
  cerr << "Now creating a proto-symbol space...." << endl;
  work->SpaceCreate(Dim, D_type);
  work->SpaceFileOut("../../script/symbol_data/tmp_dim.spc");
  
  delete work;

  cerr << "Completed!  Result files are ../../script/symbol_data/tmp*" << endl;
  return 0;
}
