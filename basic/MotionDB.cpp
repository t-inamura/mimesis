/*
 * MotionDB.cpp
 *
 * Modified : Tetsunari Inamura on 2008-09-30
 *
 */


/* Change log
 *
 * 2004-09-14 by inamura, LoadBehaviors
 *	Detect suitable number to be read automatically, without specification
 * 2008-09-30
 *	change method name from GetNthBeh to GetNthBeh
 *	integrate SetHMMtype and GetHMMtype into HMMtype
 */

#include <stdio.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include "MotionDB.h"

MotionDB::MotionDB()
{
  Reset();
}


/*-----------------------------------------------------------------------------------*/
// Created  : 2004 Jun 3rd   by inamura
// Function : Loading several parameters from file
/*-----------------------------------------------------------------------------------*/
// Input    : fname	: file name of the target file
/*-----------------------------------------------------------------------------------*/
MotionDB::MotionDB(const char* fname)
{
  Reset ();
  Load  (fname);
}

/**************************************************
 * Created  : 2003.09.06	
 * Function : Copy Constructor
 * Memo     : Motion patterns on memory is also copied
 **************************************************/
MotionDB::MotionDB(const MotionDB &another)
{
  int debug=0;

  if(debug&&Gdebug) cerr << "[MotionDBCopy] Start" << endl;
  load_dir = another.load_dir;
  save_dir = another.save_dir;
  label    = another.label;
  hmm_file = another.hmm_file;
  hmm_type = another.hmm_type;
  state    = another.state;
  num_of_mix    = another.num_of_mix;
  //num_of_sample = another.num_of_sample;

  Behavior* new_beh = NULL;

  for (int i=0; i<(int)another.behaviors.size(); i++)
    {
      if(debug&&Gdebug) cerr << "[MotionDBCopy] " << i << "thBeh Copy" << endl;
      new_beh = new Behavior(*(another.behaviors[i]));
      behaviors.push_back(new_beh);
      if(debug&&Gdebug) cerr << "Copy Fin" << endl;
    }
  if(debug&&Gdebug) cerr << "[MotionDBCopy] end!" << endl;
}


MotionDB::~MotionDB()
{
  int debug=0;

  if(debug&&Gdebug) cerr << "[MotionDB.~MotionDB] delete start." << endl;
  load_dir.erase();
  save_dir.erase();
  label.erase();
  hmm_file.erase();
  
  for(int i=0; i<(int)behaviors.size(); i++)
    {
      if(debug&&Gdebug) cerr << "[MotioinDB.~MotionDB] delete behaviors[" << i << "]"<< endl;
      delete behaviors[i];
    }
  behaviors.clear();
  
  if(debug&&Gdebug) cerr << "[MotionDB.~MotionDB] delete finish." << endl;
}


/**************************************************
 * Created  : 2003/08/29	
 * Function : Reset of MotionDB instance
 * Memo     : init of string is not performed (it should be done here!?)
 **************************************************/
void MotionDB::Reset()
{
  state      = DEFAULT_NUMOFSTATE;
  num_of_mix = DEFAULT_NUMOFMIX;
  hmm_type   = DEFAULT_HMMTYPE;

  //num_of_sample = 0;
  dimension     = 1;
  
  behaviors.clear();
}


// Modified : 2004 Nov 21st by inamura
// Memo     : SetNumOfState --> NumOfState
void MotionDB::NumOfState(int num)
{
  assert (num>0);

  state = num;
}


int MotionDB::NumOfState(void)
{
  return state;
}

// Modified : 2004 Nov 21st by inamura
// Memo     : SetNumOfMix --> NumOfMix
void MotionDB::NumOfMix(int num)
{
  assert (num>0);

  num_of_mix = num;
}

// Modified : 2004 Nov 21st by inamura
// Memo     : GetNumOfMix --> NumOfMix
int MotionDB::NumOfMix(void)
{
  return num_of_mix;
}

#if 0
// comment out by inamura, 2004 Sep 14th
void MotionDB::SetNumOfSample(int num)
{
  num_of_sample = num;
}
#endif

int MotionDB::NumOfSample(void)
{
  return behaviors.size();
}

void MotionDB::LoadDirectory(string &name)
{
  load_dir = name;
}

void MotionDB::LoadDirectory(const char *name)
{
  load_dir = name;
}

