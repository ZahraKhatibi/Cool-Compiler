/*
 *  The scanner definition for COOL.
 */

/*
 *  Stuff enclosed in %{ %} in the first section is copied verbatim to the
 *  output, so headers and global definitions are placed here to be visible
 * to the code in the file.  Don't remove anything that was here initially
 */
%{
#include <cool-parse.h>
#include <stringtab.h>
#include <utilities.h>

/* The compiler assumes these identifiers. */
#define yylval cool_yylval
#define yylex  cool_yylex

/* Max size of string constants */
#define MAX_STR_CONST 1025
#define YY_NO_UNPUT   /* keep g++ happy */

extern FILE *fin; /* we read from this file */

/* define YY_INPUT so we read from the FILE fin:
 * This change makes it possible to use this scanner in
 * the Cool compiler.
 */
#undef YY_INPUT
#define YY_INPUT(buf,result,max_size) \
	if ( (result = fread( (char*)buf, sizeof(char), max_size, fin)) < 0) \
		YY_FATAL_ERROR( "read() in flex scanner failed");

char string_buf[MAX_STR_CONST]; /* to assemble string constants */
char *string_buf_ptr;

extern int curr_lineno;
extern int verbose_flag;

extern YYSTYPE cool_yylval;

/*
 *  Add Your own definitions here
 */
bool check_length();
%}


%x STRING
%x COMMENT

/*
 * Define names for regular expressions here.
 */

DARROW          =>
OW              .

%%

 /*
  *  Nested comments
  */


 /*
  *  The multiple-character operators.
  */
"=>"                                { return DARROW; }
"<-"                                { return ASSIGN; }
";"                                 { return ';'; }
":"                                 { return ':'; }
"}"                                 { return '}'; }
"{"                                 { return '{'; }
"("                                 { return '('; }
")"                                 { return ')'; }
"="                                 { return '='; }
"+"                                 { return '+'; }
"-"                                 { return '-'; }
"*"                                 { return '*'; }
"/"                                 { return '/'; }
","                                 { return ','; }
"."                                 { return '.'; }
"<"                                 { return '<'; }
"=<"                                { return LE; }
"@"                                 { return '@'; }
"~"                                 { return '~'; }
[cC][lL][aA][sS][sS]                { return CLASS; }
[iI][fF]                            { return IF; }
[fF][iI]                            { return FI; }
[tT][hH][eE][nN]                    { return THEN; }
[eE][lL][sS][eE]                    { return ELSE; }
[wW][hH][iI][lL][eE]                { return WHILE; }
[lL][oO][oO][pP]                    { return LOOP; }
[pP][oO][oO][lL]                    { return POOL; }
[nN][oO][tT]                        { return NOT; }
[nN][eE][wW]                        { return NEW; }
[iI][nN]                            { return IN; }
[iI][nN][hH][eE][rR][iI][tT][sS]    { return INHERITS; }
[iI][sS][vV][oO][iI][dD]            { return ISVOID; }
[lL][eE][tT]                        { return LET; }
[cC][aA][sS][eE]                    { return CASE; }
[eE][sS][aA][cC]                    { return ESAC; }
[oO][fF]                            { return OF; }
t[rR][uU][eE]                       { cool_yylval.boolean = true; return BOOL_CONST; }
f[aA][lL][sS][eE]                   { cool_yylval.boolean = false; return BOOL_CONST; }
[A-Z][a-zA-Z0-9_]*                  { cool_yylval.symbol = idtable.add_string(yytext); return TYPEID; }
[a-z][a-zA-Z0-9_]*                  { cool_yylval.symbol = idtable.add_string(yytext); return OBJECTID; }
[0-9]+                              { cool_yylval.symbol = inttable.add_string(yytext); return INT_CONST; }
[ \t]*                              {}


 /*
  * Keywords are case-insensitive except for the values true and false,
  * which must begin with a lower-case letter.
  */


 /*
  *  String constants (C syntax)
  *  Escape sequence \c is accepted for all characters c. Except for 
  *  \n \t \b \f, the result is c.
  *
  */
\n    			           { curr_lineno++ ; }

<INITIAL>\"                        { BEGIN(STRING); string_buf_ptr = string_buf ; }
<STRING><<EOF>>                    { BEGIN(INITIAL) ; cool_yylval.error_msg ="EOF in string constant" ; return ERROR; }
<STRING>\"                         {
                                        if(!check_length()) 
                                        {
                                            cool_yylval.error_msg ="String constant too long" ; 
                                            return ERROR; 
                                        }

                                       *string_buf_ptr = '\0';
                                        cool_yylval.symbol = stringtable.add_string(string_buf);
					BEGIN(INITIAL); 
                                        return STR_CONST; 
   				    }
<STRING>\n                          { curr_lineno++ ; BEGIN(INITIAL); cool_yylval.error_msg = "Unterminated string constant" ; return ERROR;  }
<STRING>\\0                         { cool_yylval.error_msg = "String contains null character" ; return ERROR; }
<STRING>\\[t]                         {
                                        if(!check_length())
                                        {
                                            cool_yylval.error_msg ="String constant too long" ; 
                                            return ERROR; 
                                        }  
                                       *string_buf_ptr++ = '\t' ;
                                                  
                                    }

<STRING>\\[b]                         {
                                        if(!check_length())
                                        {
                                            cool_yylval.error_msg ="String constant too long" ; 
                                            return ERROR; 
                                        }  
                                       *string_buf_ptr++ = '\b' ;
                                                                       
                                    }

<STRING>\\[f]                        {
                                        if(!check_length())
                                        {
                                            cool_yylval.error_msg ="String constant too long" ; 
                                            return ERROR; 
                                        }  
                                       *string_buf_ptr++ = '\f' ;
                                                                        
                                    }
<STRING>\\[n]                        { 
                                        if(!check_length())
                                        {
                                            cool_yylval.error_msg ="String constant too long" ; 
                                            return ERROR; 
                                        }  
                                       *string_buf_ptr++ = '\n' ;                           
                                    }
<STRING>.                           {
                                        if(!check_length())
                                        {
                                            cool_yylval.error_msg ="String constant too long" ; `
                                            return ERROR; 
                                        }  
                                       *string_buf_ptr++ = *yytext;

                                    }

 /*
  **************************************************************************
  */
"(*" 					{BEGIN(COMMENT);}
"*)" 					{ cool_yylval.error_msg = "Unmatched *)"; return ERROR; }
"--".*                          	;
<COMMENT>"*)" 				{BEGIN(INITIAL);}
<COMMENT><<EOF>>                        { BEGIN(INITIAL); cool_yylval.error_msg = "EOF in comment"; return ERROR; }
<COMMENT>\n 				{ curr_lineno++; }
<COMMENT>. 				{}
{OW}				    { cool_yylval.error_msg = strdup(yytext); return ERROR; }

%%



bool check_length()
{
    return (string_buf_ptr - string_buf + 1 < MAX_STR_CONST );
}

