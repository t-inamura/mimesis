/*
 * JHMM.cpp
 *
 * Last Modified : Tetsunari Inamura on 2015 Oct 2nd
 *
 *	Copyright (c) Tetsunari Inamura 1999--2015.
 *	All Rights Reserved.
 */


#include <assert.h>
#include <string.h>

#include "JHMM.h"

using namespace std;



vector<double>* multiple_scalar_vector(double c, vector<double> &vec)
{
	vector<double>::iterator it = vec.begin();
	while (it != vec.end())
		{
			*it = *it * c;
			++it;
		}
	return &vec;
}

vector<double>* plus_vector(vector<double> &vec1, vector<double> &vec2)
{
	int	debug=0;
	vector<double> *new_vec;
	new_vec = new vector<double>;
	*new_vec = vec1;
	if (debug) tl_message ("DOF vec1 = %d, DOF new_vec = %d", (int)(vec1.size()), (int)(new_vec->size()));

	for (int i=0; i<(int)((*new_vec).size()); i++)
		{
			(*new_vec)[i] = vec1[i] + vec2[i];
			if (debug) cerr << (*new_vec)[i] << " , ";
		}
	if (debug) cerr << endl;		
			
	return new_vec;
}


// calc vec1 - vec2
vector<double>* minus_vector(vector<double> &vec1, vector<double> &vec2)
{
	int	debug=0;
	vector<double> *new_vec;
	new_vec = new vector<double>;
	*new_vec = vec1;
	if (debug) tl_message ("DOF vec1 = %d, DOF new_vec = %d", (int)(vec1.size()), (int)(new_vec->size()));

	for (int i=0; i<(int)((*new_vec).size()); i++)
		{
			(*new_vec)[i] = vec1[i] - vec2[i];
			if (debug) cerr << (*new_vec)[i] << " , ";
		}
	if (debug) cerr << endl;		
			
	return new_vec;
}


/**************************************************
 * Created  : 2004 May 27th    by inamura
 * Function : RecogUnit���ɲä���
 * Memo     :
 **************************************************/
HmmType query_hmmtype_enum (char *type_name)
{
	string	tmp_str = type_name;

	if (tmp_str == "LEFT_TO_RIGHT")
		return LEFT_TO_RIGHT;
	else if (tmp_str == "ERGOTIC")
		return ERGOTIC;
	else if (tmp_str == "PERIODIC")
		return PERIODIC;
	else
		return (HmmType)FALSE;
}


/**************************************************
 * Created  : 2003.08.24	
 * Function : ���󥹥ȥ饯��
 * Memo     :
 **************************************************/
JHMM::JHMM()
{
	Reset();
}

/**************************************************
 * Created  : 2003.09.07	
 * Function : ���ԡ����󥹥ȥ饯��
 * Memo     : 
 **************************************************/
JHMM::JHMM(const JHMM &another)
{
	int debug=0;

	if (debug&&Gdebug) tl_message ("start");

	num_of_state   = another.num_of_state;
	num_of_mixture = another.num_of_mixture;
	vector_size    = another.vector_size;
	sampling_time  = another.sampling_time;
  
	label    = another.label;
	hmm_type = another.hmm_type;

	if (debug&&Gdebug) tl_message("a_matrix");
	a_matrix.resize((int)another.a_matrix.size());
	for (int i=0; i<(int)another.a_matrix.size(); i++)
		{
			a_matrix[i] = another.a_matrix[i];
		}
	if (debug&&Gdebug) tl_message("state");
	state.resize((int)another.state.size());
	for (int i=0; i<(int)another.state.size(); i++)
		{
			state[i] = new JHMMState(*(another.state[i]));
		}

	// Added on 2007-06-18
	stay_time.resize((int)another.stay_time.size());
	virtual_time.resize((int)another.virtual_time.size());

	if (debug&&Gdebug) tl_message ("finish!");
}


/**************************************************
 * Created  : 2003.08.24	
 * Function : �ǥ��ȥ饯��
 * Memo     : a_matrix.clear()������OK?
 **************************************************/
JHMM::~JHMM()
{
	int debug=0;

	if (debug&&Gdebug) cerr << "[JHMM.~JHMM]" << endl;
	if (!state.empty())
		{
			if (debug&&Gdebug) cerr << "[JHMM.~JHMM] state clear" << endl;
			for (int i=0; i <(int)state.size(); i++)
				{
					delete state[i];
				}
			state.clear();
		}

	if (!a_matrix.empty())
		{
			if (debug&&Gdebug) cerr << "[JHMM.~JHMM] a_matrix clear" << endl;
			for (int i=0; i <(int)a_matrix.size(); i++)
				{
					a_matrix[i].clear();
				}
			a_matrix.clear();
		}
	if (!stay_time.empty())
		stay_time.clear();
	if (!virtual_time.empty())
		virtual_time.clear();

	if (debug&&Gdebug) cerr << "[JHMM.~JHMM] Finish!" << endl;
}


/**************************************************
 * Modified : 2004 Nov 21st by inamura
 * Function : �ե����뤫���ɤ߹���
 * Memo     : ���󥰥�, �ߥ������㶦���б� CheckHMM��ԤäƤ���
 **************************************************/
