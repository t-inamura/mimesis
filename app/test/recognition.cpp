/*
 * Test code to display the recognition result by proto-symbol space
 *
 * Last Modified on 2015 Sep 28th by Tetsunari Inamura
 *
 */

#include <unistd.h>
#include <string.h>

#include "mimesis.h"
#include "Behavior.h"
#include "WorkSpace.h"

#define	DEFAULT_SCRIPT_FILE	    "script/learning_scriptfile_3"
#define	DEFAULT_INPUT_BEHAVIOR  "mdb/punch/punch1.beh"

#define DEFAULT_DISTANCE_FILE   "script/symbol_data/tmp_distance_vector"
#define DEFAULT_SPACE_FILE      "script/symbol_data/tmp_dim.spc"

char script_file[MAX_STRING];
char input_behavior[MAX_STRING];


int print_usage (void)
{
	fprintf (stderr, "usage : recognition --rc [script file] --target [input_behavior_file] [--debug]\n");
	fprintf (stderr, "        (example # space_create --rc script/learning_punch --target kick.beh\n");
	fprintf (stderr, "        --debug  :  print debug message\n");
	return 0;
}


int get_args (int argc, char **argv)
{
	int	i;
	string          script_file(getenv("MIMESIS_TOP_DIR"));
	string  input_behavior_file(getenv("MIMESIS_TOP_DIR"));
 
	script_file         += "/";
	script_file         += DEFAULT_SCRIPT_FILE;
	input_behavior_file += "/";
	input_behavior_file += DEFAULT_INPUT_BEHAVIOR;

	for (i=1; i<argc;)
		{
			if (tl_strmember (argv[i], "--rc")) 
				{
					script_file = argv[++i];
					i++;
				}
			else if (tl_strmember (argv[i], "--target")) 
				{
					input_behavior_file = argv[++i];
					i++;
				}
			else if (tl_strmember (argv[i], "--debug"))
				{
					Common_SetDebug (TRUE);
					tl_message ("debug on ");
					i++;
				}
			else 
				{ print_usage(); exit(0);}
		}
	tl_message ("script file is : %s",         script_file.c_str());  
	tl_message ("input  file is : %s", input_behavior_file.c_str());  
	return TRUE;
}




int main(int argc, char **argv)
{
	vector<double>  tmp_dis, tmp_cord;
	vector<double>  likelihood;
	Behavior        *tmp_beh, *beh;
	int             debug=1;

	srand(getpid());
	get_args(argc, argv);

	WorkSpace *work = NULL;
	work = new WorkSpace();
	work->LoadMotionDB (script_file);
	work->SetHTKUnitsFromMotionDB ();
	//work->SetLabelFromRecogUnit();
	work->BeforeRecognize();
	work->DistanceLoad ("../../script/symbol_data/tmp_distance_vector");
	work->SpaceLoad    ("../../script/symbol_data/tmp_dim.spc");

	beh = new Behavior();
	beh->Load (input_behavior);
	likelihood.resize(0);
	work->CalcLikelihoodVector (beh, likelihood);
	cerr << "LikeliHood Vecotr for the total input behavior is : ";
	for (int i=0; i<(int)(likelihood.size()); i++)
		cerr << likelihood[i] << " , ";
	cerr << endl;
	tmp_dis.resize(0);
	work->CalcDistanceOfInputBehavior(beh, tmp_dis);
	tmp_cord.resize(0);
	work->GetPsymbolSpace()->CoordinateFromDistanceData(tmp_dis, tmp_cord);
	cerr << "Coordinate of the total input behavior is : ";
	for (int i=0; i<(int)(tmp_cord.size()); i++)
		cerr << tmp_cord[i] << " , ";
	cerr << endl;
	if (debug) tl_message ("DIM. of tmp_cord = %d",(int)(tmp_cord.size()) );

	int span = 20, step = 5;
	int times;

	ofstream fout("./result.dat");
	if(!fout)
		{
			tl_message ("Cannot open file: result.dat");
			return FALSE;
		}	
	times = ( beh->Length() - span)/step;
	Pose *tmp_pose;
	for (int j=0; j<times; j++)
		{
			tmp_beh = new Behavior();
			for (int i=0; i<step; i++)
				tmp_beh->AddPose(beh->PopFrontPose());
			for (int i=0; i<span-step; i++)
				{
					tmp_pose = new Pose(*(beh->NthPose(i)));
					tmp_beh->AddPose(tmp_pose);
				}
			tmp_dis.resize(0);
			work->CalcDistanceOfInputBehavior(tmp_beh, tmp_dis);
			delete tmp_beh;
			tmp_cord.resize(0);
			work->GetPsymbolSpace()->CoordinateFromDistanceData(tmp_dis, tmp_cord);
			for (int i=0; i<(int)(tmp_cord.size()); i++)
				fout << tmp_cord[i] << " ";
			fout << endl;
		}
	times = (beh->Length() - span)/step;
	fout.close();

	delete work;
	cout << "Completed!  see result.dat" << endl;
	return 0;
}
