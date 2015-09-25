/*
 * behavior.cpp
 * Last Modified : Tetsunari Inamura on 2008-10-07
 */

#include "math.h"
#include "Behavior.h"
#include "mimesis.h"
#include "iirlib.h"

#include "HShell.h"     /* HMM ToolKit Modules */
#include "HMem.h"
#include "HMath.h"
#include "HSigP.h"
#include "HAudio.h"
#include "HWave.h"
#include "HVQ.h"
#include "HParm.h"
#include "HLabel.h"
#include "HModel.h"
#include "HTrain.h"
#include "HUtil.h"
#include "HAdapt.h"
#include "HFB.h"


/*
 * ChangeLog
 * 2008-09-27 by inamura
 *	Start to use type_tag sequence slot
 *	Eliminate angle_type slot
 *	Eliminate Sclae method
 *	Eliminate CoordinateToSpeed method
 *	change the method name KthAngle to NthAngle for Pose class
 * 2008-10-07 by inamura
 *	Header information for .beh file is not required.
 *	Old file with header could be read by current version.
 *	Pose structure is read from not only file stream but also string buffer
 * 2009-07-11 by inamura
 *	Comment was changed from Japanese to English
 *	Eliminate copy method for pose
 */


/* SwapInt32: swap byte order of int32 data value *p */
void SwapInt32(int *p)
{
   char temp,*q;
   
   q = (char*) p;
   temp = *q; *q = *(q+3); *(q+3) = temp;
   temp = *(q+1); *(q+1) = *(q+2); *(q+2) = temp;
}

/* SwapShort: swap byte order of short data value *p */
void SwapShort(short *p)
{
   char temp,*q;
   
   q = (char*) p;
   temp = *q; *q = *(q+1); *(q+1) = temp;
}



/**************************************************
 * Function : Behavior Constructor
 * Memo     : 
 **************************************************/
Behavior::Behavior()
{
  Reset();
}

Behavior::Behavior(int ref_dof, int time, char *name)
{
  Reset();
  dof=ref_dof;
  sampling_time=time;
  label = name;
}

/**************************************************
 * Function : Behavior constructor
 * Memo     : to be eliminate
 **************************************************/
Behavior::Behavior(int ref_dof)
{
  Reset();
  dof=ref_dof;
}


/**************************************************
 * Function : Destructor of Behavior class
 * Memo     : 
 **************************************************/
Behavior::~Behavior()
{
  int debug=0;

  if(debug&&Gdebug) cerr << "[~Behavior] start" << endl;

  if(!pose_t.empty()){
    if(debug&&Gdebug) cerr << "[~Behavior] size:" << pose_t.size() <<  endl;
    for(int i=0; i <(int)pose_t.size(); i++){
      if(debug&&Gdebug) cerr << i << " ";
      delete pose_t[i];
    }
  }
  pose_t.clear();
  label.erase();
  
  if(debug&&Gdebug) cerr << "[~Behavior] end" << endl;
}

/**************************************************
 * Function : Copy Constructor of Behavior class
 * Modified : 2008-09-27 by inamura : for adding type_tag
 * Memo     :
 **************************************************/
Behavior::Behavior(const Behavior &another)
{
  int debug=0;
  vector<sensorimotor_type>::iterator p;

  dof = another.dof;
  if(debug&&Gdebug) cerr << "[BehCopy] dof:" << dof << endl;
  sampling_time = another.sampling_time;
  if(debug&&Gdebug) cerr << "[BehCopy] samptime:" << sampling_time << endl;
  label = another.label;
  if(debug&&Gdebug) cerr << "[BehCopy] label:" << label << endl;

  type_tag.resize(0);
  for (int i=0; i<(int)another.type_tag.size(); i++)
    type_tag.push_back (another.type_tag[i]);

  Pose* new_pose = NULL;

  for (int i=0; i<(int)another.pose_t.size(); i++)
	{
	  if(debug&&Gdebug) cerr << "[BehCopy] " << i << "thPose Copy" << endl;
	  new_pose = new Pose(*(another.pose_t[i]));
	  pose_t.push_back(new_pose);
	  if(debug&&Gdebug) cerr << "Copy Fin" << endl;
	}
  if(debug&&Gdebug) cerr << "[BehCopy] end!" << endl;
}

/**************************************************
 * Function : Copy Constructor with changing the length of the pattern
 * Memo     : 
 **************************************************/
Behavior::Behavior(const Behavior &src_beh, int target_length)
{
  double step, vindex;
  int src_length;
  int debug=0;
  
  dof    = src_beh.dof;
  sampling_time = src_beh.sampling_time;
  label = src_beh.label;
  type_tag.resize(0);
  for (int i=0; i<(int)src_beh.type_tag.size(); i++)
    type_tag.push_back (src_beh.type_tag[i]);

  step   = 1.0 / (target_length-1);
  vindex = 0.0;
  src_length   = src_beh.pose_t.size();

  Pose* before_pose = NULL;
  Pose* after_pose = NULL;
  Pose* new_pose = NULL;
  double tmp, h;

  for (int i=0; i<target_length-1; i++)	// if the target length is n, duration is 0/(n-1) to (n-2)/(n-1)
    {
      if(debug&&Gdebug) cerr << "[beh.ElasticCopy] loop:" << i << " v_index:" << vindex << endl;

      new_pose    = new Pose();
      before_pose = src_beh.pose_t[(int)vindex];
      after_pose  = src_beh.pose_t[(int)(vindex+1)];
      h = vindex - (double)((int)(vindex));
      for (int j=0; j<dof; j++)
	{
	  tmp = before_pose->NthAngle(j) * (1-h) + after_pose->NthAngle(j) * h;
	  new_pose->AddAngle(tmp);
	}
      pose_t.push_back(new_pose);
      vindex += (double)(src_length-1) / (target_length-1);
    }

  new_pose = new Pose(*src_beh.pose_t[src_length-1]);
  pose_t.push_back(new_pose);
}



