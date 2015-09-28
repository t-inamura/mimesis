/*
 * Test program for Extrapolation & Interpolation of HMM
 *
 * Last modified on 2015 Sep 28th by Tetsunari Inamura 
 *
 */

#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include <mimesis.h>
#include <JHMM.h>
#include <Behavior.h>
#include <Mds.h>
#include <MotionDB.h>
#include <WorkSpace.h>
#include <HTKUnit.h>


using namespace std;

#define NUM_MOTION 4
#define KICK	0
#define WALK	1
#define BANZAI	2
#define SQUAT	3
#define THROW	4
#define DANCE	5

JHMM		*hmm[NUM_MOTION], *hmm_center=NULL;
WorkSpace	*work = NULL;
char script_file[MAX_STRING];



int print_usage (void)
{
  fprintf (stderr, "usage : exttra_test [--debug] \n");
  return 0;
}


int get_args (int argc, char **argv)
{
  int	i;

  script_file[0]=0;

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
  return TRUE;
}



// Created on 2008-07-10
// Test for what's happen when the extrapolated motion patterns are recognized by mimesis
// also for RSJ conf. 2008
int test_extra_recognition(void)
{
  tl_message ("start");
  JHMM		*hmm1 = NULL, *hmm2 = NULL, *hmm3 = NULL, *synthesis_hmm = NULL;
  Behavior	*beh = NULL;
  JHMM		*hmm_vec[3];
  double	weight[3];
  int		i;

  hmm1      = new JHMM;
  hmm2      = new JHMM;
  hmm3      = new JHMM;
  hmm1->Load("./.tmp/punch/punch.hmm");
  hmm2->Load("./.tmp/squat/squat.hmm");
  hmm3->Load("./.tmp/kick/kick.hmm");
  hmm_vec[0] = hmm1;
  hmm_vec[1] = hmm2;
  hmm_vec[2] = hmm3;

  WorkSpace *work = NULL;
  work = new WorkSpace();
  if (script_file[0]==0) { print_usage(); exit(0); }
  work->LoadMotionDB (script_file);
  work->SetHTKUnitsFromMotionDB ();
  work->BeforeRecognize();
  work->DistanceLoad ("../../script/symbol_data/tmp_distance_vector");
  work->SpaceLoad    ("../../script/symbol_data/tmp_dim.spc");

  vector<double>	tmp_dis, new_dis, hellinger, tmp_cord;
  vector<double>	likelihood;

  tl_message ("# of motion db is : %d\n", work->GetNumOfMotionDB() );

  cout << "c_1, c_2, ";
  for (i=0; i<(int)(work->GetNumOfMotionDB()); i++)
    cout << "old_KLdistance[" << i << "] ,";
  for (i=0; i<(int)(work->GetNumOfMotionDB()); i++)
    cout << "new_KLdivergence[" << i << "] ,";
  for (i=0; i<(int)(work->GetNumOfMotionDB()); i++)
    cout << "Hellinger Distance[" << i << "] ,";
  for (i=0; i<(int)(work->GetPsymbolSpace()->GetDimension() ); i++)
    cout << "x[" << i << "] , ";
  cout << endl;

  for (int i=-10; i<20; i++)
    {
      synthesis_hmm = new JHMM;
      weight[0] = i * 0.1;
      weight[1] = 1 - weight[0];
      synthesis_hmm->InterpolationAny (hmm_vec, weight, 2);
      beh = synthesis_hmm->GenerateBehavior(100, 30);

      work->CalcHellingerDistanceOfOnlineBehavior(beh, hellinger);
      tl_message ("step hoge %d.0", i);
      for (int j=0; j<(int)(hellinger.size()); j++)
	cout << hellinger[j] << " , ";
      cout << endl;
      tl_message ("step hoge %d.1", i);
      
      work->CalcLikelihoodVector        (beh, likelihood);
      tl_message ("step hoge %d.2", i);
      work->CalcDistanceOfInputBehavior (beh, tmp_dis);
      tl_message ("step hoge %d.3", i);
      work->CalcDistanceOfOnlineBehavior(beh, new_dis);
      tl_message ("step hoge %d.4", i);

      work->GetPsymbolSpace()->CoordinateFromDistanceData(tmp_dis, tmp_cord);
      tl_message ("step hoge %d.5", i);
      cout << weight[0] << " , " << weight[1] << " , ";
      for (int j=0; j<(int)(tmp_dis.size()); j++)
	cout << tmp_dis[j] << " , ";
      for (int j=0; j<(int)(new_dis.size()); j++)
	cout << new_dis[j] << " , ";
      for (int j=0; j<(int)(hellinger.size()); j++)
	cout << hellinger[j] << " , ";
      for (int j=0; j<(int)(tmp_cord.size()); j++)
	cout << tmp_cord[j] << " , ";
      cout << endl;

      delete synthesis_hmm;
    }

  delete beh;
  delete hmm1;
  delete hmm2;
  delete hmm3;
  delete synthesis_hmm;
  return TRUE;
}