string& MotionDB::LoadDirectory(void)
{
  return load_dir;
}

void MotionDB::SaveDirectory(string &name)
{
  save_dir = name;
}

void MotionDB::SaveDirectory(const char *name)
{
  save_dir = name;
}

string& MotionDB::SaveDirectory(void)
{
  return save_dir;
}

void MotionDB::Label(string &name)
{
  label = name;
}

void MotionDB::Label(const char *name)
{
  label = name;
}

string& MotionDB::Label(void)
{
  return label;
}

void MotionDB::HMMFile(string &name)
{
  hmm_file = name;
}

void MotionDB::HMMFile(const char* name)
{
  hmm_file = name;
}

string& MotionDB::HMMFile(void)
{
  return hmm_file;
}

int MotionDB::HMMFullFile(string &filename)
{
  filename = save_dir + "/" + label + ".hmm";

  return TL_TRUE;
}


void MotionDB::HMMtype(HmmType type)
{
  hmm_type = type;
}

HmmType MotionDB::HMMtype(void)
{
  return hmm_type;
}

int MotionDB::BVHFlag(int flag)
{
  bvh_flag = flag;
  return flag;
}

int MotionDB::BVHFlag(void)
{
  return bvh_flag;
}

void MotionDB::AddBehavior(Behavior *new_beh)
{
  assert (new_beh);
  behaviors.push_back(new_beh);
}



/**************************************************
// Modified : 2004 May 26th  by inamura
// Memo     : Relative path is available. If the top char is . or / , then auto complete is not conducted
// Modified : 2004 Sep 14th  by inamura : The number of loading behavior will be detected automatically
// Function : Load MotionDB instance from loading directory
 **************************************************/
int MotionDB::LoadBehaviors(void)
{
  FILE		*fp=NULL;
  string	tmp_str;
  Behavior	*new_beh;
  char		buf[MAX_STRING];
  int		debug=0;

  tl_message ("Start! (load_dir = %s)", load_dir.c_str());
  for (int i=0; ;i++)
    {
      tmp_str.erase (tmp_str.begin(), tmp_str.end() );
      complement_dirname (load_dir.c_str(), tmp_str);
      sprintf (buf,"%d", i);
      tmp_str += "/";
      tmp_str += label;
      tmp_str += buf;
      tmp_str += ".beh";
      // check existence of the file
      if(debug&&Gdebug) tl_message ("checking ... %s", tmp_str.c_str());
      if ((fp = fopen ((const char *)tmp_str.c_str(), "r"))!=NULL)
	{
	  fclose (fp);
	  new_beh = new Behavior();
	  if(Gdebug) tl_message ("loading %s", tmp_str.c_str());
	  new_beh->Load (tmp_str.c_str());
	  AddBehavior (new_beh);
	  cerr << ".";
	}
      else 
	break;
    }
  cerr << endl;
  //tmp_str.clear();
  tmp_str.erase (tmp_str.begin(), tmp_str.end() );
  if(debug&&Gdebug) tl_message ("Finish!");
  return TRUE;
}


/**************************************************
// Created  : 2005 Apr 14th  by inamura
// Memo     : Relative path is available. If the top char is . or / , then auto complete is not conducted
// Function : Load MotionDB instance from BVH format by loading directory name
 **************************************************/
int MotionDB::LoadBVH(void)
{
  FILE		*fp=NULL;
  string	tmp_str;
  Behavior	*new_beh;
  char		buf[MAX_STRING];
  int		debug=0;

  if(debug&&Gdebug) tl_message ("Start! (load_dir = %s), label = %s", load_dir.c_str(), label.c_str());
  //for (int i=0; i<this->NumOfSample(); i++)
  for (int i=0; ;i++)
    {
      tmp_str.erase (tmp_str.begin(), tmp_str.end() );
      complement_dirname (load_dir.c_str(), tmp_str);
      sprintf (buf,"%d", i);
      tmp_str += "/";
      tmp_str += label;
      tmp_str += buf;
      tmp_str += ".bvh";
      tl_message ("try to open : %s", tmp_str.c_str());
      // check existence of the file
      if ((fp = fopen ((const char *)tmp_str.c_str(), "r"))!=NULL)
	{
	  fclose (fp);
	  new_beh = new Behavior();
	  tl_message ("loading %s", tmp_str.c_str());
	  new_beh->LoadBVH (tmp_str.c_str(), label.c_str());
	  AddBehavior (new_beh);
	}
      else 
	break;
    }

  //tmp_str.clear();
  tmp_str.erase (tmp_str.begin(), tmp_str.end() );
  if(debug&&Gdebug) NthBehavior(0)->Verify();
  tl_message ("Finish!");
  if(debug&&Gdebug) Verify();
  return TRUE;
}