/**************************************************
 * Created  : 2003.11.19 by inamura
 * Modified : 2008-09-27 by inamura : add type_tag
 * Function : Just copy the object
 * Memo     :
 **************************************************/
Behavior *Behavior::Copy(void)
{
  int debug=0;
  Behavior *new_beh = new Behavior();
  
  if(debug&&Gdebug) cerr << "[BehCopy] start" << endl;
  new_beh->dof = dof;
  if(debug&&Gdebug) cerr << "[BehCopy] dof:" << new_beh->dof << endl;
  new_beh->sampling_time = sampling_time;
  if(debug&&Gdebug) cerr << "[BehCopy] samptime:" << new_beh->sampling_time << endl;
  new_beh->label = label;
  if(debug&&Gdebug) cerr << "[BehCopy] label:" << new_beh->label << endl;
  new_beh->type_tag.resize(0);
  for (int i=0; i<(int)type_tag.size(); i++)
    new_beh->type_tag.push_back (type_tag[i]);


  Pose* new_pose = NULL;

  for (int i=0; i<(int)pose_t.size(); i++)
    {
      //if(debug&&Gdebug) cerr << "[BehCopy] " << i << "thPose Copy" << endl;
      new_pose = new Pose(*(pose_t[i]));
      new_beh->pose_t.push_back(new_pose);
      //if(debug&&Gdebug) cerr << "Copy Fin" << endl;
    }
  if(debug&&Gdebug) cerr << "[BehCopy] end" << endl;

  return new_beh;
}




/**************************************************
 * Function : Reset object
 * Memo     : 
 **************************************************/
int Behavior::Reset(void)
{
  dof           = DEFAULT_BEH_DOF;
  sampling_time = DEFAULT_BEH_TIME;
  label         = DEFAULT_BEH_LABEL;
  
  pose_t.clear();

  return TRUE;
}



/**************************************************
 * Created  : 2003.Nov.17
 * Function : replace pose_t with new pose_t_new
 * Memo     : 
 **************************************************/
int Behavior::ReplacePoses(vector<Pose *> *pose_t_new_pointer)
{
  int debug=0;
  int size = (int)pose_t.size();
  
  if(debug&&Gdebug) tl_message ("original size=%d , new size=%d", size, (int)((*pose_t_new_pointer).size()) );

  if(size < 0) {
    if(debug&&Gdebug) cerr << "[Beh.ReplacePoses] pose_t.size() = " << size << endl;
  }
  if(debug&&Gdebug) cerr << "[Beh.ReplacePoses] deleting pose_t" << endl;
  for(int i=0; i<size ; i++)
    pose_t.pop_back();

  size = (int)(*pose_t_new_pointer).size();
  if(debug&&Gdebug) cerr << "[Beh.ReplacePoses] push_back pose_t_new" << endl;

  for(int i=0; i<size; i++)
    {
      Pose *tmp_pointer = new Pose(*((*pose_t_new_pointer)[i]));
      pose_t.push_back(tmp_pointer);
    }
  if(debug&&Gdebug) tl_message ("End : new size=%d", (int)(pose_t.size()) );
  
  return TRUE;
}





/**************************************************
 * Created  : 2005/04/18 by inamura
 * Function : without lable string
 **************************************************/
int Behavior::LoadBVH(const char* fname)
{
  return LoadBVH (fname, label.c_str());
}



/**************************************************
 * Created  : 2003-10-23
 * Modified : 2005-04-18 by inamura
 * Function : Loading from BVH file directory
 * Memo     : Angle is written by degree in BVH, it might be radian in beh
 **************************************************/