int JHMM::Load(const char *fname)
{
	ifstream	fin;
	string	tmp_str;
	int		vec_size, numofstate, numofmix, cur_state, cur_mix;
	int		tmpnum1, tmpnum2, i, j, k;
	double	tmpdouble;
	char		buf[MAX_STRING], tmpbuf[MAX_STRING], tmpbuf2[MAX_STRING];
	int		debug=0;
	JHMMState	*tmp_state;

	complement_dirname (fname, tmp_str);
	fin.open (tmp_str.c_str(), ifstream::in);
	state.resize(0);		// added 2008-07-16 by inamura

	if (!fin)
		{
			if ((int)tmp_str.rfind(".hmm")==-1)
				{
					tl_warning ("Cannot open %s", tmp_str.c_str());
					return FALSE;
				}
			fin.open ( (tmp_str.substr(0, tmp_str.rfind(".hmm"))).c_str(), ifstream::in);
			if (!fin) {
				tl_warning ("Cannot open %s", (tmp_str.substr(0, tmp_str.rfind(".hmm"))).c_str());
				return FALSE;
			}
		}
	if ((fin.getline(buf,1000)) == NULL)
		{
			tl_message ("(ERROR) Fisrt Line is NULL!!");
			fin.close();
			return FALSE;
		}
  
	// line 2: error without <STREAMINFO>
	if (CheckNextLineMatching(fin, (char *)TAG_STREAMINFO, buf)==FALSE) return FALSE;
	sscanf(buf, "%s %d %d", tmpbuf, &tmpnum1, &tmpnum2);
	// line 3: error without <VECSIZE>
	if (CheckNextLineMatching(fin, (char *)TAG_VECSIZE, buf)==FALSE) return FALSE;
	sscanf(buf, "%s %d%s", tmpbuf, &vec_size, tmpbuf2);
	if (debug&&Gdebug) cerr << "[JHMM.Load] Vec Size:" << vec_size;
	vector_size = vec_size;

	// 4���� ��Ƭ�� "~h" �Ǥʤ���Х��顼
	if (CheckNextLineMatching(fin, (char *)TAG_TILDE_H, buf)==FALSE) return FALSE;
	sscanf(buf, "%s %s", tmpbuf, tmpbuf2);
	char tmpbuf3[MAX_STRING];
	strcpy(tmpbuf3, (tmpbuf2+1));
	tmpbuf3[strlen(tmpbuf3)-1] = '\0';
	SetLabel(tmpbuf3);

	// 5���� <BEGINHMM> �Ǥʤ��ȥ��顼
	if (CheckNextLineMatching(fin, (char *)TAG_BEGINHMM, buf)==FALSE) return FALSE;
	sscanf(buf, "%s", tmpbuf);
	// 6���� <NUMSTATES> �Ǥʤ��ȥ��顼
	if (CheckNextLineMatching(fin, (char *)TAG_NUMSTATES, buf)==FALSE) return FALSE;
	sscanf(buf, "%s %d", tmpbuf, &numofstate);
	if (debug&&Gdebug) cerr << "[JHMM.Load] Num Of States:" << numofstate << endl;
	num_of_state = numofstate;

	// ��˰���ɤ�Ǥ���
	fin.getline(buf, 1000);
	if (debug&&Gdebug) cerr << "[JHMM.Load] Before node loop : buf = " << buf << endl;
	// �ƥ��ơ��Ȥ��ɤ߹��� (i : �Ρ��ɤ�index)
	for (i=0; i<numofstate-2; i++)
		{
			tmp_state = new JHMMState;
			if (!tmp_state){
				cerr << "[JHMM.Load] (ERROR) Allocation error" << endl;
				return FALSE;
			}
			tmp_state->SetVecSize(vec_size);
      
			cur_mix = 0;
			// ���ߤι� <STATE> �Ǥʤ���Х��顼 ������ <GCONST> ���ä����ϥ��롼�ѥ�
			if (!strncmp(buf, "<GCONST>", 8))
				fin.getline(buf, 1000);  // GCONST ���ͤϼΤƤ롥�Ʒ׻��򤹤�����
			if (strncmp(buf, TAG_STATE, 7))  //TODO: Magic number
				{
					tl_warning ("<STATE> doesn't come, but <%s>", buf);
					fin.close();
					return FALSE;
				}
#if 0
			if (CheckThisLineMatching(fin, TAG_STATE, buf)==FALSE) return FALSE;
#endif
			sscanf (buf, "%s %d", tmpbuf, &cur_state);
			if (debug&&Gdebug) tl_message ("Setting %d-th State", cur_state);

			fin.getline(buf, 1000);
			if (strncmp(buf, TAG_NUMMIXES, 10)==0)  // TODO: Magic number
				{
					// <NUMMIXES> �ξ�� -> ���祬��������
					sscanf(buf, "%s %d", tmpbuf, &numofmix);
					num_of_mixture = numofmix;
					if (debug&&Gdebug) cerr << "[JHMM.Load] Set the number of mixture : " << numofmix << endl;

					// �ƥߥ������㡼���ɤ߹��� (j : �Ρ��ɤ�index)
					fin.getline(buf, 1000);
					for (j=0; j<numofmix; j++)
						{
							// <GCONST> �Ǥ���Х��롼�ѥ���������line�λĤ�ξ���
							if (!strncmp(buf, "<GCONST>", 8))  // TODO: Magic number
								{
									if (debug&&Gdebug) cerr << "[JHMM.Load] Through pass the <GCONST> : " << buf << endl;
									sscanf(buf, "%s %lf", tmpbuf, &tmpdouble);
									fin.getline(buf, 1000);
									if (debug&&Gdebug) cerr << "[JHMM.Load] Next string of <GCONST> : " << buf << endl;
								}
							// <STATE> ���ä��鼡�Υ��ơ��Ȥ�
							if ((strncmp(buf, TAG_STATE, 7)) == 0)  // TODO: Magic number
								{
									if (debug&&Gdebug) cerr << "[JHMM.Load] " << cur_state << "th state complete" << endl;
									break;
								}
							// <TRANSP> ���ä��� A_MATRIX �Υ��åȤ�
							if ((strncmp(buf, "<TRANSP>", 8)) == 0)  // TODO: Magic number
								{
									if (debug&&Gdebug) cerr << "[JHMM.Load] All State Setting finished!" << endl;
									break;
								}
							// ���ߤιԤ� ����ʳ��ξ�� <MIXTURE> �Ǥʤ��ȥ��顼
							if (CheckThisLineMatching(fin, (char *)TAG_MIXTURE, buf)==FALSE) return FALSE;
							sscanf(buf, "%s %d %lf", tmpbuf, &tmpnum1, &tmpdouble);
	  
							cur_mix++;
							tmp_state->SetMixWeight(tmpdouble);
							if (debug&&Gdebug) cerr << "[JHMM.Load] state:" << cur_state << " mix:"
												   << cur_mix <<" weight:" << tmpdouble << endl;
							// ���ι� <MEAN> �Ǥʤ��ȥ��顼
							if (CheckNextLineMatching(fin, (char *)TAG_MEAN, buf)==FALSE) return FALSE;
							sscanf(buf, "%s %d", tmpbuf, &tmpnum1);
	  
							// MEAN ���ɤ߹���
							if (debug&&Gdebug) tl_message ("Setting %d-th/%d Mxiture of %d-th State's mean", cur_mix, numofmix, cur_state);
							double tmp_mean;
							for (k=0; k<vec_size; k++)
								{
									fin >> tmp_mean;
									tmp_state->SetMean(cur_mix, tmp_mean);
									if (debug&&Gdebug) cerr << tmp_mean << " ";
								}
							if (debug&&Gdebug) cerr << endl;
							fin.getline(buf, 1000); // ���ԥ����ɤ��ɤ߹���
	  
							// ���ι� <VARIANCE> �Ǥʤ��ȥ��顼
							if (CheckNextLineMatching(fin, (char *)TAG_VARIANCE, buf)==FALSE) return FALSE;
							sscanf (buf, "%s %d", tmpbuf, &tmpnum1);
							// VARIANCE ���ɤ߹���
							if (debug&&Gdebug) cerr << "[JHMM.Load] Setting " << cur_mix << "th Mixture Of " << cur_state << "th State's Variance" << endl;
							double tmp_var;
							for (k=0; k<vec_size; k++)
								{
									fin >> tmp_var;
									if (debug&&Gdebug) cerr << tmp_var << " ";
									tmp_state->SetVariance(cur_mix, tmp_var);
								}
							if (debug&&Gdebug) cerr << endl;
							fin.getline(buf, 1000); // Variance �Ԥ��ɤߤ��äƤ��ʤ� ���ԥ����ɤ��ɤ߹���
							if (debug&&Gdebug) cerr << "[JHMM.Load] Rest string after VARIANCE is : " << buf << endl;
							if (debug&&Gdebug) tl_message ("i, j, k, cur_mix is : %d, %d, %d, %d", i, j, k, cur_mix);
							fin.getline(buf, 1000); // ���� mixutre loop �Τ�����ɤ߹��ߡ�<GCONST>�β�ǽ���⤢���
						} // �� mixture ���ɤ߹��ߤ� loop end
					tmp_state->SetNumMix();
					state.push_back(tmp_state);
				}
			else
				{
					// <NUMMIXES> �Ǥʤ���� -> ���󥰥�ߥ������㡼
					cur_mix = 1;
					num_of_mixture = 1;
					tmp_state->SetMixWeight(1.0);
	  
					if (debug&&Gdebug) tl_message("Single Mixture mode");

					// ���ߤι� <MEAN> �Ǥʤ���Х��顼
					if (CheckThisLineMatching(fin, (char *)TAG_MEAN, buf)==FALSE) return FALSE;
					sscanf(buf, "%s %d", tmpbuf, &tmpnum1);
					// MEAN ���ɤ߹���
					if (debug&&Gdebug) tl_message ("Setting single mixture of %d-th State's Mean", cur_state);
					double tmp_mean;
					for (k=0; k<vec_size; k++)
						{
							fin >> tmp_mean;
							tmp_state->SetMean(cur_mix, tmp_mean);
							if (debug&&Gdebug) cerr << tmp_mean << " ";
						}
					if (debug&&Gdebug) cerr << endl;
	  
					fin.getline(buf, 1000); // ���ԥ����ɤ��ɤ߹���
					// ���ι� <VARIANCE> �Ǥʤ��ȥ��顼
					if (CheckNextLineMatching(fin, (char *)TAG_VARIANCE, buf)==FALSE) return FALSE;
					sscanf(buf, "%s %d", tmpbuf, &tmpnum1);
					// VARIANCE ���ɤ߹���
					if (debug&&Gdebug) tl_message ("Setting single mixture of %d-th State's Variance", cur_state);
					double tmp_var;
					if (debug&&Gdebug) tl_message ("single mix step 1");
					for (k=0; k<vec_size; k++)
						{
							fin >> tmp_var;
							if (debug&&Gdebug) cerr << tmp_var;
							tmp_state->SetVariance(cur_mix, tmp_var);
							if (debug&&Gdebug) cerr << tmp_var;
						}
					if (debug&&Gdebug) cerr << endl;
					if (debug&&Gdebug) tl_message ("single mix step 2");
					fin.getline(buf, 1000); // Variance �ԤκǸ�β��ԥ����ɤ��ɤ߹���
					if (debug&&Gdebug) tl_message ("single mix step 3");
					tmp_state->SetNumMix();
					if (debug&&Gdebug) tl_message ("single mix step 4");
					state.push_back(tmp_state);
					if (debug&&Gdebug) tl_message ("single mix step 5");

					fin.getline(buf, 1000); // �롼����Ƭ�Ǥ�ʸ��������å��Τ��ᡤ�������ɤ�Ǥ�����
				}
		}
  
	// (��Τ⤦����ɤޤ�Ƥ���Τ�) ���ߤι� <TRANSP> �Ǥʤ��ȥ��顼�������� <GCONST>�ξ��̵��
	//if (CheckThisLineMatching (fin, "<TRANSP>", buf)==FALSE) return FALSE;
	if (strncmp(buf, "<TRANSP>", 7))
		{
			// <TRANSP> �Ǥʤ��ä���
			fin.getline(buf, 1000); 
			if (!strncmp(buf, "<GCONST>", 8))
				fin.getline(buf, 1000);  // GCONST ���ͤϼΤƤ롥�Ʒ׻��򤹤�����
			if (strncmp(buf, "<TRANSP>", 7))
				{
					tl_warning ("<TRANSP> doesn't come, but <%s>", buf);
					fin.close();
					return FALSE;
				}
		}
	sscanf(buf, "%s %d", tmpbuf, &tmpnum1);
  
	// A_MATRIX ���ɤ߹���
	if (debug&&Gdebug) tl_message ("a_matrix Setting!");
	double tmp_a;
	a_matrix.resize(numofstate);
	for (i=0; i<numofstate; i++)
		{
			for (j=0; j<numofstate; j++)
				{
					fin >> tmp_a;
					a_matrix[i].push_back(tmp_a);
					if (debug&&Gdebug) cerr << " " << tmp_a;
				}
			if (debug&&Gdebug) cerr << endl;
		}
	fin.getline(buf, 1000); // ���ԥ����ɤ��ɤ߹���
	// ���ι� <ENDHMM> �Ǥʤ��ȥ��顼���������¾��̣���ʤ��Τȡ�ENDHMM ���ä��� EndHMM ���ä����ɤ��Τǡ������å����ʤ����ȤȤ���
	// if (CheckNextLineMatching (fin, "<EndHMM>", buf)==FALSE) return FALSE;
	// sscanf(buf, "%s", tmpbuf);
	fin.close();
	CheckHmmType();

	// Added on 2007-06-18
	stay_time.resize(numofstate);
	virtual_time.resize(numofstate);
	CalcStayTime();

	// sapmling_time�Υ��å�
	char timefile[MAX_STRING];
	sprintf(timefile, "%s.time", tmp_str.c_str());
	ifstream fin2(timefile);
	if (!fin2)
		{
			tl_message ("%s not exist!", timefile);
			cerr << "So, set DEFAULT_BEH_TIME:" << DEFAULT_BEH_TIME << endl;
			SetSamplingTime(DEFAULT_BEH_TIME);    
		}
	else
		{
			fin2.getline(buf, MAX_STRING);
			if (strncmp(buf, "#sampling_time", 14))
				{
					cerr << "[JHMM.Load] (ERROR) #sampling_time -> " << buf << endl;
					cerr << "So, Set DEFAULT_BEH_TIME:" << DEFAULT_BEH_TIME << endl;
					fin2.close();
					SetSamplingTime(DEFAULT_BEH_TIME);
				} 
			else 
				{
					sscanf(buf, "%s %d", tmpbuf, &tmpnum1);
					SetSamplingTime(tmpnum1);
					fin2.close();
				}
		}

	if (debug&&Gdebug) cerr << "[JHMM.Load] Finish!" << endl;
	return TRUE;
}



/**************************************************
 * Created  : 2005 Jun 18th
 * Function : ���ιԤ��ɤ߹��ߡ�target �ȰۤʤäƤ���Хե�������Ĥ��ư۾ｪλ����
 * input    : &fin     : current file stream
 * input    : *target  : target string
 * output   : *buf     : result of stream reading
 * MEMO     : Load �᥽�åɤ���ƤФ�� Sub Function
 **************************************************/
int JHMM::CheckNextLineMatching (ifstream &fin, char *target, char *buf)
{
	//char		buf[MAX_STRING];
	fin.getline(buf, 1000);
	if (strncmp(buf, target, strlen(target)))
		{
			tl_warning ("%s doesn't come, but <%s>", target, buf);
			fin.close();
			return FALSE;
		}
	return TRUE;
}


/**************************************************
 * Created  : 2005 Jun 18th
 * Function : ���ߤιԤ� target �ȰۤʤäƤ���Хե�������Ĥ��ư۾ｪλ����
 * input    : &fin     : current file stream (for closing only, not for reading)
 * input    : *target  : search string
 * input    : *buf     : whole string for searching
 * MEMO     : Load �᥽�åɤ���ƤФ�� Sub Function
 **************************************************/
int JHMM::CheckThisLineMatching (ifstream &fin, char *target, char *buf)
{
	if (strncmp(buf, target, strlen(target)))
		{
			tl_warning ("%s doesn't come, but <%s>", target, buf);
			fin.close();
			return FALSE;
		}
	return TRUE;
}




/**************************************************
 * Created  : 2003.08.24	
 * Function : �ꥻ�å�
 * Memo     :
 **************************************************/
void JHMM::Reset()
{
	num_of_state = 0;
	num_of_mixture = 0;
	vector_size = 0;
	hmm_type = LEFT_TO_RIGHT;		// default value
	sampling_time = DEFAULT_BEH_TIME;	// default value
}


