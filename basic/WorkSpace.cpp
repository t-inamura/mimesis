/*
 * WorkSpace.cpp
 *
 * Last Modified on 2015 Sep 27th by Tetsunari Inamura
 *
 *	Copyright (c) Tetsunari Inamura 2002--2015.
 */

#include <sstream>
#include <string.h>

#include "math.h"
#include "mimesis.h"
#include "WorkSpace.h"


/*
 * ChangeLog
 * 2004 May 27th
 *	Version1,2,3 で読み取る関数を分別する
 * 2006 Aug 31st
 *	LoadLearningScript() を廃止，Load() へ統合
 * 2008-01-17
 *	Rename from RecogUnit to HTKUnit. HTKUnit is a subclass of JHMM.
 *	Creating suitable instance of the superclass JHMM at SetHTKUnits() 
 * 2008-07-16
 *	- Add CalcDistanceOfOnlineBehavior and CalcHellingerDistanceOfOnlineBehavior
 *	- Change argument of CalcDistanceOfInputBehavior
 * 2008-07-17
 *	- Change variable distance_vector to matrix_KLD because it is not vecotr but matrix.
 *	- Introduce of Hellinger Distance
 */


WorkSpace::WorkSpace()
{
  span = DEFAULT_WSPACE_SPAN;
  step = DEFAULT_WSPACE_STEP;
  space = NULL;
  //  beh_buf = new Behavior();//TODO:?
  d_type = HELLINGER;
}


WorkSpace::WorkSpace(DistanceType type)
{
  span = DEFAULT_WSPACE_SPAN;
  step = DEFAULT_WSPACE_STEP;
  space = NULL;
  //  beh_buf = new Behavior();//TODO:?
  d_type = type;
}

// TODO:メンバが変わってから書き換えてない
WorkSpace::~WorkSpace()
{
  int debug=0;

  work_dir.erase();
  //label.clear();
  //label.erase (label.begin(), label.end());
  
  for(int i=0; i<(int)motion_db.size(); i++)
    {
      if(debug&&Gdebug) cerr << "[WorkSpace.~WorkSpace] delete motion_db[" << i << "]" << endl;
      delete motion_db[i];
    }
  motion_db.clear();

  for(int i=0; i<(int)htk_units.size(); i++)
    {
      if(debug&&Gdebug) cerr << "[WorkSpace.~WorkSpace] delete htk_units[" << i << "]" << endl;
      delete htk_units[i];
    }
  htk_units.clear();

#if 0
  for(int i=0; i<(int)psymbol.size(); i++)
    {
      if(debug&&Gdebug) cerr << "[WorkSpace.~WorkSpace] delete psymbol[" << i << "]" << endl;
      delete psymbol[i];
    }
  psymbol.clear();
#endif

  for(int i=0; i<(int)matrix_KLD.size(); i++)
    matrix_KLD[i].clear();
  matrix_KLD.clear();

  for(int i=0; i<(int)matrix_HD.size(); i++)
    matrix_HD[i].clear();
  matrix_HD.clear();
}


/**************************************************
 * Created  : 2004 Sep 12th   by inamura
 * Function : 現在保有している MotionDB 全てについて学習を行う
 *            学習された HMM の結果はこの段階ではまだメモリにロードされない
 **************************************************/
int WorkSpace::ExecLearning(void)
{
  int	debug=0;
  vector<MotionDB *>::iterator p;

  p = motion_db.begin();
  while (p!=motion_db.end())
    {
      while ( (*p)->LearningHMM ()==FALSE )
	{
	  if(debug&&Gdebug) tl_message ("N_of_sample = %d", (*p)->NumOfSample());
	  if ((*p)->NumOfSample()>1000)
	    {
	      tl_message ("Give up! N_of_sample > 1000");
	      return FALSE;
	    }
	  if(debug) tl_message("Increasing samples 50");
	  (*p)->IncreaseSamples(50);
	}
      p++;
    }
  return TRUE;
}



/**************************************************
 * Function : MotionDBを追加する
 * Memo     :
 **************************************************/
int WorkSpace::AddMotionDB(MotionDB* mdb)
{
  if (!mdb)
    {
      tl_warning ("target MotionDB is NULL!");
      return FALSE;
    }
  motion_db.push_back(mdb);

  return TRUE;
}

/**************************************************
 * Function : HTKUnitを追加する
 * Memo     :
 **************************************************/
int WorkSpace::AddHTKUnit(HTKUnit* rec)
{
  if (!rec)
    {
      tl_warning ("target HTKUnit is NULL!");
      return FALSE;
    }
  htk_units.push_back(rec);

  return TRUE;
}


/**************************************************
 * Created  : 2004 May 28th by inamura
 * Function : RecUnit を取得
 **************************************************/
HTKUnit *WorkSpace::GetNthHTKUnit (int nth)
{
  if (nth<0 || nth>=(int)htk_units.size())
    {
      tl_warning ("Your request is %d, but the size of htk_units is %d", nth, (int)htk_units.size());
      return NULL;
    }
  if (htk_units[nth]==NULL)
    tl_warning ("Opps! result is NULL");

  return htk_units[nth];
}


/**************************************************
 * Function : HTKUnitをセット
 * Memo     : 各MotionDBにつき一つずつHTKUnitをセットする
 * Last Modified on 2008-01-17 by inamura
 **************************************************/
int WorkSpace::SetHTKUnitsFromMotionDB(void)
{
  int num;
  int debug=0;

  if(debug&&Gdebug) tl_message ("start");
  num = (int)motion_db.size();
  if (num==0)
    {
      tl_warning("Number of motion data = 0! Cannot set HTKUnit");
      return FALSE;
    }
  htk_units.resize(num);
  string tmp_hmm;

  for(int i=0; i<num; i++)
    {
#if 0
      // Before 2008-01-17
      htk_units[i] = new HTKUnit();
      tmp_hmm = motion_db[i]->SaveDirectory() + motion_db[i]->HMMFile();
      if(debug&&Gdebug) cerr << "[WorkSpace.SetRecUnits] hmm file:" << tmp_hmm << endl;
      htk_units[i]->SetHMMFile(tmp_hmm);
      htk_units[i]->SetLabel(motion_db[i]->Label() );
#else
      // After 2008-01-17
      htk_units[i] = new HTKUnit( motion_db[i]->Label(), motion_db[i]->SaveDirectory(), motion_db[i]->HMMFile() );
#endif
    }
  if(debug&&Gdebug) tl_message ("end");
  return TRUE;
}



#if 1 // 2007-05-14 change 0 to 1
/**************************************************
 * Function : HTKUnitをセット
 * Memo     : ファイルから
 * TODO     : Load メソッドに統合
 **************************************************/