int Behavior::LoadBVH(const char* fname, const char *label_str)
{
  ifstream	fin(fname);
  char		buf[MAX_STRING], tmpbuf[MAX_STRING], tmpbuf2[MAX_STRING];
  char		*charp=NULL;
  int		length;
  double	data;
  int		debug=0;
  int		tempcounter=0;

  if(!fin) {
    tl_message ("Cannot open '%s'", fname);
    return FALSE;
  }
  if(debug&&Gdebug) tl_message ("Open: %s", fname);

  dof = 57;		// DOF of BVH is always 57
  Label (label_str);
  tl_message ("label (%s) is set , result is %s", label_str, Label().c_str());

  /* Checking the syntax of bvh format */
  /* 1st line : HIERARCHY */
  fin.getline(buf, MAX_STRING);
  if(strncmp (buf, "HIERARCHY", 9)){
    cerr << "[Beh.LoadBVH] File doesn't contain HIERARCHY entry. :" << buf << endl;
    return FALSE;
  }
  /* 2nd line : ROOT */
  fin.getline(buf, MAX_STRING);
  if(strncmp (buf, "ROOT Hips", 4)){
    cerr << "[Beh.LoadBVH] File doesn't contain ROOT entry. :" << buf << endl;
    return FALSE;
  }
  /* counting number of '( )' (?) ---- under construction... */
  /* reading until MOTION comes */
  while(1)
    {
      tempcounter++;
      //    cerr << "buf: " << tempcounter << ": " << buf << endl;
      if(tempcounter > 150) { /* magic number ?? */
	cerr << "[Beh.LoadBVH] MOTION not found." << endl;
	return FALSE;
      }
      fin.getline(buf, MAX_STRING);
      if (strncmp (buf, "MOTION", 5) == 0) {
	if(debug&&Gdebug) cerr << "found MOTION entry." << endl;
	break;
      }
    }
  // If Frames doesn't come after MOTION, then error
  fin.getline(buf, MAX_STRING);
  if (strncmp(buf, "Frames:", 7)) {
    cerr << "[Beh.LoadBVH] File doesn't contain Frames entry :" << buf << endl;
    return FALSE;
  }
  sscanf(buf, "%s %d", tmpbuf, &length);
  if(debug&&Gdebug) cerr << "[Beh.LoadBVH] NUM_POSE:" << length << endl;
  // If no Frame Time, then error
  fin.getline(buf, MAX_STRING);
  if (strncmp(buf, "Frame Time:", 11))
    {
      cerr << "[Beh.LoadBVH] File doesn't contain Frames entry :" << buf << endl;
      return FALSE;
    }
#if 1
  sscanf(buf, "%s %s %lf", tmpbuf, tmpbuf2, &data);
  SamplingTime ((int)(data * 1000)); // Fit the unit in [msec]
  if(debug&&Gdebug) tl_message ("Sampling Time = %d", SamplingTime());
#endif  
  Pose* new_pose = NULL;

  for (int i=0; i<length; i++)
    {
      fin.getline(buf, MAX_STRING);
      if(debug&&Gdebug) cerr << "[Beh.LoadBVH] pose getting:" << i << endl;
      new_pose = new Pose;
      if (new_pose==NULL) {
	tl_warning ("failed to malloc new Pose : loop %d", i);
	return FALSE;
      }
      charp = buf;
      for (int j=0; ;j++)
	{
	  if (sscanf_double_with_pointer_move (&charp, &data) == FALSE)
	    break;
	  data *= (M_PI / 180);
	  new_pose->AddAngle (data);
	}
      pose_t.push_back(new_pose);
      if (debug && (i%50==0))
	{
	  tl_message ("verifing %d-th pose", i);
	  NthPose(i)->Verify();
	}
    }
  fin.close();
  if(debug&&Gdebug) 
    {
      tl_message ("Finish!  time length = %d", Length());
      tl_message ("label = %s", Label().c_str());
    }
  return TRUE;
}




int Behavior::Load(const char* fname)
{
  int	debug=0;

  return Load(fname, debug);
}



/**************************************************
 * Modified : 2008-10-07 by inamura : options could be omitted
 * Function : Loading from .beh file
 * Memo     : If the input string is not absolute path, it will be transformed in relative path
 **************************************************/
int Behavior::Load(const char* fname, int debug)
{
  ifstream	fin;
  string	tmp_str;
  char		tmpbuf[MAX_STRING], buf[MAX_STRING];
  char		name[MAX_STRING];
  int		ref_dof, length, t_step;

  if (TypeTagList==NULL)
    {
      if(debug&&Gdebug) tl_message ("TypeTagList is null");
      mimesis_init_typetaglist();
    }
  else
    if(debug&&Gdebug) tl_message ("TypeTagList is not null");

  complement_dirname (fname, tmp_str);
  fin.open (tmp_str.c_str(), ifstream::in);

  if(!fin){
    cerr << "[Beh.Load] Cannot open '" << fname << "'." << endl;
    return FALSE;
  }
  if(debug&&Gdebug) cerr << "[Beh.Load] Open:" << fname << endl;

  dof = 0;
  length = 0;
  sampling_time = 0;
  label = "unknown";
  type_tag.resize(0);
  for(;;)		// Reading Header information
    {
      fin.getline(buf, MAX_STRING);
      if (!strncmp (buf, "#dof", 4))
	{
	  sscanf(buf, "%s %d", tmpbuf, &ref_dof);
	  dof = ref_dof;
	  if(debug&&Gdebug) cerr << "[Beh.Load] DOF:" << dof << endl;
	}
      else if (!strncmp(buf, "#num_of_poses", 13))
	{
	  sscanf(buf, "%s %d", tmpbuf, &length);
	  if(debug&&Gdebug) cerr << "[Beh.Load] NUM_POSE:" << length << endl;
	}
      else if (!strncmp (buf, "#sampling_time", 14))
	{
	  sscanf(buf, "%s %d", tmpbuf, &t_step);
	  if(debug&&Gdebug) cerr << "[Beh.Load] T_STEP:" << t_step << endl;
	  sampling_time = t_step;
	}
      else if (!strncmp(buf, "#angle_type", 11))
	{
	  tl_message ("This file seems to be old version. check carefully!! because angle_type is included");
	}
      else if (!strncmp(buf, "#label", 6))
	{
	  sscanf(buf, "%s %s", tmpbuf, name);
	  label = name;
	  if(debug&&Gdebug) cerr << "[Beh.Load] LABEL:" << name << endl;
	}
      else if (!strncmp(buf, "#type_tag", 9))
	{
	  GList	*tag_list = NULL;
	  int	index;
	  fin.getline(buf, MAX_STRING);
	  tag_list = tl_csv_to_glist (buf);
	  for (int i=0; i<(int)g_list_length(tag_list); i++)
	    {
	      index = tl_glist_search_string (TypeTagList, (char *)g_list_nth_data (tag_list, i));
	      if(debug&&Gdebug) tl_message ("%d-th tag : %s (# %d)", i, (char *)g_list_nth_data (tag_list, i), index);
	      type_tag.push_back ((sensorimotor_type)index);
	    }
	}
      else if (buf[0]>='0' && buf[0] <='9')
	break;
    }
  
  Pose* new_pose = NULL;

  for(int i=0;;i++)
    {
      if(debug&&Gdebug) cerr << "[Beh.Load] pose getting:" << i << endl;
      new_pose = new Pose;
      new_pose->Load(buf);	// new method on 2008-10-07
      pose_t.push_back(new_pose);
      if (fin.getline(buf, MAX_STRING)==NULL)
	break;
    }
  fin.close();

  dof = NthPose(0)->Dof();
  sampling_time = NthPose(0)->SamplingTime();

  if(debug&&Gdebug)
    {
      tl_message ("Finish!");
      Verify();
    }
  return TRUE;
}




