/*
 * Mds.cpp
 *
 * Last Modified on  2005 Jun 18th  by Tetsunari Inamura
 */

/*
 * TODO
 * ニュートンラフソン法の閾値が Magic Number 
 * 状況に応じて適応的に閾値が求まって欲しい
 */

#include "mimesis.h"
#include "Mds.h"



/**************************************************
 * Created   : 2002/11/01	
 * Function  : ニュートン・ラフソン法により
 *	       f(X) = a*X^4 + b*X^3 + c*X^2 + d*X の極小値を与えるXを求める
 * Input     : f(X)の a, b, c, dの値
 *             init_value -> ニュートンラフソン法をいくつから始めるか?
 * Memo      : 規格化の値よりXの絶対値が大きくなることはない
 ***************************************************/
double _newton (double a, double b, double c, double d, double init_value)
{
  double	inflect_value, inflect_point, result1, result2;
  double	x, init_x, last_x, diff=0.0, pre_diff=0.0, tmp_x;
  //double	e = 0.00000001; // Magic Number  original value before 2006-10-24
  //double	e = 0.0000001; // Magic Number   used between 2006--2008 Sep
  double	e = 0.4; // Magic Number    used after 2008 Oct
  int		debug=0;

  // 変曲点 X = -b/(4a)
  inflect_point = -b/(4*a);

  // 変曲点での f'(X) の計算
  inflect_value = 4*a*(inflect_point)*(inflect_point)*(inflect_point) 
                   + 3*b*(inflect_point)*(inflect_point) + 2*c*(inflect_point) + d;

  diff = 2;	pre_diff = diff;

  // 変曲点で正なら f'(X)=0 の３つの解のうち最小のものが f(X) を最小化!!
  // マイナス側から探していく
  if(inflect_value > 0) init_x = - init_value;     //Magic Number

  // 変曲点で負ならプラス側から探す．
  else if(inflect_value < 0) init_x = init_value; //Magic Number

  // 変曲点で０の場合は,念のため最大解と最小解を求め
  // どちらが f(X) を最小にするか確認       TODO:正の場合に含めてもOK？
  else {
    if(debug&&Gdebug) cerr << "[_newton] Inflect Value zero!" << endl;

    // 最小解を求める    
    init_x = - init_value;
    x = init_x;
    while( ((diff >= 0) && (diff > e)) || ((diff < 0) && (diff < -e)) ){
      if(Gdebug&&debug) tl_message ("in while loop 0 : x=%g", x);
      pre_diff = diff;
      last_x = x;
      x     += - (4*a*x*x*x + 3*b*x*x + 2*c*x + d)/(12*a*x*x + 6*b*x + 2*c);
      diff   = last_x - x;
      //if ( (pre_diff==diff) || ((pre_diff>diff)&&(pre_diff-diff)<e) || ((pre_diff<diff)&&(diff-pre_diff)<e) ) break;
    }
    // 最小解の時のf(X)の値を計算 
    result1 = a*x*x*x*x + b*x*x*x + c*x*x + d*x;
    tmp_x   = x;


    // 最大解を求める
    init_x = init_value;
    x      = init_x;
    diff   = 1;
    while( ((diff >= 0) && (diff > e)) || ((diff < 0) && (diff < -e)) ){
      pre_diff = diff;
      if(Gdebug&&debug) tl_message ("in while loop 1 : x=%g", x);
      last_x = x;
      x     += - (4*a*x*x*x + 3*b*x*x + 2*c*x + d)/(12*a*x*x + 6*b*x + 2*c);
      diff   = last_x - x;
      //if ( (pre_diff==diff) || ((pre_diff>diff)&&(pre_diff-diff)<e) || ((pre_diff<diff)&&(diff-pre_diff)<e) ) break;
    }
    // 最大解の時のf(X)の値を計算
    result2 = a*x*x*x*x + b*x*x*x + c*x*x + d*x;
    
    // f(X)を最小にする方のXを返す
    if(result1 < result2){
      if(debug&&Gdebug) cerr << "[_newton] newton x:" << tmp_x << endl;
      return tmp_x;
    }
    else {
      if(debug&&Gdebug) cerr << "[_newton] newton x:"<<  x << endl;
      return x;
    }
  }

  x = init_x;
  
  // 変位がeより小さくなるまで計算
  while( ((diff >= 0) && (diff > e)) || ((diff < 0) && (diff < -e)) ){
    if(Gdebug&&debug) tl_message ("in while loop 2 : x=%g", x);
    pre_diff = diff;
    last_x = x;
    x     += - (4*a*x*x*x + 3*b*x*x + 2*c*x + d)/(12*a*x*x + 6*b*x + 2*c);
    diff   = last_x - x;
    //if ( (pre_diff==diff) || ((pre_diff>diff)&&(pre_diff-diff)<e) || ((pre_diff<diff)&&(diff-pre_diff)<e) ) break;
  }
  
  if(debug&&Gdebug) cerr << "[_newton] newton init_x:" << init_x << " diff:" << diff << " x:" << x << endl;

  return x;
}



/**************************************************
 * Created  : 2003/08/28	
 * Function : MDSのコンストラクタ
 * Memo     : 
 **************************************************/
MDS::MDS()
{
  Reset();
}


/**************************************************
 * Created  : 2003/08/28	
 * Function : MDSのコンストラクタ
 * Memo     : dimensionを設定できる
 **************************************************/
MDS::MDS(int dim)
{
  dimension = dim;
}


/**************************************************
 * Created  : 2003/09/09	
 * Function : コピーコンストラクタ
 * Memo     :
 **************************************************/
MDS::MDS(const MDS& another)
{
  int debug=0;

  if(debug&&Gdebug) cerr << "[MDS.MDS] copy constructor!" << endl;

  i_num = another.i_num;
  j_num = another.j_num;
  
  original_data.resize((int)another.original_data.size());
  for(int i=0; i<(int)another.original_data.size(); i++){
    original_data[i] = another.original_data[i];
  }

  target_distance.resize((int)another.target_distance.size());
  for(int i=0; i<(int)another.target_distance.size(); i++){
    target_distance[i] = another.target_distance[i];
  }

  normalize = another.normalize;
  dimension = another.dimension;
  coordinate.resize((int)another.coordinate.size());
  for(int i=0; i<(int)another.coordinate.size(); i++){
    coordinate[i] = another.coordinate[i];
  }
  distance.resize((int)another.distance.size());
  for(int i=0; i<(int)another.distance.size(); i++){
    distance[i] = another.distance[i];
  }
  
  if(debug&&Gdebug) cerr << "[MDS.MDS] copy constructor finish!" << endl;
}


