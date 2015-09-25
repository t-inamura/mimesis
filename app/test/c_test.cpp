#include <fstream>
#include <iostream>
#include <vector>

using namespace std;

int main (void)
{
  vector<double>   vec1, vec2;
  vector<double>   *p_vec = NULL;
  vector< vector<double> >   mat1, mat2;

  vec1.resize(3);
  vec1.push_back (3.0);
  vec1.push_back (2.0);
  vec1.push_back (1.0);

  cerr << "vec 1 : " << vec1[0] << "," << vec1[1] << "," << vec1[2] << endl;
  vec1[0] = 1.0;
  vec1[1] = 2.0;
  vec1[2] = 3.0;
  cerr << "vec 1 : " << vec1[0] << "," << vec1[1] << "," << vec1[2] << endl;

  vec2 = vec1;
  cerr << "vec 2 : " << vec2[0] << "," << vec2[1] << "," << vec2[2] << endl;

  vec2.resize(3);
  cerr << "vec 2 : " << vec2[0] << "," << vec2[1] << "," << vec2[2] << endl;

  vec1[0] = 0.0;
  vec1[1] = 0.0;
  vec1[2] = 0.0;
  cerr << "vec 1 : " << vec1[0] << "," << vec1[1] << "," << vec1[2] << endl;
  cerr << "vec 2 : " << vec2[0] << "," << vec2[1] << "," << vec2[2] << endl;


  mat1.resize(2);
  mat1[0].resize(2);
  mat1[1].resize(2);
  mat1[0][0] = 1;
  mat1[0][1] = 2;
  mat1[1][0] = 3;
  mat1[1][1] = 4;

  mat2 = mat1;
  cerr << "mat 2 : " << mat2[0][0] << "," << mat2[0][1] << "," << mat2[1][0] << "," << mat2[1][1] << endl;

  exit(0);
}