/*---------------------------------------------------------------------------*/
// Create : 2004 Jun 10th
// Function : Add noise for amplitute
/*---------------------------------------------------------------------------*/
int Behavior::AddNoise(void)
{
  int i, j, debug=0;
  if(debug&&Gdebug) cerr << "[Beh.AddNoise] start" << endl;

  int dof = Dof();
  double noise[dof];
  if(debug&&Gdebug) cerr << "[Beh.AddNoise] start" << endl;
  Pose* new_pose = NULL;
  Pose* before_pose = NULL;
  if(debug&&Gdebug) cerr << "[Beh.AddNoise] start" << endl;
  vector<Pose *> pose_t_new;
  
  if(debug&&Gdebug) cerr << "AddNoise called" << endl;
  
  for(i=0; i <dof; i++)
    {
      noise[i] = rand();
      noise[i] = (double)noise[i] / RAND_MAX;
      noise[i] *= 0.2;
      noise[i] += 0.9;  //0.9-1.1
    }
  for (i=0; i<Length(); i++)
    {
      new_pose = new Pose();
      before_pose = pose_t[i];
      for (j=0; j<Dof(); j++)
	{
	  new_pose->AddAngle(before_pose->NthAngle(j) * noise[j]);
	}
      pose_t_new.push_back(new_pose);
    }
  ReplacePoses (&pose_t_new);

  if(debug&&Gdebug) cerr << "[Beh.AddNoise] replaced poses" << endl;
  
  for(int i=0; i<(int)pose_t_new.size(); i++) {
    pose_t_new.pop_back();
  }

  if(debug&&Gdebug) cerr << "[Beh.AddNoise]End" << endl;
  return TRUE;
}




/**************************************************
 * Function : Reference of DOF
 * Memo     : 
 **************************************************/
int Behavior::Dof()
{
  return dof;
}


/**************************************************
 * Function : Set DOF
 * Memo     : debuged on 2005 Apr 18th by inamura
 **************************************************/
int Behavior::Dof(const int value)
{
  if (value>0)
    {
      dof = value;
      return TRUE;
    }
  tl_warning ("dof value is not appropriate : %d", value);
  return FALSE;
}


/**************************************************
 * Modified : 2004 Sep 13th	by inamura
 * Function : Sampling_Timeを返す
 **************************************************/
int Behavior::SamplingTime(void)
{
  if(sampling_time==0){
    cerr << "[Beh.SapmleTime] (ERROR) is 0!" << endl;
    return 0;
  }
  return sampling_time;
}

/**************************************************
 * Created  : 2004 Sep 13th	by inamura
 * Function : Set Sampling_Time
 **************************************************/
int Behavior::SamplingTime(const int val)
{
  if (val<0)
    {
      tl_message ("The value %d is wrong", val);
      return FALSE;
    }
  sampling_time = val;
  return TRUE;
}

/**************************************************
 * Function : Return number of poses
 * Memo     : 
 **************************************************/
int Behavior::Length()
{
  return (int)pose_t.size();
}


/*-----------------------------------------------------------------------------------*/
// Created  : 2004 Jun 8th   by inamura
// Function : return pointer for pose sequence, which is private
/*-----------------------------------------------------------------------------------*/
vector<Pose *> Behavior::GetPoses (void)
{
  return pose_t;
}


/**************************************************
 * Function : Reference for Label
 * Memo     : 
 **************************************************/
string& Behavior::Label()
{
  //  if(label.empty()){
  //    cerr << "Behavior.Label is 0" << endl;
  //    return "unknown";
  //  }
  return label;
}


/**************************************************
 * Function : Set Label
 * Debugged : 2005 Apr 19th by inamura
 **************************************************/
int Behavior::Label(const char *str)
{
  if (str) {
    label = str;
    return TRUE;
  }
  tl_warning ("given label is strange <%s>", str);
  return FALSE;
}


/**************************************************
 * Function : Set Label
 * Memo     : 
 **************************************************/
int Behavior::Label(string &str)
{
  label = str;
  return TRUE;
}


/**************************************************
 * Function : Reference for n-th pose
 * Memo     : 
 **************************************************/
Pose* Behavior::NthPose(int nth)
{
  if(nth >= (int)pose_t.size()){
    cerr << "[Beh.NthPose] (ERROR) " << nth << "thPose not exist!" << endl;
    return NULL;
  }
  return pose_t[nth];
}

/**************************************************
 * Function : Add new Pose
 * Memo     : 
 **************************************************/
void Behavior::AddPose(Pose* new_pose)
{
  pose_t.push_back(new_pose);
}

/**************************************************
 * Function : debug display of Behavior
 * Memo     : 
 **************************************************/
void Behavior::Show()
{
  cerr << "[Beh.Show] dof:" << dof << endl;
  cerr << "[Beh.Show] stime:" << sampling_time << endl;
  cerr << "[Beh.Show] label:" << label << endl;

  for(int i=0; i<Length(); i++){
    cerr << "[Beh.Show] " << i << "thPose Show" << endl;
    pose_t[i]->Show();
  }
}



