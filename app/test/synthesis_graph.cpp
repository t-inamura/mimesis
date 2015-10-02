/*
 * Graph creation program for Extrapolation & Interpolation of HMM
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

char        hmm_file[10][MAX_STRING];
double      weight[10];
WorkSpace   *work = NULL;
int         hmm_index=0;


int get_args (int argc, char **argv)
{
	int	i;

	for (i=1; i<argc;)
		{
			if (tl_strmember (argv[i], "--debug"))
				{
					Common_SetDebug (TRUE);
					tl_message ("debug on ");
					i++;
				}
			else 
				{ exit(0);}
		}
	return TRUE;
}



int main(int argc, char **argv)
{
	srand (getpid());
	Common_SetDebug (TRUE);

	get_args(argc, argv);

	JHMM        *synthesis_hmm = NULL, *hmm[10];
	Behavior    *beh = NULL;

	tl_message ("Loading hmm factor for synthesis");
	hmm[0] = new JHMM;
	hmm[1] = new JHMM;
	hmm[2] = new JHMM;
	hmm[0]->Load("./.tmp/squat/squat.hmm");
	hmm[1]->Load("./.tmp/kick/kick.hmm");
	hmm[2]->Load("./.tmp/punch/punch.hmm");


	WorkSpace *work = NULL;
	work = new WorkSpace();
	work->LoadMotionDB ("../../script/learning_scriptfile_2");
	//work->LoadHTKUnits ("../../script/htkunit_scriptfile_2");
	work->SetHTKUnitsFromMotionDB ();
	work->SetLabelFromHTKUnit();
	work->BeforeRecognize();
	//work->DisVectorLoad("../../script/symbol_data/tmp_distance_vector");
	work->SpaceLoad    ("../../script/symbol_data/tmp_dim.spc");
	cerr << "num of htk_units = " << work->NumOfHTKUnits() << endl;

#if 0
	work->CalcKLDivergenceMatrix();
	work->SpaceFileOut ("./tmp_space_output.txt");

	tl_message ("KL distance from 0 to 1 (by new algo.) = %g", work->GetNthHTKUnit(0)->CalcKLDistance ( work->GetNthHTKUnit(1) ) );
	tl_message ("KL distance from 1 to 0 (by new algo.) = %g", work->GetNthHTKUnit(1)->CalcKLDistance ( work->GetNthHTKUnit(0) ) );
	tl_message ("KL distance from 0 to 2 (by new algo.) = %g", work->GetNthHTKUnit(0)->CalcKLDistance ( work->GetNthHTKUnit(2) ) );
	tl_message ("KL distance from 2 to 0 (by new algo.) = %g", work->GetNthHTKUnit(2)->CalcKLDistance ( work->GetNthHTKUnit(0) ) );
	tl_message ("KL distance from 1 to 2 (by new algo.) = %g", work->GetNthHTKUnit(1)->CalcKLDistance ( work->GetNthHTKUnit(2) ) );
	tl_message ("KL distance from 2 to 1 (by new algo.) = %g", work->GetNthHTKUnit(2)->CalcKLDistance ( work->GetNthHTKUnit(1) ) );
#else
	work->CalcHellingerMatrix();
	work->SpaceFileOut ("./tmp_space_output.txt");

	tl_message ("Hellinger Distance from 0 to 1 = %g", work->GetNthHTKUnit(0)->CalcHellingerDistance ( *(work->GetNthHTKUnit(1)) ) );
	tl_message ("Hellinger Distance from 1 to 0 = %g", work->GetNthHTKUnit(1)->CalcHellingerDistance ( *(work->GetNthHTKUnit(0)) ) );
	tl_message ("Hellinger Distance from 0 to 2 = %g", work->GetNthHTKUnit(0)->CalcHellingerDistance ( *(work->GetNthHTKUnit(2)) ) );
	//tl_message ("Hellinger Distance from 2 to 0 = %g", work->GetNthHTKUnit(2)->CalcHellingerDistance ( work->GetNthHTKUnit(0) ) );
	tl_message ("Hellinger Distance from 1 to 2 = %g", work->GetNthHTKUnit(1)->CalcHellingerDistance ( *(work->GetNthHTKUnit(2)) ) );
	//tl_message ("Hellinger Distance from 2 to 1 = %g", work->GetNthHTKUnit(2)->CalcHellingerDistance ( work->GetNthHTKUnit(1) ) );
#endif

	vector<double>	tmp_dis, tmp_cord;
	tl_message ("Now generate a general synthesised HMM");
	ofstream fout("./result.dat");
	fout << "weight[0], weight[1], pseudo_distance to [0], pseudo_distance to [1], position_1, position_2" << endl;
	for (int i=-20; i<=20; i++)
		{
			synthesis_hmm = new JHMM;
			weight[0] = 0.1 * i;
			weight[1] = 1.0 - 0.1 * i;
			synthesis_hmm->InterpolationAny (hmm, weight, 2);
			beh = synthesis_hmm->GenerateBehavior(100, 30);
			delete synthesis_hmm;

			work->CalcDistanceOfInputBehavior(beh, tmp_dis);
			work->GetPsymbolSpace()->CoordinateFromDistanceData(tmp_dis, tmp_cord);
			fout << weight[0] << "," << weight[1] << ",";

			for (int i=0; i<(int)(tmp_dis.size()); i++)
				fout << tmp_dis[i] << ",";
			for (int i=0; i<(int)(tmp_cord.size()); i++)
				fout << tmp_cord[i] << ",";
			fout << endl;
			cerr << "size of distance vector = " << tmp_dis.size() << ", and size of coordinate = " << tmp_cord.size() << endl;
		}

	delete beh;
	for (int i=0; i<3; i++)
		delete hmm[i];
	return TRUE;
	return TRUE;
}
