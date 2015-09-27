/*
 * HTKUnit.cpp
 * 
 * Last Modified by Tetsunari Inamura on 2015 Sep 27
 * based on RecogUnit.cpp (old source code)
 *
 * This class manages only filename and directory name for HTK interface.
 * Real parameter data like A and B are managed by JHMM super-class.
 *
 *	Copyright (c) Tetsunari Inamura 2002--2015.
 */


/*
 * 2008-01-19
 *	Tried to implement Hellinger Distance, but not finished.
 * 2008-07-17
 *	Second trial of the Hellinger Distance implementation based on advice from Prof. Minematsu@UT
 */

#include <string.h>

#include "HTKUnit.h"

HTKUnit::HTKUnit()
{
  Reset();
}


/*-----------------------------------------------------------------------------------*/
// Created  : 2004 Sep 15th	by inamura
// Function : 諸条件付きのコンストラクタ
// ----------------------------------------------------------------------------------
// 返値     : label		: ラベル
// 返値     : save_dir		: 結果を保存するディレクトリ名
// 返値     : hmmfile		: HMM のファイル名
/*-----------------------------------------------------------------------------------*/
HTKUnit::HTKUnit(char *label, char *save_dir, char *hmmfile)
{
  string	tmp_str;
  int		debug=1;

  if(debug) tl_message ("hmmfile = %s", hmmfile);
  Reset();
  SetLabel(label);
  complement_dirname (save_dir, tmp_str);
  tmp_str += "/";
  tmp_str += hmmfile;
  SetHMMFile (tmp_str.c_str());
  if(debug) tl_message ("hmmfile = %s", hmmfile);
}


/*-----------------------------------------------------------------------------------*/
// Created  : 2008-01-17 by inamura
// Function : Same as the above method
// ----------------------------------------------------------------------------------
/*-----------------------------------------------------------------------------------*/
HTKUnit::HTKUnit(string label, string save_dir, string hmmfile)
{
  string	tmp_str;
  int		debug=1;

  if(debug) tl_message ("hmmfile = %s", hmmfile.c_str());
  Reset();
  SetLabel(label);
  complement_dirname (save_dir.c_str(), tmp_str);
  tmp_str += "/";
  tmp_str += hmmfile;
  SetHMMFile (tmp_str.c_str());
  if(debug) tl_message ("hmmfile = %s", tmp_str.c_str());
  Load (tmp_str.c_str());
}


/**************************************************
 * Created  : 2003/09/12	
 * Function : デストラクタ
 * Memo     : 
 **************************************************/
HTKUnit::~HTKUnit()
{
  work_dir.erase();
  hmm_file.erase();
  //label.erase();
}

/**************************************************
 * Created  : 2003/09/12	
 * Function : 初期設定
 * Memo     :
 **************************************************/
int HTKUnit::Reset(void)
{
  work_dir = DEFAULT_RECOG_WORK_DIR;
  
  return TRUE;
}

/**************************************************
 * Created  : 2003/09/12	
 * Function : WorkDirをセットする
 * Memo     : 
 **************************************************/
int HTKUnit::SetWorkDir(string &name)
{
  work_dir = name;

  return TRUE;
}

/**************************************************
 * Created  : 2003/09/12	
 * Function : WorkDirをセットする
 * Memo     : 
 **************************************************/
int HTKUnit::SetWorkDir(const char *name)
{
  work_dir = name;

  return TRUE;
}

/**************************************************
 * Created  : 2003/09/12	
 * Function : WorkDirを返す
 * Memo     :
 **************************************************/
string& HTKUnit::GetWorkDir()
{
  return work_dir;
}



#if 0
/**************************************************
 * Created  : 2003/09/12	
 * Function : labelをセットする
 * Memo     : work_dirもセットしてしまおう
 **************************************************/
int HTKUnit::SetLabel(string &name)
{
  label = name;
  work_dir = ".rec/" + label + "/";

  return TRUE;
}
/**************************************************
 * Created  : 2003/09/12	
 * Function : labelをセットする
 * Memo     : work_dirもセットしてしまおう
 **************************************************/
