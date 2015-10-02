/*
 * Behavior Generation Test
 *
 * Last modified on 2015 Sep 28th by Tetsunari Inamura 
 *
 */

#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <mimesis.h>
#include <JHMM.h>
#include <Behavior.h>
#include <Mds.h>
#include <MotionDB.h>
#include <WorkSpace.h>
#include <HTKUnit.h>

using namespace std;

char script_file[MAX_STRING];
int  Using_ss = 0;


int print_usage (void)
{
	fprintf (stderr, "usage : generation --rc [script file] [--debug] [--with-space] [--without-space] \n");
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
			else if (tl_strmember (argv[i], "--with-space"))
				{
					Using_ss = 1;
					i++;
				}
			else if (tl_strmember (argv[i], "--without-space"))
				{
					Using_ss = 0;
					i++;
				}
			else 
				{ print_usage(); exit(0);}
		}
	if (script_file[0]==0)
		{ print_usage(); exit(0);}
	return TRUE;
}


int generate_with_symbol_space(void)
{
  
	Behavior	*tmp_beh, *beh, *beh2;
	WorkSpace	*work = NULL;
	work = new WorkSpace();
	tl_message ("Now loading motionDBs...");
	work->LoadMotionDB (script_file);
	work->SetHTKUnitsFromMotionDB ();
	tl_message ("step 1");
	//work->SetLabelFromRecogUnit();
	tl_message ("step 2");
	work->BeforeRecognize();
	tl_message ("step 3");
	work->CalcKLDivergenceMatrix();
	tl_message ("step 4");
	work->SymmentrizeDisVector();
	tl_message ("step 5");
	work->SpaceCreate(3, KL_DIVERGENCE);

	// beh $B$KMM!9$J<oN`$N9TF0$r;~4V<4>e$G$D$J$2$?D9$$9TF0$rBeF~(B
	beh = new Behavior();
	//beh->Load("ashibum/ashibum0.beh");
	beh->Load("1radio/1radio0.beh");
	beh2 = new Behavior();
	//beh2->Load("kick/kick0.beh");
	beh2->Load("2radio/2radio0.beh");
	beh->Cat(*beh2);
	delete beh2;

	vector<double> tmp_dis;
	vector<double> tmp_cord;
	int span = 20, step = 5;
	int times;
	times = ( beh->Length() - span)/step;
	cerr << "----------------  times:" << times << endl;
	Pose* tmp_pose;

	tmp_cord.push_back(0.0);
	tmp_cord.push_back(0.0);
	tmp_cord.push_back(0.0);
	tl_message ("Single Point is started!");
	work->BehaviorGenerateFromSinglePoint (tmp_cord);
	tl_message ("Single Point is succeeded!");

	// tmp_beh $B$K(B beh $B$N0lItJ,$r$9$3$7$:$:%7%U%H$7$J$,$iBeF~$7$F$$$/(B
	// tmp_beh $B$ND9$5$O(B span=20, $B%7%U%H$9$kNL$O(B step=5
	for (int j=0; j<times; j++)
		{
			tmp_beh = new Behavior();
			cerr << "----------------  pop" << endl;
			for (int i=0; i<step; i++)
				tmp_beh->AddPose(beh->PopFrontPose());
			for (int i=0; i<span-step; i++)
				{
					tmp_pose = new Pose(*(beh->NthPose(i)));
					tmp_beh->AddPose(tmp_pose);
				}
			cerr << "----------------  CalcDis" << endl;
			// $B$=$N0lItJ,$NF0:n$r6u4V$K<M1F$7$?>l9g$N!$3F4{CN86;O%7%s%\%k$+$i$N5wN%$r7WB,(B
			tmp_dis.clear();
			work->CalcDistanceOfInputBehavior(tmp_beh, tmp_dis);
			delete tmp_beh;
			cerr << "----------------  GetCoordinate" << endl;
			// $B5a$a$?5wN%$K1~$8$F!$6u4V$K<M1F$9$k$Y$-:BI8$r;;=P(B
			tmp_cord.clear();
			work->GetPsymbolSpace()->CoordinateFromDistanceData(tmp_dis, tmp_cord);
			cerr << "!!!!! " << j << "th Gene Start!" << endl;
			// $B$=$N:BI8$K$*$1$kF0:n$r@8@.$9$k!%(B
			work->BehGeneFromTransition(tmp_cord);
			cerr << "!!!!! " << j << "th Gene Finish!" << endl;
		}
	work->BehBufFileOut("beh_buf.beh");


	return TRUE;
}