// Created  : 2003/08/28
MDS::~MDS()
{ 
  int debug=0;

  if(debug&&Gdebug) cerr << "[MDS::~MDS] original_data destroy start" << endl;
  if(!original_data.empty())
    {
      for(int i=0; i <(int)original_data.size(); i++)
	original_data[i].clear();
      
      original_data.clear();
      if(debug&&Gdebug) cerr << "[MDS::~MDS] original_data clear" << endl;
    } 
  if(debug&&Gdebug) cerr << "[MDS::~MDS] target_distance destroy start" << endl;
  if(!target_distance.empty()){
    for(int i=0; i <(int)target_distance.size(); i++){
      target_distance[i].clear();
    }
    target_distance.clear();
    if(debug&&Gdebug) cerr << "[MDS::~MDS] target_distance clear" << endl;
  }
  if(debug&&Gdebug) cerr << "[MDS::~MDS] coordinate destroy start" << endl;
  if(!coordinate.empty()){
    for(int i=0; i <(int)coordinate.size(); i++){
      coordinate[i].clear();
    }
    coordinate.clear();
    if(debug&&Gdebug) cerr << "[MDS::~MDS] coordinate clear" << endl;
  }
  if(debug&&Gdebug) cerr << "[MDS::~MDS] distance destroy start" << endl;
  if(!distance.empty())
    {
      for(int i=0; i<(int)distance.size(); i++)
	{
	  distance[i].clear();
	}
      distance.clear();
      if(debug&&Gdebug) cerr << "[MDS::~MDS] distance clear" << endl;
    }
  if(i_num.empty())
    i_num.clear();
  if(j_num.empty())
    j_num.clear();

  if(debug&&Gdebug) cerr << "[MDS::~MDS] Finish!" << endl;
}


/**************************************************
 * Created  : 2003/08/28	
 * Function : MDSの初期化
 * Memo     : まだ全然
 **************************************************/
int MDS::Reset(void)
{
  dimension=0;
  normalize=1;

  return TRUE;
}


/**************************************************
 * Created  : 2003/08/28	
 * Function : dimenstionのセット
 * Memo     : 
 **************************************************/
int MDS::SetDimension(int dim)
{
  dimension = dim;

  return TRUE;
}


/**************************************************
 * Created  : 2003/08/28	
 * Function : ファイルから距離データの読み込み
 * Memo     : 
 **************************************************/
int MDS::SetData(const char* fname)
{
  ifstream	fin(fname);
  char		buf[MAX_STRING], tmpbuf[MAX_STRING], tmplabel[MAX_STRING];
  double	tmp_d;
  int		num;
  int		debug=0;

  if(debug&&Gdebug) tl_message ("start");
  original_data.clear();
  if(!fin)
    {
      cerr << "Cannot open '" << fname << "'." << endl;
      return FALSE;
    }
  if((fin.getline(buf, 1000)) == NULL)
    {
      tl_warning ("file \"%s\" is empty!", fname);
      fin.close();
      return FALSE;
    }
  // 1行目 #num_of_data でないとエラー
  if(strncmp(buf, "#num_of_data", 12)){
    cerr << "[MDS.SetData] " << buf << endl;
    return FALSE;
  }
  sscanf(buf, "%s %d", tmpbuf, &num);
  if(debug&&Gdebug) cerr << "[MDS.SetData] NumOfData:" << num << endl;
  
  original_data.resize(num);
  for(int i=0; i<num; i++)
    {
      fin >> tmplabel;
      if(debug&&Gdebug) cerr << "[MDS.SetData] Label:" << tmplabel << endl;
      //    MDS_SetNthLabel (mds, tmplabel, i);
      for (int j=0; j<num; j++)
	{
	  fin >> tmp_d;
	  original_data[i].push_back(tmp_d);
	  if(debug&&Gdebug) tl_message ("data[%d][%d] = %g", i, j, tmp_d);
	}
      if(debug&&Gdebug) tl_message ("push_back");
      fin.getline(buf, 1000); // 改行コードの読み取り
    }
  fin.close();
  return TRUE;
}


/**************************************************
 * Created  : 2003/08/28	
 * Function : 距離データを2次元配列で読み込む
 * Memo     : 使わない?
 **************************************************/
int MDS::SetData(double **data, int size)
{
  if(!original_data.empty()){
    for(int i=0; i <(int)original_data.size(); i++){
      original_data[i].clear();
    }
    original_data.clear();
  } 
  
  for(int i=0; i<size; i++)
    AddData(data[i], size);

  return TRUE;
}



/**************************************************
 * Created  : 2003/08/28	
 * Function : 距離データを一つ分読み込む
 * Memo     : 使わない?
 **************************************************/
int MDS::AddData(double *data, int size)
{
  int num;

  num = original_data.size();
  original_data.resize((num+1));

  for(int i=0; i<size; i++)
    original_data[num].push_back(data[i]);

  return TRUE;
}


/**************************************************
 * Created  : 2003/08/28	
 * Function : 距離データを一つ分読み込む
 * Memo     : 使わない?
 *            TODO:vectorは中身を一個ずつコピーする必要なし?
 **************************************************/
int MDS::AddData(vector<double> vec)
{
  int num;
  int debug=0;

  if(debug&&Gdebug) cerr << "[MDS.AddData] start" << endl;
  num = original_data.size();

  original_data.resize((num+1));
  for(int i=0; i < (int)vec.size(); i++){
    if(debug&&Gdebug) cerr << "[MDS.AddData] push " << vec[i]  << endl;
    original_data[num].push_back(vec[i]);
  }

  if(debug&&Gdebug) cerr << "[MDS.AddData] end" <<  endl;

  return TRUE;
}