int RecogUnit::SetLabel(const char *name)
{
  label = name;
  work_dir = ".rec/" + label + "/";
  
  return TRUE;
}
#endif


/**************************************************
 * Created  : 2003/09/12	
 * Function : labelを返す
 * Memo     :
 **************************************************/
string HTKUnit::GetLabel()
{
  return label;
}


/**************************************************
 * Created  : 2003/09/12	
 * Function : hmm_fileをセットする
 * Memo     : 
 **************************************************/
int HTKUnit::SetHMMFile(string& name)
{
  hmm_file = name;

  return TRUE;
}


/**************************************************
 * Created  : 2003/09/12	
 * Function : hmm_fileをセットする
 * Memo     : 
 **************************************************/
int HTKUnit::SetHMMFile(const char *name)
{
  hmm_file = name;

  return TRUE;
}

/**************************************************
 * Created  : 2003/09/12	
 * Function : hmm_fileを返す
 * Memo     :
 **************************************************/
string& HTKUnit::GetHMMFile()
{
  return hmm_file;
}

/**************************************************
 * Created  : 2003/09/12	
 * Function : work_dirをつくる
 * Memo     : work_dirが存在していてもよい
 **************************************************/
int HTKUnit::CreateWorkDir()
{
  string tmp;
  string com;
  int debug=0;
  
  if(debug&&Gdebug) cerr << "[RecogUnit.CheckWorkDir] Creating SaveWokrDir" << endl;
  
  tmp = "mkdir -p ";
  com = tmp + work_dir;
  if(debug&&Gdebug) cerr << "[RecogUnit.CheckWorkDir] com:" << com << endl;
  if (system(com.c_str())==-1)
	  tl_warning("command line : <%s> failed", com.c_str());

  tmp.erase();
  com.erase();
  
  return TRUE;
}


/**************************************************
 * Created  : 2003.09.06	
 * Function : networkfileを作成
 * Memo     :
 **************************************************/
int HTKUnit::NetworkFileOut(const char *fname)
{
  ofstream fout(fname);

  fout << "$phn = " << label << "\n(<$phn>)\n";
  fout.close();

  return TRUE;
}


/**************************************************
 * Created  : 2003.09.06	
 * Function : latticefileを作成
 * Memo     :
 **************************************************/
int HTKUnit::LatticeFileOut(const char *fname)
{
  string zenhann;
  string kouhann;

  zenhann = "VERSION=1.0\nN=3 L=2\nSTART=1\nEND=2\nI=0 W=";
  kouhann = "\nI=1 W=!NULL\nI=2 W=!NULL\nJ=0 S=1 E=0\nJ=1 S=0 E=2\n";

  ofstream fout(fname);

  fout << zenhann << label << kouhann;

  zenhann.erase();
  kouhann.erase();
  fout.close();

  return TRUE;  
}


/**************************************************
 * Created  : 2003.09.06	
 * Function : dictfileを作成
 * Memo     :
 **************************************************/
int HTKUnit::DictFileOut(const char *fname)
{
  ofstream fout(fname);

  fout << label << " " << label << endl;
  fout.close();

  return TRUE;
}


/**************************************************
 * Created  : 2003.09.06	
 * Function : hlistfileを作成
 * Memo     :
 **************************************************/
int HTKUnit::HlistFileOut(const char *fname)
{
  ofstream fout(fname);

  fout << label << endl;

  fout.close();

  return TRUE;
}