//**************************************************
// Create : 2006 Sep 21st
//**************************************************
void JHMM::SetDefault()
{
	num_of_state   = DEFAULT_NODE_N;
	num_of_mixture = DEFAULT_MIX_N;
	vector_size    = 20;
	hmm_type       = LEFT_TO_RIGHT;	// default value
	sampling_time	 = DEFAULT_BEH_TIME;	// default value

	SetLabel ("none");
}


/**************************************************
 * Created  : 2003.08.24	
 * Function : ɽ����
 * Memo     :
 **************************************************/
void JHMM::Show()
{
	int debug=0;
	cerr << "[JHMM.Show] Num Of State:" << num_of_state << endl;
	cerr << "[JHMM.Show] Num Of Mix:" << num_of_mixture << endl;
	cerr << "[JHMM.Show] Vector Size:" << vector_size << endl;
	cerr << "[JHMM.Show] Label:" << label << endl;
	int length;
	length = EstimatePeriod();
	cerr << "[JHMM.Show] Estimate Period Length:" << length << endl;

	int aMatColNum;
	int aMatRowNum;

	aMatRowNum = a_matrix.size();
	aMatColNum = a_matrix[0].size();
	tl_message ("aMatRowNum = %d,  aMatColNum = %d", aMatRowNum, aMatColNum);
  
	if (debug&&Gdebug)
		{
			for (int i=0;i<num_of_state-2; i++)
				{
					cerr << i << "th State" << endl;
					state[i]->Verify();
				}
			cerr << endl;
		}

	cerr << "[JHMM.Show] a_matrix" << endl;
	for (int i=0; i<aMatRowNum; i++)
		{
			cerr << i << "th Row: ";
			for (int j=0; j<aMatColNum; j++)
				{
					cerr << a_matrix[i][j] << " ";
				}
			cerr << endl;
		}
	cerr << "[JHMM.Show] Stay time : " << endl;
	for (int i=0;i<num_of_state-1; i++)
		{
			cerr << stay_time[i] << "  ";
		}
	cerr << endl;
	cerr << "[JHMM.Show] virtual time : " << endl;
	for (int i=0;i<num_of_state; i++)
		{
			cerr << virtual_time[i] << "  ";
		}
	cerr << endl;
}


/**************************************************
 * Created  : 2003.08.24	
 * Function : ���ơ��Ȥο����֤�
 * Memo     : a_matrix�Υ������Ȱ��ס�
 *            state�Υ������Ȥϰ��פ��ʤ���
 **************************************************/
int JHMM::GetNumState(void)
{
	return num_of_state;
}


/**************************************************
 * Created  : 2003.08.24	
 * Function : �ߥ�������ο����֤�
 * Memo     : ����ߥ�������ο�
 **************************************************/
int JHMM::GetNumMix(void)
{
	return num_of_mixture;
}


/**************************************************
 * Created  : 2003.08.24	
 * Function : �٥��ȥ�Υ��������֤�
 * Memo     :
 **************************************************/
int JHMM::GetVecSize(void)
{
	return vector_size;
}

/**************************************************
 * Created  : 2003.09.07	
 * Function : Label�Υ��å�
 * Memo     : 
 **************************************************/
int JHMM::SetLabel(const char *name)
{
	int debug=0;

	assert (name);
	if (debug&&Gdebug) cerr << "[JHMM.SetLabel] label:" << name << endl;
	label = name;

	return TRUE;
}


int JHMM::SetLabel(string &name)
{
	int debug=0;
  
	if (debug&&Gdebug) cerr << "[JHMM.SetLabel] label:" << name << endl;
	label = name;

	return TRUE;
}


/**************************************************
 * Created  : 2003.08.24	
 * Function : HmmType������å�����
 * Memo     : TODO:���ޤ�Ȥ�L_T_R��PERIODIC�Τ�
 **************************************************/
int JHMM::CheckHmmType()
{
	int a_size;
	int debug=0;

	a_size = GetNumState();
  
	if (a_matrix[1][a_size-1]==0)
		{
			if (debug&&Gdebug) cerr << "[JHMM.CheckHmmType] LEFT_TO_RIGHT" << endl;
			SetHmmType(LEFT_TO_RIGHT);
		} 
	else 
		{
			if (debug&&Gdebug) cerr << "[JHMM.CheckHmmType] PERIODIC" << endl;
			SetHmmType(PERIODIC);
		}

	return TRUE;
}

/**************************************************
 * Modified : 2004 Sep 12th		by inamura
 * Function : HmmType�򥻥åȤ���
 * Memo     : 
 **************************************************/
int JHMM::SetHmmType(HmmType type)
{
	if (type==ERGOTIC || type==PERIODIC || type==LEFT_TO_RIGHT)
		{
			hmm_type = type;
			return TRUE;
		}
	else
		{
			tl_warning ("No such HmmType (%d)", (int)type);
			return FALSE;
		}
}

/**************************************************
 * Created  : 2003.08.24	
 * Function : HmmType���֤�
 * Memo     : 
 **************************************************/
HmmType JHMM::GetHmmType()
{
	return hmm_type;
}

/**************************************************
 * Created  : 2003.08.24	
 * Function : PERIODIC�����Ѥ���
 * Memo     : 
 **************************************************/
int JHMM::ConvertToPERIODIC()
{
	int numofstate;

	if (hmm_type==PERIODIC)
		{
			cerr << "[JHMM.ConvertToPERIODIC] already Periodic!" << endl;
			return TRUE;
		}

	numofstate = GetNumState();
	double prob_init, probtofinal, probtoinit;

	prob_init = 0;

	for (int i=1; i<numofstate-1; i++)
		prob_init += a_matrix[i][i];
 
	// State no.1
	for (int i=1; i<numofstate-1; i++)
		a_matrix[0][i] = a_matrix[i][i]/prob_init;

	// State no.2
	probtofinal = 0;
	for (int j=0; j<numofstate-1; j++)
		{
			probtofinal += a_matrix[1][j]*0.01; 
			a_matrix[1][j] *= 0.99;
		}
	a_matrix[1][numofstate-1] = probtofinal + a_matrix[1][numofstate-1];

	// State no.3 no.N-1
	for (int i=2; i<numofstate-1; i++)
		{
			probtoinit = 0;
			for (int j=0; j<numofstate; j++)
				probtoinit += a_matrix[i][j]*0.01;
			a_matrix[i][0] = 0.0;
			a_matrix[i][1] = a_matrix[i][numofstate-1]*0.99;
			for (int j=2; j<numofstate-1; j++)
				a_matrix[i][j] *= 0.99;
			a_matrix[i][numofstate-1] = probtoinit;
		}

	// State no.N
	// ���ΤޤޤǤ褤��

	SetHmmType(PERIODIC);

	return TRUE;
}


/**************************************************
 * Created  : 2003.08.24	
 * Function : N���ܤΥ��ơ��ȤΥݥ��󥿤��֤���
 * Memo     :
 **************************************************/
JHMMState *JHMM::GetNthState(int nth)
{
	return state[nth];
}


/**************************************************
 * Created  : 2003.08.24	
 * Function : ʿ�Ѳ�����Behavior������
 * Memo     : ���ư�����ޤ��԰�
 *            �����ƽФ��Ƥߤ����¹�®�٤��٤��ʤ�ʤ��ä���
 *            �������������פ���
 **************************************************/
Behavior *JHMM::GenerateBehavior(int num, int num_q)
{
	vector<int>	seq;
	Behavior	*tmp_beh[num];
	Behavior	*beh[num];
	int		length, len;
	int		debug=0;

	length=0;
	for (int i=0; i<num; i++)
		{
			if (debug&&Gdebug) tl_message ("%dth Behavior Generating", i);
#if HYOUZI
			if (i%(num/10+1) == 0 ) cerr << "*";
#endif
			if (debug&&Gdebug) tl_message ("AverageQ Start (hmm_type = %d)", (int)hmm_type);
			seq = AverageQ(num_q, hmm_type);
			if (debug&&Gdebug) tl_message ("BehWithQSeq Start");
			tmp_beh[i] = BehWithQSeq(seq);
			length    += tmp_beh[i]->Length();
			seq.clear();
		}
#if HYOUZI
	cerr << endl;
#endif
	len = length/num;
	if (debug&&Gdebug) tl_message ("Elastic To Length:%d", len);
	for (int i=0; i<num; i++)
		{
			beh[i] = new Behavior(*(tmp_beh[i]), len);
			delete tmp_beh[i];
		}
	double tmp_angle[vector_size];
	Pose* tmp_pose=NULL;
	Pose* new_pose=NULL;
	Behavior* ave_beh=NULL;

	ave_beh = new Behavior (vector_size, sampling_time, (char *)DEFAULT_BEH_LABEL);

	for (int i=0; i<len; i++)
		{
#if HYOUZI
			if (i%(len/10) == 0 ) cerr << "*";
#endif
			for (int k=0; k<vector_size; k++)
				tmp_angle[k] = 0;
			for (int j=0; j<num; j++)
				{
					tmp_pose = beh[j]->NthPose(i);
					for (int k=0; k<vector_size; k++)
						tmp_angle[k] += tmp_pose->NthAngle(k)/(double)num;
				}
			new_pose = new Pose();

			for (int k=0; k<vector_size; k++)
				new_pose->AddAngle(tmp_angle[k]);
			ave_beh->AddPose(new_pose);
		}
	for (int j=0; j<num; j++)
		delete beh[j];
	ave_beh->Label(label.c_str());
	return ave_beh;
}



Behavior* JHMM::BehWithQSeq(vector<int> q_seq)
{
	Behavior	*new_beh=NULL;
	Pose		*new_pose=NULL;
	int		next_state;
	int		debug=0;

	new_beh = new Behavior(vector_size, sampling_time, (char *)DEFAULT_BEH_LABEL);
	for (int i=0; i<(int)q_seq.size(); i++)
		{
			next_state = q_seq[i];
			if (debug&&Gdebug) cerr << "[JHMM.BehWithQseq] next_state:" << next_state << endl;
			new_pose = state[next_state-1]->GetPose(sampling_time);
			if (debug&&Gdebug) cerr << "[JHMM.BehWithQseq] addpose" << endl;
			new_beh->AddPose(new_pose);
		}
	if (debug&&Gdebug) cerr << "[JHMM.BehWithQseq] end!" << endl;
	return new_beh;  
}


Pose* JHMMState::GetPose()
{
	Pose		*new_pose=NULL;
	double	tmp;
	int 		mix_num;
	int		debug=0;

	new_pose = new Pose();
	if (debug&&Gdebug) cerr << "[JHMMState.GetPose] start!" << endl;

	for (int i=0; i < vector_size; i++)
		{
			if (num_of_mixture == 1)	mix_num = 0;  // single mixture
			else 		mix_num = SelectMixture();

			tmp = _RandomForGaussian(mean[mix_num][i], variance[mix_num][i]);
			new_pose->AddAngle(tmp);
		}

	if (debug&&Gdebug) cerr << "[JHMMState.GetPose] end!" << endl;

	return new_pose;
}