/**************************************************
 * Created  : 2003/08/28	
 * Function : MDSの表示
 * Memo     : 
 **************************************************/
int MDS::Show(void)
{
  cerr << "[MDS.Show] num_of_data:" << GetNumOfData() << endl;
  cerr << "[MDS.Show] original_data" << endl;
  if(!original_data.empty()){
    for(int i=0; i<(int)original_data.size(); i++){
      for(int j=0; j<(int)original_data[i].size(); j++)
	cerr << original_data[i][j] << " ";
      cerr << endl;	
    }
  }
  else cerr << "Not Set" << endl;
  cerr << "[MDS.Show] target_distance" << endl;
  if(!target_distance.empty()){
    for(int i=0; i<(int)target_distance.size(); i++){
      for(int j=0; j<(int)target_distance[i].size(); j++)
	cerr << target_distance[i][j] << " ";
      cerr << endl;	
    }
  }
  else cerr << "Not Set" << endl;

  cerr << "[MDS.Show] dimension:" << dimension << endl;
  cerr << "[MDS.Show] coordinate" << endl;
  if(!coordinate.empty()){
    for(int i=0; i<(int)coordinate.size(); i++){
      for(int j=0; j<(int)coordinate[i].size(); j++)
	cerr << coordinate[i][j] << " ";
      cerr << endl;	
    }
  }
  else cerr << "Not Set" << endl;
  cerr << "[MDS.Show] distance" << endl;
  if(!distance.empty()){
    for(int i=0; i<(int)distance.size(); i++) {
      for(int j=0; j<(int)distance[i].size(); j++)
	cerr << distance[i][j] << " ";
      cerr << endl;	
    }
  } else cerr << "Not Set" << endl;
  cerr << "[MDS.Show] i_num" << endl;
  if(!i_num.empty()){
    for(int i=0; i<(int)i_num.size(); i++)
      cerr << i_num[i] << " ";
    cerr << endl;	
  }
  else cerr << "Not Set" << endl;
  cerr << "[MDS.Show] j_num" << endl;
  if(!j_num.empty()){
    for(int i=0; i<(int)i_num.size(); i++)
      cerr << j_num[i] << " ";
    cerr << endl;	
  } else cerr << "Not Set" << endl;
  
  return TRUE;
}



/**************************************************
 * Created  : 2003/08/28	
 * Function : データの数を返す
 * Memo     : 
 **************************************************/
int MDS::GetNumOfData(void)
{
  return (int)original_data.size();
}



/**************************************************
 * Created  : 2003/08/28	
 * Function : dimensionを返す
 * Memo     : 
 **************************************************/
int MDS::GetDimension(void)
{
  return dimension;
}



/**************************************************
 * Created  : 2003/08/28	
 * Function : normalizeを返す
 * Memo     : 
 **************************************************/
double MDS::GetNormalize()
{
  return normalize;
}

/**************************************************
 * Created  : 2003/08/28	
 * Function : normalizeをセットする
 * Memo     : 
 **************************************************/
void MDS::SetNormalize(double normal)
{
  int debug=0;
  
  if(debug&&Gdebug) cerr << "[MDS.SetNormalize] normalize set " << normal << endl;
  normalize = normal;
}

/**************************************************
 * Created  : 2003/08/28	
 * Function : coordinateの初期化
 * Memo     : 0に設定
 **************************************************/
void MDS::InitCoordinate()
{
  int num, dim;
  int debug=0;

  num = GetNumOfData();
  dim = GetDimension();

  if(!num){
    cerr << "[MDS.InitCoordinate] Invalid NumOfData!!" << endl;
    return;
  }
  if(!dim){
    cerr << "[MDS.InitCoordinate] Invalid Dimension!!" << endl;
    return;
  }

  coordinate.resize(num);
  distance.resize(num);
  for(int i=0; i<num; i++){
    for(int j=0; j<dim; j++)
      coordinate[i].push_back(0.0);
    for(int j=0; j<num; j++)
      distance[i].push_back(0.0);
  }
  
  if(debug&&Gdebug) cerr << "[MDS.InitCoordinate] Finish" << endl;
}


/**************************************************
 * Created  : 2003/08/28	
 * Function : target_distanceの初期化
 * Memo     : 
 **************************************************/
void MDS::DataStandarize()
{
  int num;
  double sum, norm_const, tmp_d;
  int debug=0;

  num = GetNumOfData();
  sum=0;
  norm_const=GetNormalize();
  
  if(!num){
    tl_warning ("NumOfData isn't Set!");
    return;
  }
  
  target_distance.resize(num);
  for(int i=0; i<num; i++){
    for(int j=0; j<num; j++){
      tmp_d = original_data[i][j]*original_data[i][j];
      target_distance[i].push_back(tmp_d);
      sum += tmp_d;
    }
  }

  for(int i=0; i<num; i++){
    for(int j=0; j<num; j++){
      target_distance[i][j] = 2*target_distance[i][j]*norm_const/sum;
    }
  }
  
  if(debug&&Gdebug) cerr << "[MDS.DataStandarize] Finish!" << endl;
}


/**************************************************
 * Created  : 2003/08/28	
 * Function : distanceの計算
 * Memo     : 
 **************************************************/
void MDS::CalcDistance()
{
  int num, dim;
  int debug=0;

  if(debug&&Gdebug) cerr << "[MDS.CalcDistance] Start" << endl;
  num = GetNumOfData();
  dim = GetDimension();
  
  for(int i=0; i<num; i++){
    for(int j=0; j<num; j++){
      distance[i][j] = 0;
      for(int k=0; k<dim; k++){
	distance[i][j] += (coordinate[i][k] - coordinate[j][k])
	                               *(coordinate[i][k] - coordinate[j][k]);
      }
    }
  }
  if(debug&&Gdebug) cerr << "[MDS.CalcDistance] End" << endl;
}


/**************************************************
 * Created  : 2003/08/28	
 * Function : i_num, j_numの計算
 * Memo     : 
 **************************************************/