// Created on 2008-07-17 with copying the test_extra_recognition()
// Test for what's happen when the extrapolated motion patterns are recognized by mimesis
// also for RSJ conf. 2008, 
int test_extra_recognition_by_dtype(DistanceType d_type)
{
  JHMM		*hmm1 = NULL, *hmm2 = NULL, *synthesis_hmm = NULL;
  Behavior	*beh = NULL;
  JHMM		*hmm_vec[2];
  double	weight[2];
  int		debug=0;

  if(debug) tl_message ("start : d_type = %d", d_type);

  hmm1      = new JHMM;
  hmm2      = new JHMM;
  hmm1->Load("./.tmp/punch/punch.hmm");
  hmm2->Load("./.tmp/squat/squat.hmm");
  hmm_vec[0] = hmm1;
  hmm_vec[1] = hmm2;

  WorkSpace *work = NULL;
  work = new WorkSpace(d_type);
  if (script_file[0]==0) { print_usage(); exit(0); }
  work->LoadMotionDB (script_file);
  work->SetHTKUnitsFromMotionDB ();
  work->BeforeRecognize();
  work->DistanceLoad ("../../script/symbol_data/tmp_distance_vector");
  work->SpaceLoad    ("../../script/symbol_data/tmp_dim.spc");

  vector<double>	tmp_dis, new_dis, hellinger, tmp_cord;
  vector<double>	likelihood;

  tl_message ("# of motion db is : %d\n", work->GetNumOfMotionDB() );

  cout << "c_1, c_2, ";
  if (d_type==KL_DIVERGENCE)
    {
      for (int i=0; i<(int)(work->GetNumOfMotionDB()); i++)
	cout << "old_KLdistance[" << i << "] ,";
      for (int i=0; i<(int)(work->GetNumOfMotionDB()); i++)
	cout << "new_KLdivergence[" << i << "] ,";
    }
  else if (d_type==HELLINGER)
    {
      for (int i=0; i<(int)(work->GetNumOfMotionDB()); i++)
	cout << "Hellinger Distance[" << i << "] ,";
    }
  for (int i=0; i<(int)(work->GetPsymbolSpace()->GetDimension() ); i++)
    cout << "x[" << i << "] , ";
  cout << endl;

  for (int i=-10; i<20; i++)
    {
      synthesis_hmm = new JHMM;
      weight[0] = i * 0.1;
      weight[1] = 1 - weight[0];
      synthesis_hmm->InterpolationAny (hmm_vec, weight, 2);
      beh = synthesis_hmm->GenerateBehavior(100, 30);
      
      if (d_type==HELLINGER)
	{
	  work->CalcHellingerDistanceOfOnlineBehavior(beh, hellinger);
	  tl_message ("step hoge %d.1", i);
	  work->GetPsymbolSpace()->CoordinateFromDistanceData(hellinger, tmp_cord);
	  tl_message ("step hoge %d.5", i);
	}
      else if (d_type==KL_DIVERGENCE)
	{
	  work->CalcLikelihoodVector        (beh, likelihood);
	  tl_message ("step hoge %d.2", i);
	  work->CalcDistanceOfInputBehavior (beh, tmp_dis);
	  tl_message ("step hoge %d.3", i);
	  work->CalcDistanceOfOnlineBehavior(beh, new_dis);
	  tl_message ("step hoge %d.4", i);
	  work->GetPsymbolSpace()->CoordinateFromDistanceData(new_dis, tmp_cord);
	  tl_message ("step hoge %d.5", i);
	}

      cout << weight[0] << " , " << weight[1] << " , ";
      if (d_type==KL_DIVERGENCE)
	{
	  for (int j=0; j<(int)(tmp_dis.size()); j++)
	    cout << tmp_dis[j] << " , ";
	  for (int j=0; j<(int)(new_dis.size()); j++)
	    cout << new_dis[j] << " , ";
	}
      else if (d_type==HELLINGER)
	{
	  for (int j=0; j<(int)(hellinger.size()); j++)
	    cout << hellinger[j] << " , ";
	}
      for (int j=0; j<(int)(tmp_cord.size()); j++)
	cout << tmp_cord[j] << " , ";
      cout << endl;

      delete synthesis_hmm;
    }

  delete beh;
  delete hmm1;
  delete hmm2;
  delete synthesis_hmm;
  return TRUE;
}




