/*
 * WorkSpace.h
 *
 * Last Updated: 2004 Nov 18th  by inamura
 * Last Updated: 2008-07-17 by inamura
 *
 * 2008-07
 *	added CalcKLDivergenceMatrix and CalcHellingerMatrix instead of CalcDistanceMatrix
 */

#ifndef __WORKSPACE_H__
#define __WORKSPACE_H__

#include <fstream>
#include <iostream>
#include <vector>
#include <string>

#include "mimesis.h"
#include "MotionDB.h"
#include "HTKUnit.h"
#include "PsymbolSpace.h"

typedef enum
{
  KL_DIVERGENCE,
  HELLINGER,
  DISTANCE_UNKNOWN
}
DistanceType;


class WorkSpace
{
  string			work_dir;	// 作業ディレクトリ名
  ifstream			fin;		// スクリプトファイル読み込み用
  vector<MotionDB *>		motion_db;	// MotionDB:HMM学習用(学習フェーズ後は不要)
  vector<HTKUnit *>		htk_units;	// HTKUnit : for interface with HTK, subclass of JHMM
  vector<vector <double > >	matrix_KLD;	// 空間を構成するためのKL距離 (matrix)
  vector<vector <double > >	matrix_HD;	// 空間を構成するためのHellinger距離 (matrix)
  vector<string>		label;		// recog_units, motion_dbの行動のlabel   TODO : ここで label 管理しているのはおかしい
  PsymbolSpace			*space;		// 原始シンボル空間
  int				span;		// 行動を原始シンボル空間で認識するときのタイムスパン
  int				step;		// タイムステップ
  DistanceType			d_type;

 public: // for test
  Behavior* beh_buf;
  // 生成した行動を保持しておくバッファー
  // 行動を原始シンボル空間で生成したりそれを
  // 送り出したりするにつれ随時beh_bufが長さを変える

  WorkSpace();
  WorkSpace(DistanceType type);
  ~WorkSpace();
  
  //  int		LoadLearningScriptFile	(const char *fname);
  int		AddMotionDB		(MotionDB* mdb);
  MotionDB	*NthMotionDB		(int nth);
  int		GetNumOfMotionDB	(void);
  int		AddHTKUnit		(HTKUnit* rec);
  HTKUnit	*GetNthHTKUnit		(int nth);
  int		GetNumOfHTKUnits	(void);
  int		AddPsymbol		(JHMM *new_psymbol);
  //int		SetPsymbols		(void);
  int		SetWorkDir		(const char *dir_name);
  string	GetWorkDir		(void);
  int		LoadMotionDB		(const char *fname);
  int		LoadVersion1		(void);
  int		LoadVersion2		(void);
  int		LoadVersion3		(const char *arg);
  int		LoadScaled		(const char* fname);
  int		FileOut			(const char* fname);
  int		ExecLearning		(void);

  //vector<double> CalcLikelihoodVector	(Behavior *beh);
  int		CalcLikelihoodVector	(Behavior *beh, vector<double> &like_vector);
  int		BeforeRecognize		(void);
  int		CreateWorkDir		(void);
  int		NumOfHTKUnits		(void);

  //vector<vector <double > >	CalcDistanceMatrix(void);
  int		CalcKLDivergenceMatrix	(void);
  int		CalcHellingerMatrix	(void);
  int		SetHTKUnitsFromMotionDB	(void);
  int		LoadHTKUnits		(const char *fname);
  //int		SetDisVector		(void);	//deleted on 2008-07-17
  int		SymmentrizeDisVector	(void);
  int		DistanceFileOut		(const char *fname);
  int		DistanceLoad		(const char *fname);

  int		AddLabel		(const char *name);
  int		AddLabel		(string name);
  int		SetLabelFromMotionDB	(void);
  int		SetLabelFromHTKUnit	(void);
  int		GetKey			(const char *name);
  int		GetKey			(string &name);
  string	& GetNthLabel		(int nth);
  
  int		SpaceCreate				(int dim, DistanceType d_type);
  Behavior*	BehaviorGenerateFromSinglePoint		(vector<double>& pos);
  int		BehGeneFromTransition			(vector<double>& pos);
  int		BehBufFileOut				(const char* fname);
  vector<double>	GetCoordinate			(const char* name);
  //vector<double>	CalcDistanceOfInputBehavior	(Behavior* beh);
  //vector<double>	CalcDistanceOfOnlineBehavior	(Behavior *beh);
  //vector<double>	CalcHellingerDistanceOfOnlineBehavior(Behavior *beh);
  int			CalcDistanceOfInputBehavior	(Behavior* beh, vector<double> &distance);
  int			CalcDistanceOfOnlineBehavior	(Behavior *beh, vector<double> &distance);
  int			CalcHellingerDistanceOfOnlineBehavior(Behavior *beh, vector<double> &distance);

  PsymbolSpace* GetPsymbolSpace		(void);
  int		SpaceFileOut		(const char* fname);
  int		SpaceLoad		(const char* fname);
  int		GetStepTime		(void);
  Behavior*	GetLastBehavior		(void);
  Behavior	*GetBehBuf		(void);
  int		Verify			(void);
};


#endif /* __WORKSPACE_H__ */