void MDS::Junjo()
{
  int num, nc2, cur_num;
  int i, j, k, l;
  int debug=0;

  num = GetNumOfData();
  nc2 = num*(num-1)/2;
  cur_num = 0;

  i_num.resize(nc2);
  j_num.resize(nc2);
  for(i=0; i<nc2; i++){
    i_num[i]=0;
    j_num[i]=0;
  }

  for(i=0; i<num; i++){
    for(j=i+1; j<num; j++){
      if(debug&&Gdebug) cerr << "[MDS.Junjo] check data[" << i << "][" << j << "] = " << original_data[i][j] << endl;
      k = cur_num;
      while(k>0){
	if(debug&&Gdebug) cerr << " < data[" << i_num[k-1] << "][" << j_num[k-1] << "]=" << original_data[i_num[k-1]][j_num[k-1]] << "?" << endl;;
	while(original_data[i][j] < original_data[i_num[k-1]][j_num[k-1]]){
	  if(debug&&Gdebug) cerr << "yes" << endl;
	  k = k-1;
	  if(k==0) break;
	  if(debug&&Gdebug) cerr << " < data[" << i_num[k-1] << "][" << j_num[k-1] << "]=" << original_data[i_num[k-1]][j_num[k-1]] << "?" << endl;;
	}
	break;
      }
      for(l=0; l<cur_num - k; l++){
	i_num[cur_num - l] = i_num[cur_num - l -1];
	j_num[cur_num - l] = j_num[cur_num - l -1];
      }

      i_num[k] = i;
      j_num[k] = j;
      cur_num++;
    }
  }
  if(debug&&Gdebug) cerr << "[MDS.Junjo] Check Finish" << endl;
}	


/**************************************************
 * Created  : 2003/08/28	
 * Function : 誤差の計算
 * Memo     : 
 **************************************************/
double MDS::CalcEvalValue()
{
  double sum, tmp;
  int num;
  int debug=0;

  sum = 0;
  num = GetNumOfData();

  if(debug&&Gdebug) cerr << "[MDS.CalcEval] num:" << num << endl;
  for(int i=0; i<num; i++){
    for(int j=0; j<num; j++){
      if(i==j || target_distance[i][j]==0){
	if(debug&&Gdebug) cerr << "[MDS.CalcEval] target_distance[" << i << "][" << j <<"]=0" << endl;
      }else{
	tmp = target_distance[i][j] - distance[i][j];
	sum += tmp*tmp/(4*target_distance[i][j]);
	if(debug&&Gdebug) cerr << "[MDS.CalcEval] cur sum:" << sum << endl;
      }
    }
  }
  if(debug&&Gdebug) cerr << "[MDS.CalcEvalValue] E_Value:" << sum << endl;
  return sum;
}


/**************************************************
 * Created  : 2003/08/28	
 * Function : coordinateの更新
 * Memo     : 
 **************************************************/
void MDS::UpdateCoordinate()
{
  int num, dim;
  int debug=0;

  num = GetNumOfData();
  dim = GetDimension();
  
  for(int i=0; i<num; i++){
    for(int j=0; j<dim; j++){
      UpdateCoordinateEach(i,j);
      if(debug&&Gdebug) cerr << "[MDS.UpdateCoordinate] x[" << i << "][" << j << "]:" << coordinate[i][j] << endl;
    }
  }
  
  CalcDistance();
  if(debug&&Gdebug) cerr << "[MDS.UpdateCoordinate] Finish!" << endl;
}

/**************************************************
 * Created  : 2003/08/28	
 * Function : coordinateの更新(各々)
 * Memo     : 
 **************************************************/
void MDS::UpdateCoordinateEach(int num_index, int dim_index)
{
  double a, b, c, d, sigma;
  double tmp, data_square1, data_square2, diff;
  int num, dim;

  num = GetNumOfData();
  dim = GetDimension();

  a = 0;
  b = 0;
  c = 0;
  d = 0;

  for(int i=0; i<num; i++){
    sigma = 0;
    if(i!=num_index){
      tmp = coordinate[i][dim_index];
      data_square1 = target_distance[num_index][i];
      data_square2 = target_distance[i][num_index];
      for(int k=0; k<dim; k++){
	if(k!=dim_index){
	  diff = coordinate[num_index][k] - coordinate[i][k];
	  sigma = sigma+diff*diff;
	}
      }
      a = a + 1/(4 * data_square1) + 1/(4*data_square2);
      b = b - tmp*(1/data_square1 + 1/data_square2);
      c = c + (1/(2*data_square1) + 1/(2*data_square2)) * (3*tmp*tmp + sigma) - 1;
      d = d - (1/data_square1 + 1/data_square2) * (tmp*tmp*tmp + tmp*sigma) + 2*tmp; 
      sigma = 0;
    }
  }

  coordinate[num_index][dim_index] = _newton(a, b, c, d, normalize);
}


/**************************************************
 * Created  : 2003/08/28	
 * Function : target_distanceの更新
 * Memo     : 
 **************************************************/
void MDS::TargetDistanceUpdate()
{
  double	f_mean, f_sum, tmp;
  vector<double> func;
  int		num, nc;
  int		i, j, k;

  num = GetNumOfData();
  nc  = num*(num-1)/2;

  func.resize(nc);

  for(i=0; i< nc; i++){
    tmp = distance[i_num[i]][j_num[i]];
    func[i] = tmp*tmp;
  }
  
  for(i=1; i<nc; i++){
    f_sum  = func[i];
    f_mean = f_sum;
    j = i-1;
    while(func[j] > f_mean){
      f_sum = f_sum + func[j];
      j = j-1;
      f_mean = f_sum/(i-j);
      if(j==-1){
	break;
      }
    }
    if(j+1 < i){
      for(k=j; k<i; k++){
	func[k+1] = f_mean;
      }
    }
  }
  
  f_sum = 0;
  for(i=0; i<nc; i++){
    func[i] = sqrt(func[i]);
    f_sum = f_sum + func[i];
  }
  
  for(i=0; i<nc; i++){
    target_distance[i_num[i]][j_num[i]] = normalize*func[i]/f_sum;
    target_distance[j_num[i]][i_num[i]] = normalize*func[i]/f_sum;
  }

  func.clear();
}


/**************************************************
 * Created  : 2003/08/28	
 * Modified : 2008-07-16 by inamura : return value become to be int, result will be written on the second argument
 * Function : dis_dataから計算されるcoordinateを返す
 **************************************************/