int WorkSpace::LoadHTKUnits(const char* fname)
{
  ifstream	fin;
  int		tmp_numstate, tmp_nummix, tmp_numsample, tmp_label_num, tmp_space_dim;
  int		end_flag = 0;
  char		buf[MAX_STRING], tmpbuf[MAX_STRING], tmp_label[MAX_STRING], tmp_r_dir[MAX_STRING], tmp_s_dir[MAX_STRING], tmp_hmmfile[MAX_STRING],
    		tmp_hmmtype[MAX_STRING], tmp_filename[MAX_STRING], tmp_scale_dir[MAX_STRING];
  HTKUnit	*htk_unit = NULL;
  HmmType	type;
  string	tmp_hmm, tmp_str;
  char		*tmp_label2;
  vector<char *> tmp_label_list;
  vector<int>	tmp_numstate_list;
  vector<int>	tmp_nummix_list;
  vector<int>	tmp_numsample_list;
  char		r_dir_ver3[MAX_STRING], s_dir_ver3[MAX_STRING],hmmfile_ver3[MAX_STRING];
  int		debug=1;

  if(debug&&Gdebug) tl_message ("start");
  complement_dirname (fname, tmp_str);
  if(debug&&Gdebug) tl_message ("now loading script ... : %s", tmp_str.c_str());
  fin.open (tmp_str.c_str(), ifstream::in);
  //tmp_str.clear();
  tmp_str.erase (tmp_str.begin(), tmp_str.end() );

  if(!fin)
    {
      tl_warning ("Cannot open file: '%s'", tmp_str.c_str());
      return FALSE;
    }
  
  fin.getline(buf, MAX_STRING);
  
  if(!(strncmp(buf, "#version1", 9)))
    {
      // #version1
      while(1)
	{
	  if(fin.getline(buf, MAX_STRING) == NULL)
	    {
	      tl_message ("End of file");
	      end_flag = 1;
	      break;
	    }

	  if(!end_flag)
	    {
	      if(strncmp (buf, "label:", 6)) break;

	      sscanf(buf, "%s\t\t%s", tmpbuf, tmp_label);
	      fin.getline(buf, MAX_STRING);
	      if(strncmp (buf, "read_dir:", 9)) break;
	      sscanf(buf, "%s\t%s", tmpbuf, tmp_r_dir);
	      fin.getline(buf, MAX_STRING);
	      if(strncmp (buf, "save_dir:", 9))	break;
	      sscanf(buf, "%s\t%s", tmpbuf, tmp_s_dir);
	      fin.getline(buf, MAX_STRING);
	      if(strncmp (buf, "hmmfile:", 8))	break;
	      sscanf(buf, "%s\t%s", tmpbuf, tmp_hmmfile);
	      fin.getline(buf, MAX_STRING);
	      if(strncmp (buf, "num_state:", 10)) break;
	      sscanf(buf, "%s\t%d", tmpbuf, &tmp_numstate);
	      fin.getline(buf, MAX_STRING);
	      if(strncmp (buf, "num_mixture:", 12)) break;
	      sscanf(buf, "%s\t%d", tmpbuf, &tmp_nummix);
	      fin.getline(buf, MAX_STRING);
	      if(strncmp (buf, "num_sample:", 11)) break;
	      sscanf(buf, "%s\t%d", tmpbuf, &tmp_numsample);
	      fin.getline(buf, MAX_STRING);
	      if(strncmp (buf, "hmm_type:", 9)) break;
	      sscanf(buf, "%s\t%s", tmpbuf, tmp_hmmtype);
	      fin.getline(buf, MAX_STRING);
	      if(debug&&Gdebug)
		{
		  cerr << "buf:" << buf << endl;
		  cerr << "label:" << tmp_label << endl;
		  cerr << "r_dir:" << tmp_r_dir << endl;
		  cerr << "s_dir:" << tmp_s_dir << endl;
		  cerr << "hmmfile:" << tmp_hmmfile << endl;
		  cerr << "state:" << tmp_numstate << endl;
		  cerr << "mix:" << tmp_nummix << endl;
		  cerr << "sample:" << tmp_numsample << endl;
		}
	      htk_unit = new HTKUnit();
	      tmp_str = tmp_s_dir;
	      tmp_hmm = tmp_str + tmp_hmmfile;
	      htk_unit->SetHMMFile(tmp_hmm);
	      htk_unit->SetLabel(tmp_label);
   
	      AddHTKUnit(htk_unit);
	    }
	}
      fin.close();
      return TRUE;
    }

  else if(!(strncmp(buf, "#version2",9)))
    {
      // version2
      while(1)
	{
	  if(fin.getline(buf, MAX_STRING) == NULL)
	    {
	      cerr << "[WorkSpace.Load] NULL" << endl;
	      end_flag = 1;
	      break;
	    }
      
	  if(!end_flag)
	    {
	      if(strncmp (buf, "filename:", 9))
		{
		  if(debug&&Gdebug) cerr << "[WorkSpace.Load] buf:" << buf << endl;
		} else {
		  sscanf(buf, "%s\t\t%s", tmpbuf, tmp_filename);
	  
		  htk_unit = new HTKUnit();
		  htk_unit->LoadFromFile(tmp_filename);
		  AddHTKUnit(htk_unit);
		}
	    }
	}
      fin.close();
      return TRUE;
    } else if(!(strncmp(buf, "#version3",9)))
      {
	if(debug&&Gdebug) cerr << "version3" << endl;
	// version3
	while(1)
	  {
	    if(fin.getline(buf, MAX_STRING) == NULL)
	      {
		cerr << "[WorkSpace.Load] NULL" << endl;
		end_flag = 1;
		break;
	      }
	    if(!end_flag)
	      {
		if(strncmp (buf, "label_num:", 10)) break;
		sscanf(buf, "%s\t%d", tmpbuf, &tmp_label_num);
		if(debug&&Gdebug) cerr << "tmp_label_num:" <<  tmp_label_num << endl;
		if(debug&&Gdebug)
		  { if(tmp_label_num > 10) break; }
		fin.getline(buf,MAX_STRING);
		if(strncmp (buf, "label:", 6)) break;
		if(debug&&Gdebug) cerr << "label:";
		for(int lnum=0; lnum<tmp_label_num; lnum++)
		  {
		    fin.getline(buf,MAX_STRING);
		    tmp_label2 = new char[MAX_STRING];
		    sscanf(buf, "\t%s", tmp_label2);
		    tmp_label_list.push_back(tmp_label2);
		    if(debug&&Gdebug) cerr << tmp_label2 << " ";
		  }
		if(debug&&Gdebug) cerr << endl;
		fin.getline(buf, MAX_STRING);
		if(strncmp (buf, "read_dir:", 9)) break;
		sscanf(buf, "%s\t%s", tmpbuf, tmp_r_dir);
		fin.getline(buf, MAX_STRING);
		if(strncmp (buf, "scale_dir:", 10)) break;
		sscanf(buf, "%s\t%s", tmpbuf, tmp_scale_dir);
		fin.getline(buf, MAX_STRING);
		if(strncmp (buf, "save_dir:", 9)) break;
		sscanf(buf, "%s\t%s", tmpbuf, tmp_s_dir);
		fin.getline(buf, MAX_STRING);
		if(strncmp (buf, "num_state:", 10)) break;
		if(debug&&Gdebug) cerr << "num_state: ";
		for(int lnum=0; lnum<tmp_label_num; lnum++)
		  {
		    fin.getline(buf,MAX_STRING);
		    sscanf(buf, "\t%d", &tmp_numstate);
		    tmp_numstate_list.push_back(tmp_numstate);
		    if(debug&&Gdebug) cerr << tmp_numstate << " ";
		  }
		if(debug&&Gdebug) cerr << endl;
		fin.getline(buf, MAX_STRING);
		if(strncmp (buf, "num_mixture:", 12)) break;
		if(debug&&Gdebug) cerr << "num_mixture: ";
		for(int lnum=0; lnum<tmp_label_num; lnum++)
		  {
		    fin.getline(buf,MAX_STRING);
		    sscanf(buf, "\t%d", &tmp_nummix);
		    tmp_nummix_list.push_back(tmp_nummix);
		    if(debug&&Gdebug) cerr << tmp_nummix << " ";
		  }
		if(debug&&Gdebug) cerr << endl;
		fin.getline(buf, MAX_STRING);
		if(strncmp (buf, "num_sample:", 11)) break;
		if(debug&&Gdebug) cerr << "num_sample: ";
		for(int lnum=0; lnum<tmp_label_num; lnum++)
		  {
		    fin.getline(buf,MAX_STRING);
		    sscanf(buf, "\t%d", &tmp_numsample);
		    tmp_numsample_list.push_back(tmp_numsample);
		    if(debug&&Gdebug) cerr << tmp_numsample << " ";
		  }
		if(debug&&Gdebug) cerr << endl;
		fin.getline(buf, MAX_STRING);
		if(strncmp (buf, "hmm_type:", 9)) break;
		sscanf(buf, "%s\t%s", tmpbuf, tmp_hmmtype);
		fin.getline(buf, MAX_STRING);
		if(strncmp (buf, "space_dim:", 10)) break;
		sscanf(buf, "%s\t%d", tmpbuf, &tmp_space_dim);

		// script file の中の記述から HmmType を求める
		type = query_hmmtype_enum (tmp_hmmtype);
		if ((int)type==FALSE) tl_warning ("HmmType is strange : %s", buf);

		fin.getline(buf, MAX_STRING);
      
		for(int lnum=0; lnum<tmp_label_num; lnum++)
		  {
		    strcpy(r_dir_ver3, tmp_r_dir);
		    strcat(r_dir_ver3, tmp_label_list[lnum]);
		    strcat(r_dir_ver3, "/");
		    strcpy(s_dir_ver3, tmp_s_dir);
		    strcat(s_dir_ver3, tmp_label_list[lnum]);
		    strcat(s_dir_ver3, "/");
		    strcpy(hmmfile_ver3, tmp_label_list[lnum]);
		    strcat(hmmfile_ver3, ".hmm");
		    htk_unit = new HTKUnit();
		    tmp_str = s_dir_ver3;
		    tmp_hmm = tmp_str + hmmfile_ver3;
		    if(debug&&Gdebug) cerr << "tmp_hmm:" << tmp_hmm << endl;
		    if(debug&&Gdebug) cerr << "tmp_label_list[" << lnum << "]:" << tmp_label_list[lnum] << endl;
		    htk_unit->SetHMMFile(tmp_hmm);
		    htk_unit->SetLabel(tmp_label_list[lnum]);
		    AddHTKUnit(htk_unit);
		  }
	      }
	  }
	fin.close();
	return TRUE;
      }
  else
    {
      tl_message ("ERROR");
      fin.close();
      return FALSE;
    }
}
#endif



/**************************************************
 * Function : Psymbolを追加する
 * Memo     :
 **************************************************/
/*
  int WorkSpace::AddPsymbol(HMM *new_psymbol)
  {
  psymbol.push_back(new_psymbol);

  return TRUE;
  }
*/

/**************************************************
 * Function : MotionDBからPsymbolをセットする
 * Memo     : Mapクラスを使うべき?
 *            MotionDBからでなくRecogUnitからセット
 **************************************************/
/*
  int WorkSpace::SetPsymbols()
  {
  JHMM* tmp_psymbol;
  string tmp_str;

  for(int i=0; i<(int)recog_units.size(); i++)
  {
  tmp_str = recog_units[i]->GetHMMFile();
  tmp_psymbol = new JHMM();
  tmp_psymbol->Load(tmp_str.c_str());
  AddPsymbol(tmp_psymbol);
  }

  tmp_str.erase();
  return TRUE;
  }
*/


/**************************************************
 * Function : work_dirのセット
 * Memo     : 
 **************************************************/
int WorkSpace::SetWorkDir(const char* dir_name)
{
  work_dir = dir_name;

  return TRUE;
}

/**************************************************
 * Function : work_dirを返す
 * Memo     : 
 **************************************************/
string WorkSpace::GetWorkDir(void)
{
  return work_dir;
}



/**************************************************
 * Created	: 2004 May 27th by inamura
 * Modified 	: 
 * Function	: Version 1 の文法で書かれたスクリプトファイルを読み込む
 * Memo		: Load や LoadLearningScriptFile から分離モジュール化する
 **************************************************/
