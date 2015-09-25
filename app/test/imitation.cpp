/*
 * Robot Imitation Sample
 *
 * Last modified on 2006 Mar 29th by Tetsunari Inamura 
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
#include <RecogUnit.h>


using namespace std;


int generate_with_symbol_space(void)
{
  
  Behavior	*tmp_beh, *beh;
  WorkSpace	*work = NULL;
  work = new WorkSpace();
  tl_message ("Now loading motionDBs...");
  work->Load          ("../../script/learning_scriptfile_3");
  tl_message ("Now loading recogunits...");
  work->SetRecogUnits ("../../script/learning_scriptfile_3");
  work->SetLabelFromRecogUnit();
  work->BeforeRecognize();
  work->SetDisVector();
  work->SymmentrizeDisVector();
  work->SpaceCreate(3);

  vector<double> tmp_dis;
  vector<double> tmp_cord;
  int span = 20, step = 5;
  Pose *pose;

  cur_beh = new Behavior();
  for(int i=0; i<span; i++)
    {
      data = TOYOTA_GetMotionCaptureData();
      joint_angle = TOYOTA_TransferCaptureToJoint(data);
      pose = new Pose(dof, joint_angle);  // dof = number of DoF, joint_angle : sequence of double
      cur_beh->AddPose(pose);
    }
  for(int j=0; j<1000; j++)
    {
      cerr << "----------------  pop" << endl;
      for(int i=0; i<step; i++)
	{
	  cur_beh->PopFrontPose();
	  data = TOYOTA_GetMotionCaptureData();
	  joint_angle = TOYOTA_TransferCaptureToJoint(data);
	  pose = new Pose(dof, joint_angle);  // dof = number of DoF, joint_angle : sequence of double
	  cur_beh->AddPose(pose);
	}
      cerr << "----------------  CalcDis" << endl;
      // その一部分の動作を空間に射影した場合の，各既知原始シンボルからの距離を計測
      tmp_dis = work->CalcDistanceOfInputBehavior(cur_beh);
      cerr << "----------------  GetCoordinate" << endl;
      // 求めた距離に応じて，空間に射影するべき座標を算出
      tmp_cord = work->GetPsymbolSpace()->CoordinateFromDistanceData(tmp_dis);
      cerr << "!!!!! " << j << "th Gene Start!" << endl;
      // その座標における動作を生成する．
      work->BehGeneFromTransition(tmp_cord);
      cerr << "!!!!! " << j << "th Gene Finish!" << endl;
      tmp_beh = work->GetLastBehavior ();
      for (int i=0; i<step; i++)
	{
	  TOYOTA_ActuateRobot (tmp_beh->NthPose(i));
	}
    }
  return TRUE;
}



int main()
{
  srand (getpid());
  Common_SetDebug(TRUE);

  // モーションキャプチャとの通信の初期化
  // ロボットシミュレータとの通信の初期化
  // データ変換プロセスとの通信の初期化

  generate_with_symbol_space();
  return TRUE;
}
