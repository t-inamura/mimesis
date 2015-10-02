/* 異なる種類の動作を連結させた行動データを作り，
  それに対して「squat」の動作をマッチングさせると，上記の連結行動データの
  うちどの当たりの時間帯にマッチングするか，というのを，尤度 (likelihood) 
  を用いて算出してみるテスト

  Last modified on 2008-01-17 by Tetsunari Inamura
*/


#include <unistd.h>
#include <mimesis.h>
#include <JHMM.h>
#include <Behavior.h>
#include <Mds.h>
#include <MotionDB.h>
#include <WorkSpace.h>
#include <HTKUnit.h>

using namespace std; 

Behavior *make_data();
int calculation(Behavior *beh);


int main()
{
	int      cal;
	Behavior *beh; 

	Common_SetDebug(TRUE);
	beh = make_data();
	cal = calculation(beh);
  
	return 0;
}


Behavior *make_data()
{
	Behavior        *beh, *beh2; 
  
	// beh に様々な種類の行動を時間軸上でつなげた長い行動を代入  
	beh = new Behavior();

	fprintf(stderr, "step0\n");

	beh2 = new Behavior();
	beh2->Load("kick/kick0.beh");
	fprintf(stderr, "step1\n");
	beh->Cat(*beh2);
	fprintf(stderr, "step2\n");
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
  
	beh->FileOut ("./test.beh");
	return beh;
}



int calculation(Behavior *beh)
{
	vector<double>     tmp_dis, tmp_cord;
	vector<double>     result;
	Behavior           *tmp_beh;

	srand(getpid());

	WorkSpace *work = NULL;
	work = new WorkSpace();
	work ->LoadMotionDB ("../../script/learning_scriptfile_3");
	work ->SetHTKUnitsFromMotionDB();
	//work -> SetLabelFromRecogUnit();
	work -> BeforeRecognize();
	//work -> DisVectorLoad("../../script/symbol_data/tmp_distance_vector"); 
	//work -> SpaceLoad("../../script/symbol_data/tmp_distance_vector"); 
   

	//beh2 = new Behavior();
	//beh2->Load("squat/squat1.beh");
	//result = work->CalcLikelihoodVector (beh2);
  

	int span=20, step=5;
	int times;

	ofstream fout("./test_result.dat");
	if(!fout)
		{
			tl_message ("Cannot open file: test_result.dat");
			return FALSE;
		}

	times=(beh->Length() -span)/step;
	fprintf (stderr, "times = %d\n", times);
  
	Pose *tmp_pose;
	for (int j=0; j<times; j++)
		{
			tmp_beh = new Behavior();
			for(int i=0;i<step; i++)
				tmp_beh->AddPose(beh->PopFrontPose());
			for( int i=0; i<span-step;i++){
				tmp_pose= new Pose(*(beh -> NthPose(i)));
				tmp_beh->AddPose(tmp_pose);
			}
      
			work->CalcLikelihoodVector (tmp_beh, result);
			fprintf (stderr, "result[%d] = %g\n", j, result[5]);
		}

	cout << "Completed see test_result.dat" << endl;
	return 0;
}