int WorkSpace::LoadVersion1 (void)
{
  char		buf[MAX_STRING], tmpbuf[MAX_STRING], label[MAX_STRING], read_dir[MAX_STRING], save_dir[MAX_STRING];
  char		hmmfile[MAX_STRING];
  int		numstate=0, nummix=0, numsample=0;
  int		end_flag = FALSE, debug=1;
  HmmType	type;
  MotionDB*	mdb = NULL;

  if(Gdebug) tl_message ("start");
  while(1) 
    {
      if (fin.getline(buf, MAX_STRING) == NULL)
	{
	  if(Gdebug&&debug) tl_message ("end of file");
	  end_flag = TRUE;
	  break;
	}
      if (!end_flag)
	{
		if (tl_string_getkeyword(buf, (char *)"label:", label)==TL_FALSE) break;
	  
	  fin.getline(buf, MAX_STRING);
	  if (tl_string_getkeyword(buf, (char *)"read_dir:", read_dir)==TL_FALSE) break;
	  
	  fin.getline(buf, MAX_STRING);
	  if (tl_string_getkeyword(buf, (char *)"save_dir:", save_dir)==TL_FALSE) break;
	  
	  fin.getline(buf, MAX_STRING);
	  if (tl_string_getkeyword(buf, (char *)"hmmfile:", hmmfile)==TL_FALSE) break;
	  
	  fin.getline(buf, MAX_STRING);
	  if (tl_string_getkeyword(buf, (char *)"num_state:", tmpbuf)==TL_FALSE) break;
	  tl_atoi(tmpbuf, &numstate);
	  
	  fin.getline(buf, MAX_STRING);
	  if (tl_string_getkeyword(buf, (char *)"num_mixture:", tmpbuf)==TL_FALSE) break;
	  tl_atoi(tmpbuf, &nummix);
	  
	  fin.getline(buf, MAX_STRING);
	  if (tl_string_getkeyword(buf, (char *)"num_sample:", tmpbuf)==TL_FALSE) break;
	  tl_atoi(tmpbuf, &numsample);

	  fin.getline(buf, MAX_STRING);
	  if (tl_string_getkeyword(buf, (char *)"hmm_type:", tmpbuf)==TL_FALSE) break;
	  // script file の中の記述から HmmType を求める
	  type = query_hmmtype_enum (tmpbuf);
	  if ((int)type==FALSE) tl_warning ("HmmType is strange : %s", buf);
	  
	  fin.getline(buf, MAX_STRING);
	  if(debug&&Gdebug)
	    {
	      cerr << "## --------------- check loading result --------------------" << endl;
	      cerr << "## label  :" << label << endl;
	      cerr << "## r_dir  :" << read_dir << endl;
	      cerr << "## s_dir  :" << save_dir << endl;
	      cerr << "## hmmfile:" << hmmfile << endl;
	      cerr << "## state  :" << numstate << endl;
	      cerr << "## mix    :" << nummix << endl;
	      cerr << "## sample :" << numsample << endl;
	      cerr << "## type   :" << tmpbuf << endl;
	      cerr << "## --------------- check loading result --------------------" << endl;
	    }
	  mdb = new MotionDB();
	  mdb->Load (read_dir, save_dir, label, hmmfile, numsample, nummix, numstate, type);
	  
	  AddMotionDB(mdb);
	}
    }
  fin.close();
  if (end_flag!=TRUE) tl_warning ("load failed");
  return end_flag;
}



/**************************************************
 * Created	: 2004 May 27th by inamura
 * Modified 	: 
 * Function	: Version 2 の文法で書かれたスクリプトファイルを読み込む
 * Memo		: Load や LoadLearningScriptFile から分離モジュール化する
 **************************************************/
int WorkSpace::LoadVersion2 (void)
{
  int		end_flag=FALSE, debug = 1;
  char		buf[MAX_STRING], tmpbuf[MAX_STRING];
  MotionDB*	mdb = NULL;

  if(debug&&Gdebug) cerr << "## version2" << endl;
  while(1)
    {
      if(fin.getline(buf, MAX_STRING) == NULL)
	{ end_flag = TRUE;   break; }
      if (!end_flag)
	{
	  if (tl_string_getkeyword(buf, (char *)"filename:", tmpbuf)==TL_FALSE) break;
	  mdb = new MotionDB();
	  mdb->Load(tmpbuf);
	  if(debug&&Gdebug) tl_message ("Add: %s", tmpbuf);
	  AddMotionDB(mdb);
	}
    }
  fin.close();
  if (end_flag!=TRUE) tl_warning ("load failed");
  return TRUE;
}




/**************************************************
 * Created	: 2004 May 27th by inamura
 * Modified 	: 
 * Function	: Version 3 の文法で書かれたスクリプトファイルを読み込む
 * Memo		: Load や LoadLearningScriptFile から分離モジュール化する
 **************************************************/
int WorkSpace::LoadVersion3 (const char *arg)
{
  int		label_num, nummix, numsample, numstate, space_dim;
  char		buf[MAX_STRING], tmpbuf[MAX_STRING];
  string	*newstring;
  char		read_dir[MAX_STRING], scale_dir[MAX_STRING], save_dir[MAX_STRING], typestr[MAX_STRING];
  MotionDB*	mdb = NULL;
  HmmType	hmmtype;
  int		end_flag=FALSE, debug = 1;
  vector<string *>label_list;
  vector<int>	numstate_list;
  vector<int>	nummix_list;
  vector<int>	numsample_list;

  if(debug&&Gdebug) tl_message ("start");
  while(1)
    {
      if(fin.getline(buf, MAX_STRING) == NULL)
	{ end_flag = TRUE;  break;  }
      if (end_flag==TRUE)
	{
	  if (tl_string_getkeyword(buf, (char *)"label_num:", tmpbuf)==TL_FALSE) break;
	  tl_atoi (tmpbuf, &label_num);

	  fin.getline(buf,MAX_STRING);
	  if (tl_strmember (buf, "label:")==TL_FALSE) break;
	  for (int lnum=0; lnum<label_num; lnum++)
	    {
	      fin.getline(buf,MAX_STRING);
	      //label2 = new char[MAX_STRING];     // TODO : メモリもったいない
	      sscanf(buf, "\t%s", tmpbuf);
	      newstring = new string (tmpbuf);
	      label_list.push_back (newstring);
	      if(debug&&Gdebug) tl_message ("label %s is added", newstring->c_str());
	    }
	  if(debug&&Gdebug) cerr << endl;
	  fin.getline(buf, MAX_STRING);
	  if (tl_string_getkeyword(buf, (char *)"read_dir:", read_dir)==TL_FALSE) break;

	  fin.getline(buf, MAX_STRING);
	  if (tl_string_getkeyword(buf, (char *)"scale_dir:", scale_dir)==TL_FALSE) break;

	  fin.getline(buf, MAX_STRING);
	  if (tl_string_getkeyword(buf, (char *)"save_dir:", save_dir)==TL_FALSE) break;

	  fin.getline(buf, MAX_STRING);
	  if (tl_strmember(buf, "num_state:")==TL_FALSE) break;
	  for(int lnum=0; lnum<label_num; lnum++)
	    {
	      fin.getline(buf,MAX_STRING);
	      sscanf(buf, "\t%d", &numstate);
	      numstate_list.push_back(numstate);
	      if(debug&&Gdebug) cerr << numstate << " ";
	    }
	  if(debug&&Gdebug) cerr << endl;
	  fin.getline(buf, MAX_STRING);
	  if (tl_strmember(buf, "num_mixture:")==TL_FALSE) break;
	  if(debug&&Gdebug) cerr << "num_mixture: ";
	  for(int lnum=0; lnum<label_num; lnum++)
	    {
	      fin.getline(buf,MAX_STRING);
	      sscanf(buf, "\t%d", &nummix);
	      nummix_list.push_back (nummix);
	      if(debug&&Gdebug) cerr << nummix << " ";
	    }
	  if(debug&&Gdebug) cerr << endl;
	  fin.getline(buf, MAX_STRING);
	  if (tl_strmember(buf, "num_sample:")==TL_FALSE) break;
	  if(debug&&Gdebug) cerr << "num_sample: ";
	  // TODO : 本当はこの数字は記述する必要は無いし，書くべきではない
	  for(int lnum=0; lnum<label_num; lnum++)
	    {
	      fin.getline(buf,MAX_STRING);
	      sscanf(buf, "\t%d", &numsample);
	      numsample_list.push_back (numsample);
	      if(debug&&Gdebug) cerr << numsample << " ";
	    }
	  if(debug&&Gdebug) cerr << endl;
	  fin.getline(buf, MAX_STRING);
	  if (tl_string_getkeyword(buf, (char *)"hmm_type:", typestr)==TL_FALSE) break;
	  hmmtype = query_hmmtype_enum (typestr);
	  if ((int)hmmtype==FALSE) tl_warning ("HmmType is strange : %s", buf);

	  fin.getline(buf, MAX_STRING);
	  if (tl_string_getkeyword(buf, (char *)"space_dim:", tmpbuf)==TL_FALSE) break;
	  tl_atoi (tmpbuf, &space_dim);

	  fin.getline(buf, MAX_STRING);
      
	  for(int lnum=0; lnum<label_num; lnum++)
	    {
	      string	read_file, save_file, hmmfile;
	      if (tl_strmember(arg, "scaled"))
		{
		  read_file = read_dir;
		  read_file += *(label_list[lnum]);
		  read_file += "/";
		}
	      else if (tl_strmember(arg, "normal"))
		{
		  read_file = read_dir;
		  read_file += *(label_list[lnum]);
		  read_file += "/";
		}
	      else {
		tl_warning ("arg is strange : %s", arg);
		fin.close();
		return FALSE;
	      }
	      save_file = save_dir;
	      save_file += *(label_list[lnum]);
	      save_file += "/";
	      hmmfile  = *(label_list[lnum]);
	      hmmfile += ".hmm";
	      mdb = new MotionDB();
	      mdb->Load (read_file.c_str(), save_file.c_str(), &(*(label_list[lnum])->c_str()),
			 hmmfile.c_str(), numsample_list[lnum],
			 nummix_list[lnum], numstate_list[lnum], hmmtype);
	      AddMotionDB(mdb);
	    }
	}
    }
  fin.close();
  if (end_flag!=TRUE) tl_warning ("load failed");
  if(debug&&Gdebug) tl_message ("load succeeded");
  return end_flag;
}



/**************************************************
 * Function	: スクリプトファイルを読み込む
 * Memo		: version 1, 2, 3 違いを把握し，別関数に分離
 * Memo		: renamed as LoadMotionDB from Load on 2008-01-17
 * Last Modified 2008-01-17 by inamura
 **************************************************/