/**************************************************
 * Function : Dump contents of the instance into file
 * Modified : 2008-09-27 by inamura
 * Memo     : type_tag is added for output fields
 **************************************************/
int Behavior::FileOut(const char* fname)
{
  ofstream fout(fname);
  int debug=0;
  if(debug&&Gdebug) tl_message ("start");
  if(debug&&Gdebug) Verify();
  
  if(!fout){
    cerr << "[Beh.FileOut] (ERROR) Cannot open file:" << fname << endl;
    return FALSE;
  }

  if(debug&&Gdebug) tl_message ("step 1");
  if(debug&&Gdebug) cerr << "[Beh.FileOut] dof:" << dof << endl;
  fout << "#dof	" << dof << endl;
  fout << "#num_of_poses	" << Length() << endl;
  fout << "#sampling_time	" << sampling_time << endl;
  //fout << "#angle_type	" << 1 << endl; // deleted on 2008-09-27
  fout << "#label	" << Label() << endl;
  if(debug&&Gdebug) tl_message ("step 2");
  if (type_tag.size()!=0)
    {
      fout << "#type_tag" << endl;
      for (int i=0; i<dof-1; i++)
	{
	  if(debug&&Gdebug) tl_message ("type tag #%d <%s>", (int)(type_tag[i]), TypeTagString[(int)(type_tag[i])]);
	  fout << TypeTagString[(int)(type_tag[i])] << ",";
	}
      if(debug&&Gdebug) tl_message ("type tag #%d <%s>", (int)(type_tag[dof-1]), TypeTagString[(int)(type_tag[dof-1])]);
      fout << TypeTagString[(int)(type_tag[dof-1])] << endl;
    }

  if(debug&&Gdebug) cerr << "pose_t.size()" << pose_t.size() << endl;
  
  for (int i=0; i<Length(); i++)
    pose_t[i]->FileOut(fout, sampling_time);

  fout.close();
  if(debug&&Gdebug) cerr << endl <<  "[Beh.FileOut] End" << endl;
  return TRUE;
}


/**************************************************
 * Function : Shift a phase of Behavior instance
 * Memo     : 
 **************************************************/
void Behavior::Phase_Shift(int start)
{
  int length;
  int debug=0;

  length = Length();
  
  if(debug&&Gdebug)
    cerr <<"[Beh.Phase_Shift] Length:" << length << " Start:" << start << endl;
  vector<Pose *>::iterator begin_i = pose_t.begin();
  vector<Pose *>::iterator start_i = begin_i+start;
  vector<Pose *>::iterator end_i = pose_t.end();

  vector<Pose *> temp;
  temp.assign(start_i, end_i);
  pose_t.erase(start_i, end_i);
  start_i = temp.begin();
  end_i = temp.end();
  pose_t.insert(begin_i, start_i, end_i);
}  




/*-----------------------------------------------------------------------------------*/
// Function : Remove n-th pose (element)
/*-----------------------------------------------------------------------------------*/
// 入力	    : nth		: Natural number starts from 0
/*-----------------------------------------------------------------------------------*/
int Behavior::RemoveNthFrame (int nth)
{
  vector<Pose *>::iterator	index = pose_t.begin();
  int		i;

  for (i=0; i<nth; index++,i++)

  if (nth<0 || nth>=(int)pose_t.size())
    {
      tl_warning ("index %d is wrong, The size of pose_t is %d", nth, (int)(pose_t.size()));
      return FALSE;
    }
  
  pose_t.erase (index);
  return TRUE;
}



/**************************************************
 * Modified : 2004 May 29th by inamura
 * Function : Remove i-th angle from behavior instance
 * Memo     : old name was Remove
 **************************************************/
int Behavior::RemoveNthAngle (int i)
{
  int debug=0;
  vector<Pose *> pose_t_new;

  if(debug&&Gdebug) tl_message ("start");
  
  for(int j=0; j<Length(); j++)
    {
#if 0
      Pose* new_pose(pose_t[j]);
      new_pose->RemoveNthAngle(i);
      pose_t_new.push_back(new_pose);
#endif
      pose_t[j]->RemoveNthAngle(i);
    }  
#if 0
  ReplacePoses(&pose_t_new);
  for(int j=0; j<(int)pose_t_new.size(); j++) {
    pose_t_new.pop_back();
  }
  dof = pose_t[0]->Dof(); //update of DoF of Beh instance
#endif
  dof--;

  if(debug&&Gdebug) tl_message ("Dof = %d", Dof());
  if(debug&&Gdebug) tl_message ("end");

  return TRUE;
}

 


/**************************************************
 * Created  : 2005.04.15
 * Debugged : 2008-10-01 by inamura : type_tag copy
 * Function : Create new instance with changing time length
 **************************************************/