int MDS::CoordinateFromDistanceData(vector<double> &dis_data, vector<double> &result)
{
  vector<double>	real_distance;
  int			num, dim;
  double		last_val, val, ref;
  int			debug=0;

  num = GetNumOfData();
  dim = GetDimension();
  //ref = 0.00001;	// TODO:Magic Number!   original value before 2006-10-24
  //ref = 0.01;	// TODO:Magic Number! 2008-10-01 deleted
  ref = 0.0001;	// TODO:Magic Number! 2008-10-01 added

  if(debug&&Gdebug) tl_message ("start");
  if(debug&&Gdebug) tl_message ("# of dimension = %d", dim);
  real_distance = InputDataConvertRealDistance(dis_data);
  if(debug&&Gdebug) tl_message ("step 0");
  
  result.resize(dim);
  for(int i=0; i<num; i++)
    {
      if(real_distance[i] == 0)
	{
	  if(debug&&Gdebug) tl_message ("InputaData matched %d", i);
	  for(int j=0; j<dim; j++)
	    result[j] = coordinate[i][j];
	  real_distance.clear();
	  return TL_TRUE;
	}
    }
  if(debug&&Gdebug) tl_message ("step 0.5");
  val = CalcEvalValueOfInputData(real_distance, result);
  if(debug&&Gdebug) tl_message ("step 0.8");
  last_val = val + ref + 1;

  if(debug&&Gdebug) tl_message ("step 1");
  while(last_val - val > ref)
    {
      if(debug&&Gdebug) tl_message ("0: val = %g", val);
      last_val = val;
      UpdateCoordinateFromInputData (real_distance, result);
      if(debug&&Gdebug) tl_message ("1:");
      val = CalcEvalValueOfInputData(real_distance, result);
      if(debug&&Gdebug) tl_message ("2: val = %g", val);
    }
  real_distance.clear();
  
  if(debug&&Gdebug) tl_message ("end : # of dimension = %d", (int)(result.size()));
  return TL_TRUE;
}



/**************************************************
 * Created  : 2003/08/28	
 * Function : KL Divergence (擬距離)のベクトルから，ユークリッド空間上の距離の二乗に変換する
 * Memo     : sqrt を取らないと，ユークリッド空間上の距離にはならない
 **************************************************/
vector<double> MDS::InputDataConvertRealDistance (vector<double> &input)
{
  vector<double> real_distance;
  double ratio, tmp_mean;
  int	num, i, new_nc, k;
  int	debug=0;

  if(debug&&Gdebug) tl_message("start");
  num = GetNumOfData();
  real_distance.resize(num);
  if(debug&&Gdebug) tl_message("step 1 : num = %d", num);
  // 擬距離のチェックと変数初期化
  for(i=0; i<num; i++)
    {
      if(input[i] < 0)
	{
	  tl_message ("Input Data is minus, so replace zero!");
	  real_distance[i] = 0;
	}
      else 
	real_distance[i] = input[i];
      if(debug&&Gdebug) tl_message ("data[%d]: %g", i, real_distance[i]);
    }
  if(debug&&Gdebug) tl_message("step 2");
  new_nc = num*(num-1)/2;
  for(k=0; k<num; k++)
    {
      for(i=0; i<new_nc; i++)
	{
	  if(real_distance[k] < original_data[i_num[i]][j_num[i]])
	    {
	      if(debug&&Gdebug) cerr << "[MDS.Convert] data[" << k << "] < ori_data[" << i_num[i] << "][" << j_num[i] << "]" << endl;
	      break;
	    }
	}
      if(debug&&Gdebug) cerr << "[MDS.Convert] data[" << k << "] is " << i << "dth smallest" << endl;
      
      if(i == new_nc)
	{
	  ratio    = real_distance[k]/original_data[i_num[i-1]][j_num[i-1]];
	  tmp_mean = sqrt(target_distance[i_num[i-1]][j_num[i-1]])*ratio;
	}
      else
	{
	  if(i>0)
	    {
	      ratio = (real_distance[k] - original_data[i_num[i-1]][j_num[i-1]])/
		( original_data[i_num[i]][j_num[i]] - original_data[i_num[i-1]][j_num[i-1]]);
	      tmp_mean = sqrt(target_distance[i_num[i-1]][j_num[i-1]]) +
		(sqrt(target_distance[i_num[i]][j_num[i]]) - sqrt(target_distance[i_num[i-1]][j_num[i-1]]))*ratio;
	    }
	  else 
	    {
	      ratio    = real_distance[k]/original_data[i_num[i]][j_num[i]];
	      tmp_mean = sqrt(target_distance[i_num[i]][j_num[i]])*ratio;
	    }
	}
      real_distance[k] = tmp_mean * tmp_mean;    
    }
  return real_distance;
}


/**************************************************
 * Created  : 2003/08/28	
 * Function : 誤差の計算
 * Memo     : 
 **************************************************/
double MDS::CalcEvalValueOfInputData(vector<double> &real_distance, vector<double> &tmp_cord)
{
  double	sum, tmp;
  vector<double> tmp_distance;
  int		num, j;
  int		debug=0;

  sum = 0;
  tmp = 0;
  num = GetNumOfData();
  
  
  tmp_distance = CalcDistanceFromOneCoordinate(tmp_cord);

  for(j=0; j<num; j++){
    if(tmp_distance[j]==0){
      if(debug&&Gdebug) cerr << "[MDS.CalcEvalOfInputData] tmp_distance[" << j << "] is zero!!" << endl;
    }
    else {
      tmp = real_distance[j] - (tmp_distance[j] * tmp_distance[j]);
#if 1
      sum += tmp*tmp/(real_distance[j]*real_distance[j]);
#endif
#if 0
      double tmpdis;
      tmpdis = sqrt(real_distance[j]);
      sum += tmp*tmp/(tmpdis*tmpdis*tmpdis);
#endif
    }
  }
  
  tmp_distance.clear();
  if(debug&&Gdebug) cerr << "[MDS.CalcEvalOfInputData] E_value:" << sum << endl;

  return sum;
}


