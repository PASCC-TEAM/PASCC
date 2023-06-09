%option case-insensitive
%{   
#include "parser.h"
#include "log.h"
#include "parser.tab.h"
    
int line_count = 1;             // 当前行 号
int char_count = 1;             // 当前列 号
int left_brackets_num = 0;      // 当前未被匹配的左小括号数
int left_Mbrackets_num = 0;     // 当前未被匹配的左中括号数
bool new_line_flag;             // 标志换行符的读入 
int i;
int last_line_count;            // 当前行的前一个非空行 号
std::string cur_line_info;      // 当前行已读入信息
std::string last_line_info;     // 当前行的前一个非空行 信息
int lex_error_flag = 0;         // 标志词法分析过程中是否出现错误

void wrap();
void process();
void process_anno();
void cache_line();
void print_syntax_error(const char *msg);
void print_lex_error(const char *msg);

void to_str();
void to_str_case_sensitive();
void to_int();
void to_float();
void to_char();
void to_bool();

extern "C"
{
    int yywrap();
    int yylex();
    void yyerror(const char *s);
}

%}

delim                   [ ]
ws                      {delim}+
letter                  [a-zA-Z]
digit                   [0-9]
digits                  {digit}+
fraction                (\.{digits})

/* 关键字 */
PROGRAM                 "program"
CONST                   "const"
TYPE                    "type"
RECORD                  "record"
ARRAY                   "array"
OF                      "of"
VAR                     "var"
FUNCTION                "function"
PROCEDURE               "procedure"
BEGIN_                  "begin"
END                     "end"
IF                      "if"
THEN                    "then"
ELSE                    "else"
CASE                    "case"
WHILE                   "while"
REPEAT                  "repeat"
UNTIL                   "until"
FOR                     "for"
TO                      "to"
DOWNTO                  "downto"
DO                      "do"
READ                    "read"
READLN                  "readln"
WRITE                   "write"
WRITELN                 "writeln"
NOT                     "not"
BASIC_TYPE              ("integer"|"real"|"Boolean"|"char")
TRUE                    "true"
FALSE                   "false"
PLUS                    "+"
UMINUS                  "-"

/* 常数 */
INT_NUM                 {digits}
REAL_NUM                {digits}{fraction}

/* 注释 */
ANNO_1                  (\{[^}]*\})      
ANNO_2                  ("//"[^\n]*)

/* 标识符 */
ID                      ({letter}|[_])({letter}|{digit}|[_])*