Behavior *Behavior::ElasticCopy (int target_length)
{
  double step, vindex;
  int src_length, count;
  int debug=0;
  vector<Pose *> pose_t_new;
  Behavior   *new_beh = new Behavior();

  new_beh->Dof (dof);
  if(debug&&Gdebug) {
    tl_message ("src dof is %d", dof);
    tl_message ("dof becomes %d", new_beh->Dof());
  }
  new_beh->SamplingTime (sampling_time);
  if(debug&&Gdebug) tl_message ("sampling time becomes %d", sampling_time);
  new_beh->Label (label);
  if(debug&&Gdebug) tl_message ("label becomes %s", label.c_str());

  step   = 1.0 / (target_length-1);
  vindex = 0.0;
  src_length = Length();

  Pose* before_pose = NULL;
  Pose* after_pose = NULL;
  Pose* new_pose = NULL;
  double tmp, h;

  if(debug&&Gdebug) tl_message ("Start");
  if(debug&&Gdebug) Verify();
  
  for (int i=0; i<target_length-1; i++)	// in case of the time length was n, loop duration is from 0 to (n-2) (loop length is (n-1))
    {
      if(debug&&Gdebug) tl_message ("loop: %d\t v_index: %g,\t src_length : %d, target_length : %d", i, vindex, src_length, target_length);

      new_pose    = new Pose();
      before_pose = NthPose((int)vindex);
      after_pose  = NthPose((int)(vindex+1));
      h = vindex - (double)((int)(vindex));
      if(debug&&Gdebug) tl_message ("vindex = %g", vindex);
      
      for (int j=0; j<dof; j++)
	{
	  //tl_message ("before_pose->KthAngle(%d) : %g\t\tafter_pose->KthAngle(%d) : %g", j, before_pose->KthAngle(j), j, after_pose->KthAngle(j));
	  tmp = before_pose->NthAngle(j) * (1-h) + after_pose->NthAngle(j) * h;
	  new_pose->AddAngle(tmp);
	}
      pose_t_new.push_back (new_pose);
      vindex += (double)(src_length-1) / (target_length-1);
    }

  new_pose = new Pose (*NthPose(src_length-1));
  pose_t_new.push_back(new_pose);

  new_beh->ReplacePoses(&pose_t_new);	// ReplacePoses doesn't remove received data
  count = (int)pose_t_new.size();
  for(int i=0; i<count; i++) {
    pose_t_new.pop_back();
  }
  new_beh->TypeTags (TypeTags());    // added on 2008-10-01 by inamura
  if(debug&&Gdebug) tl_message ("Finish! new length is %d", new_beh->Length());
  if(debug&&Gdebug) tl_message ("dof becomes %d", new_beh->Dof());
  if(debug&&Gdebug) tl_message ("sampling time becomes %d", new_beh->SamplingTime());
  if(debug&&Gdebug) tl_message ("label becomes %s", new_beh->Label().c_str());

  return new_beh;
}




/**************************************************
 * Function : Concatenate of Behavior instance
 * Memo     : add_beh could be erased
 **************************************************/
void Behavior::Cat(Behavior &add_beh)
{
  Pose* tmp = NULL;

  for(int i=0; i<(int)add_beh.Length(); i++){
    //    cerr << "--------------        add pose " << i << endl;
    tmp = new Pose(*(add_beh.NthPose(i)));
    AddPose(tmp);
  }
}


/**************************************************
 * Modified : 2005 Jun 21st by inamura
 * Function : Create HTK data file
 * Memo     : MAXDOF was extinct
 **************************************************/
int Behavior::HTKFileOut (const char *filename)
{
  int		i, j, vecsize, numofsamples;
  float		*ptr;
  FILE 		*fp=NULL;
  htk_header_t	header;
  Pose *pose;
  int debug=0;

  if(debug&&Gdebug) Verify();
  if(debug&&Gdebug) tl_message ("Start!");
  vecsize      = dof;
  numofsamples = Length();
  ptr = (float *)malloc (dof * sizeof(float));

  if(debug&&Gdebug) cerr << "[Behavior.HTKFileOut] vecsize: " << vecsize << endl;
  
  fp = fopen(filename, "w");
  if(debug&&Gdebug) cerr << "[Behavior.HTKFileOut] fopen:" << filename << endl;

  // if samplPeriod indicates large value, HTK would be error. So, checking max value
  if (sampling_time>60) {
    tl_message ("sampling_time is too big, set the value to 60.");
    sampling_time = 60;
  }
  header.nSamples   = numofsamples;		// number of samples
  header.sampPeriod = sampling_time * 10 * 1000;// unit of sampling time is [ms], unit of endtime is [100ns]
  header.sampSize   = vecsize * sizeof(float);	// size (byte) of each sample
  header.parmKind = (short)H_USER;
  
  if(debug&&Gdebug) tl_message ("header.nSamples:   %d", header.nSamples);
  if(debug&&Gdebug) tl_message ("header.sampPeriod: %d", header.sampPeriod);
  if(debug&&Gdebug) tl_message ("header.sampSize:   %d", header.sampSize);
  if(debug&&Gdebug) tl_message ("header.paramKind:  %d", header.parmKind);

  SwapInt32 (&header.nSamples);  SwapInt32 (&header.sampPeriod);
  if(debug&&Gdebug) tl_message ("Step 0");
  SwapShort (&header.sampSize);  SwapShort (&header.parmKind);
  if(debug&&Gdebug) tl_message ("Step 1");
  fwrite (&header.nSamples,      sizeof(int),   1, fp);
  if(debug&&Gdebug) tl_message ("Step 2");
  fwrite (&header.sampPeriod,    sizeof(int),   1, fp);
  if(debug&&Gdebug) tl_message ("Step 3");
  fwrite (&header.sampSize,      sizeof(short), 1, fp);
  fwrite (&header.parmKind,      sizeof(short), 1, fp);
  if(debug&&Gdebug) tl_message ("Before for-loop");
  for (i=0; i<numofsamples; i++)
    {
      int	dummy, *dummyp;
      if(debug&&Gdebug) tl_message ("Step %d.0", i);
      pose = NthPose(i);
      if(debug&&Gdebug) tl_message ("Step %d.1", i);
      for (j=0; j<vecsize; j++)
      	{
      	  ptr[j] = pose->NthAngle(j);
      	}
      if(debug&&Gdebug) tl_message ("Step %d.2", i);
      for (j=0; j<vecsize; j++)
	{
	  dummyp = (int *)(ptr+j);
	  dummy = *dummyp;
	  SwapInt32 (&dummy);// convert into integer to swap 
	  if(fwrite (&dummy, sizeof(float), 1, fp) != 1) {
	    fclose (fp);
	    return FALSE;
	  }
	}
      if(debug&&Gdebug) tl_message ("Step %d.3", i);
    }
  fclose(fp);
  free (ptr);
  if(debug&&Gdebug) cerr << "[Behavior.HTKFileOut] Finish!" << endl;
  return TRUE;
}