Pose* JHMMState::GetPose(int time)
{
	Pose		*new_pose=NULL;
	double	tmp;
	int 		mix_num;
	int		debug=0;

	new_pose = new Pose();
	if (debug&&Gdebug) cerr << "[JHMMState.GetPose] start! : vector_size =" << vector_size << endl;

	for (int i=0; i < vector_size; i++)
		{
			if (num_of_mixture == 1)
				mix_num = 0;  // Using 0-th mixture if single mixture
			else
				mix_num = SelectMixture(); // Select a using mixture with random sampling
			if (debug&&Gdebug) cerr << "[JHMMState.GetPose] using " << mix_num << "-th mixture for " << i << "-th element" << endl;
			tmp = _RandomForGaussian(mean[mix_num][i], variance[mix_num][i]);
			new_pose->AddAngle(tmp);
		}

	if (debug&&Gdebug) cerr << "[JHMMState.GetPose] end!" << endl;

	return new_pose;
}

// ��ǧ
int JHMMState::SelectMixture()
{
	int		mix_num;
	double	prob_sum, random_num;
	int		key, i;
	int debug=0;

	key = 1;
	mix_num = 0;

	// �����(?)�ˤ��mix_weight���¤�1�Ǥʤ����Ȥ�����Τ�,
	// ��ޤ�ޤǷ����֤�
	while (key)
		{
			prob_sum = 0;
			random_num = rand()/(double)RAND_MAX;
			if (debug&&Gdebug) cerr << "[JHMMState.SelectMixture] randnum:" << random_num << endl;
    
			for (i=0; i<num_of_mixture; i++)
				{
					prob_sum += mix_weight[i];
					if (prob_sum >= random_num)
						{
							mix_num = i;
							key = 0;
							break;
						}
				}
		}

	return mix_num;
}



vector<int> JHMM::AverageQ(int num_q, HmmType hmm_type)
{
	vector<int>* seq_buf;
	vector<double>* seq_buf2;
	vector<int> q_seq;

	int max_length=1000;
	int cur_state, next_state;
	int debug=0;

	if (debug&&Gdebug) Verify();
	seq_buf  = new vector<int>[num_q];
	seq_buf2 = new vector<double>[num_q];

	for (int i=0; i<num_q; i++)
		{
			cur_state 	= 0;
    
			if (debug&&Gdebug) cerr << "[JHMM.AverageQ] " << i << "th Qseq" << endl;
			for (int j=0; j<max_length; j++)
				{
					next_state = NextState(cur_state);
					if (debug&&Gdebug) cerr << next_state << " ";
      
					if (hmm_type == LEFT_TO_RIGHT || hmm_type == ERGOTIC)
						{
							if (next_state == (num_of_state-1))
								{
									if (debug&&Gdebug) cerr << "[JHMM.AverageQ] Stop Traject" << endl;
									break;
								}
						}
					else if (hmm_type == PERIODIC)
						{
							if (cur_state != 1 && cur_state != 0)
								if (next_state == 1)
									break;
						}
					seq_buf[i].push_back(next_state);
					cur_state = next_state;
				}
		}

	// Ĺ����ʿ�Ѥ����
	double tmp_len;
	int ave_len;
	tmp_len   = 0;
	for (int i=0; i<num_q; i++)
		tmp_len += (double)seq_buf[i].size()/num_q;
	ave_len = (int)tmp_len;

	// Ĺ����·����
	if (debug&&Gdebug) cerr << "[JHMM.AverageQ] ave_len:" << ave_len << endl;
	double step, vindex, h;
	int src_length;
	double tmp_state;
	step       = 1.0 / (ave_len-1);
	for (int i=0; i<num_q; i++)
		{
			vindex     = 0.0;
			src_length = seq_buf[i].size();
			for (int j=0; j<ave_len-1; j++)
				{
					if (debug&&Gdebug) cerr << "[JHMM.AverageQ] loop:" << j << " vindex:" << vindex << endl;
					h = vindex - (double)((int)(vindex));

					tmp_state = seq_buf[i][(int)vindex] * (1-h) + seq_buf[i][(int)(vindex+1)] * h;
					seq_buf2[i].push_back(tmp_state);
					vindex += (double)(src_length-1) / (ave_len-1);
				}
			seq_buf2[i].push_back(seq_buf[i][src_length-1]);
		}  
  
	for (int i=0; i<ave_len; i++)
		{
			tmp_state = 0;
			for (int j=0; j<num_q; j++)
				{
					tmp_state += seq_buf2[j][i]/(double)num_q;
				}
			q_seq.push_back((int)(tmp_state + 0.5));
		}

	// TODO:�ä����Ϥ����OK?
	delete[] seq_buf;
	delete[] seq_buf2;

	return q_seq;
}


/**************************************************
 * Created  : 2007 May 11th
 * Function : Calculate estimated period length
 * Memo     :
 **************************************************/
int JHMM::EstimatePeriod(void)
{
	double	e_length = 0;
  
	for (int i=0; i<num_of_state-1; i++)
		{
			e_length += 1.0 / a_matrix[i][i+1];
		}
	return (int) e_length;
}


/**************************************************
 * Created  : 2007 Jun 18th
 * Function : Calculate estimated stay time for each node
 * Memo     : 2007-09-08 ������HMM��Left����Right�ޤ����ܤ������Τ�ɬ�פʻ���(virtual_time)���Ѥ��
 **************************************************/
int JHMM::CalcStayTime(void)
{
	if (!stay_time.empty() && !virtual_time.empty())
		{
			for (int i=0; i<num_of_state-1; i++)
				stay_time[i] = 1.0 / a_matrix[i][i+1];
			virtual_time[0] = 0.0;
			for (int i=1; i<num_of_state; i++)
				virtual_time[i] = virtual_time[i-1] + stay_time[i-1];
		}
	return TRUE;
}


/**************************************************
 * Created  : 2007 Jun 18th
 * Function : Calculate transition prob. matrix from estimated stay time
 * Memo     : a[i][i+1] sould be a inverse number of stay_time[i]
 **************************************************/
int JHMM::CalcTransitProbFromStayTime(void)
{
	if (!stay_time.empty())
		{
			a_matrix[0][1] = 1.0;
			for (int i=1; i<num_of_state-1; i++)
				{
					a_matrix[i][i+1] = 1.0 / stay_time[i];
					a_matrix[i][i]   = 1.0 - a_matrix[i][i+1];
				}
		}
	return TRUE;
}


/**************************************************
 * Modified : 2005 Jun 18th
 * Function : ���ߤξ��֤��鼡�˾������ܤ�����֤��ΨŪ�˷��ꤹ��
 * Memo     : ������ hmm_type �����ä��Τ�̵�̤��ä��ΤǺ������ by inamura
 **************************************************/
int JHMM::NextState(int cur_state)
{
	int		next_state;
	double	prob_sum, random_num;
	int		debug=0;

	next_state=0;
	switch(hmm_type)
		{
		case LEFT_TO_RIGHT:
		case ERGOTIC:
			if (debug&&Gdebug) cerr << "hmm_type:LEFT_TO_RIGHT (or Ergotic)" << endl;

			while (!next_state)
				{
					prob_sum = 0;
					random_num = rand()/(double)RAND_MAX;
					for (int i=0; i<num_of_state; i++)
						{
							prob_sum += a_matrix[cur_state][i];
							if (prob_sum >= random_num)
								{
									next_state = i;
									break;
								}
						}
				}
			break;

		case PERIODIC:
			// �Ϥ����1���ܤΥΡ��ɤ���Ϥ����
			if (cur_state == 0)
				return 1;
    
			while (!next_state)
				{
					prob_sum = 0;
					random_num = rand()/(double)RAND_MAX;
					for (int i=0; i<num_of_state-1; i++)
						{
							prob_sum += a_matrix[cur_state][i];
							if (prob_sum >= random_num)
								{
									next_state = i;
									break;
								}
						}
				}
			break;
      
		default:
			tl_warning ("Unknown HMMtype: %d", hmm_type);
			return 1;
		}

	return next_state;
}



JHMMState::JHMMState()
{

}


/**************************************************
 * Created  : 2003.09.07	
 * Function : ���ԡ����󥹥ȥ饯��
 * Memo     :
 **************************************************/
JHMMState::JHMMState(const JHMMState& another)
{
	num_of_mixture = another.num_of_mixture;
	vector_size = another.vector_size;
	mix_weight  = another.mix_weight;
	mean        = another.mean;
	variance    = another.variance;

#if 0
	mean.resize((int)another.mean.size());
	for (int i=0; i<(int)another.mean.size(); i++)
		{
			mean[i] = another.mean[i];
		}
	variance.resize((int)another.variance.size());
	for (int i=0; i<(int)another.variance.size(); i++)
		{
			variance[i] = another.variance[i];
		}
#endif
}


JHMMState::~JHMMState()
{
	int debug=0;

	if (debug&&Gdebug) cerr << "[JHMMState.~JHMMState] start" << endl;
	mix_weight.clear();
	if (!mean.empty())
		for (int i=0; i <(int)mean.size(); i++)
			{
				mean[i].clear();
			}
	if (!variance.empty())
		for (int i=0; i <(int)variance.size(); i++)
			{
				variance[i].clear();
			}
  
	mean.clear();
	variance.clear();

	if (debug&&Gdebug) cerr << "[JHMMState.~JHMMState] end" << endl;
}


/**************************************************
 * Created  : 
 * Function : Recalculate the number of mixture based on current memory 
 * Memo     : This func. will be called just after modification of mixture state, especially the number of mixture.
 **************************************************/
void JHMMState::SetNumMix()
{
	num_of_mixture = mix_weight.size();
}



void JHMMState::SetVecSize(int vec_size)
{
	vector_size = vec_size;
}

void JHMMState::SetMixWeight(double weight)
{
	mix_weight.push_back(weight);
}

void JHMMState::SetMean(int cur_mix, double m)
{
	int size;
	size = mean.size();

	if (cur_mix > size)
		{
			mean.resize(cur_mix);
		} 
	mean[cur_mix-1].push_back(m);

}    
 
  
void JHMMState::SetVariance(int cur_mix, double v)
{  
	int size;
	size = variance.size();
  
	if (cur_mix > size)
		{
			variance.resize(cur_mix);
		}

	variance[cur_mix-1].push_back(v);
}    
 
  
void JHMMState::Verify()
{
	cerr << "Mix:" << num_of_mixture << endl;
	cerr << "Vec:" << vector_size << endl;
  
	for (int i=0; i < num_of_mixture; i++)
		{
			cerr << i << "-th Mixture Weight:" << mix_weight[i] << endl; 
			cerr << "Mean Size:" << mean[i].size() << endl;
			int size;
			size = mean[i].size();
			for (int j=0; j<size; j++)
				{
					cerr << mean[i][j] << " ";
				}
			cerr << endl;
			cerr << "Var Size:" << variance[i].size() << endl;
			size = variance[i].size();
			for (int j=0; j<size; j++)
				{
					cerr << variance[i][j] << " ";
				}
			cerr << endl;
		}
}


/**************************************************
 * Created  : 2003/08/29	
 * Function : FileOut
 * Memo     : �Фä���äݤ�
 **************************************************/