int WorkSpace::LoadMotionDB(const char* fname)
{
  char		buf[MAX_STRING];
  vector<char *>tmp_label_list;
  vector<int>	tmp_numstate_list;
  vector<int>	tmp_nummix_list;
  vector<int>	tmp_numsample_list;
  string	tmp_str;
  int		debug=1;

  if(debug&&Gdebug) tl_message ("start");
  complement_dirname (fname, tmp_str);
  if(debug&&Gdebug) tl_message ("now loading script ... : <%s>", tmp_str.c_str());
  fin.open (tmp_str.c_str(), ifstream::in);

  if(!fin)
    {
      perror ("<WorkSpace>::LoadMotionDB");
      tl_message ("cannot open file : %s", tmp_str.c_str());
      return FALSE;
    }
  
  fin.getline(buf, MAX_STRING);
  if(debug&&Gdebug) tl_message ("checking version ... %s", buf);
  if (tl_strmember (buf, "#version1"))
    LoadVersion1 ();
  else if (tl_strmember(buf, "#version2"))
    LoadVersion2 ();
  else if (tl_strmember(buf, "#version3"))
    LoadVersion3 ("normal");
  // 2005 Aug 30th: *.spc files don't includes any version info. on euslisp sample, so following code is for normal euslisp version. by inamura
  else
    tl_message ("Unknown version : %s", buf);

  return TRUE;
}



/**************************************************
 * Created  : 2005 Jun 15th by inamura
 * Function : WorkSpace の状態をファイルダンプする
 * Memo     : Not implemented yet
 **************************************************/
int WorkSpace::FileOut(const char* fname)
{
  return TRUE;
}




/**************************************************
 * Function : Recognizeなどをするまえの準備
 * Memo     :
 **************************************************/
int WorkSpace::BeforeRecognize()
{
  for(int i=0; i<(int)htk_units.size(); i++)
    {
      htk_units[i]->BeforeRecognize();
    }

  return TRUE;
}


/**************************************************
 * Function : WorkDirを作成
 * Memo     : WorkDirが既に存在していてもよい．
 **************************************************/
int WorkSpace::CreateWorkDir()
{
  string tmp;
  string com;
  int debug=0;
  
  if(debug&&Gdebug) tl_message ("Creating WorkDir");
  
  tmp = "mkdir -p ";
  com = tmp + work_dir;

  if(debug&&Gdebug) tl_message ("com: %s", com.c_str());
  if (system(com.c_str())==-1)
	  tl_warning("system: [%s] returns error", com.c_str());
  
  tmp.erase();
  com.erase();

  return TRUE;
}


//****************************************************************************************************
//* Created on 2008-01-17
//****************************************************************************************************
int WorkSpace::NumOfHTKUnits (void)
{
  return htk_units.size();
}


/****************************************************************************************************
 * Function : behの各MotionDBに対する尤度を計算し，その尤度ベクトルを返す．
 * Input           : Behavior *beh
 * Output Result   : vector<double> : distances between each HMM
 * Memo            : vector<double> が返値だったのを訂正し，vector<double> &like_vector で受けることにする 2008-07-16
 **************************************************/
int WorkSpace::CalcLikelihoodVector (Behavior *beh, vector<double> &like_vector)
{
  int		debug=0;
  int		num, i;
  //vector<double> like_vector;

  like_vector.resize(0);
  num = htk_units.size();    // TODO : 空間を構成する基底原始シンボルの数はここで管理するべきではない
  if(debug) tl_message ("step 1 : htk_size = %d", num);
  //num = (int)space->GetNumOfData();
  for(i=0; i<num; i++)
    {
      if(debug) tl_message ("step 2.%d", i);
      like_vector.push_back(htk_units[i]->Recognize(beh));
    }
  if(debug) tl_message ("end");
  //return like_vector;
  return TL_TRUE;
}



/**************************************************
 * Modified : 2008-07-17 by inamura KLDである事を明記するように，また old: SetDisVector と融合
 * Function : 各MotioinDB間の距離のマトリクスを作成（非対称である事に注意）
 * Memo     : MotionDBがBehを保持している場合のバージョン
 * Memo     : MotionDBが無い場合のバージョンは？-->HTKunit.cpp での ClacKLDistance
 **************************************************/
int WorkSpace::CalcKLDivergenceMatrix(void)
{
  //vector<vector <double > > matrix;    <- final outpu as distance_vector (old)
  int num;
  int debug=1;
  num = motion_db.size();

  vector<double > tmp_vector;
  vector<vector <vector <double > > > tmp_matrix;
  Behavior* tmp_beh;
  double tmp11, tmp12;
  double dis1;
  int length1;

  if(debug&&Gdebug) tl_message ("start");
  tmp_matrix.resize(num);
  for(int i=0; i<num; i++)
    {
      for(int j=0; j<motion_db[i]->NumOfSample(); j++)
	{
	  tmp_beh = motion_db[i]->NthBehavior(j);
	  CalcLikelihoodVector (tmp_beh, tmp_vector);
	  tmp_matrix[i].push_back(tmp_vector);
	  if(debug&&Gdebug) tl_message ("loop (%d,%d)", i, j);
	}
    }
  matrix_KLD.resize(num);
  for(int i=0; i<num; i++)
    matrix_KLD[i].resize(num);

  for (int key1 = 0; key1<num; key1++)
    {
      for (int key2=0; key2<num; key2++)
	{
	  dis1=0;
	  for (int i=0; i< motion_db[key1]->NumOfSample(); i++)
	    {
	      tmp11   = tmp_matrix[key1][i][key1];
	      tmp12   = tmp_matrix[key1][i][key2];
	      tmp_beh = motion_db[key1]->NthBehavior(i);
	      length1 = tmp_beh->Length();
	      dis1   += (tmp11 - tmp12)/length1;
	      if(debug&&Gdebug) tl_message ("loop (%d,%d,%d)", key1, key2, i);
	    }
	  dis1 = dis1/motion_db[key1]->NumOfSample();
	  //matrix[key1][key2] = (dis1)*log(10.0);    // comment out by inamura on 2008-07-14
	  matrix_KLD[key1][key2] = dis1;
	}
    }

  tmp_vector.clear();
  for(int i=0; i<(int)tmp_matrix.size(); i++)
    {
      for(int j=0; j<(int)tmp_matrix[i].size(); j++)
	{
	  tmp_matrix[i][j].clear();
	  if(debug&&Gdebug) tl_message ("final loop (%d,%d)", i, j);
	}
      tmp_matrix[i].clear();
    }
  tmp_matrix.clear();
      
  if(debug&&Gdebug) tl_message ("end");

  return TL_TRUE;
}



/**************************************************
 * Created  : 2008-07-17 by inamura
 * Function : 各MotionDB 間の Hellinger距離のマトリクスを作成
 * 		結果は slot variable, matrix_HD に格納
 **************************************************/
int WorkSpace::CalcHellingerMatrix(void)
{
  int num;
  int debug=1;

  num = motion_db.size();


  if(debug&&Gdebug) tl_message ("start");

  matrix_HD.resize(num);
  for(int i=0; i<num; i++)
    matrix_HD[i].resize(num);

  for (int key1 = 0; key1<num; key1++)
    {
      for (int key2=0; key2<num; key2++)
	{
	  matrix_HD[key1][key2] = htk_units[key1]->CalcHellingerDistance (*htk_units[key2]);
	  if(debug&&Gdebug) tl_message ("loop (%d,%d)", key1, key2);
	}
    }
      
  if(debug&&Gdebug) tl_message ("end");

  return TL_TRUE;
}




#if 0
/**************************************************
 * Function : distance_vectorをセットする．
 * Memo     : 
 **************************************************/
int WorkSpace::SetDisVector()
{
  distance_vector = CalcDistanceMatrix();

  return TRUE;
}
#endif


/**************************************************
 * Function : KLDistance Matrix を対称形に変える
 * Memo     :
 **************************************************/
int WorkSpace::SymmentrizeDisVector()
{
  int num;
  double tmp;

  num = matrix_KLD.size();

  for(int i=0; i<num; i++)
    {
      for(int j=0; j<i; j++)
	{
	  tmp = matrix_KLD[i][j] + matrix_KLD[j][i];
	  tmp /= 2.0;
	  matrix_KLD[i][j] = tmp;
	  matrix_KLD[j][i] = tmp;
	}
    }

  return TRUE;
}


/**************************************************
 * Function : 距離matrix をファイルに出力
 * Memo     : なんでこのファイルが別になっているのだ？一緒になっているべきでは？あるいはPsymbolクラスで管理すべき
 **************************************************/
int WorkSpace::DistanceFileOut(const char* fname)
{
  int num;
  int debug=1;
  ofstream fout(fname);
  string tmp;

  if (d_type == KL_DIVERGENCE)
    fout << "#type KL-Divergence" << endl;
  else if (d_type == HELLINGER)
    fout << "#type Hellinger" << endl;
  else
    fout << "#type Unknown" << endl;
  
  num = (int)GetNumOfMotionDB();
  fout << "#num_of_data " << num << endl;

  if(debug)
    {
      tl_message ("just a test message");
      for(int i=0; i<num; i++)
	cerr << GetNthLabel(i) << endl;
    }

  for(int i=0; i<num; i++)
    {
      if(debug&&Gdebug) tl_message ("%d-th loop", i);
      //    fout << motion_db[i]->GetLabel() << " ";
      tmp = GetNthLabel(i);
      if(debug&&Gdebug) tl_message ("label: %s", tmp.c_str());
      fout << tmp << " ";
      for(int j=0; j<num; j++)
	{
	  if(debug&&Gdebug) tl_message ("loop %d-%d", i, j);
	  if (d_type == KL_DIVERGENCE)
	    {
	      if(debug&&Gdebug) tl_message ("KLD distance[%d][%d]", i, j);
	      if(debug&&Gdebug) tl_message ("distance[%d][%d] = %g", i, j, matrix_KLD[i][j]);
	      fout << " " << matrix_KLD[i][j];
	    }
	  else if (d_type == HELLINGER)
	    {
	      if(debug&&Gdebug) tl_message ("Hellinger distance[%d][%d]", i, j);
	      fout << " " << matrix_HD[i][j];
	    }
	  else
	    {
	      if(debug&&Gdebug) tl_message ("wrong type...%d", d_type);
	      fout << " Nan";
	    }
	}
      fout << endl;
    }

  fout.close();

  return TRUE;
}