/**************************************************
 * Function : Create label file for HTK
 * Memo     :
 **************************************************/
int Behavior::MakeLabelFile (const char *filename)
{
  int		starttime=0, endtime=0;	// unit of the time is [*100ns] in HTK

  starttime = 0;
  // unit of sampling time is [ms], unit of endtime is [100ns]
  endtime   = sampling_time * Length() * 10 * 1000; 

  ofstream fout(filename);
  fout << starttime << " " << endtime << " " << label << endl;

  fout.close();

  return TRUE;
}


/**************************************************
 * Function : Pop (remove) the first pose
 * Memo     : 
 **************************************************/
Pose* Behavior::PopFrontPose()
{
  Pose* first_pose;
  int debug=0;
  
  if(debug&&Gdebug) cerr << "[Behavior.PopFrontPose] before length:" << Length() << endl;
  first_pose = pose_t[0];
  
  vector<Pose *>::iterator p = pose_t.begin();
  pose_t.erase(p, p+1);

  if(debug&&Gdebug) cerr << "[Behavior.PopFrontPose] after length:" << Length() << endl;
  return first_pose;
}


// Created by inamura on 2008-09-19
// Function : Refer the sensorimotor type of i-th element
sensorimotor_type Behavior::NthType (int i)
{
  if (i<0 || i > Dof()-1)
    {
      tl_warning ("input index %d is wroing", i);
    }
  return type_tag[i];
}


// Created by inamura on 2008-09-30
int Behavior::TypeTags (vector <sensorimotor_type> tags)
{
  type_tag.resize(0);
  for (int i=0; i<(int)tags.size(); i++)
    {
      type_tag.push_back (tags[i]);
    }
  return 1;
}

// Created by inamura on 2008-09-30
vector<sensorimotor_type> Behavior::TypeTags (void)
{
  return type_tag;
}


/*---------------------------------------------------------------------------*/
// Create : 2004 Jun 10th
// Function : Verify the structure of instance
/*---------------------------------------------------------------------------*/
int Behavior::Verify (void)
{
  d_print ("##-------------------   Behavior_Verify  (start)  ----------------\n");
  d_print ("##    dof              = %d\n", Dof());
  d_print ("##    length           = %d\n", Length());
  d_print ("##    sampling_time    = %d\n", SamplingTime());
  d_print ("##    label            = %s\n", Label().c_str());
  d_print ("##    type_tag length  = %d\n", type_tag.size());
  for (int i=0; i<(int)type_tag.size(); i++)
    d_print ("%s(%d)   ", TypeTagString[type_tag[i]], (int)type_tag[i]);
  d_print ("\n");

  if (Length()>0)
    {
      d_print ("##    check first pose...\n");
      pose_t[0]->Verify();
      if (Length()>100) {
	d_print ("##    check 100th pose...\n");
	pose_t[100]->Verify();
      }
      if (Length()>200) {
	d_print ("##    check 200th pose...\n");
	pose_t[200]->Verify();
      }
    }
  d_print ("##-------------------   Behavior_Verify   (end)   ----------------\n");
  
  return TL_TRUE;
}
  




/**************************************************
 * Function : Constructor of Pose
 * Memo     : 
 **************************************************/
Pose::Pose()
{
  Reset();
}


/**************************************************
 * Function : Destructor of Pose
 * Memo     : 
 **************************************************/
Pose::~Pose()
{
  int debug=0;
  
  if(debug&&Gdebug) cerr << "[Pose.~Pose] start" << endl;
  if(!angle.empty()){
    if(debug&&Gdebug) cerr << "[Pose.~Pose] angle clear" << endl;
    angle.clear();
  }
  if(debug&&Gdebug) cerr << "[Pose.~Pose] Finish!" << endl;
}


/**************************************************
 * Function : Copy constructor of Pose
 * Memo     : TODO: this method could be erased because sampling_time and dof is not used
 **************************************************/
Pose::Pose(const Pose &another)
{
  double tmp;

  for(int i=0; i<(int)another.angle.size(); i++){
    tmp = another.angle[i];
    angle.push_back(tmp);
  }
  
}


/**************************************************
 * Created  : 2005.Jun.23 by sonoda
 * Function : Pose to another instance
 * Memo     : 
 **************************************************/
int Pose::CopyTo(Pose &another) const
{
  another.angle.resize(Dof());
  std::vector<double>::iterator out_i=another.angle.begin();
  for (std::vector<double>::const_iterator in_i=angle.begin();in_i!=angle.end();in_i++)
    {
      *out_i=*in_i;
      out_i++;
    }
  return Dof();;
}


/**************************************************
 * Function : Reset of Pose
 * Memo     : 
 **************************************************/
void Pose::Reset()
{
  angle.clear();
}



/**************************************************
 * Function : Loading from file pointer
 * Memo     : ref_dof is required to load
 **************************************************/