void JHMM::FileOut(const char* fname)
{
	int num, vec, mix, i, l, j;
  
	num = num_of_state;
	vec = vector_size;
	mix = num_of_mixture;

	ofstream fout(fname);

	fout << "~o" << endl;
	fout << TAG_STREAMINFO << " 1 "  << vec   << endl;
	fout << TAG_VECSIZE    << " "    << vec   << "<NULLD><USER>" << endl;
	fout << TAG_TILDE_H    << " \""  << label << "\"" << endl;
	fout << TAG_BEGINHMM   << endl;
	fout << TAG_NUMSTATES  << " "    << num   << endl;
  
	for (i=2; i<=num-1; i++)
		{
			fout << "<STATE> " << i << endl;
			if (mix!=1)
				{
					fout << TAG_NUMMIXES << " " << mix << endl;
				}

			for (l=0; l<(int)state[i-2]->mix_weight.size(); l++)
				{
					if (mix!=1)
						{
							fout << TAG_MIXTURE << " " << l+1 << " " << state[i-2]->mix_weight[l] << endl;
						}
					fout << TAG_MEAN << " " << vec << endl;

					for (j=0; j<vec; j++)
						{
							fout << " " << state[i-2]->mean[l][j];
						}
					fout << endl;
					fout << TAG_VARIANCE << " " << vec << endl;
					for (j=0; j<vec; j++)
						{
							fout << " " << state[i-2]->variance[l][j];
						}
					fout << endl;
					// gcon�η׻�
					// �ޤ�̤��
					// fprintf (fp, "\n<GCONST> %f\n", gcon);
				}
		}

	fout << "<TRANSP> " << num << endl;

	// State No.1 ���� No.n-2 �ޤ�
	for (i=0; i<num; i++)
		{
			for (j=0; j<num; j++)
				{	
					fout << " " << a_matrix[i][j];
				}
			fout << endl;
		}

	fout << "<EndHMM>" << endl;
	fout.close();
}


/**************************************************
 * Created  : 2003/08/31	
 * Function : FileOut
 * Memo     : char�Ǥʤ�string��
 **************************************************/
void JHMM::FileOut(const string &fname)
{
	int num, vec, mix, i, l, j;
  
	num = num_of_state;
	vec = vector_size;
	mix = num_of_mixture;

	ofstream fout(fname.c_str());

	fout << "~o" << endl;
	fout << TAG_STREAMINFO << " 1 "  << vec   << endl;
	fout << TAG_VECSIZE    << " "    << vec   << "<NULLD><USER>" << endl;
	fout << TAG_TILDE_H    << " \""  << label << "\"" << endl;
	fout << TAG_BEGINHMM   << endl;
	fout << TAG_NUMSTATES  << " "    << num   << endl;
  
	for (i=2; i<=num-1; i++)
		{
			fout << "<STATE> " << i << endl;
			if (mix!=1)
				{
					fout << TAG_NUMMIXES << " " << mix << endl;
				}

			for (l=0; l<(int)state[i-2]->mix_weight.size(); l++)
				{
					if (mix!=1)
						{
							fout << "<MIXTURE> " << l+1 << " " << state[i-2]->mix_weight[l] << endl;
						}
					fout << TAG_MEAN << " " << vec << endl;

					for (j=0; j<vec; j++)
						{
							fout << " " << state[i-2]->mean[l][j];
						}
					fout << endl;
					fout << TAG_VARIANCE << " " << vec << endl;
					for (j=0; j<vec; j++)
						{
							fout << " " << state[i-2]->variance[l][j];
						}
					fout << endl;
					// gcon�η׻� :  ̤����
					// fprintf (fp, "\n<GCONST> %f\n", gcon);
				}
		}

	fout << "<TRANSP> " << num << endl;

	// State No.1 ���� No.n-2 �ޤ�
	for (i=0; i<num; i++)
		{
			for (j=0; j<num; j++)
				{	
					fout << " " << a_matrix[i][j];
				}
			fout << endl;
		}

	fout << "<EndHMM>" << endl;
	fout.close();
}

void JHMM::SetNumState(int num)
{
	num_of_state = num;
}

void JHMM::SetNumMix(int num)
{
	num_of_mixture = num;
}

void JHMM::SetVecSize(int num)
{
	vector_size = num;
}


/**************************************************
 * Created  : 2003.08.29
 * Function : state, a_matrix�ν����
 * Memo     : num_of_state, num_of_mixture, 
 *            vector_size�����ꤷ����˹Ԥ���
 **************************************************/
void JHMM::Initialize(HmmType type)
{
	int i,j,k;
	int debug=0;

	JHMMState *tmp_state=NULL;
	for (i=0; i<num_of_state-2; i++)
		{
			if (debug&&Gdebug) tl_message ("%d-th state set start", i);
			tmp_state = new JHMMState;
			tmp_state->SetVecSize(vector_size);
    
			for (k=0; k<num_of_mixture; k++)
				{
					if (debug&&Gdebug) tl_message ("%d mix set start", k);
					tmp_state->SetMixWeight(1.0/num_of_mixture);
					for (j=0; j<vector_size; j++)
						{
#if 0
							tmp_d = rand()/(double)RAND_MAX;
							tmp_state->SetMean(k+1, tmp_d);
							tmp_d = rand()/(double)RAND_MAX;
							tmp_state->SetVariance(k+1, tmp_d);
#else
							tmp_state->SetMean(k+1, 0.0);
							tmp_state->SetVariance(k+1, 0.1);
#endif
						}
				}
			tmp_state->SetNumMix();
			state.push_back(tmp_state);
		}
	if (debug&&Gdebug) tl_message ("state set");

	a_matrix.clear();
  
	switch(type)
		{
		case ERGOTIC:
			a_matrix.resize(num_of_state);
			for (i=0; i<num_of_state; i++)
				{
					for (j=0; j<num_of_state; j++)
						{
							a_matrix[i].push_back(1.0/num_of_state);
						}
				}
			break;
		case PERIODIC:
			a_matrix.resize(num_of_state);
			for (i=0; i<num_of_state; i++)
				{
					for (j=0; j<num_of_state; j++)
						{
							a_matrix[i].push_back(1.0/num_of_state);
						}
				}
			break;
		case LEFT_TO_RIGHT:
			a_matrix.resize(num_of_state);

			a_matrix[0].push_back(0.0);
			a_matrix[0].push_back(1.0);
			for (i=2; i<num_of_state; i++)
				a_matrix[0].push_back(0.0);

			for (i=1; i<num_of_state-1; i++)
				{
					for (j=0; j < i; j++)
						a_matrix[i].push_back(0.0);
#if 0
					double prob = 1.0/2.0;
					for (j=0; j<2; j++)
						a_matrix[i].push_back(prob);
					// �Ȥɤޤ��Ψ����˿ʤ��Ψ���Ȥ��0.5�ξ��
#else
					a_matrix[i].push_back(0.75);
					a_matrix[i].push_back(0.25);
					// TODO : Magic Number ���ο����ǻ�����ѥ������Ĺ������ޤ�Τǡ������ʤ�Ĵ�ᤷ����
#endif
					if (i+2 <= num_of_state)
						for (j=i+2; j<=num_of_state; j++)
							a_matrix[i].push_back(0.0);
				}
			// State No.n
			for (j=0; j<num_of_state; j++)
				a_matrix[num_of_state-1].push_back(0.0);
			break;
		default:
			break;
		}
}


/**************************************************
 * Created  : 2007-06-19
 * Function : Find the nearest node which has the nearest virtual time from input value
 * Input    : vt : target virtual time
 * Return   : node index of the nearest node
 * Memo     : ���Τˤϰ��ֶᤤ�Ρ��ɤǤϤʤ������Ϥ��줿virtual time �򶴤���ˤʤ���ĤΥΡ��ɤΤ�������¦�ΥΡ���index���֤�
 *		input virtual time �������ֱ�¦�� node ��virtual time ����礭�����ˤϡ�����ƤȤ��Ʊ���������ܤΥΡ��ɤ��֤�
 *		2007-09-09���ߤΰơ��椯�椯�ϡ�����ܤΥΡ��ɤ�����ֱ��ΥΡ��ɤ˸����äƳ��ޤ��ƺ��٤���
 **************************************************/
int JHMM::FindNearestVirtualTime (double vt)
{
	int		i, max_index=0;
	double	max_vt = 0.0;

	if (vt <= 1.0)
		{
			tl_warning ("virtual time should be larger than 1.0; input value is %g", vt);
			return 0;
		}

	for (i=0; i<num_of_state; i++)
		{
			if (vt < virtual_time[i])
				{
					max_index = i-1;
					max_vt = virtual_time[i-1];
					break;
				}

		}
	// �⤷ vt ���ǽ��Ρ��ɤ� virtual time ����礭���ä��顤�����飲���ܤ��֤�
	if (i>=num_of_state-1)
		max_index = num_of_state-2;
	if (max_vt==0.0)
		max_index = num_of_state-2;

	return max_index;
}




/**************************************************
 * Created  : 2007-06-19
 * Function : Interpolation of two HMMs
 * Input    : c1, c2 are weights for interpolation
 * Memo     : not using operator+ nor operator*
 *          : There is an assumption that the number of nodes are same for two HMMs
 **************************************************/
int JHMM::Interpolation (JHMM& hmm1, JHMM& hmm2, double c1, double c2)
{
	int		num;
	num = hmm1.num_of_state;
	int		debug=0;
	JHMMState	*tmp_state1 = NULL;
	//JHMMState	*tmp_state2 = NULL;

	num_of_state   = hmm1.num_of_state;
	num_of_mixture = hmm1.num_of_mixture;
	vector_size    = hmm1.vector_size;
	label          = "interpolation";
	hmm_type       = LEFT_TO_RIGHT;

	hmm1.CalcStayTime();
	hmm2.CalcStayTime();

	if (Gdebug) tl_message ("step 1");
	a_matrix.resize((int)hmm1.a_matrix.size());
	for (int i=0; i<(int)hmm1.a_matrix.size(); i++)
		{
			a_matrix[i] = hmm1.a_matrix[i];
			for (int j=0; j<(int)hmm1.a_matrix.size(); j++)
				a_matrix[i][j] = 0.0;
		}
	if (Gdebug) tl_message ("step 2");
	stay_time.resize((int)hmm1.num_of_state);
	virtual_time.resize((int)hmm1.num_of_state);

	if (Gdebug) tl_message ("step 3");
	for (int i=0; i < num; i++)
		{
			stay_time[i] = c1 * hmm1.stay_time[i] + c2 * hmm2.stay_time[i];
		}
	if (Gdebug) tl_message ("step 4");
	CalcTransitProbFromStayTime();

	// Interpolation of output probabilities (B)
	if (Gdebug) tl_message ("step 5");
	state.resize(num-2);
#if 0
	// 2003���ν���κ�����ά
	for (int i=0; i < num-2; i++)
		{
			tmp_state1 = *(hmm1.state[i]) * c1;
			tmp_state2 = *(hmm2.state[i]) * c2;
			state[i] = *tmp_state1 + *tmp_state2;
			delete tmp_state1;
			delete tmp_state2;
		}
#else
	// 2007 June 30th �ο���ά�ʤ���ʳ���July 1st ����ɾ����
	for (int i=0; i < num-2; i++)
		{
			tmp_state1 = new JHMMState(*(hmm1.state[i]));
			tmp_state1->Interpolation (*(hmm1.state[i]), *(hmm2.state[i]), c1, c2);
			state[i] = tmp_state1;
		}
#endif
	if (Gdebug) tl_message ("final check");
	if (Gdebug&&debug) state[0]->Verify();
	if (Gdebug) tl_message ("step end");
	return TRUE;
}



