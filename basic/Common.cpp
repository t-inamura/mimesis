#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <fstream>
#include <iostream>
#include <string.h>
#define __COMMON_CPP__
#include "mimesis.h"


// " ./ " から始まっている場合は $PWD を補間する．
// "/home/xxxxx" などのように絶対パスになっていればそのまま．
// "../" から始まっていれば，PWD からの相対パスとして求める．
// 上記のいずれでもなければ，曖昧なパスであると判断して，環境変数の MIMESIS_TOP_DIR を補完する
// memo : 2005 May 23 : kojo君の環境で getenv("PWD")が c:/cygwin/home/...となるので，対応策を練る
// -------------------------------------------------------------------------------
// 入力 : filename	: 対象となるファイル名
// 出力 : &result	: パスを含んだファイル名
int complement_dirname(const char *filename, std::string &result)
{
  int  debug=0;
  if (filename[0]=='/')
    result = filename;
  else if (filename[0]=='.' && filename[1]=='/')
    {
      result = getenv("PWD");
      if (tl_strmember (result.c_str(), ":/cygwin"))
	result = getenv("PWD")+strlen("c:/cygwin");
      result += "/";
      result += filename+2;      // add string with removing "./"
    }
  else if (!strncmp (filename, "../", 3))
    {
      if(debug) tl_message ("input  : %s", filename);
      result = getenv("PWD");
      if (tl_strmember (result.c_str(), ":/cygwin"))
	result = getenv("PWD")+strlen("c:/cygwin");
      result += "/";
      result += filename;
      if(debug) tl_message ("output : %s", result.c_str());
    }
  else if (filename[0]=='.' && filename[1]=='\0')
    {
      result = getenv("PWD");
      if (tl_strmember (result.c_str(), ":/cygwin"))
	result = getenv("PWD")+strlen("c:/cygwin");
      result += "/";
    }
  else
    {
      if(debug) tl_message ("input  : %s", filename);
      result += getenv("MIMESIS_TOP_DIR");
      result += "/";
      result += filename;
      if(debug) tl_message ("output : %s", result.c_str());
    }
  return TRUE;
}



/*-----------------------------------------------------------------------------------*/
// 動作 : デバッグ用プリント
/*-----------------------------------------------------------------------------------*/
int d_print (char const *format, ...)
{
  char		s[MAX_STRING];
  va_list	list;

  va_start(list, format);
  vsprintf(s, format, list);
  va_end(list);
  fprintf (stderr, "%s", s);
  fflush (stderr);
  return TL_TRUE;
}



/*-----------------------------------------------------------------------------------*/
// 動作 : デバッグ用プリント その２
//        第一引数の flag が ON の時だけ表示する */
/*-----------------------------------------------------------------------------------*/
int d_printf (int flag, const char *format, ...)
{
  char		s[MAX_STRING];
  va_list		list;

  if (flag) {
    va_start(list, format);
    vsprintf(s, format, list);
    va_end(list);
    fprintf( stderr, "%s", s );
  }
  return TL_TRUE;
}



/*-----------------------------------------------------------------------------------*/
// 新設 : 1999 May 11th
// 変更 : 1999 Jul 27th : リスト表現はひとまとまりとして読み込むように
// 動作 : *src の文字列の中に，:hoge という型の *keyword があった場合には
//	  *result にその次の word をコピーして，1 を返す
//	  存在しなければ，result は NULL で 0 を返す
// 変更 : 1999 Jul 19th : result が ()で囲まれていたら，全部一度に読む
// ----------------------------------------------------------------------------------
// 入力 : *src		: 検索する対象の文字列
// 入力 : *keyword	: 検索するキー文字列
// 出力 : *result	: 結果を格納する配列
/*-----------------------------------------------------------------------------------*/
int tl_string_getkeyword (char *src, char *keyword, char *result)
{
  char		*charp=NULL, tmp[MAX_STRING];
  int		i, j, debug=0;

  charp = src;

  if (!tl_strmember(src, keyword))
    {
      tl_warning ("No such keyword {%s} in {%s}", keyword, src );
      result = NULL;
      return TL_FALSE;
    }
  for(;;) {
    sscanf_word_with_pointer_move( &charp, tmp );
    if( !strcmp( tmp, keyword ) ) {
      // 一致した時
      tl_skip_pointer( &charp );
      if( *charp == '(' ) {
	// Result の最初が '(' だった場合 ')' が来るまで読んでいっぺんに返答
	for( i=0; ;i++ ) {
	  if( *(charp+i)==')'  ) break;
	  if( *(charp+i)=='\0' ) { i--; break; }
	}
	for( j=0; j<=i; j++ ) result[j] = *(charp+j);
	result[j] = '\0';
	//strncpy( result, charp, i+1 );// これだと'\0'なし文字列に対応できない
	d_printf( debug, "[tetsulib] getkeyword with '()' : i=%d : result=%s\n",
		  i, result );
	return TL_TRUE;
      }
      else {
	// () で囲まれていない，普通の 1 word の場合
	if( sscanf_word_with_pointer_move( &charp, tmp )==TL_TRUE ) {
	  // 普通の Result だった場合, 最後の閉じカッコを削除して，返答する
	  strcpy( result, tmp );
	  remove_blacket_end( result );
	  return TL_TRUE;
	}
	else {
	  // 一致したが，Result が存在していない時
	  strcpy(result, "\0");
	  return TL_FAIL;
	}
      }
    }
    else {
      // Keyword 一致していない時,もっと探すぞ
      if( *charp=='\0' ) {
	strcpy(result, "\0");
	return TL_FAIL;
      }
    }
  }
}