/**************************************************
 * Created  : 2003/08/28	
 * Function : 座標を入力してその座標と各HMMの距離を得る
 * Memo     : 
 **************************************************/
vector<double> MDS::CalcDistanceFromOneCoordinate (vector<double> &tmp_cord)
{
  vector<double> tmp_distance;
  int		 num, dim, i, k;
  int		 debug=0;

  num = GetNumOfData();
  dim = GetDimension();

  tmp_distance.resize(num);

  for(i=0; i<num; i++)
    {
      tmp_distance[i] = 0;
      for(k=0; k<dim; k++)
	{
	  tmp_distance[i] += (coordinate[i][k] - tmp_cord[k]) * (coordinate[i][k] - tmp_cord[k]);
	  if(debug&&Gdebug) cerr << "[MDS.CalcDistanceFrom] coordinate[" << k << "]:" << tmp_cord[k]<< endl;
	}
      tmp_distance[i] = sqrt(tmp_distance[i]);
    }
  
  if(debug&&Gdebug)
    for(i=0; i<num; i++)
      cerr << "distance " << i << ":" << tmp_distance[i] << endl;

  return tmp_distance;
}



/**************************************************
 * Created  : 2003/08/28	
 * Function : tmp_cordの計算
 * Memo     : tmp_distから計算される座標をtmp_cordへ
 **************************************************/
void MDS::UpdateCoordinateFromInputData (vector<double> &tmp_dist, vector<double> &tmp_cord)
{
  int	num, dim, i, j, k;
  double a, b, c, d;
  double sigma, diff, tmp;
  int		debug=0;
  num           = GetNumOfData();
  dim           = GetDimension();

  if(Gdebug&&debug) tl_message ("start");
  for(i=0; i<dim; i++)
    {
      a = 0;
      b = 0;
      c = 0;
      d = 0;
      if(Gdebug&&debug) tl_message ("i = %d", i);
      for(j=0; j<num; j++)
	{
	  sigma = 0;
	  tmp = coordinate[j][i];
	  if(Gdebug&&debug) tl_message ("(i,j) = (%d, %d)", i, j);
	  for(k=0; k<dim; k++)
	    {
	      if(k!=i){
		diff = tmp_cord[k] - coordinate[j][k];
		sigma += diff*diff;
	      }
	    }
#if 1
	  a += 1/(tmp_dist[j]*tmp_dist[j]);
	  b += - 4*tmp/(tmp_dist[j]*tmp_dist[j]);
	  c += 6*tmp*tmp/(tmp_dist[j]*tmp_dist[j]) + 2*sigma/(tmp_dist[j]*tmp_dist[j]) - 2/(tmp_dist[j]);
	  d += - 4*tmp*tmp*tmp/(tmp_dist[j]*tmp_dist[j]) - 4*tmp*sigma/(tmp_dist[j]*tmp_dist[j]) + 4*tmp/(tmp_dist[j]);
#endif
#if 0
	  double tmpdis;
	  tmpdis = sqrt(tmp_dist[j]);
	  a += 1/(tmpdis*tmpdis*tmpdis);
	  b += - 4*tmp/(tmpdis*tmpdis*tmpdis);
	  c += 6*tmp*tmp/(tmpdis*tmpdis*tmpdis) + 2*sigma/(tmpdis*tmpdis*tmpdis) - 2/(tmpdis);
	  d += - 4*tmp*tmp*tmp/(tmpdis*tmpdis*tmpdis) - 4*tmp*sigma/(tmpdis*tmpdis*tmpdis) + 4*tmp/(tmpdis);
#endif
	  sigma = 0;
      
	}
      if(Gdebug&&debug) tl_message ("before _newton");
      tmp_cord[i] = _newton(a/4, b/4, c/4, d/4, normalize);
    }
  if(Gdebug&&debug) tl_message ("end");
}


/**************************************************
 * Created  : 2003/09/17
 * Function : 座標から各原始シンボルとの距離の比を計算
 * Memo     :
 **************************************************/
vector<double> MDS::GetRatio(vector<double>& tmp_cord)
{
  int num, nc;
  int i, j;
  vector<double> dis;
  vector<double> ratio;
  vector<double> tmp_dis;
  double tmp_ratio, distance_sum;

  num = original_data.size();
  ratio.resize(num);
  nc  = num*(num-1)/2;
  
  dis = CalcDistanceFromOneCoordinate(tmp_cord);
  for(i=0; i<num; i++)
    {
      // distacne[i]が0となるものがある場合の処理
      if(dis[i] == 0)
	{
	  for(j=0; j<num; j++)
	    ratio[j] = 0;
	  ratio[i] = 1;
	  dis.clear();
	  return ratio;
	}
    }

  tmp_dis.resize(num);
  
  for(i=0; i<num; i++)
    {
      for(j=0; j<nc; j++)
	{
	  if(dis[i] < sqrt(target_distance[i_num[j]][j_num[j]])){
	    break;
	  }
	}
      if(j== nc)
	{
	  tmp_ratio  = dis[i]/sqrt(target_distance[i_num[j-1]][j_num[j-1]]);
	  tmp_dis[i] = original_data[i_num[j-1]][j_num[j-1]] * tmp_ratio;
	}
      else
	{
	  if(j > 0)
	    {
	      tmp_ratio = (dis[i] - sqrt(target_distance[i_num[j-1]][j_num[j-1]]))/
		(sqrt(target_distance[i_num[j]][j_num[j]]) - sqrt(target_distance[i_num[j-1]][j_num[j-1]]));
	      tmp_dis[i] = original_data[i_num[j-1]][j_num[j-1]] + 
		(original_data[i_num[j]][j_num[j]] - original_data[i_num[j-1]][j_num[j-1]])*tmp_ratio;
	    }
	  else
	    {
	      tmp_ratio = dis[i]/sqrt(target_distance[i_num[j]][j_num[j]]);
	      tmp_dis[i] = original_data[i_num[j]][j_num[j]] * tmp_ratio;
	    }
	}
    }

  distance_sum = 0;
  for(i=0; i<num; i++)
    distance_sum += 1/tmp_dis[i];
  for(i=0; i<num; i++)
    ratio[i] = 1/tmp_dis[i] / distance_sum;

  tmp_dis.clear();
  dis.clear();

  return ratio;
}