/**************************************************
// Created  : 2004 Sep 14th	
 * Function : Save MotionDB instance in save directory
 **************************************************/
int MotionDB::SaveBehaviors(void)
{
  string	fname;
  char buf[MAX_STRING];
  int debug=0;

  if(debug&&Gdebug) tl_message ("Start! (save_dir = %s)", save_dir.c_str());
  for (int i=0; i<this->NumOfSample(); i++)
    {
      fname.erase (fname.begin(), fname.end() );
      complement_dirname (save_dir.c_str(), fname);
      sprintf (buf,"%d", i);
      fname += "/";
      fname += label;
      fname += buf;
      fname += ".beh";

      behaviors[i]->FileOut (fname.c_str());
    }

  fname.erase (fname.begin(), fname.end() );
  if(debug&&Gdebug) tl_message ("Finish!");
  return TRUE;
}




/**************************************************
 * Modified : 2004 Nov 18th by inamura, add complemention process
 * Function : Create HMM files for HTK based learning
 * Memo     : This function should be called after SetBehavior
 * TODO     :  +-> This is problem. order should be free
 **************************************************/
void MotionDB::CreateHMMSrcFile(void)
{
  string	tmp;
  JHMM		*tmp_hmm;
  int		debug=1;

  if(debug&&Gdebug) tl_message ("Initialize.");
  tmp_hmm = new JHMM();
  if(debug&&Gdebug) cerr << "[MDB.CreateHMMSrcFile] NumOfNode =" << state << endl;
  tmp_hmm->SetNumState(state);
  if(debug&&Gdebug) cerr << "[MDB.CreateHMMSrcFile] NumOfMix =" << num_of_mix << endl;
  tmp_hmm->SetNumMix(num_of_mix);
  if (NumOfSample()<=0)
    {
      tl_warning ("There is no time-series data in this MotionDB");
      return (void)NULL;
    }
  if(debug&&Gdebug) tl_message ("vecsize: %d", behaviors[0]->Dof());  // TODO magic number!!

  tmp_hmm->SetVecSize(behaviors[0]->Dof());
  tmp_hmm->Initialize(HMMtype());
  tmp_hmm->SetLabel(label.c_str());

  complement_dirname (save_dir.c_str(), tmp);
  tmp += "/";
  tmp += hmm_file;
  //if(debug&&Gdebug) cerr << "[MotionDB.CreateHMMSrcFile] filename:" << tmp << endl;
  if(debug&&Gdebug) tl_message ("filename : %s", tmp.c_str());

  tmp_hmm->FileOut(tmp.c_str());
  
  delete tmp_hmm;
  tmp.erase();
}


/**************************************************
 * Modified : 2004 Nov 21st by inamura
 * Function : Main function of learning process
 * Memo     : Motion pattern is not loaded in memory during learning
 *          : Motion pattern should be stored specified directory, output will be written in file
 **************************************************/
int MotionDB::LearningHMM (void)
{
  string filename;
  int	ret, debug=1;

  CreateSaveDir();

  // Set of continuous HMM as initialization of learning
  if(debug&&Gdebug) tl_message ("Now calling CreateHMMFile");
  CreateHMMSrcFile();

  // Create continuous HTK files in specified directory
  if(debug&&Gdebug) tl_message ("Now calling MakeContinuousTrainingFiles");
  MakeContinuousTrainingFiles();

  if(debug&&Gdebug) tl_message ("ExecHRest");
  ret = ExecHRest();

  // TODO: output of sampling_time file
  TimeFileOut();
  
  return ret;
}


/**************************************************
 * Created  : 2003.09.01
 * Modified : 2004 Sep 13th	by inamura ; add "/" between directory name and filename
 * Function : creating TrainingFile(.htk, .lab)
 **************************************************/