/**************************************************
 * Function : 距離matrix をファイルから読み込む
 * Memo     : なんでこのファイルが別になっているのだ？一緒になっているべきでは？あるいはPsymbolクラスで管理すべき
 **************************************************/
int WorkSpace::DistanceLoad(const char* fname)
{
  char		buf[MAX_STRING], tmpbuf[MAX_STRING];
  double	tmp_double;
  int		num;
  int		debug=0;
  ifstream	fin;
  string	tmp_str;

  complement_dirname (fname, tmp_str);
  fin.open (tmp_str.c_str(), ifstream::in);

  fin.getline(buf, MAX_STRING);
  if (tl_strmember (buf, "KL-Divergence"))
    d_type = KL_DIVERGENCE;
  else if (tl_strmember (buf, "Hellinger"))
    d_type = HELLINGER;
  else 
    d_type = DISTANCE_UNKNOWN;

  fin.getline(buf, MAX_STRING);
  if(strncmp(buf, "#num_of_data", 12))
    {
      tl_message ("(ERROR) Not Entry \"#num_of_data\", but <%s> comes", buf);
      fin.close();
      return FALSE;
    }
  
  sscanf(buf, "%s %d", tmpbuf, &num);
  matrix_KLD.resize(num);
  for(int i=0; i<num; i++)
    {
      fin >> tmpbuf;
      if(debug&&Gdebug) tl_message ("Load from <%s> start", tmpbuf);
      for(int j=0; j<num; j++)
	{
	  fin >> tmp_double;
	  matrix_KLD[i].push_back(tmp_double);
	}
    }  

  fin.close();

  if(debug&&Gdebug) tl_message ("Finish!");
  return TRUE;
}



/**************************************************
 * Function : map labelにnameをpairで新たに追加する
 * Memo     : 
 **************************************************/
int WorkSpace::AddLabel(const char* name)
{
  string tmp;
  tmp = name;
  label.push_back(tmp);
  return TRUE;
}

/**************************************************
 * Function : map labelにnameをpairで新たに追加する
 * Memo     : 
 **************************************************/
int WorkSpace::AddLabel(string name)
{
  label.push_back(name);
  return TRUE;
}


/**************************************************
 * Function : MotionDBからLabelを読み取りセット 
 * Memo     : 
 **************************************************/
int WorkSpace::SetLabelFromMotionDB(void)
{
  int num;

  num = motion_db.size();

  for(int i=0; i<num; i++)
    {
      AddLabel(motion_db[i]->Label());
    }

  return TRUE;
}


/**************************************************
 * Function : RecogUnitからLabelを読み取りセット
 * Memo     : 
 **************************************************/
int WorkSpace::SetLabelFromHTKUnit(void)
{
  int num;

  num = htk_units.size();
  for(int i=0; i<num; i++)
    {
      AddLabel(htk_units[i]->GetLabel());
    }
  return TRUE;
}



/**************************************************
 * Function : Labelに対応する番号を返す
 * Memo     : 
 **************************************************/
int WorkSpace::GetKey(const char* name)
{
  string tmp;
  int i;

  tmp = name;
  for(i=0; i<(int)label.size(); i++)
    {
      if(label[i]==tmp)
	break;
    }
  
  return i;
}


/**************************************************
 * Function : Labelに対応する番号を返す
 * Memo     : 
 **************************************************/
int WorkSpace::GetKey(string& name)
{
  int i;

  for(i=0; i<(int)label.size(); i++)
    {
      if(label[i]==name)
	break;
    }
  return i;
}
  

/**************************************************
 * Modified : 2008-09-30 by inamura
 * Function : n番目のmotion_dbへのポインタを返す
 * Memo     : Get という接頭語を抜く
 **************************************************/
MotionDB* WorkSpace::NthMotionDB(int nth)
{
  if (nth<0 || nth>=(int)motion_db.size())
    {
      tl_message ("input nth(=%d) is wrong!", nth);
      return NULL;
    }

  return motion_db[nth];
}



/**************************************************
 * Created  : 2004.Jan.9 by marika
 * Function : MotionDBの個数を返す
 * Memo     : 
 **************************************************/
int WorkSpace::GetNumOfMotionDB(void)
{
  return motion_db.size();
}

/**************************************************
 * Created  : 2005.June.23 by sonoda
 * Function : HTKUnitの個数を返す
 * Memo     : 
 **************************************************/
int WorkSpace::GetNumOfHTKUnits(void)
{
  return htk_units.size();
}



/**************************************************
 * Function : N番目のLabelを返す
 * Memo     : 0から数える
 **************************************************/
string& WorkSpace::GetNthLabel(int nth)
{
  return label[nth];  
}


/**************************************************
 * Function : PsymbolSpaceの作成
 * Memo     : 
 **************************************************/
int WorkSpace::SpaceCreate(int dim, DistanceType d_type)
{
  int		num;
  double	val, last_val, ref;
  int		debug=1;
  
  if (d_type != KL_DIVERGENCE && d_type != HELLINGER)
    {
      tl_warning ("wrong distance_type <%d>", (int)d_type);
      return TL_FAIL;
    }
  if(debug&&Gdebug) tl_message ("start");
  
  num = GetNumOfMotionDB();
  if(debug&&Gdebug) tl_message ("num = %d", num);

  if (space!=NULL)
    delete space;		// added on 2008-11-11
  
  space = new PsymbolSpace();
  for(int i=0; i<num; i++)
    {
      if (d_type==KL_DIVERGENCE)
	space->AddData(matrix_KLD[i]);
      else if (d_type==HELLINGER)
	space->AddData(matrix_HD[i]);
    }
  if(debug&&Gdebug) tl_message ("AddData finished");
  
  // HMMのセットをする(まだ未完)
  for(int i=0; i<num; i++)
    {
      space->AddPsymbol((htk_units[i]->GetHMMFile()).c_str());
    }
  if(debug&&Gdebug) tl_message ("AddPsymbol finished");

  space->Junjo();
  if(debug&&Gdebug) tl_message ("Junjo finished");
  space->DataStandarize();
  if(debug&&Gdebug) tl_message ("DataStandarize finished");
  space->SetDimension(dim);
  if(debug&&Gdebug) tl_message ("SetDimension finished");
  space->InitCoordinate();
  if(debug&&Gdebug) tl_message ("InitCoordinate finished");
  space->CalcDistance();
  if(debug&&Gdebug) tl_message ("CalcDistance finished");
  
  val = space->CalcEvalValue();
  last_val = val + 1;
  ref = 0.000001;	// TODO : Magic Number
  if(debug&&Gdebug) tl_message ("init CalcEvalValue finished");

  while(last_val - val >ref)
    {
      last_val = val;
      for(int i=0; i<3; i++)		// What's this???
	space->UpdateCoordinate();
      val = space->CalcEvalValue();
      space->TargetDistanceUpdate();
      val = space->CalcEvalValue();
    }

  if(debug&&Gdebug) tl_message ("CalcEvalValue finished");

  return TRUE;
}


/**************************************************
 * Function : beh_bufferをファイルアウト
 * Memo     : 
 **************************************************/
int WorkSpace::BehBufFileOut(const char* fname)
{
  
  beh_buf->FileOut(fname);
  
  return TRUE;
}



/**************************************************
 * Function : 原始シンボル空間上の点を与えて行動生成
 * Modified : 2008-09-30 by inamura : type_tag を付け加えるように
 **************************************************/
Behavior *WorkSpace::BehaviorGenerateFromSinglePoint(vector<double>& pos)
{
  static vector<double>	max_ratio;
  static vector<int>	max_index;
  vector<double>	ratio2;
  int			num;
  int			i, index, key;
  double		now_sum, sum;
  JHMM			*new_monte, *tmp_monte;
  Behavior		*output_beh;
  int			debug=0;

  num = htk_units.size();
  max_ratio.resize(num);
  max_index.resize(num);
  
  now_sum = 0;
  index = 0;
  key = 0;
  for(i = 0; i < num; i++)
    max_ratio[i] = 0;
  
  ratio2 = space->GetRatio(pos);
    
  while(now_sum < 0.90)
    { // TODO:Magic Number!
      for(int l=0; l<num; l++)
	{
	  if(index > 0)
	    {
	      for(int t=0; t<index; t++)
		if(max_index[t] == l) key = 1;
	    }
	  if(key);
	  else if(ratio2[l] > max_ratio[index])
	    {
	      max_index[index] = l;
	      max_ratio[index] = ratio2[l];
	    }
	  key = 0;
	}
      now_sum += max_ratio[index];
      index++;
    }

  if(index!=1)
    {
      if(debug) tl_message ("index = %d", index);
      now_sum = max_ratio[0];
      new_monte = new JHMM(*(space->GetNthPsymbol(max_index[0])));
      for(int l=0; l<index-1; l++)
	{
	  sum = now_sum + max_ratio[l+1];
	  if(debug) 
	    {
	      tl_message ("begin of loop");
	      tl_message ("new_monte = %p, now_sum = %g, sum = %g, max_ratio[l+1] = %g", new_monte, now_sum, sum, max_ratio[l+1]);
	    }
	  tmp_monte = (*(*new_monte * (now_sum/sum))) + (*(*(space->GetNthPsymbol(max_index[l+1])) * (max_ratio[l+1]/sum)));
	  if(debug) tl_message ("middle of loop");
	  delete new_monte;
	  new_monte = tmp_monte;
	  now_sum = sum;
	  if(debug) tl_message ("end of loop");
	}
      output_beh = new_monte->GenerateBehavior(GEN_NUM, GEN_NUM_Q);
      delete new_monte;
    } 
  else
    {
      output_beh = space->GetNthPsymbol(max_index[0])->GenerateBehavior(GEN_NUM, GEN_NUM_Q);
    }
  output_beh->TypeTags (NthMotionDB(0)->NthBehavior(0)->TypeTags());
  return output_beh;
}