int Pose::Load(ifstream &fin, int ref_dof)
{
  double tmp;
  char str;
  int tmp_int;
  int debug=0;

  fin >> tmp_int;
  sampling_time = tmp_int;
  
  for(int i=0; i<ref_dof; i++){
    fin >> str;
    fin >> tmp;
    angle.push_back(tmp);
  }
  if(debug&&Gdebug){
    for(int i=0; i<(int)angle.size(); i++)
      cerr << "[Pose.Load] Set angle[" << i << "]:" << angle[i] << " ";
    cerr << endl;
  }

  return TRUE;
}


/**************************************************
 * Created  : 2008-10-07 by inamura
 * Function : Loading from char buffer
 * Memo     : Execute loading even if dof is unknown.
 *		separator is ' ' or ',' or '\t'
 **************************************************/
int Pose::Load(char *buf)
{
  char eliminator;
  int debug=0;
  GList *read_list;

  if (tl_strmember (buf, " "))       eliminator = ' ';
  else if (tl_strmember (buf, ","))  eliminator = ',';
  else if (tl_strmember (buf, "\t")) eliminator = '\t';
  else {
    tl_warning ("There is no eliminator ' ' ',' '\t'");
    return FALSE;
  }

  read_list = tl_word_array_to_glist (buf, eliminator);

  sampling_time = atoi ((char *)g_list_nth_data (read_list, 0));
  free (g_list_nth_data(read_list, 0));	// free string for the sampling time

  for (int i=1; i<(int)g_list_length(read_list); i++)	// i=0 indicates sampling time, so it is not used
    {
      angle.push_back (atof ((char *)g_list_nth_data(read_list, i)));
      if(debug&&Gdebug) cerr << "[Pose.Load] Set angle [" << i << "]:" << angle[i] << " ";
      free (g_list_nth_data(read_list, i));
    }
  if(debug&&Gdebug) cerr << endl;

  g_list_free (read_list);

  return TRUE;
}



/**************************************************
 * Function : Reference of Dof
 * Memo     : 
 **************************************************/
int Pose::Dof() const
{
  return angle.size();
}

int Pose::SamplingTime() const
{
  return sampling_time;
}
  
/**************************************************
 * Function : Reference of n-th angle
 * Memo     : 
 **************************************************/
double Pose::NthAngle(int n)
{
  if(n >= (int)angle.size())
    {
      tl_message ("%d-th data does not exist!", n);
      return -1.0;
    }
  return angle[n];
}

/**************************************************
 * Created  : 2003-11-04
 * Modified : 2008-09-27 by inamura
 * Function : Set n-th value
 **************************************************/
int Pose::NthAngle(int n, double val)
{
  if(n >= (int)angle.size())
    {
      tl_message ("%d-th data does not exist!", n);
      return FALSE;
    }
  angle[n] = val;
  return TRUE;
}


/**************************************************
 * Function : Display Pose
 * Memo     : 
 **************************************************/
void Pose::Show()
{
  cerr << "[Pose.Show] angle" << endl;

  for(int i=0; i<(int)angle.size(); i++)
    cerr << " " << angle[i];

  cerr << endl;
}


/**************************************************
 * Function : File out of Pose
 * Memo     : 
 **************************************************/
int Pose::FileOut(ofstream &fout, int tstep)
{
  int debug=0;
  fout << tstep;
  if(debug&&Gdebug) cerr << "[Pose.FileOut] angle.size: " << (int)angle.size();
  for(int i=0; i<(int)angle.size(); i++){
    //    if(debug&&Gdebug) cerr << "[Pose.FileOut] i: " << i;
    fout << "," << NthAngle(i);
  }
  fout << endl;
  if(debug&&Gdebug) cerr << endl;

  return TRUE;
}


/**************************************************
 * Add a new angle with changing the size of target vector
 **************************************************/
void Pose::AddAngle(double new_angle)
{
  angle.push_back(new_angle);
}



/**************************************************
 * Modified : 2008-10-07 by inamura : name change from Ith to Nth
 * Function : Remove i-th angle
 * Memo     : TODO: it can be smart using STL's remove()
 **************************************************/
int Pose::RemoveNthAngle(int i)
{
  int debug=0, ii;
  //int old_dof = Dof(); //Dof() === angle.size()
  vector<double>::iterator	itr;

  if(debug&&Gdebug) cerr << "[Pose.RemoveNthAngle] start" << endl;
  if(debug&&Gdebug) cerr << "[Pose.RemoveNthAngle] angle.size() (before) : " << angle.size() << endl;

  itr = angle.begin();
  for (ii=0; ii<i; ii++)
    itr++;
  angle.erase (itr);

#if 0
  for(int j=i; j<old_dof-1; j++) {
    angle[j] = angle[j+1];
  }
  angle.pop_back();
#endif

  if(debug&&Gdebug) cerr << "[Pose.RemoveNthAngle] angle.size() (end)" << angle.size() << endl;
  if(debug&&Gdebug) cerr << "[Pose.RemoveNthAngle] end" << endl;

  return TRUE;

}




/*---------------------------------------------------------------------------*/
// Create : 2004 Jun 10th
// Function : Checking memory leak and display
/*---------------------------------------------------------------------------*/
int Pose::Verify (void)
{
  //tl_message ("Gdebug = %d", Gdebug);
  d_print ("##---------------------   Pose_Verify  (start)  -----------------\n");
  d_print ("##    dof              = %d\n", Dof());
  //d_print ("##    next_time_length = %d\n", sampling_time);
  for (int i=0; i<Dof(); i++)
    d_print ("##    joint angles[%d]  = %g\n", i, angle[i]);
  d_print ("##---------------------   Pose_Verify   (end)   -----------------\n");
  
  return TL_TRUE;
}