/**************************************************
 * Created  : 2003/09/29	
 * Function : N番目の対象の座標を返す
 * Memo     : 
 **************************************************/
vector<double> MDS::GetNthCoordinate(int nth)
{
  return coordinate[nth];
}


/**************************************************
 * Created  : 2003/09/30	
 * Function : MDSをファイルアウト
 * Memo     :
 **************************************************/
int MDS::FileOut(const char* fname)
{
  ofstream fout(fname);

  int num;
  num = original_data.size();

  fout << "#num\t" << num << endl;

  fout << "#original_data" << endl;
  for(int i=0; i<num; i++){
    for(int j=0; j<num; j++)
      fout << original_data[i][j] << "\t";
    fout << endl;
  }
  fout << "#target_distance" << endl;
  for(int i=0; i<num; i++){
    for(int j=0; j<num; j++)
      fout << target_distance[i][j] << "\t";
    fout << endl;
  }
  fout << "#i_num\t" << i_num.size() << endl;
  for(int i=0; i<(int)i_num.size(); i++)
    fout << i_num[i] << "\t";
  fout << endl;
  fout << "#j_num\t" << j_num.size() << endl;
  for(int j=0; j<(int)i_num.size(); j++)
    fout << j_num[j] << "\t";
  fout << endl;
  fout << "#normalize\t" << normalize << endl;
  fout << "#dimension\t" << dimension << endl;
  fout << "#coordinate" << endl;
  for(int i=0; i<num; i++){
    for(int j=0; j<dimension; j++)
      fout << coordinate[i][j] << "\t";
    fout << endl;
  }
  fout << "#distance" << endl;
  for(int i=0; i<num; i++){
    for(int j=0; j<num; j++)
      fout << distance[i][j] << "\t";
    fout << endl;
  }

  fout.close();

  return TRUE;
}


/**************************************************
 * Created  : 2003/09/30	
 * Function : ファイルからロードする
 * Memo     :
 **************************************************/
int MDS::Load(const char* fname)
{
  ifstream fin(fname);
  
  int num, tmp_num;
  double tmp_dbl;
  char buf[MAX_STRING], tmpbuf[MAX_STRING];
  
  fin.getline(buf, MAX_STRING);
  if(strncmp(buf, "#num", 4))
    {
      cerr << "[MDS.Load] (ERROR) \"#num\" -> " << buf << endl;
      return FALSE;
    }
  sscanf(buf, "%s\t%d", tmpbuf, &num);

  fin.getline(buf, MAX_STRING);
  if(strncmp(buf, "#original_data", 14))
    {
      cerr << "[MDS.Load] (ERROR) \"#original_data\" -> " << buf << endl;
      return FALSE;
    }
  original_data.resize(num);
  for(int i=0; i<num; i++)
    {
      for(int j=0; j<num; j++)
	{
	  fin >> tmp_dbl;
	  original_data[i].push_back(tmp_dbl);
	}
      fin.getline(buf, MAX_STRING); // 改行コードの読み込み
    }

  fin.getline(buf, MAX_STRING);
  if(strncmp(buf, "#target_distance", 16))
    {
      cerr << "[MDS.Load] (ERROR) \"#target_distance\" -> " << buf << endl;
      return FALSE;
    }
  target_distance.resize(num);
  for(int i=0; i<num; i++)
    {
      for(int j=0; j<num; j++)
	{
	  fin >> tmp_dbl;
	  target_distance[i].push_back(tmp_dbl);
	}
      fin.getline(buf, MAX_STRING);
    }
  
  fin.getline(buf, MAX_STRING);
  if(strncmp(buf, "#i_num", 6))
    {
      cerr << "[MDS.Load] (ERROR) \"#i_num\" -> " << buf << endl;
      return FALSE;
    }
  sscanf(buf, "%s\t%d", tmpbuf, &tmp_num); 
  i_num.resize(tmp_num);
  for(int i=0; i<tmp_num; i++)
    {
      fin >> i_num[i];
    }
  fin.getline(buf, MAX_STRING);
  
  fin.getline(buf, MAX_STRING);
  if(strncmp(buf, "#j_num", 6))
    {
      cerr << "[MDS.Load] (ERROR) \"#j_num\" -> " << buf << endl;
      return FALSE;
    }
  sscanf(buf, "%s\t%d", tmpbuf, &tmp_num); 
  j_num.resize(tmp_num);
  for(int i=0; i<tmp_num; i++)
    {
      fin >> j_num[i];
    }
  fin.getline(buf, MAX_STRING);
  
  fin.getline(buf, MAX_STRING);
  if(strncmp(buf, "#normalize", 10))
    {
      cerr << "[MDS.Load] (ERROR) \"#normalize\" -> " << buf << endl;
      return FALSE;
    }
  sscanf(buf, "%s\t%lf", tmpbuf, &normalize);
 
  fin.getline(buf, MAX_STRING);
  if(strncmp(buf, "#dimension", 10))
    {
      cerr << "[MDS.Load] (ERROR) \"#dimension\" -> " << buf << endl;
      return FALSE;
    }
  sscanf(buf, "%s\t%d", tmpbuf, &dimension);

  fin.getline(buf, MAX_STRING);
  if(strncmp(buf, "#coordinate", 10))
    {
      cerr << "[MDS.Load] (ERROR) \"#coordinate\" -> " << buf << endl;
      return FALSE;
    }
  coordinate.resize(num);
  for(int i=0; i<num; i++)
    {
      for(int j=0; j<dimension; j++)
	{
	  fin >> tmp_dbl;
	  coordinate[i].push_back(tmp_dbl);
	}
      fin.getline(buf, MAX_STRING);
    }

  fin.getline(buf, MAX_STRING);
  if(strncmp(buf, "#distance", 9))
    {
      cerr << "[MDS.Load] (ERROR) \"#distance\" -> " << buf << endl;
      return FALSE;
    }
  distance.resize(num);
  for (int i=0; i<num; i++)
    {
      for(int j=0; j<num; j++)
	{
	  fin >> tmp_dbl;
	  distance[i].push_back(tmp_dbl);
	}
      fin.getline(buf, MAX_STRING);
    }
  fin.close();

  return TRUE;
}