int MotionDB::MakeContinuousTrainingFiles(void)
{
  string	filename;
  string	htkfname;
  string	labfname;
  char		buf[MAX_STRING];
  Behavior	*beh=NULL;
  int		i;
  int		debug=0;
  
  filename = save_dir + "/filelist.txt";
  ofstream fout(filename.c_str());
  if(debug&&Gdebug) tl_message ("filename: %s", filename.c_str());

  for (i=0; i<this->NumOfSample(); i++)
    {
      filename.erase();
      sprintf(buf,"%d", i);
      filename = save_dir + "/" + label + buf;
      htkfname = filename + ".htk";
      labfname = filename + ".lab";
      if(debug&&Gdebug) tl_message ("step %d.1", i);
      beh = behaviors[i];
      if(debug&&Gdebug) tl_message ("step %d.2", i);
      
      beh->HTKFileOut(htkfname.c_str());
      if(debug&&Gdebug) tl_message ("step %d.3", i);
      beh->MakeLabelFile(labfname.c_str());
      
      if(debug&&Gdebug) tl_message ("step %d.4", i);
      htkfname = label + buf + ".htk";
      fout << htkfname << "  ";
      if(debug&&Gdebug) tl_message ("step %d.5", i);
    }	
  
  fout.close();
  filename.erase();
  htkfname.erase();
  labfname.erase();

  return TRUE;
}


/**************************************************
 * Created  : 2004 Sep 13th
 * Function : Add noise for all Behavior instance
 **************************************************/
int MotionDB::AddNoise(void)
{
  string	filename;
  int		i=0;
  char		buf[10];
  vector<Behavior *>::iterator beh;
  
  if(Gdebug) tl_message ("start");
  for (beh=behaviors.begin(); beh!=behaviors.end(); beh++,i++)
    {
      (*beh)->AddNoise ();
      sprintf(buf,"%d", i);
#if 0
      if (bvh_flag==1)
	{
	  if(debug&&Gdebug) tl_message ("in case of bvh");
	  filename = save_dir + "/" + label + buf + ".bvh";
	}
      else
	{
	  if(debug&&Gdebug) tl_message ("in case of normal beh");
	  filename = save_dir + "/" + label + buf + ".beh";
	}
      filename = save_dir + "/" + label + buf + ".beh";
      if(debug&&Gdebug) tl_message ("now file out : %s", filename.c_str());
      (*beh)->FileOut (filename.c_str());
#endif
    }	
  
  filename.erase();
  if(Gdebug) tl_message ("end");

  return TRUE;
}


/**************************************************
 * Modified : 2006 Sep 1st by inamura
 * Function : Learning HMM with Hinit and HRest. If it failed, number of patterns will be increased
 * Memo     : 2004 Nov 18th : Complement of directory name is conducted
 * Memo     : 2006 Sep 1st  : A Function added when the process failed, the number of patterns increased automatically
 **************************************************/
