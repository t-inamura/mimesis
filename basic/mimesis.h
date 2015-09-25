/*
 *
 * Modified : Tetsunari Inamura on 2004 May 25th
 *
 */

#ifndef __COMMON_HEADER__
#define __COMMON_HEADER__

#include <string>
#include <glib.h>

#ifndef uchar
#define uchar	unsigned char
#endif


/* #define HTKDIR	"/usr/local/bin/" */
/* #ifndef TRUE */
/* #define TRUE		(1) */
/* #endif */
#ifndef FALSE
#define FALSE		(0)
#endif


//define : .htk関連
#define H_USER      		(9)
#define DUMP_ERR		(0)
#define HASZMEAN		(0x0800)

#define	HTK_DICT_FILE_FOR_ALL_MOTION	"all_motions.dict"

typedef struct
{
  int	nSamples;
  int	sampPeriod;
  short sampSize;
  short parmKind;
}
htk_header_t;



//define
#define DEFAULT_BEH_LABEL 	"unknown"
#define DEFAULT_BEH_DOF		(20)
#define DEFAULT_BEH_TIME 	(33)

#define DEFAULT_NUMOFSTATE	(10)
#define DEFAULT_NUMOFMIX	(3)
#define DEFAULT_HMMTYPE		LEFT_TO_RIGHT

#define DEFAULT_RECOG_WORK_DIR ".tmp_rec/"

//HMMから生成するときに表示をするかどうか
#define HYOUZI 0

#define DEFAULT_WSPACE_SPAN	(20)
#define DEFAULT_WSPACE_STEP	(5)





/*---------------------------------------------------------------------------*/
// 新設 : 2002 Jan 30th
// 動作 : glib のウォーニングにならう
/*---------------------------------------------------------------------------*/
#define	tl_warning(format, args...)	G_STMT_START{         \
   fprintf(stderr, "\n\n[%s:line.%d] <%s> ", __FILE__, __LINE__, __PRETTY_FUNCTION__);\
   g_log (G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, format, ##args);\
   fprintf(stderr, "\n\n");}G_STMT_END

#define	tl_message(format, args...)	G_STMT_START{         \
   fprintf(stderr, "[%s:line.%d] <%s> : ", __FILE__, __LINE__, __PRETTY_FUNCTION__);\
   g_log (G_LOG_DOMAIN, G_LOG_LEVEL_MESSAGE, format, ##args);}G_STMT_END



/*---------------------------------------------------------------------------*/
// 新設 : 2002 Jan 30th
// 動作 : glib のウォーニングにならう
/*---------------------------------------------------------------------------*/
#define tl_return_val_if_fail(expr,mes,val)  G_STMT_START{		\
     if (!(expr))							\
       {								\
	 tl_warning(mes);						\
	 return val;							\
       };				}G_STMT_END


/*---------------------------------------------------------------------------*/
// 新設 : 2002 Feb 17th
// 動作 : 良く使うデバッグ文のまとめ
/*---------------------------------------------------------------------------*/
#define tl_debug_step(mes)  G_STMT_START{				\
  fprintf(stderr, "[%s:line.%d] <%s> Step %s\n", __FILE__, __LINE__,	\
	   __PRETTY_FUNCTION__, mes);    }G_STMT_END



#define TL_TRUE			(1)
#define TL_SUCCESS		(1)
#define TL_FALSE		(-1)
#define TL_FAIL			(-1)
#define	MAX_STRING		(20000)



#define NUM_OF_TYPETAG		(6)

#ifdef __COMMON_CPP__
int Gdebug = FALSE;
GList *TypeTagList = NULL;	// added on 2008-09-27
char TypeTagString[NUM_OF_TYPETAG][255] =
  {
    "JOINT_ANGLE",
    "JOINT_VELOSITY",
    "JOINT_TORQUE",
    "SOUND",
    "TOUCH_SENSOR",
    "VISION_SENSOR"
  };
#else
extern int Gdebug;
extern GList *TypeTagList;
extern char TypeTagString[NUM_OF_TYPETAG][255];
#endif


int	complement_dirname		(const char *filename, std::string &result);
int	d_print				(char const *format, ...);
int	d_printf			(int flag, const char *format, ...);
int	tl_string_getkeyword		(char *src, char *keyword, char *result);
int	tl_strmember			(const char *reference, const char *search);
int	sscanf_word_with_pointer_move	(char **charp, char *word);
int	sscanf_double_with_pointer_move (char **charp, double *data);
int	tl_skip_pointer			(char **charp);
int	remove_blacket_end		(char *string);
int	tl_atoi				(char *string, int *value);
void	my_warning			(char *process, char *function);
int	integerp			(char *string);

extern "C"
{
  int	Common_SetDebug			(int flag);
  int	mimesis_init_typetaglist	(void);
}

#endif /* __COMMON_HEADER__ */
