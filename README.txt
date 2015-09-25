******************************************************
0. How to execute sample programs  (app/test)
******************************************************

* Learning of patterns

$ ./learning [--debug] --rc ../../script/learning_scriptfile_3 

Target patterns should be described in the script file named learning_scriptfile_3 
Result will be output in a directory named .tmp/[name of pattern]/
Use --debug option to show detail debug messages


* Generation of pattern from HMM

$ ./generation

Result will be output as a file named generated.beh


* Creation of proto-symbol space

$ ./space_create

Result (structure of the proto-symbol space) will be output as a file
named   ../../script/symbol_data 


* Recognition of patterns

$ ./recognition

Result will be output as a file named result.dat
This file describes the location of recognized proto-symbol in the phase space.




******************************************************
1. Directory & file configuration
******************************************************

mimesis/
  |
  |
  |----	basic/     libraries
  |	  |
  |  	  + Behavior		for each pattern
  |	  + JHMM		for Hidden Markov Model
  |	  + Mds			Multi-dimensional scaling method
  |	  + MotionDB		set of Behavior instances
  |	  + HTKUnit		interface for HTK
  |	  + WorkSpace.cpp	Total Manager
  |
  |----	app/	sample programs
  |       | 
  |	  + Makefile, README, 
  |	  |----	test/
  |	         + learning.cpp	sample for HMM learning
  |	         + generation.cpp	sample for generation using proto-symbol space
  |	         + space_create.cpp	sample for configuration of proto-symbol space
  |	         + recognition.cpp	sample for recognition
  |	         + imitation.cpp	sample for imitate patterns
  |	         + synthesis.cpp	sample for synthesis of patterns using interpolation
  |	         + extra_test.cpp	test program for extrapolation based synthesis
  |
  |
  |----	lib/
  |	  | Makefile,(basic.a,libHTK.a)
  |
  |----	mdb/	sample motion patterns
  |	  |----	ashibum/,banzai/,dance/,kick/.....
  |
  |----	script/	script files for test programs
  |	  | learning_script_{1,2,3}   : Learning script file
  |	  | mdb_[name of motion]      : Motion pattern
  |
  |----	ext/	Installer of HTK
	  |
          |--- htk-3.3/  htk version 3.3 



******************************************************
2. mimesis/basic/ 
******************************************************

Behavior
  Class for Sensorimotor pattern 
  - read and write files (.beh)
  - creation of .htk and .lab

MotionDB
  Class for set of Behavior instances

JHMM
  Class for HMM (HTK based) 

HTKUnit
  Interface for HTK
  management of .net, .lattice, .dict, .hlist and so on

Mds
  Multi-dimensional scaling method