//**************************************************
// Created  : 2003.09.12	
// Function : behの認識するための準備．HTK 用のファイルを準備している
// Memo     : HMMFileはPERIODICにしてコピーしている
//**************************************************
int HTKUnit::BeforeRecognize(void)
{
  int debug=0;

  string net_filename;
  string lattice_filename;
  string dict_filename;
  string hlist_filename;
  string com;
  string tmp_str;

  net_filename     = work_dir + label + ".net";
  lattice_filename = work_dir + label + ".lattice";
  dict_filename    = work_dir + label + ".dict";
  hlist_filename   = work_dir + label + ".hlist";

  if(debug&&Gdebug) cerr << "[RecogUnit.BeforeRecog] CreateWorkDir:" << work_dir << endl;
  CreateWorkDir();

  JHMM *tmp_hmm = NULL;
  tmp_hmm = new JHMM();
  if (tmp_hmm->Load(hmm_file.c_str())==FALSE)
    {
      cerr << "[RecogUnit.BeforeRecog] Failed to open HMM file" << endl;
      return FALSE;
    }
  tmp_hmm->ConvertToPERIODIC();
  tmp_str = work_dir + label;
  tmp_hmm->FileOut(tmp_str);
  
  NetworkFileOut (net_filename.c_str());
  LatticeFileOut (lattice_filename.c_str());
  DictFileOut    (dict_filename.c_str());
  HlistFileOut   (hlist_filename.c_str());
  
  net_filename.erase();
  lattice_filename.erase();
  dict_filename.erase();
  hlist_filename.erase();
  com.erase();
  tmp_str.erase();

  return TRUE;
}


/*-----------------------------------------------------------------------------------*/
// Created  : 2003.09.12	
// Function : behの認識．ある単一のクラスの認識尤度を求める．
// Memo     : 
// ----------------------------------------------------------------------------------
// 返値     : (double)		: 尤度
// 入力     : (Behavior *)	: 認識する時系列データ
/*-----------------------------------------------------------------------------------*/
double HTKUnit::Recognize(Behavior *beh)
{
  string htk_file;
  string tmp = "cd ";
  string com;
  string rec_filename;
  int	ret, debug=0;

  if(debug&&Gdebug) tl_message ("start");
  if (!beh)
    {
      tl_warning ("beh is NULL");
      return 0.0;
    }
  
  htk_file = work_dir + "test.htk";
  beh->HTKFileOut(htk_file.c_str());
  
  com = tmp + work_dir + "; HVite -w " + label + ".net -w " + label + ".lattice "
    + label + ".dict " + label + ".hlist test.htk";
  if(debug&&Gdebug) tl_message ("Exec command : %s", com.c_str());
  ret = system(com.c_str());
  if ((int)ret != 0) {
    tl_warning ("failed Recognition");
    rec_filename.erase();
    tmp.erase();
    com.erase();
    return 0.0;
  }
  rec_filename = work_dir + "test.rec";
  
  char buf[MAX_STRING], tmpbuf[MAX_STRING];
  ifstream fin(rec_filename.c_str());
  fin.getline(buf, MAX_STRING);

  int start, end;
  double value;

  sscanf (buf, "%d %d %s %lf", &start, &end, tmpbuf, &value);
  fin.close();

  htk_file.erase();
  tmp.erase();
  com.erase();
  rec_filename.erase();
  if(debug&&Gdebug) tl_message ("Recog value is %f", value);

  return value;
}


#if 1
/**************************************************
 * Created  : 2003.09.16
 * Function : ファイルから HTKUnitを読み込む
 * Memo     : MotionDB のと同じファイルを使用できる
 **************************************************/
