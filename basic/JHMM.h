/*
 * JHMM.h
 * Hidden Markov Model structure
 * Update : 2007-06-18 : for ICONIP, estimated stay time is added, Interpolation method is added
 * update : 2007-09-08 : for ICRA,   interpolation even if the num of states are different, is added
 * 
 * Last modified by Tetsunari Inamura on 2015 Sep 27th
 */


#ifndef __JHMM_H__
#define __JHMM_H__

#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <math.h>
#include <stdio.h>
#include <cstdlib>

#include "Behavior.h"

using namespace std;

#define DEFAULT_NODE_N	(20)
#define DEFAULT_MIX_N	(5)

#define GEN_NUM			30		// ʿ�Ѽ�ˡ����������ݤ� O �����Ȥ��η����֤����
#define GEN_NUM_Q		10		// ʿ�Ѽ�ˡ����������ݤ� Q �����Ȥ��η����֤����

#define TAG_STREAMINFO    "<STREAMINFO>"
#define TAG_VECSIZE       "<VECSIZE>"
#define TAG_TILDE_H       "~h"
#define TAG_BEGINHMM      "<BEGINHMM>"
#define TAG_NUMSTATES     "<NUMSTATES>"
#define TAG_MIXTURE       "<MIXTURE>"
#define TAG_STATE         "<STATE>"
#define TAG_NUMMIXES      "<NUMMIXES>"
#define TAG_MEAN          "<MEAN>"
#define TAG_VARIANCE      "<VARIANCE>"



/*
 * HMM�Υ�����
 */
typedef enum
{
  ERGOTIC,
  PERIODIC,
  LEFT_TO_RIGHT
}
HmmType;


/*----------------------------------------------------------------------*/
// Created  : 2002/11/15
// Function : ʿ��0,ʬ��1������ʬ�ۤ�������֤�
// Memo     : NUMERICAL RECIPES in C ����
/*----------------------------------------------------------------------*/
inline double _gasdev (void)
{
  static double gset;
  static int 	iset = 0;
  double 	fac, rsq, v1, v2;

  if(iset == 0){
    do {
      v1  = 2.0 * rand()/(double)RAND_MAX - 1.0;
      v2  = 2.0 * rand()/(double)RAND_MAX - 1.0;
      rsq = v1*v1 + v2*v2;
    } while (rsq >= 1.0 || rsq == 0.0);
    fac = sqrt(-2.0 * log(rsq)/rsq);

    gset = v1*fac;
    iset = 1;
    return v2*fac;
  } else {
    iset = 0;
    return gset;
  }	
}


/*----------------------------------------------------------------------*/
// Created  : 2002/11/15
// Function : ʿ�� mean, ʬ�� var ������ʬ�ۤ�������֤�
// Memo     :
/*----------------------------------------------------------------------*/
inline double _RandomForGaussian (double mean, double var)
{ 
  double gset;
  
  gset = sqrt(var) * _gasdev() + mean;
  
  return gset;
}




// HMM�γƥΡ��ɤ��������ݻ�����
class JHMMState
{
  friend class JHMM;
  
  int num_of_mixture;	// ����ʬ�ۤο�
  int vector_size;	// ���Ϥ��٤�����μ���

  vector<double> mix_weight; // ����ʬ�ۤνŤ�
  vector<vector<double> > mean;	// mean[i][k]:i���ܤκ���ʬ�ۤ�k�����ܤ�ʿ����
  vector<vector<double> > variance;	// variance[i][k] : ʬ����

public:
  JHMMState();
  JHMMState(const JHMMState &another); // TODO:�ޤ�����

  ~JHMMState();

  void		Verify		(void);
  void		SetNumMix	(void);
  void		SetVecSize	(int vec_size);
  void		SetMixWeight	(double weight);
  void		SetMean		(int cur_mix, double m);
  void		SetVariance	(int cur_mix, double v);
  int		SelectMixture	(void);
  Pose*		GetPose		(void);
  Pose*		GetPose		(int time);
  int		Interpolation	(JHMMState& state1, JHMMState& state2, double c1, double c2);
  int		Extrapolation	(JHMMState& state1, JHMMState& state2, double alpha);
  int		InterpolationAny(JHMMState *state_vec[], double *weight_vec, int state_num);

  //  JHMMState* operator+(JHMMState state2);
  friend JHMMState* operator+		(JHMMState& state1, JHMMState& state2);
  friend JHMMState* operator*		(JHMMState& state1, double dbl);
  int		WeightDouble		(double dbl);
  double	HellingerDistance	(JHMMState &other);
};




// class to handle hmm
class JHMM
{
  friend class JHMMState;
  
  int	num_of_state;	// State�ο�(a_matrix�Υ������Ȱ���)(state�Υ�������num_of_state-2)
  int	num_of_mixture;	// ����ߥ�������ο�
  int	vector_size;	// ���ϥ٥��ȥ�ο�
  vector<vector<double> >	a_matrix;
  vector<JHMMState *>		state;
  vector<double>		stay_time;	// for each node
  vector<double>		virtual_time;	// Added on 2007-06-18
  HmmType			hmm_type;
  int				sampling_time;	// ��ư�������˻��Ѥ���

 public:
  string			label;

  JHMM();
  JHMM(const char* _name);
  JHMM(const JHMM& another);
  ~JHMM();

  int	Load			(const char* fname);
  int	CheckNextLineMatching	(ifstream &fin, char *target, char *buf);
  int	CheckThisLineMatching	(ifstream &fin, char *target, char *buf);
  void	Show			(void);
  void	Reset			(void);
  void	SetDefault		(void);
  void	FileOut			(const char* fname);
  void	FileOut			(const string &fname);
  int	GetNumState		(void);
  void	SetNumState		(int num);
  int	GetNumMix		(void);
  void	SetNumMix		(int num);
  int	GetVecSize		(void);
  void	SetVecSize		(int num);
  JHMMState *GetNthState	(int nth);
  Behavior *GenerateBehavior	(int num, int num_q);
  vector<int> AverageQ(int num_q, HmmType hmm_type);
  //int NextState(int cur_state, HmmType hmm_type);
  int	EstimatePeriod		(void);
  int	CalcStayTime		(void);
  int	CalcTransitProbFromStayTime(void);
  int	NextState		(int cur_state);
  Behavior*			BehWithQSeq(vector<int> q_seq);
  
  void Initialize(HmmType type);
  int	FindNearestVirtualTime	(double vt);
  int	Interpolation		(JHMM& hmm1, JHMM& hmm2, double c1, double c2);
  int	InterpolationGeneral	(JHMM& hmm1, JHMM& hmm2, double c1, double c2);
  int	Extrapolation		(JHMM& hmm1, JHMM& hmm2, double alpha);
  int	InterpolationAny	(JHMM *hmm_vec[], double *weight_vec, int hmm_num);

  // Psymbol����δؿ�
  int		SetLabel(const char* name);
  int		SetLabel(string& name);
  string	GetLabel(void);
  void *	GetLabelChar(void);
  int		CheckHmmType(void);
  int		SetHmmType(HmmType type);
  HmmType	GetHmmType(void);
  int		ConvertToPERIODIC(void);

  //  JHMM* operator+(JHMM hmm2);
  friend JHMM* operator+(JHMM& hmm1, JHMM& hmm2);
  friend JHMM* operator*(JHMM& hmm1, double dbl);

  int		SetSamplingTime		(int time);
  int		GetSamplingTime		();
  double	HellingerDistance	(JHMM &hmm2);
  int		Verify(void);
};


HmmType query_hmmtype_enum (char *type_name);


#endif /* __JHMM_H__ */