int MotionDB::ExecHRest()
{
  char		com[MAX_STRING];
  int		debug=1;
  string	tmp_str;
  struct stat	st1, st2;

  if(Gdebug) tl_message ("start");
  complement_dirname (save_dir.c_str(), tmp_str);
  // If save directory is not found, try to make directory
  if (stat (tmp_str.c_str(), &st1)!=0)
    {
      cerr << "[MDB::ExecHRest] Directory not found, try to mkdir" << endl;
      mkdir (tmp_str.c_str(), 0755);
    }
  // Exec HInit [label].hmm --> [label]
  // TODO: It is needed to remove [label] without .hmm at first
  sprintf (com, "%s/%s", tmp_str.c_str(), label.c_str());
  unlink (com);  // rm [label] file
  if (this->NumOfSample() < 4)
    sprintf (com, "cd %s; HInit -S filelist.txt -m %d -w 1.0 -l %s %s.hmm", tmp_str.c_str(), this->NumOfSample(), label.c_str(), label.c_str());
  else
    sprintf (com, "cd %s; HInit -S filelist.txt -w 1.0 -l %s %s.hmm", tmp_str.c_str(), label.c_str(), label.c_str());
  
  if(debug&&Gdebug) cerr << "[MotionDB.ExecHRest] com:" << com << endl;
  if(debug) tl_message("Now executing HInit...");
  system(com);
  // checking failure of HInit
  sprintf (com, "%s/%s", tmp_str.c_str(), label.c_str());
  if (stat (com, &st1)!=0)
    {
      tl_message ("HInit failed.");
      return FALSE;
    }
  if(debug&&Gdebug) tl_message ("%s : time = %d : size = %d", com, (int)st1.st_mtime, (int)st1.st_size);
  // change HMM filename (add suffix ".hmm")
  sprintf (com, "cp %s/%s %s/%s", tmp_str.c_str(), label.c_str(), tmp_str.c_str(), hmm_file.c_str());
  if(debug&&Gdebug) cerr << "[MotionDB.ExecHRest] com:" << com << endl;
  system(com);
  // Exec HRest
  if (NumOfSample() < 4)
    sprintf (com, "cd %s; HRest -S filelist.txt -w 1.0 -l %s %s", tmp_str.c_str(), label.c_str(), hmm_file.c_str());
  else
    sprintf (com, "cd %s; HRest -S filelist.txt -w 1.0 -l %s %s", tmp_str.c_str(), label.c_str(), hmm_file.c_str());
  if(debug&&Gdebug) cerr << "[MotionDB.ExecHRest] com:" << com << endl;
  if(debug) tl_message("Now executing HRest...");
  system(com);
  // checking failure of HRest by whether time stamp is updated or not
  sprintf (com, "%s/%s", tmp_str.c_str(), label.c_str());
  stat (com, &st2);
  if(debug&&Gdebug) tl_message ("%s : time = %d : size = %d", com, (int)st2.st_mtime, (int)st2.st_size);
  if ((int)st1.st_mtime==(int)st2.st_mtime && (int)st1.st_size==(int)st2.st_size)
    {
      tl_message ("HRest failed.");
      return FALSE;
    }
  // change HMM filename (add suffix ".hmm")
  sprintf (com, "cp %s/%s %s/%s", tmp_str.c_str(), label.c_str(), tmp_str.c_str(), hmm_file.c_str());
  return TRUE;
}



/**************************************************
 * Created  : 2003.09.05
 * Function : Create SaveDir
 * Memo     : If SaveDir is not existed, it will be created
 **************************************************/
int MotionDB::CreateSaveDir()
{
  string tmp;
  string com;
  int debug=1;
  
  if(debug&&Gdebug) cerr << "[MotionDB.CheckSaveDir] Creating SaveDir" << endl;
  
  tmp = "mkdir -p ";
  com = tmp + save_dir;
  if(debug&&Gdebug) tl_message ("command line : <%s>", com.c_str());
  system(com.c_str());

  tmp.erase();
  com.erase();

  return TRUE;
}



/**************************************************
 * Created  : 2005.09.16
 * Function : Increase the number of patterns with adding noises (with parameter)
 * MEMO     : Old version only copied [0]-th data, but now, all of the data is referred on copy
 **************************************************/
int MotionDB::IncreaseSamples(int num, double rate, int debug)
{
  int		i, sample, max_sample, length, src_length;
  double	noise;
  Behavior	*tempbeh;

  tl_message ("start : debug=%d", debug);
  debug = 0;
  rate = 0.15;
  max_sample = NumOfSample();
  if(debug) tl_message ("num=%d : rate = %g : sample=%d\n", num, rate, max_sample);
  for (sample=0; sample<max_sample; sample++)
    {
      for (i=0; i<num; i++)
	{
	  noise = rand();
	  noise = (double)noise / RAND_MAX;
	  if(debug) tl_message ("step 0, noise = %f", noise);
	  noise = noise * (double)(rate*2.0);    // rate=0.2 --> noise:0.4
	  if(debug) tl_message ("step 1, noise = %f", noise);
	  noise += (1.0-rate);  // rate=0.2 --> noise:0.8-1.2
	  if(debug) tl_message ("step 2, noise = %f", noise);
	  src_length = NthBehavior(sample)->Length();
	  length = (int)(src_length * noise);
	  if(debug) tl_message ("step 3, noise = %f", noise);
	  tempbeh = NthBehavior(sample)->ElasticCopy (length);
	  if(debug) 
	    {
	      tl_message ("Length of tempbeh just after ElasticCopy : %d", tempbeh->Length());
	      tl_message ("Length of Behavior [0] : %d",  NthBehavior(sample)  ->Length());
	      tl_message ("Length of tempbeh      : %d",  tempbeh->Length());
	    }
	  if(debug) tl_message ("step 2");
	  AddBehavior (tempbeh);
	  if(debug) tl_message ("Length of tempbeh after AddNoise : %d", tempbeh->Length());
	  if(debug) tl_message ("made length of %d , i=%d", length, i);
	}
    }
  if(debug) tl_message ("end");
  return 0;
}