/*-----------------------------------------------------------------------------------*/
// 新設 : 1999 Jan 21st
// 変更 : 1999 Aug 2nd	: reference の方が短い場合に対応させる
// 入力 : reference		: 探索被対象の文字列
// 入力 : search		: 探索する文字列
// 動作 : reference の中に search があるかどうか，チェックする
// 返値 : あった場所を返す reference の何文字目がヒットしたか
//        (1から始まる自然数)   なければ0
/*-----------------------------------------------------------------------------------*/
int tl_strmember (const char *reference, const char *search)
{
  int		i;

  if (reference==NULL || search==NULL)
    return 0;
  // 探される文字列 reference の方が短い場合は即刻 return
  if( strlen(reference) < strlen(search) )
    return 0;
  for (i=0; i<(int)strlen(reference)-(int)strlen(search)+1; i++)
    {
      if( !strncmp( reference+i, search, strlen(search) ) )
	return i+1;
    }
  return 0;
}




/*-----------------------------------------------------------------------------------*/
// 変更 : 2001 Nov 22nd : カンマ ',' 区切りにも対応できるようにする
// 新設 : 1999 May 11th 
// 動作 : sscanf を，fscanf と同じように，ポインタを動かしながら行なうツール
//	  int, double 版に引続き word 版を作る
// 入力 : **charp	: 読み込むべき文字列の先頭アドレスへのポインタ
// 出力 : *word		: 読み取った word
// 返値 : (int)		: Success or Fail
/*-----------------------------------------------------------------------------------*/
int sscanf_word_with_pointer_move (char **charp, char *word)
{
  if( **charp=='\0' || **charp=='\n' )
    return TL_FAIL;
  // 文字と文字のすき間をスキップするように，文字列のポインタを動かす
  for(;;) {
    if( **charp==' ' || **charp=='\t' || **charp==',') {
      (*charp)++;
      if( **charp=='\0' || **charp=='\n' )
	return TL_FAIL;
      // 何もしないで最後まで行ってしまったら，Fail 終り
    }
    else break;
  }
  // fgets された文字列から word を読み取る
  sscanf( *charp, "%s", word );
  // 読み込んだ word の長さ分だけ，ポインタを移動する
  (*charp) += strlen( word );
  return TL_TRUE;
}



/*-----------------------------------------------------------------------------------*/
// 変更 : 2001 Nov 22nd : カンマ ',' 区切りにも対応できるようにする
// 新設 : 1999 Jul 19th
// 動作 : 文字列ポインタに対して，空白,TABをskipして何か文字がある所まで移動する
// 入力 : 文字列ポインタのポインタ
/*-----------------------------------------------------------------------------------*/
int tl_skip_pointer (char **charp)
{
  if( **charp=='\0' ) return TL_FAIL;
  // 文字と文字のすき間をスキップするように，文字列のポインタを動かす
  for(;;) {
    if( **charp==' ' || **charp=='\t' || **charp==',')
      {
	(*charp)++;
	if( **charp=='\0' ) return TL_FAIL;
	// 何もしないで最後まで行ってしまったら，Fail 終り
      }
    else return TL_TRUE;
  }
}