/**************************************************
 * Function : 原始シンボル空間上の点を与えて行動生成
 * Memo     : staticでbehをいくつか保持させておき，
 *            過去のbehと混ぜ合わせて生成していく．
 **************************************************/
int WorkSpace::BehGeneFromTransition(vector<double>& pos)
{
  static vector<Behavior *> buffer; // 過去の周期をそろえたbehを保持
  static int phase = 0; //1周期を100とした時の現在の位相
  static int flag = 0; //初めてこの関数を呼び出すかどうか
  // 毎回resizeするのはめんどいのでstaticにしてしまおう
  static vector<double> max_ratio;
  //static vector<double> max_index; by inamura
  static vector<int> max_index;
  vector<double> ratio2;
  static int nums;
  static int dof;
  static int num;

  int		i, new_step, index, key;
  int		buf_size, length;
  double	now_sum, sum;
  JHMM		*new_monte, *tmp_monte;
  Behavior	*mix_beh,   *new_beh;
  Pose		*tmp_pose = NULL;
  Behavior	*tmp_beh = NULL, *add_beh;
  double	tmp_angle;
  int		debug=0;

  // 初めて呼び出されたとき
  if(flag==0)
    {
      nums = span/step;
      if(debug&&Gdebug) cerr << "[WorkSpace.   ] nums:" << nums << endl;

      num = htk_units.size();
      max_ratio.resize(num);
      max_index.resize(num);
  
      now_sum = 0;
      index = 0;
      key = 0;
      for(i = 0; i < num; i++)
	max_ratio[i] = 0;
  
      ratio2 = space->GetRatio(pos);

      if(debug&&Gdebug) cerr << "[WorkSpace.BehGene] ratio2.size():" << ratio2.size() << endl;
      if(debug&&Gdebug)
	for(int ii=0; ii<(int)ratio2.size(); ii++) cerr << "ratio2[" << ii << "]:" << ratio2[ii] << endl;
      if(debug&&Gdebug) cerr << "[WorkSpace.BehGene] max_ratio.size():" << max_ratio.size() << endl;
    
      // TODO:もっと効率的な方法ないか
      // 0.9以上になるまで足す
      while(now_sum < 0.90)
	{ // TODO:Magic Number!
	  for(int l=0; l<num; l++)
	    {
	      if(index > 0)
		{
		  for(int t=0; t<index; t++)
		    if(max_index[t] == l) key = 1;
		}
	      if(key);
	      else if(ratio2[l] > max_ratio[index])
		{
		  max_index[index] = l;
		  max_ratio[index] = ratio2[l];
		}
      
	      key = 0;
	    }
	  now_sum += max_ratio[index];
	  index++;
	}

      if(index!=1)
	{
	  if(debug&&Gdebug) cerr << "[WorkSpace.   ] index (01):" << max_index[0] << endl;
	  now_sum = max_ratio[0];
	  new_monte = new JHMM(*(space->GetNthPsymbol(max_index[0])));
	  if(debug&&Gdebug) cerr <<"[WorkSpace.BehGene] max_index[0]:" << max_index[0] << endl;
	  if(debug&&Gdebug) cerr << "[WorkSpace.BehGene] debug 10  index:" << index << endl;
	  for(int l=0; l<index-1; l++)
	    {
	      if(debug&&Gdebug) cerr << "[WorkSpace.   ] index (02):" << max_index[l+1] << endl;
	      sum = now_sum + max_ratio[l+1];
	
	      tmp_monte = (*(*new_monte * (now_sum/sum))) + (*(*(space->GetNthPsymbol(max_index[l+1])) * (max_ratio[l+1]/sum)));
	      delete new_monte;
	      new_monte = tmp_monte;
	      now_sum = sum;
	    }
	  mix_beh = new_monte->GenerateBehavior(GEN_NUM, GEN_NUM_Q);
	  delete new_monte;
	} 
      else
	{
	  if(debug&&Gdebug) cerr << "[WorkSpace.   ] index:" << max_index[0] << endl;
	  mix_beh = space->GetNthPsymbol(max_index[0])->GenerateBehavior(GEN_NUM, GEN_NUM_Q);
	}
      dof = mix_beh->Dof();

      length = mix_beh->Length();
      new_step = 100*span*step/length; // TODO:

      new_beh = new Behavior(*mix_beh, span*100); // TODO:
      buffer.push_back(new_beh);	// bufferに追加
      delete mix_beh;

      //まず足そう startからnew_step分切り出して足す
      buf_size = buffer.size();
      tmp_beh = new Behavior(dof);

      for(int j=0; j<new_step; j++)
	{
	  if((j+phase) >= span*100) // TODO:Magic Number!
	    phase = -j;

	  tmp_pose = new Pose();
	  for(int k=0; k<dof; k++)
	    {
	      tmp_angle = 0;
	      tmp_angle += (buffer[0]->NthPose(j+phase)->NthAngle(k));
	      tmp_pose->AddAngle(tmp_angle);
	    }
	  tmp_beh->AddPose(tmp_pose);
	}
  
      add_beh = new Behavior(*tmp_beh, step);
      delete tmp_beh;
  
      beh_buf = new Behavior(*add_beh);
      beh_buf->Label("gen_beh");	//TODO:
      delete add_beh;

      phase += new_step;
      if(debug&&Gdebug) cerr << "****************     phase:" << phase << endl;

      ratio2.clear();
      flag = 1;
    }
  // 二回目以降の呼び出しの場合
  else {  
    now_sum = 0;
    index = 0;
    key = 0;

    for(i = 0; i < num; i++)
      {
	max_ratio[i] = 0;
	max_index[i] = 0;
      }
    ratio2 = space->GetRatio(pos);

    // TODO:もっと効率的な方法ないか
    // 0.9以上になるまで足す
    while(now_sum < 0.90)
      { // TODO:Magic Number!
	for(int l=0; l<num; l++)
	  {
	    if(index > 0)
	      {
		for(int t=0; t<index; t++)
		  if(max_index[t] == l) key = 1;
	      }
	    if(key);
	    else if(ratio2[l] > max_ratio[index])
	      {
		max_index[index] = l;
		max_ratio[index] = ratio2[l];
	      }
      
	    key = 0;
	  }
	now_sum += max_ratio[index];
	index++;
      }

    if(index!=1)
      {
	if(debug&&Gdebug) cerr << "[WorkSpace.   ] index:" << max_index[0] << endl;
	now_sum = max_ratio[0];
	new_monte = new JHMM(*(space->GetNthPsymbol(max_index[0])));
	for(int l=0; l<index-1; l++)
	  {
	    if(debug&&Gdebug) cerr << "[WorkSpace.   ] index:" << max_index[l+1] << endl;
	    sum = now_sum + max_ratio[l+1];
	    tmp_monte = (*(*new_monte * (now_sum/sum))) 
	      + (*(*(space->GetNthPsymbol(max_index[l+1])) * (max_ratio[l+1]/sum)));
	    delete new_monte;
	    new_monte = tmp_monte;
	    now_sum = sum;
	  }
	mix_beh = new_monte->GenerateBehavior(100, 10); // TODO:Magic Number!
	delete new_monte;
      }
    else
      {
	if(debug&&Gdebug) cerr << "[WorkSpace.   ] index:" << max_index[0] << endl;
	mix_beh = space->GetNthPsymbol(max_index[0])->GenerateBehavior(100, 10); // TODO: Magic Number
	if(debug&&Gdebug) cerr << "[WorkSpace.   ] mix_beh OK" << endl;
      }
    length = mix_beh->Length();
    if(debug&&Gdebug) cerr << "[WorkSpace.  ] mix_beh.length:" << length << endl;
    new_step = 100*span*step/length; // TODO: Magic Number
    if(debug&&Gdebug) cerr << "[WorkSpace.  ] new_step:" << new_step << endl;
  
    new_beh = new Behavior(*mix_beh, span*100); // TODO:
    buffer.push_back(new_beh);	// bufferに追加
    delete mix_beh;

    if((int)buffer.size() > nums)
      {
	vector<Behavior *>::iterator p = buffer.begin();
	buffer.erase(p, p+1);
      }

    //まず足そう startからnew_step分切り出して足す
    buf_size = buffer.size();
    tmp_beh = new Behavior(dof);

    for(int j=0; j<new_step; j++)
      {
	if((j+phase) >= span*100) // TODO:Magic Number!
	  phase = -j;

	tmp_pose = new Pose();
	for(int k=0; k<dof; k++)
	  {
	    tmp_angle = 0;
	    for(int l=0; l<buf_size; l++)
	      tmp_angle += (buffer[l]->NthPose(j+phase)->NthAngle(k)) / (double)buf_size;
	    tmp_pose->AddAngle(tmp_angle);
	  }
	tmp_beh->AddPose(tmp_pose);
      }
  
    add_beh = new Behavior(*tmp_beh, step); // TODO:stepをちゃんと考える．長さを変えるコピーコンストラクタ
    delete tmp_beh;
  
    beh_buf->Cat(*add_beh);	// TODO:Catより効率的なのを
    delete add_beh;

    phase += new_step;
    if(debug&&Gdebug) cerr << "****************     phase:" << phase << endl;
  
    ratio2.clear();
  }

  return TRUE;
}


/**************************************************
 * Function : 原始シンボルのラベル名からその座標を返す
 * Memo     : Renamed on 2008-01-17, from GetXPosition
 **************************************************/
vector<double> WorkSpace::GetCoordinate(const char* name)
{
  int key;
  int debug=0;

  key = GetKey(name);
  if(debug&&Gdebug) tl_message ("Name <%s> is %d", name, key);

  return space->GetNthCoordinate(key);
}


/**************************************************
 * Function : 入力behとPsymbolとのKullback-Leibler Divergenceを計算
 * Input    : Behavior *beh  : Time series data for the calculation of distances between protosymbols
 * Output   : vector<double> : Distances for each proto-symbols
 * Memo     : 
 **************************************************/