/**************************************************
 * Created  : 2005.04.15	
 * Function : Increase the number of patterns with adding noises (without parameter)
 **************************************************/
int MotionDB::IncreaseSamples(int num)
{
  int		i, length, src_length, debug=1;
  double	noise;
  Behavior	*tempbeh;

  if(Gdebug) tl_message ("start");
  src_length = NthBehavior(0)->Length();
  for (i=0; i<num; i++)
    {
      noise = rand();
      noise = (double)noise / RAND_MAX;
      noise *= 0.4;
      noise += 0.8;  //0.8-1.2
      length = (int)(src_length * noise);
      if(debug&&Gdebug) tl_message ("step 1");
      tempbeh = NthBehavior(0)->ElasticCopy (length);
      if(debug&&Gdebug) tl_message ("Length of tempbeh just after ElasticCopy : %d", tempbeh->Length());
      if(debug&&Gdebug) {
	tl_message ("Length of Behavior [0] : %d",  NthBehavior(0)  ->Length());
	tl_message ("Length of tempbeh      : %d",  tempbeh->Length());
      }
      if(debug&&Gdebug) tl_message ("step 2");
      AddBehavior (tempbeh);
      if(debug&&Gdebug) tl_message ("Length of tempbeh after AddNoise : %d", tempbeh->Length());
      if(debug&&Gdebug) tl_message ("made length of %d", length);
    }
  if(Gdebug) tl_message ("end");
  return 0;
}






/**************************************************
 * Created  : 2003.09.06	
 * Modified : 2004 May 28th  by inamura
 * Function : Loading MotionDB from script file
 * Memo     : At first, parameters is loaded from script file
 *          : Motion pattern is not loaded on memory. Patterns will be loaded In other function (below).
 * Memo     : The script file is shared by both of learning and loading process
 **************************************************/
int MotionDB::Load(const char* fname)
{
  ifstream	fin;
  string	tmp_str;
  int		numstate, nummix, numsample;
  char		buf[MAX_STRING], tmpbuf[MAX_STRING], locallabel[MAX_STRING], loaddir[MAX_STRING], savedir[MAX_STRING], hmmfile[MAX_STRING];
  HmmType	hmmtype;
  int		debug=1;

  complement_dirname (fname, tmp_str);
  if(Gdebug&&debug) tl_message ("loading motionDB %s", tmp_str.c_str());
  fin.open (tmp_str.c_str(), ifstream::in);
  
  if(!fin)
    {
      tl_message ("Cannot open file: %s", tmp_str.c_str());
      return FALSE;
    }

  fin.getline(buf, MAX_STRING);
  if (tl_string_getkeyword (buf, "label:", locallabel)==TL_FALSE) return FALSE;
  fin.getline(buf, MAX_STRING);
  if (tl_string_getkeyword (buf, "read_dir:", loaddir)==TL_FALSE) return FALSE;
  fin.getline(buf, MAX_STRING);
  if (tl_string_getkeyword (buf, "save_dir:", savedir)==TL_FALSE) return FALSE;
  fin.getline(buf, MAX_STRING);
  if (tl_string_getkeyword (buf, "hmmfile:", hmmfile)==TL_FALSE) return FALSE;
  fin.getline(buf, MAX_STRING);
  if (tl_string_getkeyword (buf, "num_state:", tmpbuf)==TL_FALSE) return FALSE;
  tl_atoi (tmpbuf, &numstate);
  fin.getline(buf, MAX_STRING);
  if (tl_string_getkeyword (buf, "num_mixture:", tmpbuf)==TL_FALSE) return FALSE;
  tl_atoi (tmpbuf, &nummix);
  fin.getline(buf, MAX_STRING);
  if (tl_string_getkeyword (buf, "num_sample:", tmpbuf)==TL_FALSE) return FALSE;
  tl_atoi (tmpbuf, &numsample);
  fin.getline(buf, MAX_STRING);
  if (tl_string_getkeyword (buf, "hmm_type:", tmpbuf)==TL_FALSE) return FALSE;
  hmmtype = query_hmmtype_enum (tmpbuf);
  fin.close();
  Load (loaddir, savedir, locallabel, hmmfile, numsample, nummix, numstate, hmmtype);

  if(debug&&Gdebug) tl_message ("end");
  return TRUE;
}