int test_extrapolation(void)
{
  tl_message ("start");
  JHMM		*hmm1 = NULL, *hmm2 = NULL, *hmm3 = NULL, *extra_hmm = NULL;
  JHMM		*hmm_vec[3];
  double	weight[3];
  Behavior	*beh = NULL;
  hmm1      = new JHMM;
  hmm2      = new JHMM;
  hmm3      = new JHMM;
  extra_hmm = new JHMM;
  hmm1->Load("./.tmp/squat/squat.hmm");
  hmm2->Load("./.tmp/punch/punch.hmm");
  hmm3->Load("./.tmp/kick/kick.hmm");
  hmm1->Show();
  hmm2->Show();
  hmm3->Show();
  hmm_vec[0] = hmm1;   weight[0] = -1.0;
  hmm_vec[1] = hmm2;   weight[1] = -13;
  hmm_vec[2] = hmm3;   weight[2] =  1.3;

  tl_message ("Now generate a extrapolation HMM");
  extra_hmm->InterpolationAny (hmm_vec, weight, 3);
  extra_hmm->Show();
  beh = extra_hmm->GenerateBehavior(100, 30);
  beh->FileOut("interpolation_any.beh");

  delete beh;
  delete hmm1;
  delete hmm2;
  delete hmm3;
  delete extra_hmm;
  return TRUE;
}



int test_for_iros08(void)
{
  tl_message ("start");
  JHMM		*hmm1 = NULL, *hmm2 = NULL, *inter_hmm = NULL, *extra_hmm = NULL;
  JHMM		*hmm_vec[3];
  double	weight[3];
  Behavior	*beh = NULL;
  hmm1      = new JHMM;
  hmm2      = new JHMM;
  inter_hmm = new JHMM;
  extra_hmm = new JHMM;
  hmm1->Load("./.tmp/squat/squat.hmm");
  hmm2->Load("./.tmp/punch/punch.hmm");
  hmm1->Show();
  hmm2->Show();
  hmm_vec[0] = hmm1;   weight[0] = 0.5;
  hmm_vec[1] = hmm2;   weight[1] = 0.5;

  tl_message ("Now generate a interpolation HMM");
  inter_hmm->InterpolationAny (hmm_vec, weight, 2);
  inter_hmm->Show();
  beh = inter_hmm->GenerateBehavior(100, 30);
  beh->FileOut("interpolation_any.beh");

  hmm_vec[0] = hmm1;   weight[0] = -0.5;
  hmm_vec[1] = hmm2;   weight[1] = 1.5;

  tl_message ("Now generate a extrapolation HMM");
  extra_hmm->InterpolationAny (hmm_vec, weight, 2);
  extra_hmm->Show();
  beh = extra_hmm->GenerateBehavior(100, 30);
  beh->FileOut("extrapolation_any.beh");

  delete beh;
  delete hmm1;
  delete hmm2;
  delete inter_hmm;
  delete extra_hmm;
  return TRUE;
}



int test_for_iros08_method2(void)
{
  tl_message ("start");
  JHMM		*hmm1 = NULL, *hmm2 = NULL, *inter_hmm = NULL, *extra_hmm = NULL;
  Behavior	*beh = NULL;
  hmm1      = new JHMM;
  hmm2      = new JHMM;
  inter_hmm = new JHMM;
  extra_hmm = new JHMM;
  hmm1->Load("./.tmp/squat/squat.hmm");
  hmm2->Load("./.tmp/punch/punch.hmm");
  hmm1->Show();
  hmm2->Show();

  tl_message ("Hellinger Distance between squat and punch = %g", hmm1->HellingerDistance(*hmm2));

  tl_message ("Now generate a interpolation HMM");
  inter_hmm->Interpolation (*hmm1, *hmm2, 0.5, 0.5);
  inter_hmm->Show();
  beh = inter_hmm->GenerateBehavior(100, 30);
  beh->FileOut("interpolation_any2.beh");

  tl_message ("Now generate a extrapolation HMM");
  extra_hmm->Interpolation (*hmm1, *hmm2, -1.0, 2.0);
  extra_hmm->Show();
  beh = extra_hmm->GenerateBehavior(100, 30);
  beh->FileOut("extrapolation_any2.beh");

  delete beh;
  delete hmm1;
  delete hmm2;
  delete inter_hmm;
  delete extra_hmm;
  return TRUE;
}





