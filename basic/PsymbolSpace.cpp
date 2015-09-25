/*
 * Create  : 2003.09.17, 
 * 
 * 
 */


#include "PsymbolSpace.h"

PsymbolSpace::PsymbolSpace()
{

}

PsymbolSpace::~PsymbolSpace()
{
  if ((int)psymbol.size()>0)
    {
      vector<JHMM *>::iterator p=psymbol.begin();
      for (int i=0; i<(int)psymbol.size(); i++)
	{
	  delete *p;
	  p++;
	}
    }
}

/**************************************************
 * Created  : 2003/09/17	
 * Function : Psymbolをファイルから読み込んでセット
 * Memo     : 
 **************************************************/
int PsymbolSpace::AddPsymbol(const char* fname)
{
  int num;

  num = psymbol.size();
  psymbol.resize(num+1);

  psymbol[num] = new JHMM();
  psymbol[num]->Load(fname);

  return TRUE;
}


/**************************************************
 * Created  : 2003/09/18
 * Modified : 2004 May 29th  by inamura
 * Function : n番目のPsymbolを返す
 * Memo     : 
 **************************************************/
JHMM* PsymbolSpace::GetNthPsymbol(int nth)
{
  int	debug=1;

  if(debug&&Gdebug) tl_message ("start");
  if (nth<0 || nth>=(int)psymbol.size())
    {
      tl_warning ("Your request is %d, but the size of psymbols is %d", nth, psymbol.size());
      return NULL;
    }
  if (psymbol[nth]==NULL)
    {
      tl_warning ("Opps! result is NULL");
    }
  if(debug&&Gdebug) tl_message ("end");
  return psymbol[nth];
}


//
// Created : 2005 Aug 30th : by inamua
//
int PsymbolSpace::Verify(void)
{
  int	i;

  for (i=0; i<(int)psymbol.size(); i++)
    psymbol[i]->Verify();

  return 1;  
}