/**************************************************
 * Created  : 2007-12-24
 * Function : Interpolation/Extrapolation of any number of HMMs
 * Input	: hmm_vec	: sequence of pointer of each HMM
 *		: weight_vec	: sequence of weight value for each HMM
 *		: hmm_num	: number of HMM used for the interpolation/extrapolation
 * Memo         : There is an assumption that the number of nodes are same for two HMMs
 **************************************************/
int JHMM::InterpolationAny (JHMM *hmm_vec[], double *weight_vec, int hmm_num)
{
	int		num;
	num = hmm_vec[0]->num_of_state;
	int		debug=0;
	JHMMState	*tmp_state1 = NULL;

	if (Gdebug&&debug) 
		{
			for (int j=0; j<hmm_num; j++)
				tl_message ("weight[%d] = %g", j, weight_vec[j]);
		}
	num_of_state   = hmm_vec[0]->num_of_state;
	num_of_mixture = hmm_vec[0]->num_of_mixture;
	vector_size    = hmm_vec[0]->vector_size;
	label          = "online";
	hmm_type       = LEFT_TO_RIGHT;

	for (int j=0; j<hmm_num; j++)
		{
			hmm_vec[j]->CalcStayTime();
		}
	if (Gdebug&&debug) tl_message ("step 1");
	a_matrix.resize((int)hmm_vec[0]->a_matrix.size());
	for (int i=0; i<(int)hmm_vec[0]->a_matrix.size(); i++)
		{
			a_matrix[i] = hmm_vec[0]->a_matrix[i];
			for (int j=0; j<(int)hmm_vec[0]->a_matrix.size(); j++)
				a_matrix[i][j] = 0.0;
		}
	if (Gdebug&&debug) tl_message ("step 2");
	stay_time.resize((int)hmm_vec[0]->num_of_state);
	virtual_time.resize((int)hmm_vec[0]->num_of_state);

	if (Gdebug&&debug) tl_message ("step 3");
	for (int i=0; i < num; i++)
		{
			stay_time[i] = 0.0;
			for (int j=0; j<hmm_num; j++)
				{
					if (Gdebug&&debug) tl_message ("additional value of stay_time is %g * %g", weight_vec[j], hmm_vec[j]->stay_time[i]);
					stay_time[i] += weight_vec[j] * hmm_vec[j]->stay_time[i];
				}
		}
	if (Gdebug&&debug) tl_message ("step 4");
	CalcTransitProbFromStayTime();

	// Interpolation of output probabilities (B)
	if (Gdebug&&debug) tl_message ("step 5");
	state.resize(num-2);

	// 2007 RSJ, RoboSym ���Υ��르�ꥺ��
	JHMMState **state_vec;
	state_vec = (JHMMState **)malloc (sizeof (JHMMState *) * hmm_num);

	for (int i=0; i < num-2; i++)
		{
			for (int j=0; j<hmm_num; j++)
				state_vec[j] = hmm_vec[j]->state[i];
			tmp_state1 = new JHMMState(*(hmm_vec[0]->state[i]));
			tmp_state1->InterpolationAny (state_vec, weight_vec, hmm_num);
			state[i] = tmp_state1;
		}
	free (state_vec);

	if (Gdebug&&debug) tl_message ("final check");
	if (Gdebug&&debug) state[0]->Verify();
	if (Gdebug&&debug) tl_message ("step end");
	return TRUE;
}



/**************************************************
 * Created  : 2007-09-08
 * Function : Interpolation of two HMMs even if the number of states are different
 * Input    : c1, c2 are weights for interpolation
 * Memo     : assumption : num of HMM1 is larger than the num of HMM2
 **************************************************/
int JHMM::InterpolationGeneral (JHMM& hmm1, JHMM& hmm2, double c1, double c2)
{
	int		num1, num2;
	double	virtual_finish_time, stay_time_new;
	int		debug=0;
	JHMMState	*tmp_state = NULL, *tmp_state1=NULL, *tmp_state2=NULL;
	int		l1, l2;
	double	d1, d2;

	if (Gdebug) tl_message ("step 0");
	num1 = hmm1.num_of_state;
	num2 = hmm2.num_of_state;
	if (debug)
		{
			hmm1.Show();
			hmm2.Show();
		}
	if (num1 < num2) 
		// num2 �������礭����н��֤�դˤ��ƸƤӽФ�
		{
			InterpolationGeneral (hmm2, hmm1, c2, c1);
			return TRUE;
		}
	// �ޤ���������HMM��Left����Right�ޤ����ܤ������Τ�ɬ�פʻ���(virtual_time)���Ѥ��
	hmm1.CalcStayTime();
	hmm2.CalcStayTime();
	// ��������������HMM�ξ��ֿ��ϡ�ʬ��ǽ��ͤ���¿������ num1 �Ȥ���ʺ����׸�Ƥ��
	// mixture �� 1 �Ȥ��롥
	num_of_state   = hmm1.num_of_state;
	num_of_mixture = hmm1.num_of_mixture;
	vector_size    = hmm1.vector_size;
	label          = "interpolation";
	hmm_type       = LEFT_TO_RIGHT;
	// ��������������HMM�����ܴ�λ����ϡ�hmm1 �Τ���� hmm2 �Τ����c1:c2 �γ��Ǻ��礷����ΤȤ����c1+c2��1���б������
	virtual_finish_time = hmm1.virtual_time[num1-1] * c1 / (c1+c2) + hmm2.virtual_time[num2-1] * c2 / (c1+c2);
	// ��������������HMM�ξ������ܳ�Ψ�ϳƥΡ��ɤǰ�Χ�Ȳ��������Ѱ�� estimate stay time �����
	// 1������Ƥ���Τϡ��ǽ�ΥΡ��ɤ����ܻ��֤�1�����顤�������������֤� num1-2 ������ܤǷв᤹��Τǡ��ʲ��μ��Ȥʤ�
	stay_time_new = (virtual_finish_time - 1.0) / (num1 - 2);

	if (Gdebug) tl_message ("step 1");
	// �Ѱ�� stay time �˴�Ť��ơ��������������ܳ�Ψ�򻻽Ф���
	a_matrix.resize((int)hmm1.a_matrix.size());
	for (int i=0; i<(int)hmm1.a_matrix.size(); i++)
		{
			a_matrix[i] = hmm1.a_matrix[i];	// TODO: �ץ����å�!! ���졤hmm1.a_matrix ����Ȥ�������ˤʤ�ʤ���
			for (int j=0; j<(int)hmm1.a_matrix.size(); j++)
				a_matrix[i][j] = 0.0;
		}
	if (Gdebug) tl_message ("step 2");
	stay_time.resize((int)hmm1.num_of_state);
	virtual_time.resize((int)hmm1.num_of_state);

	if (Gdebug) tl_message ("step 3");
	stay_time[0] = 1.0;
	virtual_time[0] = 0.0;
	for (int i=1; i < num_of_state-1; i++)	//�Ǹ�ΥΡ��ɤϽ�ü�Ρ��ɤʤΤ� stay_time ���������ʤ�
		{
			stay_time[i] = stay_time_new;
			virtual_time[i] = virtual_time[i-1] + stay_time_new;
		}
	virtual_time[num_of_state-1] = virtual_time[num_of_state-2] + stay_time_new;
	if (debug) tl_message ("stay time of new HMM");
	for (int i=0;i<num_of_state-1; i++)
		cerr << stay_time[i] << "  ";
	cerr << endl;
	if (debug) tl_message ("virtual time of new HMM");
	for (int i=0;i<num_of_state; i++)
		cerr << virtual_time[i] << "  ";
	cerr << endl;
	if (Gdebug) tl_message ("step 4");
	CalcTransitProbFromStayTime();

	// Interpolation of output probabilities (B)
	if (Gdebug) tl_message ("step 5");
	state.resize(num_of_state-2);	// state �ο��ϥΡ��ɤο���ꣲ���ʤ�!!
	// k=0 : �����ܤΥΡ��ɡ����ʤؤ����ܤ�������ƤΥΡ���
	// k=1 : �����ܤΥΡ��ɡ�virtual_time �� 1+s �Ȥʤ�Ρ���
	// k�� 0, 1, .... , num_of_state -3 �ޤǤȤʤ��num_of_state -2 �Ĥ� JMMState���󥹥��󥹤����������ˤʤ�)
	// �ޤ���k=0 �ˤĤ��Ƥϡ����̤˹������ƹԤ���
	tmp_state = new JHMMState(*(hmm1.state[0]));
	tmp_state->Interpolation (*(hmm1.state[0]), *(hmm2.state[0]), c1, c2);
	state[0] = tmp_state;
	// k=1 ���� k=num_of_state-3 �ޤǤϡ������르�ꥺ��� (for ICRA'08)
	for (int k=1; k <= num_of_state - 3; k++)
		// index k �ϡ�HTK ή���� index 
		{
			double phase_ratio=0.0;
			if (Gdebug) tl_message ("step 6.1");
			// �ޤ��Ρ��ɿ����礭�� HMM1¦�Ƕ���ʹ���Interpolation
			// k+1 �� JHMM�Ǥ� node index ����������
			phase_ratio = hmm1.virtual_time[num1-1] / virtual_time[num_of_state-1];
			l1 = hmm1.FindNearestVirtualTime (virtual_time[k+1] * phase_ratio);
			//����������HMM�� virtual time �� HMM1 �� virtual time �ΰ�����Ѵ����Ƥ��顤�Ǥ�ᤤ����õ��
			d1 = virtual_time[k+1] * phase_ratio - hmm1.virtual_time[l1];
			d2 = hmm1.virtual_time[l1+1] - virtual_time[k+1] * phase_ratio;
			if (debug) tl_message ("target vt = %g, l1 = %d, d1 = %g, d2 = %g", virtual_time[k+1], l1, d1, d2);
			if (l1 == hmm1.num_of_state-2)
				{
					if (debug) tl_message ("last node case for HMM1");
					tmp_state1 = new JHMMState(*(hmm1.state[l1-1]));
				}
			else
				{
					tmp_state1 = new JHMMState(*(hmm1.state[0]));
					tmp_state1->Interpolation (*(hmm1.state[l1-1]), *(hmm1.state[l1]), d2/(d1+d2), d1/(d1+d2));      
					// l1 - 1 �� HTKή���� node index �����
					if (debug)
						{
							tl_message ("sub-interpolation in HMM1 : %g * state[%d] + %g * state[%d]", d2/(d1+d2), l1, d1/(d1+d2), l1+1);
							tmp_state1->Verify();
						}
				}
			// ���˥Ρ��ɿ��ξ��ʤ���HMM2¦�Ƕ��ߤ���Interpolation
			phase_ratio = hmm2.virtual_time[num2-1] / virtual_time[num_of_state-1];
			l2 = hmm2.FindNearestVirtualTime (virtual_time[k+1] * phase_ratio);
			d1 = virtual_time[k+1] * phase_ratio - hmm2.virtual_time[l2];
			d2 = hmm2.virtual_time[l2+1] - virtual_time[k+1] * phase_ratio;
			if (debug) tl_message ("target vt = %g, l2 = %d, d1 = %g, d2 = %g", virtual_time[k+1], l2, d1, d2);
			if (l2 == hmm2.num_of_state-2)
				{
					// �Ρ��ɤ�������¦�� HMM2 �Ǥϡ�target virtual time����ʬ�κǸ�ΥΡ��ɤǶ�������Ǥ��ʤ���������
					if (debug) tl_message ("last node case for HMM2");
					tmp_state2 = new JHMMState(*(hmm2.state[l2-1]));
				}
			else
				{
					tmp_state2 = new JHMMState(*(hmm2.state[0]));
					tmp_state2->Interpolation (*(hmm2.state[l2-1]), *(hmm2.state[l2]), d2/(d1+d2), d1/(d1+d2));
					if (debug)
						{
							tl_message ("sub-interpolation in HMM2 : %g * state[%d] + %g * state[%d]", d2/(d1+d2), l2, d1/(d1+d2), l2+1);
							tmp_state2->Verify();
						}
				}
			// �Ǹ�ˡ�ICONIP'07 �Υ��르�ꥺ��Ǻǽ� Interpolation
			if (Gdebug) tl_message ("step 6.2");
			tmp_state = new JHMMState(*tmp_state1);
			tmp_state->Interpolation (*tmp_state1, *tmp_state2, c1, c2);
			state[k] = tmp_state;
			if (debug)
				{
					tl_message ("step 6.3 : mixture of single Gaussian is %g", tmp_state->mix_weight[0]);
					tmp_state->Verify();
				}
			delete tmp_state1;
			delete tmp_state2;
		}
	if (Gdebug) tl_message ("final check");
	if (Gdebug&&debug) state[0]->Verify();
	if (Gdebug) tl_message ("step end");
	return TRUE;
}



