/*
 * test
 *
 * Last Modified on 2015 Sep 28th by Tetsunari Inamura
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


WorkSpace	*Work = NULL;


int get_args (int argc, char **argv)
{
  int	i;

  for (i=1; i<argc;)
    {
      if (tl_strmember (argv[i], "--debug"))
	{
	  Common_SetDebug (TRUE);
	  i++;
	}
    }
  return TRUE;
}


int test_hmm(void)
{
  tl_message ("start");
#if 0
  // For Learning
  WorkSpace *work = NULL;
  srand (getpid());
  work = new WorkSpace();
  work->LoadMotionDB ("../../script/learning_scriptfile_3");
  work->ExecLearning();
  delete work;
#endif
  JHMM		*hmm = NULL;
  Behavior	*beh = NULL;
  hmm = new JHMM;
  hmm->Load("./.tmp/kick/kick.hmm");
  cerr << "Result of HMM of kick is ...." << endl;
  hmm->Show();

  JHMM		*hmm_new = NULL;
  hmm_new = (*hmm) * 0.5;
  hmm_new->Show();

  // For Generation
  tl_message ("Now generate a behavior from the HMM");
  beh = hmm->GenerateBehavior(100, 30);
  beh->FileOut("tmp_test.beh");
  delete beh;
  delete hmm;

  return TRUE;
}



int test_mds (void)
{

  MDS* mds = NULL;
  mds = new MDS();
  mds->Load("3dim.mds");
  mds->FileOut("3dim_fout.mds");
  delete mds;

#if 0
  double val, last_val, ref;
  ref = 0.01;

  MDS* mds = NULL;
  MDS* mds2 = NULL;
  mds = new MDS();
  //  mds->normalization_constant = 10000;	// Magic Number!!
  mds->SetData("mds_data");
  mds->Junjo();
  mds->DataStandarize();
  mds->SetDimension(3);
  mds->InitCordinate();
  mds->CalcDistance();
  mds->Show();
  val = mds->CalcEvalValue();
  last_val = val + 1;
  //  i = 0;

  
  while(last_val - val >ref){
    last_val = val;
    mds->UpdateCordinate();
    val = mds->CalcEvalValue();
    //    printf("%d times E_val=%f $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n",i,val);
    mds->TargetDistanceUpdate();
    val = mds->CalcEvalValue();
    //    printf("%d times E_val2=%f $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n",i,val);
    //    i++;
  }
  mds->Show();
  mds->FileOut("3dim.mds");
  delete mds;
#endif

#if 0
  vector<double> tmp;
  vector<double> input;
  int rand_i;
  for(int i=0; i<6; i++){
    rand_i = rand()%2000;
    input.push_back((double)rand_i);
  }
  tmp = mds->CordinateFromDistanceData(input);
  //  tmp = mds->InputDataConvertRealDistance(input);
  cerr << "Input :";
  for(int i=0; i<(int)input.size(); i++)
    cerr << input[i] << " " ;
  cerr << endl;
  cerr << "real :";
  for(int i=0; i<(int)tmp.size(); i++)
    cerr << tmp[i] << " " ;
  cerr << endl;
  mds->Show();

  mds2 = new MDS(*mds);
  delete mds;
  mds2->Show();
  delete mds2;
#endif
  return TRUE;
}


void test_behavior (void)
{
  Behavior *beh =NULL, *beh2=NULL;
  char	buf[255];

  cerr << "Please input behavior file for test (you can omit)" << endl;
  fgets (buf, 255, stdin);

  beh = new Behavior();
  if (buf[0]=='\n')
    beh->Load ("../../mdb/banzai/banzai0.beh", 1);
  else
    {
      buf[strlen(buf)-1] = '\0';
      beh->Load (buf, 1);
    }

  if(Gdebug) beh->Verify();
  //beh2 = beh->ElasticCopy (150);
  beh2 = new Behavior(*beh);
  if(Gdebug) beh2->Verify();
  beh2->FileOut ("test.beh");
}


int test_motiondb (void)
{
  MotionDB *m_db = NULL;

  m_db = new MotionDB();
  m_db->Load ("templete/mdb_loadfile_3");
  m_db->LearningHMM ();
  delete m_db;
  
#if 0
  string tmp_name = "mdb/ashibum/";
  m_db->SetDirName(tmp_name);
  tmp_name = "./.tmp/ashibum2/";
  m_db->SetSaveDir(tmp_name);
  tmp_name = "ashibum";
  m_db->SetLabel(tmp_name);
  tmp_name = "ashibum.hmm";
  m_db->SetNumOfSample(24);
  m_db->SetBehaviors();
  m_db->SetHMMFile(tmp_name);
  m_db->SetNumOfMix(5);	
  m_db->SetNumOfState(20);
  m_db->LearningHMM();
#endif

  return TRUE;
}



int test_htkunit (void)
{
  HTKUnit* rec = NULL;
  rec = new HTKUnit();
  //  rec->SetWorkDir("");
  rec->Load("templete/mdb_ashibum");
  //rec->SetLabel("ashibum");
  //rec->SetHMMFile(".tmp/ashibum/ashibum");
  rec->BeforeRecognize();
  
  Behavior* beh = NULL;
  beh = new Behavior();
  beh->Load("mdb/ashibum/ashibum0.beh");
  double result;
  result = rec->Recognize(beh);
  cerr << "result:" << result << endl;
  delete beh;
  delete rec;  

  return TRUE;
}



int test_workspace (void)
{
  tl_message ("start");

  Work = new WorkSpace();
  Work->LoadMotionDB ("../../script/learning_scriptfile_3");
  Work->SetHTKUnitsFromMotionDB();
  //Work->SetLabelFromRecogUnit();
  Work->BeforeRecognize();
  Work->CalcKLDivergenceMatrix();
  Work->SymmentrizeDisVector();
  Work->DistanceFileOut ("../../script/symbol_data/tmp_distance_vector");
  // Work->DisVectorFileOut("temp.txt");
  // Work->DisVectorLoad("templete/distance_vector2");
  tl_message ("step 7");

  // Space の作成
  Work->SpaceCreate(3, KL_DIVERGENCE);  // first arg : # of dimension , second arg : type of distance measurement
  Work->SpaceFileOut("../../script/symbol_data/tmp_dim.spc");



#if 0
  vector<vector <double > >  transition;
  double tmp_pos;
  vector<double> ashibum_pos;
  vector<double> kick_pos;
  ashibum_pos = work->GetXPosition("ashibum");
  kick_pos = work->GetXPosition("kick");
  double h;
  h = 0;
  int traj_num = 100;
  transition.resize(traj_num);
  for(int i=0; i<traj_num; i++){
    for(int j=0; j<5; j++){
      tmp_pos = (ashibum_pos[j] * (1-h)) + (kick_pos[j] * h);
      transition[i].push_back(tmp_pos);
    }
    h += 1/(double)traj_num;
  }
  for(int i=0; i<traj_num; i++){
    cerr << "!!!!! " << i << "th Gene Start!" << endl;
    work->BehGeneFromTransition(transition[i]);
    cerr << "!!!!! " << i << "th Gene Finish!" << endl;
  }
  work->BehBufFileOut("beh_buf.beh");
#endif
  
  //work->SetLabelFromMotionDB();
  //work->DisVectorLoad("templete/distance_vector1");
  //work->SymmentrizeDisVector();
  //work->DisVectorFileOut("templete/distance_vector2");

  //  vector<vector <double > > tmp_mat;
  //  tmp_mat = work->CalcDistanceMatrix();
  //  for(int i=0; i<(int)tmp_mat.size(); i++){
  //    cerr << "mat[" << i << "]" << endl;
  //    for(int j=0; j<(int)tmp_mat[i].size(); j++)
  //      cerr << tmp_mat[i][j] << " ";
  //    cerr << endl;
  //  }

  //work->BeforeRecognize();
  //vector<double> like_vec;
  //htBehavior* beh;
  //beh = new htBehavior();
  //beh->Load("ashibum0.beh");
  //like_vec = work->CalcLikelihoodVector(beh);
  //for(int i=0; i<(int)like_vec.size(); i++){
  //  cerr << like_vec[i] << " ";
  //}

  //  work->LoadLearningScriptFile("templete/learning_scriptfile_3");

  //  work->SetLabelFromMotionDB();
  //  work->SetLabelFromRecogUnit();
  //  htMotionDB* tmp_mdb;
  //  int result;
  //  string tmp;
  //  tmp_mdb = work->GetNthMotionDB(3);
  //  tmp = tmp_mdb->GetLabel();
  //  result = work->GetKey(tmp.c_str());
  //  cout << tmp << ":" << result << endl;
  
  return TRUE;
}


int test_transit_generation (void)
{

  Behavior* tmp_beh;
  Behavior* beh;
  Behavior* beh2;
  // beh に様々な種類の行動を時間軸上でつなげた長い行動を代入
  beh = new Behavior();
  beh->Load("ashibum/ashibum0.beh");
  beh2 = new Behavior();
  beh2->Load("kick/kick0.beh");
  beh->Cat(*beh2);
  delete beh2;
  beh2 = new Behavior();
  beh2->Load("squat/squat0.beh");
  beh->Cat(*beh2);
  delete beh2;
  beh2 = new Behavior();
  beh2->Load("throw/throw0.beh");
  beh->Cat(*beh2);
  delete beh2;
  beh2 = new Behavior();
  beh2->Load("punch/punch0.beh");
  beh->Cat(*beh2);
  delete beh2;
  vector<double> tmp_dis;
  vector<double> tmp_cord;
  int span = 20, step = 5;
  int times;
  times = ( beh->Length() - span)/step;
  cerr << "----------------  times:" << times << endl;
  Pose* tmp_pose;
  // tmp_beh に beh の一部分をすこしずずシフトしながら代入していく
  // tmp_beh の長さは span=20, シフトする量は step=5
  for(int j=0; j<times; j++)
    {
      tmp_beh = new Behavior();
      cerr << "----------------  pop" << endl;
      for(int i=0; i<step; i++)
	tmp_beh->AddPose(beh->PopFrontPose());
      for(int i=0; i<span-step; i++)
	{
	  tmp_pose = new Pose(*(beh->NthPose(i)));
	  tmp_beh->AddPose(tmp_pose);
	}
      cerr << "----------------  CalcDis" << endl;
      // その一部分の動作を空間に射影した場合の，各既知原始シンボルからの距離を計測
      Work->CalcDistanceOfInputBehavior(tmp_beh, tmp_dis);
      delete tmp_beh;
      cerr << "----------------  GetCoordinate" << endl;
      // 求めた距離に応じて，空間に射影するべき座標を算出
      Work->GetPsymbolSpace()->CoordinateFromDistanceData(tmp_dis, tmp_cord);
      cerr << "!!!!! " << j << "th Gene Start!" << endl;
      // その座標における動作を生成する．
      Work->BehGeneFromTransition(tmp_cord);
      cerr << "!!!!! " << j << "th Gene Finish!" << endl;
    }
  Work->BehBufFileOut("beh_buf.beh");
  return TRUE;
}




int main (int argc, char **argv)
{
  char	buf[255];
  srand(getpid());

  tl_message ("mimesis test program");
  get_args (argc, argv);

  while (1)
    {
      cout << "Please input following commands" << endl;
      cout << "    hmm" << endl;
      cout << "    mds" << endl;
      cout << "    workspace" << endl;
      cout << "    transit" << endl;
      cout << "    behavior" << endl;
      cout << "    htkunit" << endl;
      cout << "    quit" << endl;

      fgets (buf, 255, stdin);
      if (tl_strmember (buf, "hmm"))
	test_hmm();
      else if (tl_strmember (buf, "mds"))
	test_mds();
      else if (tl_strmember (buf, "behavior"))
	test_behavior();
      else if (tl_strmember (buf, "htkunit"))
	test_htkunit();
      else if (tl_strmember (buf, "workspace"))
	test_workspace();
      else if (tl_strmember (buf, "transit"))
	test_transit_generation();
      else if (tl_strmember (buf, "quit"))
	break;
    }
  return TRUE;
}