int WorkSpace::CalcDistanceOfInputBehavior(Behavior *beh, vector<double> &distance)
{
  int			length;
  //vector<double>	distance;

  length = beh->Length();
  distance.resize(0);
  CalcLikelihoodVector(beh, distance);
  for(int i=0; i<(int)motion_db.size(); i++)
  //for(int i=0; i<(int)space->GetNumOfData(); i++)
    {
      // 尤度が正になるものがあるのでオフセットをとる
      //distance[i] = - (distance[i] - 15 * length)/(double)length; // Constant of ICHR'03 Eq.(9) till 2008-01-17
      distance[i] = 20 - (distance[i] / (double)length); // Constant of ICHR'03 Eq.(9) modified on 2008-01-17
      if(distance[i] < 0) distance[i] = 0;
    }
  return TL_TRUE;
}



/***************************************************************************************************
 * Function : 新しくonlineで入力されたbehと既知のPsymbolとの Kullback-Leibler Divergenceを計算
 * Input    : Behavior *beh  : Time series data for the calculation of distances between protosymbols
 * Output   : vector<double> : Distances for each proto-symbols
 * Created on 2008-07-14 by inamura
 * Memo     : CalcDistanceOfInputBehavior は旧バージョン，こちらは 15という定数を用いない
 *          : Constant of ICHR'03 Eq.(9) is not used, but calculate directory
 ***************************************************************************************************/
int WorkSpace::CalcDistanceOfOnlineBehavior(Behavior *beh, vector<double> &result)
{
  int			debug=0;
  int			length;
  vector<double>	term1, term2;

  length = beh->Length();
  term1.resize(0);
  term2.resize(0);
  result.resize(0);
  CalcLikelihoodVector(beh, term1);

  WorkSpace	*work;
  MotionDB	*mdb;
  if(debug) tl_message ("step 1");
  beh->FileOut("./.tmp/online/online0.beh");
  work = new WorkSpace();
  mdb = new MotionDB();
  mdb->Load ("./.tmp/online", "./.tmp/online", "online", "online.hmm", 1, 1, 15, LEFT_TO_RIGHT);
  if(debug) mdb->Verify();
  if(debug) tl_message ("step 2");
  work->AddMotionDB(mdb);
  if(debug) tl_message ("step 3");
  work->ExecLearning ();
  if(debug) tl_message ("step 4");
  work->SetHTKUnitsFromMotionDB ();
  if(debug) tl_message ("step 5");
  work->BeforeRecognize();
  if(debug) tl_message ("step 6");
  work->CalcLikelihoodVector (beh, term2);
  if(debug) tl_message ("step 6.1");

  for(int i=0; i<(int)motion_db.size(); i++)
    {
      if(debug) tl_message ("step 7.%d", i);
      result.push_back ((term2[0] - term1[i]) / (double)length);
      if(result[i] < 0) result[i] = 0;
    }
  if(debug) tl_message ("step 8");
  term1.resize(0);
  term2.resize(0);
  if(debug) tl_message ("step 9");
  delete work;
  if(debug) tl_message ("end");
  return TL_TRUE;
}



/***************************************************************************************************
 * Function : 新しくonlineで入力されたbehと既知のPsymbolとの Hellinger Distance を計算
 * Input    : Behavior *beh  : Time series data for the calculation of distances between protosymbols
 * Output   : vector<double> : Hellinger Distances for each proto-symbols
 * Created on 2008-07-16 by inamura
 ***************************************************************************************************/
int WorkSpace::CalcHellingerDistanceOfOnlineBehavior(Behavior *beh, vector<double> &distance)
{
  int			debug=1;
  //vector<double>	distance;


  WorkSpace	*work;
  MotionDB	*mdb;
  JHMM		*hmm1, *hmm2;
  hmm1 = new JHMM;
  distance.resize(0);
  string	filename;

  if(debug) tl_message ("step 1");
  beh->FileOut("./.tmp/online/online0.beh");
  work = new WorkSpace();
  mdb  = new MotionDB();
  mdb->Load ("./.tmp/online", "./.tmp/online", "online", "online.hmm", 1, 1, 15, LEFT_TO_RIGHT);
  if(debug) mdb->Verify();
  if(debug) tl_message ("step 2");
  work->AddMotionDB(mdb);
  if(debug) tl_message ("step 3");
  work->ExecLearning ();
  if(debug) tl_message ("step 4");
  hmm1->Load("./.tmp/online/online.hmm");
  if(debug) tl_message ("step 5");

  for(int i=0; i<(int)motion_db.size(); i++)
    {
      hmm2 = new JHMM;
      double dis = 0.0;
      if(debug) tl_message ("step 6.%d.1", i);
      motion_db[i]->HMMFullFile(filename);
      if(debug) tl_message ("step 6.%d.2 : HMM file = %s", i, filename.c_str());
      hmm2->Load (filename.c_str());
      if(debug) tl_message ("step 6.%d.3", i);
      dis = hmm1->HellingerDistance (*hmm2);
      if(debug) tl_message ("step 6.%d.4", i);
      distance.push_back(dis);
      if(debug) tl_message ("step 6.%d.5 : distance = %g", i, dis);
      delete hmm2;
    }
  if(debug) tl_message ("step 7");
  delete work;
  if(debug) tl_message ("step 8");
  delete hmm1;
  if(debug) tl_message ("end");
  return 1;
}



/**************************************************
 * Function : PsymbolSpaceを返す
 * Memo     : 
 **************************************************/
PsymbolSpace* WorkSpace::GetPsymbolSpace(void)
{
  if (space==NULL)
    tl_warning ("Opps! result will be NULL");
  return space;
}


/**************************************************
 * Function : Spaceのファイルアウト
 * Memo     : 
 **************************************************/
int WorkSpace::SpaceFileOut(const char* fname)
{
  ofstream fout(fname);
  
  int num;
  if (d_type==KL_DIVERGENCE)
    num = matrix_KLD.size();
  else if (d_type==HELLINGER)
    num = matrix_HD.size();
  else
    {
      tl_warning ("Wrong distance type : %d", (int)d_type);
      return TL_FAIL;
    }

  fout << "#num\t" << num << endl;
  for(int i=0; i<num; i++)
    fout << htk_units[i]->GetHMMFile() << endl;

  (GetPsymbolSpace())->FileOut(fout);

  fout.close();

  return TRUE;
}


/**************************************************
 * Function : Spaceの読み込み
 * Memo     : 
 **************************************************/
int WorkSpace::SpaceLoad(const char* fname)
{
  ifstream	fin;
  string	tmp_str;
  int		num, debug=1;
  char		buf[MAX_STRING], tmpbuf[MAX_STRING], tmp_hmmfile[MAX_STRING];

  if(debug) tl_message ("start");
  space = new PsymbolSpace();

  complement_dirname (fname, tmp_str);
  fin.open (tmp_str.c_str(), ifstream::in);
  
  fin.getline(buf, MAX_STRING);
  if(strncmp(buf, "#num", 4))
    {
      cerr << "[WorkSpace.SpaceLoad] \"#num\" -> " << buf << endl;
      return FALSE;
    }
  sscanf(buf, "%s\t%d", tmpbuf, &num);
  for(int i=0; i<num; i++)
    {
      fin >> tmp_hmmfile;
      space->AddPsymbol(tmp_hmmfile);
      fin.getline(buf, MAX_STRING); //改行コードの読み込み
    }
  
  (GetPsymbolSpace())->Load(fin);

  fin.close();
  tl_message ("end");

  return TRUE;
}


/**************************************************
 * Function : stepを返す
 * Memo     : 
 **************************************************/
int WorkSpace::GetStepTime(void)
{
  return step;
}

/****************************************************************************************************
 * Function : GetLastBehavior (void)
 * Memo     : 生成されたBehaviorのうち，最新の[step]サンプル数分のデータのみを返す
 ****************************************************************************************************/
Behavior* WorkSpace::GetLastBehavior (void)
{
  Behavior *new_buf = new Behavior(beh_buf->Dof());
  int length = beh_buf->Length();
  if (length<step)
    return NULL;

  for (int i=1; i<=step; i++)
    {
      Pose *pose = new Pose();
      pose = beh_buf->NthPose(length-step+i);
      new_buf->AddPose (pose);
    }
  return new_buf;
}

/**************************************************
 * Function : 生成されたBehaviorを返す
 * Memo     : 
 **************************************************/
Behavior* WorkSpace::GetBehBuf(void)
{
  return beh_buf;
}



/**************************************************
 * Function : Verify
 * Created  : 2005 Aug 30th by inamura
 * Memo     : 
 **************************************************/
int WorkSpace::Verify(void)
{
  int	i;

  for (i=0; i < (int)motion_db.size(); i++)
    motion_db[i]->Verify();

  space->Verify();
  
  return 1;
}





#if 0
/**************************************************
 * Modified : 2004 May 26th   by inamura
 * Function : スクリプトファイルを読み込んで学習する．
 *            学習された HMM の結果はこの段階ではまだメモリにロードされない
 * TODO     : 関数名と挙動がいまいち合っていない
 **************************************************/