/**************************************************
 * Created  : 2003/09/30	
 * Function : MDSをファイルアウト
 * Memo     :
 **************************************************/
int MDS::FileOut(ofstream& fout)
{
  int num;
  num = original_data.size();

  fout << "#num\t" << num << endl;

  fout << "#original_data" << endl;
  for(int i=0; i<num; i++){
    for(int j=0; j<num; j++)
      fout << original_data[i][j] << "\t";
    fout << endl;
  }
  fout << "#target_distance" << endl;
  for(int i=0; i<num; i++){
    for(int j=0; j<num; j++)
      fout << target_distance[i][j] << "\t";
    fout << endl;
  }
  fout << "#i_num\t" << i_num.size() << endl;
  for(int i=0; i<(int)i_num.size(); i++)
    fout << i_num[i] << "\t";
  fout << endl;
  fout << "#j_num\t" << j_num.size() << endl;
  for(int j=0; j<(int)i_num.size(); j++)
    fout << j_num[j] << "\t";
  fout << endl;
  fout << "#normalize\t" << normalize << endl;
  fout << "#dimension\t" << dimension << endl;
  fout << "#coordinate" << endl;
  for(int i=0; i<num; i++){
    for(int j=0; j<dimension; j++)
      fout << coordinate[i][j] << "\t";
    fout << endl;
  }
  fout << "#distance" << endl;
  for(int i=0; i<num; i++){
    for(int j=0; j<num; j++)
      fout << distance[i][j] << "\t";
    fout << endl;
  }

  return TRUE;
}


/**************************************************
 * Created  : 2003/09/30
 * Function : ファイルからロードする
 * Memo     :
 **************************************************/
int MDS::Load(ifstream &fin)
{
  int		num, tmp_num;
  double	tmp_dbl;
  char		buf[MAX_STRING], tmpbuf[MAX_STRING];
  
  fin.getline(buf, MAX_STRING);
  if(strncmp(buf, "#num", 4))
    {
      cerr << "[MDS.Load] (ERROR) \"#num\" -> " << buf << endl;
      return FALSE;
    }
  sscanf(buf, "%s\t%d", tmpbuf, &num);

  fin.getline(buf, MAX_STRING);
  if(strncmp(buf, "#original_data", 14))
    {
      cerr << "[MDS.Load] (ERROR) \"#original_data\" -> " << buf << endl;
      return FALSE;
    }
  original_data.resize(num);
  for(int i=0; i<num; i++)
    {
      for(int j=0; j<num; j++)
	{
	  fin >> tmp_dbl;
	  original_data[i].push_back(tmp_dbl);
	}
      fin.getline(buf, MAX_STRING); // 改行コードの読み込み
    }

  fin.getline(buf, MAX_STRING);
  if(strncmp(buf, "#target_distance", 16))
    {
      cerr << "[MDS.Load] (ERROR) \"#target_distance\" -> " << buf << endl;
      return FALSE;
    }
  target_distance.resize(num);
  for(int i=0; i<num; i++)
    {
      for(int j=0; j<num; j++)
	{
	  fin >> tmp_dbl;
	  target_distance[i].push_back(tmp_dbl);
	}
      fin.getline(buf, MAX_STRING);
    }
  fin.getline(buf, MAX_STRING);
  if(strncmp(buf, "#i_num", 6)){
    cerr << "[MDS.Load] (ERROR) \"#i_num\" -> " << buf << endl;
    return FALSE;
  }
  sscanf(buf, "%s\t%d", tmpbuf, &tmp_num); 
  i_num.resize(tmp_num);
  for(int i=0; i<tmp_num; i++)
    fin >> i_num[i];
  fin.getline(buf, MAX_STRING);
  
  fin.getline(buf, MAX_STRING);
  if(strncmp(buf, "#j_num", 6))
    {
      cerr << "[MDS.Load] (ERROR) \"#j_num\" -> " << buf << endl;
      return FALSE;
    }
  sscanf(buf, "%s\t%d", tmpbuf, &tmp_num); 
  j_num.resize(tmp_num);
  for(int i=0; i<tmp_num; i++)
    fin >> j_num[i];
  fin.getline(buf, MAX_STRING);
  
  fin.getline(buf, MAX_STRING);
  if(strncmp(buf, "#normalize", 10))
    {
      cerr << "[MDS.Load] (ERROR) \"#normalize\" -> " << buf << endl;
      return FALSE;
    }
  sscanf(buf, "%s\t%lf", tmpbuf, &normalize);
 
  fin.getline(buf, MAX_STRING);
  if(strncmp(buf, "#dimension", 10))
    {
      cerr << "[MDS.Load] (ERROR) \"#dimension\" -> " << buf << endl;
      return FALSE;
    }
  sscanf(buf, "%s\t%d", tmpbuf, &dimension);

  fin.getline(buf, MAX_STRING);
  if(strncmp(buf, "#coordinate", 10))
    {
      cerr << "[MDS.Load] (ERROR) \"#coordinate\" -> " << buf << endl;
      return FALSE;
    }
  coordinate.resize(num);
  for(int i=0; i<num; i++)
    {
      for(int j=0; j<dimension; j++)
	{
	  fin >> tmp_dbl;
	  coordinate[i].push_back(tmp_dbl);
	}
      fin.getline(buf, MAX_STRING);
    }
  fin.getline(buf, MAX_STRING);
  if(strncmp(buf, "#distance", 9))
    {
      cerr << "[MDS.Load] (ERROR) \"#distance\" -> " << buf << endl;
      return FALSE;
    }
  distance.resize(num);
  for(int i=0; i<num; i++)
    {
      for(int j=0; j<num; j++)
	{
	  fin >> tmp_dbl;
	  distance[i].push_back(tmp_dbl);
	}
      fin.getline(buf, MAX_STRING);
    }
  return TRUE;
}


/*
 * added b toyota 2007-05-14
 */
void MDS::GetCoordinate( vector<vector<double> >&  vecCoordinate)
{
  vecCoordinate = coordinate;
}