#if 0
/**************************************************
 * Created  : 2007-06-19
 * Function : Extrapolation of two HMMs
 * Input    : alpha is weights for extrapolation
 * Input    : hmm1 is origin of the extrapolation
 * Input    : hmm2 is effector side's HMM
 * Memo     : not using operator+ nor operator*
 *          : There is an assumption that the number of nodes are same for two HMMs
 **************************************************/
int JHMM::Extrapolation (JHMM& hmm1, JHMM& hmm2, double alpha)
{
	int num;
	num = hmm1.num_of_state;
	JHMMState	*tmp_state1 = NULL;
	//JHMMState	*tmp_state2 = NULL;

	num_of_state   = hmm1.num_of_state;
	num_of_mixture = hmm1.num_of_mixture;
	vector_size    = hmm1.vector_size;
	label          = "extrapolation";
	hmm_type       = LEFT_TO_RIGHT;

	hmm1.CalcStayTime();
	hmm2.CalcStayTime();

	if (Gdebug) tl_message ("step 1");
	a_matrix.resize((int)hmm1.a_matrix.size());
	for (int i=0; i<(int)hmm1.a_matrix.size(); i++)
		{
			a_matrix[i] = hmm1.a_matrix[i];
			for (int j=0; j<(int)hmm1.a_matrix.size(); j++)
				a_matrix[i][j] = 0.0;
		}
	if (Gdebug) tl_message ("step 2");
	stay_time.resize((int)hmm1.num_of_state);
	virtual_time.resize((int)hmm1.num_of_state);

	if (Gdebug) tl_message ("step 3");
	for (int i=0; i < num; i++)
		{
			stay_time[i] = alpha * hmm2.stay_time[i] + (1-alpha) * hmm1.stay_time[i];
			if (Gdebug) tl_message ("st1 = %g, st2 = %g --> st = %g", hmm1.stay_time[i], hmm2.stay_time[i], stay_time[i]);
		}
	if (Gdebug) tl_message ("step 4");
	CalcTransitProbFromStayTime();

	if (Gdebug) tl_message ("step 5");
	state.resize(num-2);
	for (int i=0; i < num-2; i++)
		{
#if 0
			tmp_state1 = *(hmm1.state[i]) * (-1.0);
			tmp_state2 = *(hmm2.state[i]) * alpha;
			state[i] = *tmp_state1 + *tmp_state2;
			delete tmp_state1;
			delete tmp_state2;
#else
			// 2007 July 1st �ο���ά�ʤ���ǡ�
			tmp_state1 = new JHMMState(*(hmm1.state[i]));
			tmp_state1->Extrapolation (*(hmm1.state[i]), *(hmm2.state[i]), alpha);
			state[i] = tmp_state1;
#endif
		}

	if (Gdebug) tl_message ("step end");
	return TRUE;
}
#endif




/**************************************************
 * Created  : 2003/10/01	
 * Function : sampling_time�򥻥åȤ���
 * Memo     :
 **************************************************/
int JHMM::SetSamplingTime(int time)
{
	sampling_time = time;

	return TRUE;
}


/**************************************************
 * Created  : 2003/10/01	
 * Function : sampling_time���֤�
 * Memo     : 
 **************************************************/
int JHMM::GetSamplingTime()
{
	return sampling_time;
}

/**************************************************
 * Created  : 2003/10/02
 * Function : Label���֤�
 **************************************************/
string JHMM::GetLabel()
{
	return label;
}


/**************************************************
 * Created  : 2004 Sep 12th	by inamura
 * Function : Label���֤�
 **************************************************/
void *JHMM::GetLabelChar(void)
{
	return (void *)label.c_str();
}



/**************************************************
 * Created  : 2008-07-16	by inamura
 * Function : Calculation of Hellinger Distance
 **************************************************/
double JHMM::HellingerDistance (JHMM &hmm2)
{
	int		debug=0;
	double	result=0.0;

	if (debug) tl_message ("start");
	for (int i=0; i<num_of_state-2; i++)
		{
			if (debug) tl_message ("step 1.%d", i);
			result += state[i]->HellingerDistance( *(hmm2.state[i]) );
		}
	if (debug) tl_message ("end");
  
	return result;
}


/**************************************************
 * Created  : 2005 Jun 18th	by inamura
 * Function : Verify
 **************************************************/
int JHMM::Verify(void)
{
	int	i, j;
	if (Gdebug)
		{
			fprintf (stderr, "number of state   : %d\n", num_of_state);
			fprintf (stderr, "number of mixture : %d\n", num_of_mixture);
			fprintf (stderr, "vector size       : %d\n", vector_size);
			fprintf (stderr, "vector type       : %d\n", hmm_type);
			fprintf (stderr, "label             : %s\n", label.c_str());
			fprintf (stderr, "A matrix          : \n");
			for (i=0; i<num_of_state; i++)
				{
					for (j=0; j<num_of_state; j++)
						fprintf (stderr, "%g ", a_matrix[i][j]);
					fprintf (stderr, "\n");
				}
		}
	if (!(hmm_type==ERGOTIC || hmm_type==PERIODIC || hmm_type==LEFT_TO_RIGHT))
		{
			tl_message ("hmm_type is wrong value");
			tl_message ("please press enter key");
			getchar();
			return FALSE;
		}
	return TRUE;
}




#if 0
/**************************************************
 * Created  : 2003/09/18	
 * Function : 
 * Memo     : Old version
 **************************************************/
JHMM* JHMM::operator+(JHMM hmm2)
{
	JHMM* temp;
	int debug=0;

	temp = new JHMM();
	temp->num_of_state = this->num_of_state;
	temp->num_of_mixture = this->num_of_mixture;
	temp->vector_size = this->vector_size;
  
	temp->label = this->label;
	temp->hmm_type = this->hmm_type;

	temp->a_matrix.resize((int)this->a_matrix.size());
	if (debug&&Gdebug) cerr << "Resize:" << this->a_matrix.size() << endl;
	for (int i=0; i<(int)this->a_matrix.size(); i++)
		temp->a_matrix[i] = this->a_matrix[i];

	if (debug&&Gdebug) cerr << "Num:" << num_of_state << endl;
	for (int i=0; i < num_of_state; i++)
		for (int j=0; j < num_of_state; j++)
			temp->a_matrix[i][j] += hmm2.a_matrix[i][j];

	temp->state.resize(num_of_state-2);
	for (int i=0; i < num_of_state-2; i++)
		temp->state[i] = (*(this->state[i])) + (*(hmm2.state[i]));

	return temp;
}
#endif


/**************************************************
 * Created  : 2003/09/18	
 * Modified : 2006-09-08 by inamura ��Ψ�ͤ�[0.0-1.0]��Ķ���륱���������������н�
 * Function : ��Ĥ�HMM�ι���
 * Memo     : 
 **************************************************/
JHMM* operator+(JHMM& hmm1, JHMM& hmm2)
{
	JHMM*	temp;
	int	num, debug=0;

	if (debug&&Gdebug) tl_message ("start");
	if (debug&&Gdebug) hmm1.Verify();
	if (debug&&Gdebug) hmm2.Verify();

	num = hmm1.num_of_state;
	temp = new JHMM();
	temp->num_of_state = hmm1.num_of_state;
	temp->num_of_mixture = hmm1.num_of_mixture;
	temp->vector_size = hmm1.vector_size;
  
	temp->label = hmm1.label;
	temp->hmm_type = hmm1.hmm_type;

	temp->a_matrix.resize((int)hmm1.a_matrix.size());
	for (int i=0; i<(int)hmm1.a_matrix.size(); i++)
		{
			temp->a_matrix[i] = hmm1.a_matrix[i];
		}

	for (int i=0; i < num; i++)
		{
			double sum = 0.0;
			for (int j=0; j < num; j++)
				{
					temp->a_matrix[i][j] += hmm2.a_matrix[i][j];
					if (temp->a_matrix[i][j] > 1.0) temp->a_matrix[i][j] = 0.99;
					if (temp->a_matrix[i][j] < 0.0) temp->a_matrix[i][j] = 0.01;
					sum += temp->a_matrix[i][j];
				}
			if (sum > 1.0)
				for (int j=0; j < num; j++)
					temp->a_matrix[i][j] /= sum;
		}
	temp->state.resize(num-2);
	for (int i=0; i < num-2; i++)
		temp->state[i] = (*(hmm1.state[i])) + (*(hmm2.state[i]));

	if (debug&&Gdebug) tl_message ("end");
	return temp;
}


#if 0
/**************************************************
 * Created  : 2007-05-14
 * Function : 
 * Memo     : �¿��ܤ�����򹹿����������¿��ܤˤʤ�褦��A��Ĵ��
 **************************************************/