int create_behavior_against_zero ()
{
  JHMM		*hmm_ex = NULL, *hmm_zero=NULL, *hmm_tmp=NULL, *hmm_tmp2=NULL, *hmm=NULL;
  Behavior	*beh=NULL;
  
  hmm_ex    = new JHMM();
  hmm_zero  = new JHMM();
  hmm_tmp   = new JHMM();
  hmm_tmp2  = new JHMM();
  hmm       = new JHMM();

  tl_message ("step 0");
  hmm_zero->SetDefault();
  tl_message ("step 1");
  hmm_zero->Initialize (LEFT_TO_RIGHT);
  beh = hmm_zero->GenerateBehavior(300, 100);
  beh-> FileOut("./zero_hmm.beh");
  hmm_zero->FileOut("./zero_hmm.hmm");
  tl_message ("step 2");
  hmm->Load ("./.tmp/kick/kick.hmm");
  hmm->Verify();
  beh = hmm->GenerateBehavior(GEN_NUM, GEN_NUM_Q);
  beh-> FileOut("./kick31dofs.beh");
  tl_message ("step 3");
  hmm_tmp  = *hmm * 2;
  hmm_tmp->Verify();
  tl_message ("step 4");
  hmm_tmp2 = *hmm_zero * (-1);
  hmm_tmp2->Verify();
  tl_message ("step 5");
  hmm_ex  = *hmm_tmp + *hmm_tmp2;
  hmm_ex->Verify();
  tl_message ("step 6");
  beh = hmm_ex->GenerateBehavior(GEN_NUM, GEN_NUM_Q);
  beh->FileOut ("./from_zero_gene.beh");
  hmm_ex->FileOut ("./from_zero_gene.hmm");
  return TRUE;
}



int create_previous_motion ()
{
  JHMM		*hmm_1 = NULL, *hmm_2=NULL, *hmm_tmp=NULL, *hmm_tmp2=NULL, *hmm=NULL;
  Behavior	*beh=NULL;
  
  hmm_1 = new JHMM();
  hmm_2 = new JHMM();
  hmm_tmp   = new JHMM();
  hmm_tmp2  = new JHMM();
  hmm       = new JHMM();

  tl_message ("step 0");

  hmm_1->Load ("./.tmp/punch/punch.hmm");
  hmm_1->Verify();
  hmm_2->Load ("./.tmp/squat/squat.hmm");
  hmm_2->Verify();


  tl_message ("step 3");
  hmm_tmp  = *hmm_1 * 0.5;
  hmm_tmp->Verify();
  tl_message ("step 4");
  hmm_tmp2 = *hmm_2 * 0.5;
  hmm_tmp2->Verify();
  tl_message ("step 5");
  hmm  = *hmm_tmp + *hmm_tmp2;
  hmm->Verify();
  tl_message ("step 6");

  beh = hmm->GenerateBehavior(GEN_NUM, GEN_NUM_Q);

  beh->FileOut ("./previous.beh");
  return TRUE;
}



Behavior *create_behavior_from_coord (vector<double> coord)
{
  PsymbolSpace  *pspace = NULL;
  vector<double> ratio;
  JHMM		*hmm_ex = NULL, *hmm_tmp=NULL;

  hmm_ex      = new JHMM();
  hmm_tmp     = new JHMM();
  pspace = work->GetPsymbolSpace();
  ratio = pspace->GetRatio(coord);
  fprintf(stderr, "ratio = ");
  for (int i=0; i<NUM_MOTION; i++)
    fprintf(stderr, "%g, ", ratio[i]);
  fprintf(stderr, "\n");

  hmm_ex = *hmm[0] * ratio[0];
  for (int i=1; i<NUM_MOTION; i++)
    {
      tl_message ("calc loop %d  step 1\n", i);
      hmm_tmp = *hmm_ex + *(  *hmm[i] * ratio[i] );
      tl_message ("calc loop %d  step 2\n", i);
      hmm_ex = hmm_tmp;
      tl_message ("calc loop %d  step 3\n", i);
    }
  hmm_ex->Verify();
  return hmm_ex->GenerateBehavior(GEN_NUM, GEN_NUM_Q);
}