/*
 * $B86;O%7%s%\%k6u4V$O:n$i$:$K!$4{B8$N(BHMM$B$+$iC1=c$KF0:n$r@8@.$9$k(B
 */
int generate_without_symbol_space(void)
{
	int          debug=1;
	string       hmm_file;
	Behavior     *behavior = NULL;
	HTKUnit      *htk_unit;
	JHMM         *hmm      = NULL;

	tl_message ("start");
  
	WorkSpace* work = NULL;
	work = new WorkSpace();
	if (debug) tl_message ("Now loading recogunits...");
	work->LoadMotionDB (script_file);
	work->SetHTKUnitsFromMotionDB ();
	if (debug) tl_message ("Now generating...");
	htk_unit = work->GetNthHTKUnit (0);
	if (debug) tl_message ("... step 1 ...");
	hmm_file = htk_unit->GetHMMFile ();
	if (debug) tl_message ("... step 2 ...");
	hmm = new JHMM();
	hmm->Load (hmm_file.c_str());
	if (debug) tl_message ("... step 3 ...");
	behavior = hmm->GenerateBehavior(GEN_NUM, GEN_NUM_Q);
	if (debug) tl_message ("... step 4 ...");
	behavior->FileOut ("./generated.beh");
  
	return TRUE;
}


// Created on 2006-09-07
// Trying to generate new proto-symbol by exterior division
int generate_exterior_division(void)
{
	JHMM		*kick = NULL, *walk = NULL, *banzai=NULL, *squat=NULL, *hmm_ex = NULL;
	Behavior	*behavior = NULL;

	kick   = new JHMM();
	walk   = new JHMM();
	banzai = new JHMM();
	squat  = new JHMM();
	hmm_ex = new JHMM();
	kick  ->Load ("./.tmp/kick/kick.hmm");
	walk  ->Load ("./.tmp/ashibum/ashibum.hmm");
	squat ->Load ("./.tmp/squat/squat.hmm");
	banzai->Load ("./.tmp/banzai/banzai.hmm");
#if 1
	behavior =   kick->GenerateBehavior(GEN_NUM, GEN_NUM_Q);
	behavior->FileOut ("./kick_gene.beh");
	behavior =  squat->GenerateBehavior(GEN_NUM, GEN_NUM_Q);
	behavior->FileOut ("./squat_gene.beh");
	behavior =   walk->GenerateBehavior(GEN_NUM, GEN_NUM_Q);
	behavior->FileOut ("./walk_gene.beh");
	behavior = banzai->GenerateBehavior(GEN_NUM, GEN_NUM_Q);
	behavior->FileOut ("./banzai_gene.beh");
#endif
	banzai->Verify();

	fprintf (stderr, "step 1\n");
	hmm_ex = *((*banzai) * 2.0) + *((*kick) * -1.0);
	behavior = hmm_ex->GenerateBehavior(GEN_NUM, GEN_NUM_Q);
	behavior->FileOut ("./out_banzai_from_kick.beh");
	hmm_ex->Verify();

	fprintf (stderr, "step 2\n");
	hmm_ex = *((*kick) * 2.0) + *((*banzai) * -1.0);
	behavior = hmm_ex->GenerateBehavior(GEN_NUM, GEN_NUM_Q);
	behavior->FileOut ("./out_kick_from_banzai.beh");
	hmm_ex->Verify();

#if 1
	hmm_ex = *((*banzai) * 0.5)  + *((*squat) * 0.5);
	behavior = hmm_ex->GenerateBehavior(GEN_NUM, GEN_NUM_Q);
	behavior->FileOut ("./banzai_and_squat.beh");
	hmm_ex = *((*kick) * 0.5)  + *((*banzai) * 0.5);
	behavior = hmm_ex->GenerateBehavior(GEN_NUM, GEN_NUM_Q);
	behavior->FileOut ("./banzai_and_kick.beh");
	hmm_ex = *((*kick) * 0.5)  + *((*walk) * 0.5);
	behavior = hmm_ex->GenerateBehavior(GEN_NUM, GEN_NUM_Q);
	behavior->FileOut ("./kick_and_walk.beh");
	hmm_ex = *((*kick) * 0.5)  + *((*banzai) * 0.5);
	behavior = hmm_ex->GenerateBehavior(GEN_NUM, GEN_NUM_Q);
	behavior->FileOut ("./kick_and_banzai.beh");
#endif
	return TRUE;
}


int main(int argc, char **argv)
{
	get_args(argc, argv);

	srand (getpid());
	Common_SetDebug(TRUE);
	if (Using_ss==1)
		generate_with_symbol_space();
	else 
		generate_without_symbol_space();
	//generate_exterior_division();
	return TRUE;
}