/**************************************************
 * Created  : 2003/09/11	
 * Function : After setting of parameters, loading motion patterns on memory from files
 * Memo     :
 **************************************************/
int MotionDB::Load(const char* l_dir, const char* s_dir, const char* tmp_label, const char* tmp_hmmfile,
		   int tmp_numofsample, int tmp_numofmix, int tmp_numofstate, HmmType tmp_hmmtype)
{
  int		debug=0;

  if(debug) tl_message ("start");
  NumOfState (tmp_numofstate);
  NumOfMix (tmp_numofmix);
  //SetNumOfSample(tmp_numofsample);    comment out by inamura, 2004 Sep 14th
  if(debug) tl_message ("step 1 : load directory <%s>", l_dir);
  LoadDirectory (l_dir);
  if(debug) tl_message ("step 2 : save directory <%s>", s_dir);
  SaveDirectory (s_dir);
  if(debug) tl_message ("step 3 : label <%s>", tmp_hmmfile);
  Label ((char *)tmp_label);
  HMMFile (tmp_hmmfile);
  HMMtype(tmp_hmmtype);
  LoadBehaviors();
  if(debug) tl_message ("end");
  if(debug) Verify(debug);
  return TRUE;
}



/**************************************************
 * Modified : 2008-09-30 by inamura : change the name into NthBehavior
 * Function : Reference of n-th behavior instance
 * Memo     : counting starts from 0
 **************************************************/
Behavior* MotionDB::NthBehavior(int nth)
{
  if(nth > (int)behaviors.size()-1)
    {
      cerr << "[MotionDB.NthBehavior] (ERROR) range over!" << endl;
      cerr << "behs.size:" << (int)behaviors.size() << " nth:" << nth << endl;
      return NULL;
    }

  return behaviors[nth];
}


/*---------------------------------------------------------------------------*/
// Modified : 2004 Nov 24th by inamura
// Function : output of file for sampling_time
// TODO     : magic number [0] should be extinct
/*---------------------------------------------------------------------------*/
int MotionDB::TimeFileOut(void)
{
  char		fname[MAX_STRING];
  string	time_file;

  complement_dirname (save_dir.c_str(), time_file);
  sprintf(fname, "%s/%s.time", save_dir.c_str(), hmm_file.c_str());

  ofstream fout(fname);
  
  fout << "#sampling_time\t" << behaviors[0]->SamplingTime() << endl;

  fout.close();
  
  return TRUE;
}


/*---------------------------------------------------------------------------*/
// Function : Veirify the memory structure
/*---------------------------------------------------------------------------*/
int MotionDB::Verify(void)
{
  d_printf (Gdebug, "*****************  MotionDB_Verify Start  ***************\n");
  d_printf (Gdebug, " Load Directory   : %s \n", load_dir.c_str());
  d_printf (Gdebug, " Save Directory   : %s \n", save_dir.c_str());
  d_printf (Gdebug, " Label            : %s \n", label.c_str());
  d_printf (Gdebug, " Number of Sample : %d \n", this->NumOfSample());
  d_printf (Gdebug, " HMM model file   : %s \n", hmm_file.c_str());
  d_printf (Gdebug, "*****************  MotionDB_Verify End    ***************\n");

  return TL_TRUE;
}


/*---------------------------------------------------------------------------*/
// Function : Veirify the memory structure
/*---------------------------------------------------------------------------*/
int MotionDB::Verify(int flag)
{
  d_printf (flag, "*****************  MotionDB_Verify Start  ***************\n");
  d_printf (flag, " Load Directory   : %s \n", load_dir.c_str());
  d_printf (flag, " Save Directory   : %s \n", save_dir.c_str());
  d_printf (flag, " Label            : %s \n", label.c_str());
  d_printf (flag, " Number of Sample : %d \n", this->NumOfSample());
  d_printf (flag, " HMM model file   : %s \n", hmm_file.c_str());
  d_printf (flag, "*****************  MotionDB_Verify End    ***************\n");

  return TL_TRUE;
}