/*-----------------------------------------------------------------------------------*/
// 新設 : 1999 Jan 21st
// 動作 : 文字列の最後にある ')' を一つ削る
// 返値 : 最後に ')' があれば，処理をして，１を返し，なければ０を返す
/*-----------------------------------------------------------------------------------*/
int remove_blacket_end( char *string )
{
  if( string[ strlen(string)-1 ] == ')' ) {
    string[ strlen(string)-1 ] = '\0';
    return TL_TRUE;
  }
  return TL_FAIL;
}



/*-----------------------------------------------------------------------------------*/
// 新設 : 1999 Sep 15th
// 入力 : string	: 整数が数字だけで構成されているはずの文字列
// 出力 : value		: 変換された値
// 返値 : Success or Fail
/*-----------------------------------------------------------------------------------*/
int tl_atoi (char *string, int *value)
{
  if (integerp(string)==TL_FALSE)
    {
      my_warning( "tetsulib", "tetsulib_atoi" );
      d_print( "input string is not integer! : %s\n", string );
      return TL_FAIL;
    }
  *value = atoi(string);
  return TL_TRUE;
}



/*-----------------------------------------------------------------------------------*/
// 新設 : 1999 Jul 28th
// 動作 : !WARNING! とたくさん表示して，内容を書く
/*-----------------------------------------------------------------------------------*/
void my_warning (char *process, char *function)
{
  int		i;

  for( i=0; i<5; i++ )
    d_print( "[%s] <%s> !!WARNING!!\n", process, function );
}


/*-----------------------------------------------------------------------------------*/
// 新設 : 1999 Jun 16th
// 動作 : 文字列で表されているものが，数字かどうか判断する
// 入力 : *string 	: 対象文字列
// 返値 : Success or Fail
/*-----------------------------------------------------------------------------------*/
int integerp (char *string)
{
  int		i;

  for (i=0; i<(int)strlen(string); i++)
    {
      if (string[i]<'0' || string[i]>'9')
	if (string[i]!='-')
	  return TL_FALSE;
    }
  return TL_TRUE;
}



/*-----------------------------------------------------------------------------------*/
// 変更 : 2001 Nov 22nd : カンマ ',' 区切りにも対応できるようにする
// 新設 : 1999 Apr 14th
// 動作 : sscanf を，fscanf と同じように，ポインタを動かしながら行なうツール
//        int 版に続き, double 版を作る
// 入力 : **charp	: 読み込むべき文字列の先頭アドレスへのポインタ
// 出力 : *data		: 読み取った値
// 返値 : int		: 成功か否か
/*-----------------------------------------------------------------------------------*/
int sscanf_double_with_pointer_move (char **charp, double *data)
{
  float		value;
  int		debug=0;

  if(debug) tl_message ("trying ... %s", *charp);
  if( **charp=='\n' || **charp=='\r' || **charp=='\0' )
    {
      if(debug) tl_warning ("End of line. You had better using 'with_seeking'");
      return FALSE;
    }
  // 数字と数字のすき間をスキップするように，文字列のポインタを動かす
  for(;;) {
    if (**charp==' ' || **charp=='\t' || **charp==',') (*charp)++;
    else break;
  }
  if( **charp=='\n' || **charp=='\r' || **charp=='\0' )
    {
      if(debug) tl_warning ("End of line. You had better using 'with_seeking'");
      return FALSE;
    }
  // fgets された文字列から小数点の値を読み取る
  sscanf( *charp, "%f", &value );
  // 数字，符号，小数点以外になるまでポインタを移動する
  for(;;)
    {
      if( (**charp>='0' && **charp<='9') || **charp=='.' || **charp=='-')
	(*charp)++;
      else break;
    }
  *data = (double)value; 
  return TRUE;
}


int Common_SetDebug (int flag)
{
  Gdebug = flag;
  tl_message ("Set Gdebug to %d", flag);
  return flag;
}



//
// Created on 2008-09-27 by inamura
//
int mimesis_init_typetaglist (void)
{
  int debug=1;
  //TypeTagList = g_list_alloc();
  for (int i=0; i<NUM_OF_TYPETAG; i++)
    {
      TypeTagList = g_list_append (TypeTagList, TypeTagString[i]);
      if(debug) tl_message ("%d-th data : %s -> %s", i, TypeTagString[i], (char *)g_list_nth_data (TypeTagList, i));
    }
  return 1;
}