// Created on 2006-09-07
// Trying to generate new proto-symbol by exterior division
int generate_exterior_division(void)
{
  Behavior	*behavior = NULL;
  vector<double> tmp_dis, tmp_cord(3);
  vector<double> coord[NUM_MOTION], vec_sum(3);
  PsymbolSpace  *pspace = NULL;
  int		i, j;

  work = new WorkSpace();
  //work->Load       ("../../script/learning_scriptfile_3");
  work->LoadMotionDB       ("../../script/four_motions");
  work->SetHTKUnitsFromMotionDB ();
  //work->SetLabelFromRecogUnit();
  work->BeforeRecognize();
  work->DistanceLoad ("../../script/symbol_data/tmp_distance_vector");
  work->SpaceLoad    ("../../script/symbol_data/tmp_dim.spc");
  pspace = work->GetPsymbolSpace();
  hmm[KICK]   = new JHMM();
  hmm[WALK]   = new JHMM();
  hmm[BANZAI] = new JHMM();
  hmm[SQUAT]  = new JHMM();
#if 0
  hmm[THROW]  = new JHMM();
  hmm[DANCE]  = new JHMM();
#endif
  hmm[KICK]  ->Load ("./.tmp/kick/kick.hmm");
  hmm[WALK]  ->Load ("./.tmp/ashibum/ashibum.hmm");
  hmm[SQUAT] ->Load ("./.tmp/squat/squat.hmm");
  hmm[BANZAI]->Load ("./.tmp/banzai/banzai.hmm");
#if 0
  hmm[THROW] ->Load ("./.tmp/squat/squat.hmm");
  hmm[DANCE] ->Load ("./.tmp/banzai/banzai.hmm");
#endif

  vector<double> cops(3);  // Center of Proto-symbols
  for (i=0; i<3; i++)
    {
      cops[i] = 0.0;
      vec_sum[i] = 0.0;
    }
  tl_message ("step 1");
  for (i=0; i<NUM_MOTION; i++)
    {
      tl_message ("step 2-%d-1", i);
      coord[i].resize(3);
      tl_message ("step 2-%d-2", i);
      behavior  = hmm[i]->GenerateBehavior(GEN_NUM, GEN_NUM_Q);
      tl_message ("step 2-%d-3", i);
      work->CalcDistanceOfInputBehavior(behavior, tmp_dis);
      tl_message ("step 2-%d-4", i);
      pspace->CoordinateFromDistanceData(tmp_dis, coord[i]);
      tl_message ("step 2-%d-5", i);
      for (j=0; j<3; j++) vec_sum[j] += coord[i][j];
    }
  for (i=0; i<NUM_MOTION; i++)
    fprintf (stderr, "Pos of No.%d motion = (%g, %g, %g)\n", i, coord[i][0], coord[i][1], coord[i][2]);
  for (j=0; j<3; j++) cops[j] = vec_sum[j] / NUM_MOTION;
  fprintf (stderr, "Center of proto-symbol = (%g, %g, %g)\n", cops[0], cops[1], cops[2]);
  behavior = create_behavior_from_coord (cops);
  behavior->FileOut ("./center.beh");

  hmm_center  = new JHMM();
  hmm_center->SetDefault();
  hmm_center->Initialize(LEFT_TO_RIGHT);
  hmm_center->Verify();

  // 重心から外側に向かって kick を２倍
  for (j=0; j<3; j++)
    tmp_cord[j] = 2 * (coord[KICK][j] - cops[j]) + cops[j];
  behavior = create_behavior_from_coord (tmp_cord);
  behavior->FileOut ("./output.beh");
  return TRUE;
}





int main(int argc, char **argv)
{
  srand (getpid());
  Common_SetDebug (FALSE);

  get_args(argc, argv);
  //test_for_iros08_method2();
  test_extra_recognition_by_dtype (KL_DIVERGENCE);
  //test_extrapolation();
  //create_previous_motion();
  //generate_exterior_division();
  //create_behavior_against_zero ();
  return TRUE;
}