int HTKUnit::LoadFromFile(const char *fname)
{
  ifstream	fin;
  string	tmp_str, tmp_hmm;
  char		buf[MAX_STRING], tmpbuf[MAX_STRING], tmp_label[MAX_STRING];
  char		tmp_s_dir[MAX_STRING], tmp_hmmfile[MAX_STRING];
  int		debug=0;

  complement_dirname (fname, tmp_str);
  fin.open (tmp_str.c_str(), ifstream::in);

  if(!fin)
    {
      tl_message ("Cannot open file:%s", tmp_str.c_str());
      return FALSE;
    }
  while((fin.getline(buf, MAX_STRING)) != NULL)
    {
      if(debug&&Gdebug) cerr << "[HTKUnit.Load] buf:" << buf << endl;
      if(!(strncmp (buf, "label:", 6)))
	{
	  sscanf(buf, "%s\t\t%s", tmpbuf, tmp_label);
	}
      if(!(strncmp (buf, "save_dir:", 9)))
	{
	  if(debug&&Gdebug) cerr << "[HTKUnit.Load] save_dir match:" << buf << endl;
	  sscanf(buf, "%s\t%s", tmpbuf, tmp_s_dir);
	}
      if(!(strncmp (buf, "hmmfile:", 8)))
	{
	  if(debug&&Gdebug) cerr << "[HTKUnit.Load] hmmfile match:" << buf << endl;
	  sscanf(buf, "%s\t%s", tmpbuf, tmp_hmmfile);
	}
    }
  tmp_str.erase();
  tmp_str = tmp_s_dir;
  tmp_hmm = tmp_str + tmp_hmmfile;
  if(Gdebug) cerr << "[HTKUnit.Load] label:" << tmp_label << endl;
  if(Gdebug) cerr << "[HTKUnit.Load] hmm:" << tmp_hmm << endl;
  SetLabel(tmp_label);
  SetHMMFile(tmp_hmm);

  tmp_hmm.erase();
  tmp_str.erase();

  return TRUE;
}
#endif



/**************************************************
 * Created  : 2008-01-09
 * Function : Claculating KL divergence between target HMM
 * Memo     : Using Monte-Calro based method
 **************************************************/
double HTKUnit::CalcKLDistance (HTKUnit *target_htk_unit)
{
  int		n = 200;	// number of Monte-Carlo sampling
  int		total_length=0;
  double	total_logp=0.0;
  Behavior	*beh;
  int		debug = 0;

  if (debug) tl_message ("start");
  for (int i=0; i<n; i++)
    {
      cerr << ".";
      beh = GenerateBehavior (GEN_NUM, GEN_NUM_Q);
      if (debug) tl_message ("step 1");
      total_length += beh->Length();
      if (debug) tl_message ("step 2");
      total_logp   += Recognize (beh);
      if (debug) tl_message ("step 3");
      total_logp   -= target_htk_unit->Recognize (beh);
    }
  cerr << endl;
  if (debug) tl_message ("end");
  total_logp /= total_length;

  return total_logp;
}



/**************************************************
 * Created   : 2008-07-17
 * Memo      : Just a copy from JHMM::HellingerDistance
 **************************************************/
double HTKUnit::CalcHellingerDistance (HTKUnit &target_htk_unit)
{
  int		debug=0;
  double	result=0.0;

  if(debug) tl_message ("start");
  for (int i=0; i<GetNumState()-2; i++)
    {
      if(debug) tl_message ("step 1.%d", i);
      result += GetNthState(i)->HellingerDistance( *(target_htk_unit.GetNthState(i)) );
    }
  if(debug) tl_message ("end");
  
  return result;
}


#if 0
/**************************************************
 * Created  : 2008-01-19
 * Function : Claculating Hellinger Distance between target HMM
 * Memo     : 
 **************************************************/
double HTKUnit::CalcHellingerDistance (HTKUnit *target_htk_unit)
{
  int		n = 200;	// number of Monte-Carlo sampling
  int		total_length=0;
  double	total_value=0.0, likelihood_o, likelihood_t;
  Behavior	*beh;
  int		debug = 1;

  if (debug) tl_message ("start");
  for (int i=0; i<n; i++)
    {
      cerr << ".";
      beh = GenerateBehavior (GEN_NUM, GEN_NUM_Q);
      if (debug) tl_message ("step 1");
      total_length += beh->Length();
      if (debug) tl_message ("step 2");
      likelihood_o = Recognize (beh);
      likelihood_t = target_htk_unit->Recognize (beh);
      total_value += ( exp (likelihood_o / 2) - exp (likelihood_t / 2) ) * ( exp (likelihood_o / 2) - exp (likelihood_t / 2) );
      if (debug) tl_message ("P(O|lambda_o) = %g, P(O|lambda_target) = %g", likelihood_o, likelihood_t);
      if (debug) tl_message ("step 3");
    }
  cerr << endl;
  if (debug) tl_message ("end");
  total_value /= total_length;

  return total_value;
}
#endif
