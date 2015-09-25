/*
 * PsymbolSpace.h
 * ProtoSymbolSpace Structure
 * Last Updated: 03.08.24
 * Modified Tetsunari Inamura
 */

#ifndef __PSPACE_H__
#define __PSPACE_H__

#include <fstream>
#include <iostream>
#include <vector>
#include <string>

#include "Mds.h"
#include "JHMM.h"

class PsymbolSpace:public MDS
{
  vector<JHMM *> psymbol;
  
 public:
  PsymbolSpace();
  ~PsymbolSpace();

  int	AddPsymbol	(const char* fname);
  JHMM *GetNthPsymbol	(int nth);
  int	Verify		(void);
  //int   Load(ifstream *fin);
};


#endif /* __PSPACE_H__ */
