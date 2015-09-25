/*
 * Behavior.h
 * Behavior Structure
 * Last Modified : Tetsunari Inamura on 2008-10-07
 */

#ifndef __BEHAVIOR_H__
#define __BEHAVIOR_H__

#include <fstream>
#include <iostream>
#include <vector>
#include <stdio.h>
#include <math.h>
#include <string>
#include "mimesis.h"

using namespace std;

typedef enum {
  JOINT_ANGLE,
  JOINT_VELOSITY,
  JOINT_TORQUE,
  SOUND,
  TOUCH_SENSOR,
  VISION_SENSOR
} sensorimotor_type;



class Pose
{
  vector<double> angle;
  int		sampling_time;

 public:
  Pose();
  Pose(const Pose& another);
  ~Pose();

  void			Reset		(void);
  int			Load		(ifstream &fin, int ref_dof);
  int			Load		(char *buf);
  int			CopyTo		(Pose &another) const;
  int			Dof		(void) const;
  int			SamplingTime	(void) const;
  double		NthAngle	(int n);
  int			NthAngle	(int n, double val);
  void			Show		(void);
  int			FileOut		(ofstream &fout, int tstep);
  void			AddAngle	(double new_angle);
  int			RemoveNthAngle	(int i);
  int			Verify		(void);
};


class Behavior
{
  friend class	Pose;

  int		dof;
  int		sampling_time;
  string	label;
  vector<sensorimotor_type>	type_tag;
  vector<Pose *> pose_t;

 public:
  Behavior		();
  Behavior		(int ref_dof, int time, char *name);
  Behavior		(const Behavior &another);
  Behavior		(const Behavior &another, int target_length);
  Behavior		(int ref_dof);
  ~Behavior();

  int			Reset		(void);
  int			Load		(const char *fname);	// Load from .beh file
  int			Load		(const char *fname, int debug);	// Load from .beh file
  int			LoadBVH		(const char *fname);	// Load from .bvh file
  int			LoadBVH		(const char *fname, const char *label_str);
  int			Length		(void); 		// Length of time-series data (# of pose)
  int			Dof		(const int value);
  int			Dof		(void);    		// Degree of freedom
  int			SamplingTime	(void);			// Get sampling time
  int			SamplingTime	(const int val);	// Set sampling time
  string		&Label		(void);
  int			Label		(const char *name);
  int			Label		(string &str);
  //char		*Label(); 				// Label
  Pose			*NthPose	(int nth);		// return n-th pose_t
  void			Show		(void);			// display contents of the instance
  void			Phase_Shift	(int start);		// 
  int			FileOut		(const char* fname);	// Output to .beh file
  vector<Pose *>	GetPoses	(void);			// return vector of pointer for post_t
  void			AddPose		(Pose *new_pose);	// Add pose
  void			Cat		(Behavior &add_beh);	// Concatenate
  int			HTKFileOut	(const char *filename);
  int			MakeLabelFile	(const char *filename);
  Pose			*PopFrontPose	(void);			// 
  Behavior		*ElasticCopy	(int target_length);	// Simple version of the below ElasticCopy
  int			ElasticCopy	(Behavior &src_beh, int target_length); // it might be not used
  int			AddNoise	(void);
  int			ReplacePoses	(vector<Pose *> *pose_t_new);
  Behavior		*Copy		(void);
  int			RemoveNthAngle	(int i);
  int			RemoveNthFrame	(int i);
  sensorimotor_type	NthType		(int i);
  int			TypeTags	(vector <sensorimotor_type> tags);
  vector<sensorimotor_type> TypeTags	(void);
  int			Verify		(void);
};



#endif /* __BEHAVIOR_H__ */
