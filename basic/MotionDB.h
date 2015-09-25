/*
 * MotionDB.h
 *
 * Last Modified : Tetsunari Inamura on 2008-09-30
 *
 */

#ifndef __MOTIONDB_H__
#define __MOTIONDB_H__

#include <fstream>
#include <iostream>
#include <vector>
#include <string>

#include "JHMM.h"
//#include "Behavior.h"
//#include "mimesis.h"

/* 
 * Mainly, this class is used in learning process of HMM
 * Learning result will be written in file
 * But, the result is not loaded on memory. loading is task of upper class
 *
 * TODO : inheritance of upper class is not conducted
 */

class MotionDB
{
  string	load_dir;	// Directory name which contains data. '/' is required as final char
  string	save_dir;	// Directory name of save target
  string	label;		// Label
  string	hmm_file;	// HMM file name
  int		state;		// number of nodes
  int		bvh_flag;	// 1 means BVH file, 0 means normal .beh file
  int		num_of_mix;
  HmmType	hmm_type;
  //int		num_of_sample;
  int		dimension;
  vector<Behavior *> behaviors;

 public:
  MotionDB		(void);
  MotionDB		(const MotionDB& another);
  MotionDB		(const char* fname);
  ~MotionDB		(void);

  void		Reset		(void);
  void		NumOfState	(int num);
  int		NumOfState	(void);
  void		NumOfMix	(int num);
  int		NumOfMix	(void);
  int		NumOfSample	(void);
  void		LoadDirectory	(string &name);
  void		LoadDirectory	(const char* name);
  string	&LoadDirectory	(void);
  void		SaveDirectory	(string &name);
  void		SaveDirectory	(const char* name);
  string	&SaveDirectory	(void);
  void		Label		(string &name);
  void		Label		(const char* name);
  string       &Label		(void);
  void		HMMFile		(string &name);
  void		HMMFile		(const char* name);
  string	&HMMFile	(void);
  int		HMMFullFile	(string &filename);
  int		BVHFlag		(int flag);
  int		BVHFlag		(void);
  void		HMMtype		(HmmType type);
  HmmType	HMMtype		(void);
  Behavior	*NthBehavior	(int nth);
  void		AddBehavior	(Behavior* new_beh);
  int		IncreaseSamples	(int num, double rate, int debug);
  int		IncreaseSamples	(int num);
  int		AddNoise	(void);
  int		LearningHMM	(void);
  void		CreateHMMSrcFile(void);
  int		MakeContinuousTrainingFiles(void);
  int		LoadBehaviors	(void);
  int		LoadBVH		(void);
  int		SaveBehaviors	(void);
  int		ExecHRest	(void);
  int		TimeFileOut	(void);
  int		CreateSaveDir	(void);
  int		Load		(const char* fname);
  int		Load		(const char* l_dir, const char* s_dir, const char* tmp_label,
				 const char* tmp_hmmfile, int tmp_numofsample, int tmp_numofmix,
				 int tmp_numofstate, HmmType tmp_hmmtype);
  int		Verify		(void);
  int		Verify		(int flag);
};


#endif /* __MOTIONDB_H__ */