int WorkSpace::LoadLearningScriptFile(const char* fname)
{
  ifstream	fin;
  int		tmp_numstate, tmp_nummix, tmp_numsample,tmp_label_num, tmp_space_dim;
  int		end_flag = 0;
  char		buf[MAX_STRING],tmpbuf[MAX_STRING],tmp_label[MAX_STRING],tmp_r_dir[MAX_STRING],tmp_s_dir[MAX_STRING];
  char		tmp_hmmfile[MAX_STRING], tmp_hmmtype[MAX_STRING], tmp_filename[MAX_STRING];
  char		tmp_scale_dir[MAX_STRING], r_dir_ver3[MAX_STRING], s_dir_ver3[MAX_STRING],hmmfile_ver3[MAX_STRING];
  MotionDB	*mdb = NULL;
  HmmType	type;
  char		*tmp_label2;
  vector<char *>tmp_label_list;
  vector<int>	tmp_numstate_list;
  vector<int>	tmp_nummix_list;
  vector<int>	tmp_numsample_list;
  string	tmp_str;
  int		debug=1;

  complement_dirname (fname, tmp_str);
  if(debug&&Gdebug) tl_message ("reading script %s", tmp_str.c_str());
  fin.open (tmp_str.c_str(), ifstream::in);

  if(!fin)
    {
      tl_message ("cannot open %s", tmp_str.c_str());
      return FALSE;
    }
  
  fin.getline(buf, MAX_STRING);
  
  if (tl_strmember(buf, "#version1"))
    {
      // #version1
      while(1)
	{
	  if(fin.getline(buf, MAX_STRING) == NULL)
	    {
	      tl_message ("read is NULL");
	      end_flag = 1;
	      break;
	    }

	  if(!end_flag)
	    {
	      if(strncmp (buf, "label:", 6))  break;
	      sscanf(buf, "%s\t\t%s", tmpbuf, tmp_label);
	      fin.getline(buf, MAX_STRING);
	      if(strncmp (buf, "read_dir:", 9)) break;
	      sscanf(buf, "%s\t%s", tmpbuf, tmp_r_dir);
	      fin.getline(buf, MAX_STRING);
	      if(strncmp (buf, "save_dir:", 9)) break;
	      sscanf(buf, "%s\t%s", tmpbuf, tmp_s_dir);
	      fin.getline(buf, MAX_STRING);
	      if(strncmp (buf, "hmmfile:", 8)) break;
	      sscanf(buf, "%s\t%s", tmpbuf, tmp_hmmfile);
	      fin.getline(buf, MAX_STRING);
	      if(strncmp (buf, "num_state:", 10)) break;
	      sscanf(buf, "%s\t%d", tmpbuf, &tmp_numstate);
	      fin.getline(buf, MAX_STRING);
	      if(strncmp (buf, "num_mixture:", 12)) break;
	      sscanf(buf, "%s\t%d", tmpbuf, &tmp_nummix);
	      fin.getline(buf, MAX_STRING);
	      if(strncmp (buf, "num_sample:", 11)) break;
	      sscanf(buf, "%s\t%d", tmpbuf, &tmp_numsample);
	      fin.getline(buf, MAX_STRING);
	      if(strncmp (buf, "hmm_type:", 9)) break;
	      sscanf(buf, "%s\t%s", tmpbuf, tmp_hmmtype);

	      type = query_hmmtype_enum (tmp_hmmtype);
	      if ((int)type==FALSE) tl_warning ("HmmType is strange : %s", buf);

	      fin.getline(buf, MAX_STRING);
	      if(debug&&Gdebug)
		{
		  tl_message ("loading via ver.1 style...");
		  cerr << "buf:" << buf << endl;
		  cerr << "label:" << tmp_label << endl;
		  cerr << "r_dir:" << tmp_r_dir << endl;
		  cerr << "s_dir:" << tmp_s_dir << endl;
		  cerr << "hmmfile:" << tmp_hmmfile << endl;
		  cerr << "state:" << tmp_numstate << endl;
		  cerr << "mix:" << tmp_nummix << endl;
		  cerr << "sample:" << tmp_numsample << endl;
		  cerr << "type:" << tmp_hmmtype << endl;
		}

	      mdb = new MotionDB();
	      mdb->Load (tmp_r_dir, tmp_s_dir, tmp_label, tmp_hmmfile, tmp_numsample, tmp_nummix, tmp_numstate, type);
	      mdb->IncreaseSamples(500);
	      mdb->LearningHMM ();
   
	      AddMotionDB(mdb);
	    }
	}
      fin.close();
      return TRUE;
    }
  else if (tl_strmember(buf, "#version2"))
    {
      // version2
      while(1)
	{
	  if(fin.getline(buf, MAX_STRING) == NULL)
	    {
	      tl_message ("End of file");
	      end_flag = 1;
	      break;
	    }
      
	  if(!end_flag)
	    {
	      if(strncmp (buf, "filename:", 9))
		{
		  tl_message ("Instead of 'filename:', %s comes", buf); 
		} else {
		  sscanf(buf, "%s\t\t%s", tmpbuf, tmp_filename);
		  mdb = new MotionDB();
		  mdb->Load (tmp_filename);
		  //mdb->IncreaseSamples(100);
		  mdb->LearningHMM ();
		  AddMotionDB (mdb);
		}
	    }
	}
      fin.close();
      return TRUE;
    }
  else if (tl_strmember(buf, "#version3"))
    {
      if(debug&&Gdebug) cerr << "version3" << endl;
      // version3
      while(1) 
	{
	  if(fin.getline(buf, MAX_STRING) == NULL)
	    {
	      tl_message ("end of file");
	      end_flag = 1;
	      break;
	    }
	  if(!end_flag)
	    {
	      if(strncmp (buf, "label_num:", 10)) break;
	      sscanf(buf, "%s\t%d", tmpbuf, &tmp_label_num);
	      if(debug&&Gdebug)
		{ if(tmp_label_num > 10) break; }
	      fin.getline(buf,MAX_STRING);
	      if(strncmp (buf, "label:", 6)) break;
	      for(int lnum=0; lnum<tmp_label_num; lnum++)
		{
		  fin.getline(buf,MAX_STRING);
		  tmp_label2 = new char[MAX_STRING];
		  sscanf(buf, "\t%s", tmp_label2);
		  tmp_label_list.push_back(tmp_label2);
		  if(debug&&Gdebug) cerr << tmp_label2 << " ";
		}

	      if(debug&&Gdebug) cerr << endl;
	      fin.getline(buf, MAX_STRING);
	      if(strncmp (buf, "read_dir:", 9)) break;
	      sscanf(buf, "%s\t%s", tmpbuf, tmp_r_dir);
	      fin.getline(buf, MAX_STRING);
	      if(strncmp (buf, "scale_dir:", 10)) break;
	      sscanf(buf, "%s\t%s", tmpbuf, tmp_scale_dir);
	      fin.getline(buf, MAX_STRING);				       
	      if(strncmp (buf, "save_dir:", 9)) break;
	      sscanf(buf, "%s\t%s", tmpbuf, tmp_s_dir);
	      fin.getline(buf, MAX_STRING);
	      if(strncmp (buf, "num_state:", 10)) break;
	      if(debug&&Gdebug) cerr << "num_state: ";
	      for(int lnum=0; lnum<tmp_label_num; lnum++)
		{
		  fin.getline(buf,MAX_STRING);
		  sscanf(buf, "\t%d", &tmp_numstate);
		  tmp_numstate_list.push_back(tmp_numstate);
		  if(debug&&Gdebug) cerr << tmp_numstate << " ";
		}
	      if(debug&&Gdebug) cerr << endl;
	      fin.getline(buf, MAX_STRING);
	      if(strncmp (buf, "num_mixture:", 12)) break;
	      if(debug&&Gdebug) cerr << "num_mixture: ";
	      for(int lnum=0; lnum<tmp_label_num; lnum++)
		{
		  fin.getline(buf,MAX_STRING);
		  sscanf(buf, "\t%d", &tmp_nummix);
		  tmp_nummix_list.push_back(tmp_nummix);
		  if(debug&&Gdebug) cerr << tmp_nummix << " ";
		}
	      if(debug&&Gdebug) cerr << endl;
	      fin.getline(buf, MAX_STRING);
	      if(strncmp (buf, "num_sample:", 11)) break;
	      if(debug&&Gdebug) cerr << "num_sample: ";
	      for(int lnum=0; lnum<tmp_label_num; lnum++)
		{
		  fin.getline(buf,MAX_STRING);
		  sscanf(buf, "\t%d", &tmp_numsample);
		  tmp_numsample_list.push_back(tmp_numsample);
		  if(debug&&Gdebug) cerr << tmp_numsample << " ";
		}
	      if(debug&&Gdebug) cerr << endl;
	      fin.getline(buf, MAX_STRING);
	      if(strncmp (buf, "hmm_type:", 9)) break;
	      sscanf(buf, "%s\t%s", tmpbuf, tmp_hmmtype);
	      fin.getline(buf, MAX_STRING);
	      if(strncmp (buf, "space_dim:", 10)) break;
	      sscanf(buf, "%s\t%d", tmpbuf, &tmp_space_dim);

	      // script file の中の記述から HmmType を求める
	      type = query_hmmtype_enum (tmp_hmmtype);
	      if ((int)type==FALSE) tl_warning ("HmmType is strange : %s", buf);

	      fin.getline(buf, MAX_STRING);
      
	      for(int lnum=0; lnum<tmp_label_num; lnum++)
		{
		  //strcpy(r_dir_ver3, tmp_r_dir);
		  //strcat(r_dir_ver3, tmp_label_list[lnum]);
		  //strcat(r_dir_ver3, "/");
		  strcpy(r_dir_ver3, tmp_scale_dir);
		  strcat(r_dir_ver3, tmp_label_list[lnum]);
		  strcat(r_dir_ver3, "/");
		  strcpy(s_dir_ver3, tmp_s_dir);
		  strcat(s_dir_ver3, tmp_label_list[lnum]);
		  strcat(s_dir_ver3, "/");
		  strcpy(hmmfile_ver3, tmp_label_list[lnum]);
		  strcat(hmmfile_ver3, ".hmm");
		  mdb = new MotionDB();
		  //mdb->Exec(r_dir_ver3, s_dir_ver3, tmp_label_list[lnum], hmmfile_ver3,
		  //          tmp_numsample_list[lnum], tmp_nummix_list[lnum], tmp_numstate_list[lnum], type);
		  mdb->Load (r_dir_ver3, s_dir_ver3, tmp_label_list[lnum], hmmfile_ver3,
			     tmp_numsample_list[lnum], tmp_nummix_list[lnum], tmp_numstate_list[lnum], type);
		  mdb->IncreaseSamples(100);
		  mdb->LearningHMM ();
		  AddMotionDB (mdb);
		}
	    }
	}
      fin.close();
      return TRUE;
    }
  else
    {
      tl_message ("ERROR (at end of function)");
      fin.close();
      return FALSE;
    }
}
#endif