/* 字符 */
CHAR                    ('[^'\n]')
/*字符串*/
STRING_                 (\'([^\\\'\n]|(\\.)){2,}\')


/* 运算符 */
CONSTASSIGNOP           "="
RELOP                   ("<>"|"<"|"<="|">"|">=")      
ADDOP                   "or"
MULOP                   ("*"|"/"|"div"|"mod"|"and")
ASSIGNOP                ":="

/* 分隔符 */
SEP                     (";"|"."|","|":")
SUBCATALOG              ".."
  
%% 
"\t"                    { cache_line(); char_count += 4; }
"\n"                    { wrap(); new_line_flag = true;}
{ws}                    { cache_line(); char_count += yyleng; }

{PROGRAM}               { cache_line(); to_str(); return PROGRAM; }
{CONST}                 { cache_line(); to_str(); return CONST; }
{TYPE}                  { cache_line(); to_str(); return TYPE; }

{ARRAY}                 { cache_line(); to_str(); return ARRAY; }
{RECORD}                { cache_line(); to_str(); return RECORD;}
{OF}                    { cache_line(); to_str(); return OF; }
{VAR}                   { cache_line(); to_str(); return VAR; }
{FUNCTION}              { cache_line(); to_str(); return FUNCTION; }
{PROCEDURE}             { cache_line(); to_str(); return PROCEDURE; }
{BEGIN_}                { cache_line(); to_str(); return BEGIN_; }
{END}                   { cache_line(); to_str(); return END; }
{IF}                    { cache_line(); to_str(); return IF; }
{THEN}                  { cache_line(); to_str(); return THEN; }
{ELSE}                  { cache_line(); to_str(); return ELSE; }
{CASE}                  { cache_line(); to_str(); return CASE; }
{WHILE}                 { cache_line(); to_str(); return WHILE; }
{REPEAT}                { cache_line(); to_str(); return REPEAT; }
{UNTIL}                 { cache_line(); to_str(); return UNTIL; }
{FOR}                   { cache_line(); to_str(); return FOR; }
{TO}                    { cache_line(); to_str(); return TO; }
{DOWNTO}                { cache_line(); to_str(); return DOWNTO; }
{DO}                    { cache_line(); to_str(); return DO; }
{WRITE}                 { cache_line(); to_str(); return WRITE;}
{WRITELN}               { cache_line(); to_str(); return WRITELN;}
{READ}                  { cache_line(); to_str(); return READ;}
{READLN}                { cache_line(); to_str(); return READLN;}

{NOT}                   { cache_line(); to_str(); return NOT; }
{BASIC_TYPE}            { cache_line(); to_str(); return BASIC_TYPE; }
{TRUE}                  { cache_line(); to_bool(); return TRUE; }
{FALSE}                 { cache_line(); to_bool(); return FALSE; }
{PLUS}                  { cache_line(); to_str(); return PLUS; }
{UMINUS}                { cache_line(); to_str(); return UMINUS; }

{INT_NUM}               { cache_line(); to_int(); return INT_NUM; }
{REAL_NUM}              { cache_line(); to_float(); return REAL_NUM; }

{ANNO_1}                { process_anno(); }
{ANNO_2}                { process_anno(); }

{CHAR}                  { cache_line(); to_char(); return CHAR; }
{STRING_}               { cache_line(); to_str_case_sensitive(); return STRING_; }

{RELOP}                 { cache_line(); to_str(); return RELOP; }
{ADDOP}                 { cache_line(); to_str(); return ADDOP; }
{MULOP}                 { cache_line(); to_str(); return MULOP; }
{ASSIGNOP}              { cache_line(); to_str(); return ASSIGNOP; }

{ID}                    { cache_line(); to_str(); return ID; }

{CONSTASSIGNOP}         { cache_line(); to_str(); return yytext[0]; }
{SEP}                   { cache_line(); to_str(); return yytext[0]; }
{SUBCATALOG}            { cache_line(); to_str(); return SUBCATALOG; }

("[")                   { cache_line(); to_str(); left_Mbrackets_num++;return yytext[0];}    
("(")                   { cache_line(); to_str(); left_brackets_num++;return yytext[0];}
("]")                   { 
                            cache_line(); to_str(); 
                            left_Mbrackets_num--;
                            if(left_Mbrackets_num<0){
                                left_Mbrackets_num=0;
                                print_syntax_error("expected '['");
                            }
                            return yytext[0];
                        }
(")")                   { 
                            cache_line(); to_str(); 
                            left_brackets_num--;
                            if(left_brackets_num<0){
                                left_brackets_num=0;
                                print_syntax_error("expected '('");
                            }
                            return yytext[0];
                        }

('')                        { cache_line(); print_lex_error("illegal character constant"); }
(''')                       { cache_line(); print_lex_error("illegal character constant"); }
('\n')                      { cache_line(); print_lex_error("illegal character constant"); process(); }

('[^'\n][^']*(\n)[^']*')    { cache_line(); print_lex_error("illegal string constant"); process(); }
('[^']*(\n)[^']*[^'\n]')    { cache_line(); print_lex_error("illegal string constant"); process(); }
              
(\{[^}]*)                   { cache_line(); print_lex_error("unclosed comment"); process(); }
('[^']*)                    { cache_line(); print_lex_error("unclosed string constant"); process(); }

.                           { cache_line(); print_lex_error("illegal character"); char_count++; }

%%
void wrap()
{
    // 跳过空行
    if(cur_line_info.size() != 0){ 
        last_line_info = cur_line_info;
        cur_line_info.clear();
        last_line_count = line_count;
    }
    line_count++; 
    char_count = 1;
}

void process()
{
    for(i = 0; i < strlen(yytext); i++)
    {
        if(yytext[i] == '\n') wrap();
        else char_count++;
    }  
}

void process_anno()
{
    log_debug("ANNO\t\t%s\t\t\t%d\t\t%d", yytext, line_count, char_count);
    for(i = 0; i < strlen(yytext); i++)
    {
        if(yytext[i] == '\n') wrap();
        else char_count++;
    }
}

void cache_line()
{
    cur_line_info += yytext;
    if(yydebug){
        std::cout<<"cur_line_info is "<<cur_line_info<<std::endl;
    }
}

void print_lex_error(const char *msg)
{
    lex_error_flag = 1;
    fprintf(stderr,"%d,%d:\033[01;31m \terror\033[0m : %s - %s\n", line_count, char_count, msg, yytext);
}

void print_syntax_error(const char *msg)
{
    lex_error_flag = 1;
    fprintf(stderr,"%d,%d:\033[01;31m \terror\033[0m : %s at '%s'\n", line_count, char_count, msg, yytext);
}

void to_str()
{
    std::string str = std::string(yytext);
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    yylval.token_info.value.set(str);
    yylval.token_info.line_num = line_count;
    yylval.token_info.column_num = char_count;
    yylval.token_info.length = yyleng;

    char_count += yyleng;
}

void to_str_case_sensitive()
{
    std::string str = std::string(yytext);
    str = str.substr(1, str.length() - 2);  
    yylval.token_info.value.set(str);
    yylval.token_info.line_num = line_count;
    yylval.token_info.column_num = char_count;
    yylval.token_info.length = yyleng;

    char_count += yyleng;
}

void to_int()
{
    yylval.token_info.value.set(std::stoi(yytext));
    yylval.token_info.line_num = line_count;
    yylval.token_info.column_num = char_count;
    yylval.token_info.length = yyleng;

    char_count += yyleng;
}

void to_float()
{
    yylval.token_info.value.set(std::stof(yytext));
    yylval.token_info.line_num = line_count;
    yylval.token_info.column_num = char_count;
    yylval.token_info.length = yyleng;

    char_count += yyleng;
}

void to_char()
{
    yylval.token_info.value.set(yytext[1]);
    yylval.token_info.line_num = line_count;
    yylval.token_info.column_num = char_count;
    yylval.token_info.length = yyleng;

    char_count += yyleng;
}

void to_bool()
{
    yylval.token_info.value.set(yytext[0] == 'T');
    yylval.token_info.line_num = line_count;
    yylval.token_info.column_num = char_count;
    yylval.token_info.length = yyleng;

    char_count += yyleng;
}

int yywrap()
{
    return 1;
}
