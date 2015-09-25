/*
 * HTKUnit.h
 *
 * Created : Tetsunari Inamura on 2008-01-09
 *
 */

/*
 * A sub-class under basic HMM class.
 * This class is designed for HTK based interface (recognition / learning)
 * If HTK is not adopted, this class would not be used anymore.
 *
 */

#ifndef __HTKUNIT_H__
#define __HTKUNIT_H__

#include <fstream>
#include <iostream>
#include <vector>
#include <string>

#include "Behavior.h"
#include "JHMM.h"


class HTKUnit : public JHMM
{
  string	work_dir;
  string	hmm_file;
  
 public:
  HTKUnit			();
  HTKUnit			(char *label, char *save_dir, char *hmmfile);
  HTKUnit			(string label, string save_dir, string hmmfile);
  ~HTKUnit			(void);
  int		Reset();
  int		SetWorkDir	(string &name);
  int		SetWorkDir	(const char* name);
  string	&GetWorkDir	(void);
  //int		SetLabel	(string &name);
  //int		SetLabel	(const char* name);
  string	GetLabel	(void);  
  int		SetHMMFile	(string &name);
  int		SetHMMFile	(const char* name);
  string	&GetHMMFile	(void);
  int		CreateWorkDir	(void);

  int		NetworkFileOut	(const char* fname);
  int		LatticeFileOut	(const char* fname);
  int		DictFileOut	(const char* fname);
  int		HlistFileOut	(const char* fname);

  int		BeforeRecognize	(void);
  double	Recognize	(Behavior* beh);
  int		LoadFromFile	(const char* fname);
  double	CalcKLDistance	(HTKUnit *target_htk_unit);
  double	CalcHellingerDistance (HTKUnit &target_htk_unit);
};
  
#endif /* __HTKUNIT_H__ */