JHMM* operator*(JHMM& hmm1, double dbl)
{
	JHMM* temp;
	int	length;
	int	debug=0;
	double  c_time=0.0;
	temp = new JHMM(hmm1);

	length = hmm1.EstimatePeriod();

	for (int i=0; i < hmm1.num_of_state-1; i++)
		{
			for (int j=i; j <= i+1; j++)
				{
					if (debug) tl_message ("[i,j] = [%d,%d] (%g)", i, j, hmm1.a_matrix[i][j]);
					c_time = (1.0 / hmm1.a_matrix[i][j]) * dbl;
					if (debug) tl_message ("step 1");
					c_time = (1.0)/c_time;
					if (debug) tl_message ("step 2");
					temp->a_matrix[i][j] = c_time;
					if (debug) tl_message ("step 3");
				}
		}
  
	for (int i=0; i < hmm1.num_of_state - 2; i++)
		// There might be bug 2007-05-14, (change from  i<hmm1.num_of_state-2)
		temp->state[i]->WeightDouble(dbl);

	return temp;
}
#endif

#if 1
/**************************************************
 * Created  : 2003/09/28	
 * Function : 
 * Memo     : 2007-05-14 storage as an original version
 **************************************************/
JHMM* operator*(JHMM& hmm1, double dbl)
{
	JHMM* temp;

	temp = new JHMM(hmm1);

	for (int i=0; i < hmm1.num_of_state; i++)
		for (int j=0; j < hmm1.num_of_state; j++)
			temp->a_matrix[i][j] *= dbl;

	for (int i=0; i < hmm1.num_of_state - 2; i++)
		temp->state[i]->WeightDouble(dbl);

	return temp;
}
#endif




#if 0
/**************************************************
 * Created  : 2003/09/28	
 * Function : 
 * Memo     :
 **************************************************/
JHMMState* JHMMState::operator+(JHMMState state2)
{
	cerr << "State plus" << endl;

	JHMMState* temp;
	int num2, num1;

	num2 = state2.num_of_mixture;
	num1 = this->num_of_mixture;

	temp = new JHMMState(*this);

	for (int i=0; i<num2; i++)
		temp->mix_weight.push_back(state2.mix_weight[i]);

	temp->mean.resize(num1+num2);
	for (int i=0; i<num2; i++)
		temp->mean[i+num1] = state2.mean[i];
	temp->variance.resize(num1+num2);
	for (int i=0; i<num2; i++)
		{
			temp->variance[i+num1] = state2.variance[i];
		}

	temp->SetNumMix();

	cerr << "State plus Finish" << endl;

	return temp;
}
#endif


/**************************************************
 * Created  : 2003/09/28	
 * Function : Composing two state of HMMs  
 * Memo     :
 **************************************************/
JHMMState* operator+(JHMMState& state1, JHMMState& state2)
{
	JHMMState* temp;
	int num2, num1;

	num2 = state2.num_of_mixture;
	num1 = state1.num_of_mixture;

	temp = new JHMMState(state1);

	for (int i=0; i<num2; i++)
		temp->mix_weight.push_back(state2.mix_weight[i]);

	temp->mean.resize(num1+num2);
	for (int i=0; i<num2; i++)
		temp->mean[i+num1] = state2.mean[i];
	temp->variance.resize(num1+num2);
	for (int i=0; i<num2; i++)
		temp->variance[i+num1] = state2.variance[i];

	temp->SetNumMix();

	return temp;
}


/**************************************************
 * Created  : 2003/09/28	
 * Function : 
 * Memo     :
 **************************************************/
JHMMState* operator*(JHMMState& state1, double dbl)
{
	JHMMState* temp;
	int num1;

	num1 = state1.num_of_mixture;

	temp = new JHMMState(state1);

	for (int i=0; i<num1; i++)
		temp->mix_weight[i] *= dbl;

	return temp;
}



/**************************************************
 * Created  : 2007/06/30
 * Function : Interpolation by two instnaces of JHMMState
 * Memo     : Copy Constructor ���Ǥ��餫���᥹��å��ѿ������ꤷ�Ƥ���ƤӽФ���
 **************************************************/
int JHMMState::Interpolation(JHMMState& state1, JHMMState& state2, double c1, double c2)
{
	//JHMMState	*temp;
	int		num2, num1;
	int		debug=0;
	vector<double>  c1_mean, c2_mean, c1_vari, c2_vari;

	if (&state1==NULL || &state2==NULL)
		{
			tl_warning ("pointer is NULL");
			exit(0);
		}
	if (Gdebug&&debug)
		{
			tl_message ("start");
			state1.Verify();
			state2.Verify();
		}
	num1 = state1.num_of_mixture;
	num2 = state2.num_of_mixture;
	if (num1!=1 || num2!=1)
		{
			tl_warning ("num1 = %d, num2 = %d : this func. requires single mixture", num1, num2);
			return FALSE;
		}

	if (debug)
		{
			tl_message ("Content of state1 is : ");
			state1.Verify();
			tl_message ("Content of new state (initial) is : ");
			Verify();
		}
	c1_mean = *(multiple_scalar_vector (c1, state1.mean[0]));
	c2_mean = *(multiple_scalar_vector (c2, state2.mean[0]));
	mean[0] = * (plus_vector (c1_mean, c2_mean));
	c1_vari     = *(multiple_scalar_vector (c1 * c1, state1.variance[0])); // modified on 2008-07-05 for IROS08 revision
	c2_vari     = *(multiple_scalar_vector (c2 * c2, state2.variance[0])); // modified on 2008-07-05 for IROS08 revision
	variance[0] = * (plus_vector (c1_vari, c2_vari));

	mix_weight[0] = 1.0;
	if (debug)
		{
			tl_message ("Content of new state (final) is: ");
			Verify();
		}
	//temp->SetNumMix();
	return TRUE;
}



/**************************************************
 * Created  : 2007-12-24
 * Function : Interpolation/Extrapolation by any number of JHMMState
 * Memo     : Copy Constructor ���Ǥ��餫���᥹��å��ѿ������ꤷ�Ƥ���ƤӽФ���
 **************************************************/
int JHMMState::InterpolationAny(JHMMState *state_vec[], double *weight_vec, int state_num)
{
	int		debug=0;

	for (int j=0; j<state_num; j++)
		{
			if (state_vec[j]==NULL)
				{
					tl_warning ("pointer is NULL");
					exit(0);
				}
		}
	if (Gdebug&&debug)
		{
			tl_message ("start");
			for (int j=0; j<state_num; j++)
				state_vec[j]->Verify();
		}
	for (int j=0; j<state_num; j++)
		{
			if (state_vec[j]->num_of_mixture != 1)
				{
					tl_warning ("all of the state should has single mixture");
					return FALSE;
				}
		}
	if (debug) {
		tl_message ("check 0-th mean vector before creating zero vector");
		for (vector<double>::iterator p=state_vec[0]->mean[0].begin(); p!=state_vec[0]->mean[0].end(); p++)
			cerr << *p << " ";
		cerr << endl;
	}
	// Creating zero vector
	mean[0].resize(state_vec[0]->mean[0].size());
	for (vector<double>::iterator p=mean[0].begin();     p!=mean[0].end();     p++) *p = 0.0;
	variance[0].resize(state_vec[0]->variance[0].size());
	for (vector<double>::iterator p=variance[0].begin(); p!=variance[0].end(); p++) *p = 0.0;

	for (int j=0; j<state_num; j++)
		{
			vector<double> mean_j (state_vec[j]->mean[0]);   // copy constructor
			multiple_scalar_vector (weight_vec[j], mean_j); // replace the contents of mean_j
			if (debug) {
				tl_message ("check %d-th original mean vector", j);
				cerr << weight_vec[j] << " x ";
				for (vector<double>::iterator p=state_vec[j]->mean[0].begin(); p!=state_vec[j]->mean[0].end(); p++)
					cerr << *p << " ";
				cerr << endl;
			}
			vector<double> vari_j (state_vec[j]->variance[0]);
			multiple_scalar_vector (weight_vec[j] * weight_vec[j], vari_j); // modified on 2008-07-05 for IROS08 revision
			if (debug) {
				tl_message ("check %d-th mean_j vector", j);
				cerr << weight_vec[j] << " x ";
				for (vector<double>::iterator p=mean_j.begin(); p!=mean_j.end(); p++)
					cerr << *p << " ";
				cerr << endl;
			}
			mean[0]     = * (plus_vector (mean[0], mean_j));
			variance[0] = * (plus_vector (variance[0], vari_j));
		}
	mix_weight[0] = 1.0;
	if (debug)
		{
			tl_message ("Content of new state (final) is: ");
			Verify();
		}
	return TRUE;
}




/**************************************************
 * Created  : 2007-07-01
 * Function : Extrapolation of JHMMState
 * Memo     :
 **************************************************/
int JHMMState::Extrapolation(JHMMState& state1, JHMMState& state2, double alpha)
{
	int		num2, num1;
	int		debug=0;
	vector<double>  extra_mean, diff_mean, extra_vari, diff_vari;

	if (Gdebug&&debug)
		{
			tl_message ("start");
			state1.Verify();
			state2.Verify();
		}
	num1 = state1.num_of_mixture;
	num2 = state2.num_of_mixture;
	if (num1!=1 || num2!=1)
		{
			tl_warning ("num1 = %d, num2 = %d : this func. requires single mixture", num1, num2);
			return FALSE;
		}
	diff_mean   = *(minus_vector (state2.mean[0], state1.mean[0]));
	extra_mean  = *(multiple_scalar_vector (alpha,  diff_mean));
	mean[0]     = *(plus_vector (state1.mean[0], extra_mean));
	diff_vari   = *(minus_vector (state2.variance[0], state1.variance[0]));
	extra_vari  = *(multiple_scalar_vector (alpha, diff_vari));
	variance[0] = *(plus_vector (state1.variance[0], extra_vari));
  
	for (vector<double>::iterator it=variance[0].begin(); it!=variance[0].end(); it++)
		{
			if (*it < 0.001) *it = 0.001;
		}
	mix_weight[0] = 1.0;
	if (debug)
		{
			tl_message ("Content of new state (final) is: ");
			Verify();
		}
	return TRUE;
}



/**************************************************
 * Created  : 2003/09/28	
 * Function : MixWeight�˷����򤫤���
 * Memo     :
 **************************************************/
int JHMMState::WeightDouble(double dbl)
{
	for (int i=0; i<num_of_mixture; i++)
		mix_weight[i] *= dbl;
  
	return TRUE;
}



double JHMMState::HellingerDistance (JHMMState &other)
{
	double	result=0.0;
	double	sum1=1.0, sum2=1.0;

	for (int i=0; i<vector_size; i++)
		{
			result += 2 * (mean[0][i] - other.mean[0][i]) * (mean[0][i] - other.mean[0][i]) / (variance[0][i] + other.variance[0][i]);

			sum1 *= 0.5 * (variance[0][i] + other.variance[0][i]);
			sum2 *= sqrt(variance[0][i]) * sqrt(other.variance[0][i]);
	
		}
	result /= 8.0;
	result += 0.5 * log(sum1 / sum2);
  
	return sqrt(result);	// <-- IMPORTANT !!  on 2008-07-18
}
