/*

Author : R Raghu Raman
This is a compiler for the JACK programming Language
Usage:
./JACK_Compiler <file1.jack> <file2.jack> ...
Output:
<file1.anz> <file1.err> <file1.vm>
..

*/
#include<bits/stdc++.h>
typedef long long ll;
#define ff first
#define ss second

using namespace std;

class Compiler
{
public:
  fstream f_jack, f_err, f_tok, f_anz, f_vm;
  vector <string> v,v1;
  set <string> symbols, keywords;
  ll tabcount,pos,currstat,field_count,static_count,local_count,argument_count,label_count;
  string currclass,currscope,currsubroutinename,currsubroutinetype;
  map < pair <string, string> , ll > Class_Symbol_Table_Static, Class_Symbol_Table_Field,Subroutine_Symbol_Table_Local,Subroutine_Symbol_Table_Argument;
  map < string , string> Class_Variables_Field,Class_Variables_Static, Subroutine_Variables_Local,Subroutine_Variables_Argument;
  Compiler (set <string> symbols1, set <string> keywords1, char * filename1)
  {
    //Creating .tok, .err, .anz, and .vm files
    f_jack.open(filename1,ios::in);
    char *filename = new char[100];
    strcpy(filename,filename1);
    ll sz=strlen(filename)-4;
    filename[sz]='t';
    filename[sz+1]='o';
    filename[sz+2]='k';
    filename[sz+3]='\0';
    f_tok.open(filename,ios::out);
    filename[sz]='e';
    filename[sz+1]='r';
    filename[sz+2]='r';
    filename[sz+3]='\0';
    f_err.open(filename,ios::out);
    filename[sz]='v';
    filename[sz+1]='m';
    filename[sz+2]='\0';
    f_vm.open(filename,ios::out);
    filename[sz]='a';
    filename[sz+1]='n';
    filename[sz+2]='z';
    filename[sz+3]='\0';
    f_anz.open(filename,ios::out);
    symbols = symbols1;
    keywords = keywords1;
    tabcount=0;
    pos=0;
    Class_Symbol_Table_Field.clear();
    Class_Symbol_Table_Static.clear();
    Class_Variables_Field.clear();
    Class_Variables_Static.clear();
    field_count=0;
    static_count=0;
    label_count=0;
  }

  void Tokenizer();
  void Token_Analyzer();
  bool check_keyword ( string &,string);
  bool check_identifier ( string &);
  bool check_symbol ( string &,string);
  bool check_integerConstant ( string &);
  bool check_stringConstant ( string &);
  void indent_store( string );
  string Trim(string);
  string Ltrim(string);
  void Compile_Class();
  void Compile_ClassVarDec();
  void Compile_SubroutineDec();
  void Compile_ParameterList();
  void Compile_SubroutineBody();
  void Compile_VarDec();
  void Compile_Statements();
  void Compile_Statement();
  void Compile_LetStatement();
  void Compile_IfStatement();
  void Compile_WhileStatement();
  void Compile_DoStatement();
  void Compile_ReturnStatement();
  void Compile_Expression();
  void Compile_Term();
  void Compile_ExpressionList();
  void Compile_SubroutineCall();
  void Stop();
  bool check_declaration(string);
  void pop_current_variable(string);
  void push_current_variable(string);
  void Code_Generator();
  void Generate_Class();
  void Generate_ClassVarDec();
  void Generate_SubroutineDec();
  void Generate_ParameterList();
  void Generate_SubroutineBody();
  void Generate_VarDec();
  void Generate_Statements();
  void Generate_Statement();
  void Generate_LetStatement();
  void Generate_IfStatement();
  void Generate_WhileStatement();
  void Generate_DoStatement();
  void Generate_ReturnStatement();
  void Generate_Expression();
  void Generate_Term();
  ll Generate_ExpressionList();
  void Generate_SubroutineCall();
};

void Compiler::Tokenizer()
{
  char ch,ch1;
  string temp;

  //Reads buffer character by character
  while(f_jack>>noskipws>>ch)
  {
    //Checking for comments
    if(ch=='/')
    {
      f_jack>>noskipws>>ch1;

      //Single line comments
      if(ch1=='/')
        while(ch!='\n')
          f_jack>>noskipws>>ch;

      //Multi-line comments
      else if(ch1=='*')
      {
        while(!f_jack.eof()&&ch1!='/')
        {
          while(!f_jack.eof()&&ch!='*')
            f_jack>>noskipws>>ch;
          f_jack>>ch1;
          ch=ch1;
        }
      }
      else
      {
          //If it is the '/' token
          f_jack.unget();
          f_tok<<"<symbol> / </symbol>\n";
          v.push_back("<symbol> / </symbol>");
      }
    }

    else
    {
      temp.clear();
      temp+=ch;

      //Keywords and identifier
      if(isalpha(ch)||ch=='_')
      {
        while(!f_jack.eof())
        {
          f_jack>>noskipws>>ch;
          if(isalpha(ch)||isdigit(ch)||ch=='_')
            temp+=ch;
          else
            break;
        }
        f_jack.unget();
        if(keywords.count(temp)==1)
        {
          f_tok<<"<keyword> "<<temp<<" </keyword>\n";
          v.push_back("<keyword> "+temp+" </keyword>");
        }
        else
        {
          f_tok<<"<identifier> "<<temp<<" </identifier>\n";
          v.push_back("<identifier> "+temp+" </identifier>");
        }
      }

      //String Constant
      else if(ch=='"')
      {
        temp.clear();
        while(!f_jack.eof())
        {
          f_jack>>noskipws>>ch;
          if(ch!='"')
            temp+=ch;
          else
            break;
        }
        if(ch!='"')
        {
          f_err<<"Expected symbol: '\"' after a string constant\n";
          Stop();
        }
        f_tok<<"<stringConstant> "<<temp<<" </stringConstant>\n";
        v.push_back("<stringConstant> "+temp+" </stringConstant>");
      }

      //Symbols
      else if(symbols.count(temp)==1)
      {
        f_tok<<"<symbol> "<<ch<<" </symbol>\n";
        v.push_back("<symbol> "+temp+" </symbol>");
      }

      //Integer Constants
      else if(isdigit(ch))
      {
        while(!f_jack.eof())
        {
          f_jack>>noskipws>>ch;
          if(isdigit(ch))
            temp+=ch;
          else
            break;
        }

        //If there are alphabets at the end of the integer
        if(isalpha(ch))
        {
          while(!f_jack.eof()&&isalpha(ch))
          {
            temp+=ch;
            f_jack>>noskipws>>ch;
          }
          f_err<<"Invalid Integer Constant: "<<temp<<'\n';
          Stop();
        }
        f_jack.unget();

        ll integer_value=0;
        for(ll i=0;i<temp.size();++i)
        {
          integer_value*=10;
          integer_value+=(ll)(temp[i]-'0');
        }
        if(integer_value>=32768)
        {
          f_err<<"Integer Constant: "<<temp<<" out of bound\n";
          Stop();
        }

        f_tok<<"<integerConstant> "<<temp<<" </integerConstant>\n";
        v.push_back("<integerConstant> "+temp+" </integerConstant>");
      }

    }

  }
  f_tok.close();
}

void Compiler::Token_Analyzer()
{
  //Initializing position in .tok file and the tab count for indentation
  pos=0;
  tabcount=0;

  //Calling Compile Class
  indent_store("<class>");
  tabcount++;
  Compile_Class();
  tabcount--;
  indent_store("</class>");

  f_anz.close();
}

bool Compiler::check_keyword( string &curr, string comp)
{
  return (curr.substr(0,9)=="<keyword>" && curr.substr((curr.size()-10),10)=="</keyword>" && curr.substr(10,(curr.size()-21))==comp);
}

bool Compiler::check_identifier( string &curr)
{
  return (curr.substr(0,12)=="<identifier>" && curr.substr((curr.size()-13),13)=="</identifier>");
}

bool Compiler::check_symbol( string &curr, string comp)
{
  return (curr.substr(0,8)=="<symbol>" && curr.substr((curr.size()-9),9)=="</symbol>" && curr.substr(9,1)==comp);
}

bool Compiler::check_integerConstant( string &curr)
{
  return (curr.substr(0,17)=="<integerConstant>" && curr.substr((curr.size()-18),18)=="</integerConstant>");
}

bool Compiler::check_stringConstant( string &curr)
{
  return (curr.substr(0,16)=="<stringConstant>" && curr.substr((curr.size()-17),17)=="</stringConstant>");
}

void Compiler::indent_store( string curr)
{
  //Storing in .anz file with proper indentation
  v1.push_back("");
  for(ll i=0;i<tabcount;++i)
    f_anz<<"  ";
  v1.back()+=curr;
  f_anz<<v1.back()<<'\n';
}

string Compiler::Trim( string curr)
{
  ll trimsize=0;
  if(curr.substr(1,7)=="keyword")
    trimsize=10;
  else if(curr.substr(1,10)=="identifier")
    trimsize=13;
  else if(curr.substr(1,6)=="symbol")
    trimsize=9;
  else if(curr.substr(1,14)=="stringConstant")
    trimsize=17;
  else if(curr.substr(1,15)=="integerConstant")
    trimsize=18;
  return curr.substr(trimsize,curr.size()-2*trimsize-1);
}

string Compiler::Ltrim( string curr)
{
  ll length = -1;
  while(curr[length+1]!='>')
    length++;
  return curr.substr(1,length);
}

void Compiler::Compile_Class()
{
  if(!check_keyword(v[pos],"class"))
  {
    f_err<<"Expected keyword: 'class' in class declaration\n";
    Stop();
  }
  indent_store(v[pos]);
  pos++;

  if(!check_identifier(v[pos]))
  {
    f_err<<"Expected identifier: className in class declaration\n";
    Stop();
  }
  indent_store(v[pos]);
  currclass=Trim(v[pos]);
  pos++;

  if(!check_symbol(v[pos],"{"))
  {
    f_err<<"Expected symbol: '{' in class declaration\n";
    Stop();
  }
  indent_store(v[pos]);
  pos++;

  while(check_keyword(v[pos],"static")||check_keyword(v[pos],"field"))
  {
    indent_store("<classVarDec>");
    tabcount++;
    Compile_ClassVarDec();
    tabcount--;
    indent_store("</classVarDec>");
  }

  while(check_keyword(v[pos],"constructor")||check_keyword(v[pos],"function")||check_keyword(v[pos],"method"))
  {
    indent_store("<subroutineDec>");
    tabcount++;
    Compile_SubroutineDec();
    tabcount--;
    indent_store("</subroutineDec>");
  }

  if(!check_symbol(v[pos],"}"))
  {
    f_err<<"Expected symbol: '}' in class declaration\n";
    Stop();
  }
  indent_store(v[pos]);
  pos++;
}

void Compiler::Compile_ClassVarDec()
{
  indent_store(v[pos]);
  pos++;

  if(!check_keyword(v[pos],"int")&&!check_keyword(v[pos],"char")&&!check_keyword(v[pos],"boolean")&&!check_identifier(v[pos]))
  {
    f_err<<"Expected type in variable declaration of class '"<<currclass<<"'\n";
    Stop();
  }
  indent_store(v[pos]);
  pos++;

  if(!check_identifier(v[pos]))
  {
    f_err<<"Expected variable name in variable declaration of class '"<<currclass<<"'\n";
    Stop();
  }
  indent_store(v[pos]);
  pos++;

  while(check_symbol(v[pos],","))
  {
    indent_store(v[pos]);
    pos++;

    if(!check_identifier(v[pos]))
    {
      f_err<<"Unexpected symbol: ',' in variable declaration of class '"<<currclass<<"'\n";
      Stop();
    }
    indent_store(v[pos]);
    pos++;
  }

  if(!check_symbol(v[pos],";"))
  {
    f_err<<"Expected symbol: ';' after variable declaration of class '"<<currclass<<"'\n";
    Stop();
  }
  indent_store(v[pos]);
  pos++;
}

void Compiler::Compile_SubroutineDec()
{
  indent_store(v[pos]);
  pos++;

  if(!check_keyword(v[pos],"void")&&!check_keyword(v[pos],"int")&&!check_keyword(v[pos],"char")&&!check_keyword(v[pos],"boolean")&&!check_identifier(v[pos]))
  {
    f_err<<"Expected type in subroutine declaration of class '"<<currclass<<"'\n";
    Stop();
  }
  indent_store(v[pos]);
  pos++;

  if(!check_identifier(v[pos]))
  {
    f_err<<"Expected subroutine name in variable declaration of class '"<<currclass<<"'\n";
    Stop();
  }
  indent_store(v[pos]);
  currscope=Trim(v[pos]);
  pos++;

  if(!check_symbol(v[pos],"("))
  {
    f_err<<"Expected symbol: '(' for parameter list of subroutine '"<<currscope<<"' of class '"<<currclass<<"'\n";
    Stop();
  }
  indent_store(v[pos]);
  pos++;

  indent_store("<parameterList>");
  tabcount++;
  Compile_ParameterList();
  tabcount--;
  indent_store("</parameterList>");

  if(!check_symbol(v[pos],")"))
  {
    f_err<<"Expected symbol: ')' for parameter list of subroutine '"<<currscope<<"' of class '"<<currclass<<"'\n";
    Stop();
  }
  indent_store(v[pos]);
  pos++;

  indent_store("<subroutineBody>");
  tabcount++;
  Compile_SubroutineBody();
  tabcount--;
  indent_store("</subroutineBody>");

}

void Compiler::Compile_ParameterList()
{
  if(check_symbol(v[pos],")"))
    return;

  if(!check_keyword(v[pos],"int")&&!check_keyword(v[pos],"char")&&!check_keyword(v[pos],"boolean")&&!check_identifier(v[pos]))
  {
    f_err<<"Expected type in parameter list of subroutine '"<<currscope<<"'\n";
    Stop();
  }
  indent_store(v[pos]);
  pos++;

  if(!check_identifier(v[pos]))
  {
    f_err<<"Expected variable name in parameter list of subroutine '"<<currscope<<"'\n";
    Stop();
  }
  indent_store(v[pos]);
  pos++;

  while(check_symbol(v[pos],","))
  {
    indent_store(v[pos]);
    pos++;

    if(!check_keyword(v[pos],"int")&&!check_keyword(v[pos],"char")&&!check_keyword(v[pos],"boolean")&&!check_identifier(v[pos]))
    {
      f_err<<"Expected type in parameter list of subroutine '"<<currscope<<"'\n";
      Stop();
    }
    indent_store(v[pos]);
    pos++;

    if(!check_identifier(v[pos]))
    {
      f_err<<"Unexpected symbol: ',' in parameter list of subroutine '"<<currscope<<"'\n";
      Stop();
    }
    indent_store(v[pos]);
    pos++;
  }
}

void Compiler::Compile_SubroutineBody()
{
  if(!check_symbol(v[pos],"{"))
  {
    f_err<<"Expected symbol: '{' in subroutine body of '"<<currscope<<"'\n";
    Stop();
  }
  indent_store(v[pos]);
  pos++;

  currstat=0;
  while(check_keyword(v[pos],"var"))
  {
    currstat++;
    indent_store("<varDec>");
    tabcount++;
    Compile_VarDec();
    tabcount--;
    indent_store("</varDec>");
  }

  currstat=0;
  indent_store("<statements>");
  tabcount++;
  Compile_Statements();
  tabcount--;
  indent_store("</statements>");

  if(!check_symbol(v[pos],"}"))
  {
    f_err<<"Expected symbol: '}' in subroutine body of '"<<currscope<<"'\n";
    Stop();
  }
  indent_store(v[pos]);
  pos++;
}

void Compiler::Compile_VarDec()
{
  indent_store(v[pos]);
  pos++;

  if(!check_keyword(v[pos],"int")&&!check_keyword(v[pos],"char")&&!check_keyword(v[pos],"boolean")&&!check_identifier(v[pos]))
  {
    f_err<<"Expected type in "<<currstat<<"-th variable declaration of subroutine '"<<currscope<<"'\n";
    Stop();
  }
  indent_store(v[pos]);
  pos++;

  if(!check_identifier(v[pos]))
  {
    f_err<<"Expected variable name in "<<currstat<<"-th variable declaration of subroutine '"<<currscope<<"'\n";
    Stop();
  }
  indent_store(v[pos]);
  pos++;

  while(check_symbol(v[pos],","))
  {
    indent_store(v[pos]);
    pos++;

    if(!check_identifier(v[pos]))
    {
      f_err<<"Unexpected symbol: ',' in "<<currstat<<"-th variable declaration of subroutine '"<<currscope<<"'\n";
      Stop();
    }
    indent_store(v[pos]);
    pos++;
  }

  if(!check_symbol(v[pos],";"))
  {
    f_err<<"Expected symbol: ';' after "<<currstat<<"-th variable declaration of subroutine '"<<currscope<<"'\n";
    Stop();
  }
  indent_store(v[pos]);
  pos++;
}

void Compiler::Compile_Statements()
{
  while(!check_symbol(v[pos],"}"))
  {
    currstat++;
    indent_store("<statement>");
    tabcount++;
    Compile_Statement();
    tabcount--;
    indent_store("</statement>");
  }
}

void Compiler::Compile_Statement()
{
  if(check_keyword(v[pos],"let"))
  {
    indent_store("<letStatement>");
    tabcount++;
    Compile_LetStatement();
    tabcount--;
    indent_store("</letStatement>");
  }
  else if(check_keyword(v[pos],"if"))
  {
    indent_store("<ifStatement>");
    tabcount++;
    Compile_IfStatement();
    tabcount--;
    indent_store("</ifStatement>");
  }
  else if(check_keyword(v[pos],"while"))
  {
    indent_store("<whileStatement>");
    tabcount++;
    Compile_WhileStatement();
    tabcount--;
    indent_store("</whileStatement>");
  }
  else if(check_keyword(v[pos],"do"))
  {
    indent_store("<doStatement>");
    tabcount++;
    Compile_DoStatement();
    tabcount--;
    indent_store("</doStatement>");
  }
  else if(check_keyword(v[pos],"return"))
  {
    indent_store("<returnStatement>");
    tabcount++;
    Compile_ReturnStatement();
    tabcount--;
    indent_store("</returnStatement>");
  }
  else
  {
    f_err<<"Invalid Statement in "<<currstat<<"-th statement subroutine body of '"<<currscope<<"'\n";
    Stop();
  }
}

void Compiler::Compile_LetStatement()
{
  indent_store(v[pos]);
  pos++;

  if(!check_identifier(v[pos]))
  {
    f_err<<"Missing variable name in variable assignment in "<<currstat<<"-th statement in subroutine '"<<currscope<<"'\n";
    Stop();
  }
  indent_store(v[pos]);
  pos++;

  if(check_symbol(v[pos],"["))
  {
    indent_store(v[pos]);
    pos++;

    indent_store("<expression>");
    tabcount++;
    Compile_Expression();
    tabcount--;
    indent_store("</expression>");

    if(!check_symbol(v[pos],"]"))
    {
      f_err<<"Expected symbol: ']' in variable assignment in "<<currstat<<"-th statement '"<<currscope<<"'\n";
      Stop();
    }
    indent_store(v[pos]);
    pos++;
  }

  if(!check_symbol(v[pos],"="))
  {
    f_err<<"Expected symbol: '=' in variable assignment in "<<currstat<<"-th statement '"<<currscope<<"'\n";
    Stop();
  }
  indent_store(v[pos]);
  pos++;

  indent_store("<expression>");
  tabcount++;
  Compile_Expression();
  tabcount--;
  indent_store("</expression>");

  if(!check_symbol(v[pos],";"))
  {
    f_err<<"Expected symbol: ';' in variable assignment in "<<currstat<<"-th statement '"<<currscope<<"'\n";
    Stop();
  }
  indent_store(v[pos]);
  pos++;
}

void Compiler::Compile_IfStatement()
{
  indent_store(v[pos]);
  pos++;

  if(!check_symbol(v[pos],"("))
  {
    f_err<<"Expected symbol: '(' after 'if' in "<<currstat<<"-th statement in subroutine '"<<currscope<<"'\n";
    Stop();
  }
  indent_store(v[pos]);
  pos++;

  indent_store("<expression>");
  tabcount++;
  Compile_Expression();
  tabcount--;
  indent_store("</expression>");

  if(!check_symbol(v[pos],")"))
  {
    f_err<<"Expected symbol: ')' after 'if' in "<<currstat<<"-th statement in subroutine '"<<currscope<<"'\n";
    Stop();
  }
  indent_store(v[pos]);
  pos++;

  if(!check_symbol(v[pos],"{"))
  {
    f_err<<"Expected symbol: '{' after 'if' in "<<currstat<<"-th statement in subroutine '"<<currscope<<"'\n";
    Stop();
  }
  indent_store(v[pos]);
  pos++;

  indent_store("<statements>");
  tabcount++;
  Compile_Statements();
  tabcount--;
  indent_store("</statements>");

  if(!check_symbol(v[pos],"}"))
  {
    f_err<<"Expected symbol: '}' after 'if' in "<<currstat<<"-th statement in subroutine '"<<currscope<<"'\n";
    Stop();
  }
  indent_store(v[pos]);
  pos++;

  if(check_keyword(v[pos],"else"))
  {
    indent_store(v[pos]);
    pos++;

    if(!check_symbol(v[pos],"{"))
    {
      f_err<<"Expected symbol: '{' after 'else' in "<<currstat<<"-th statement in subroutine '"<<currscope<<"'\n";
      Stop();
    }
    indent_store(v[pos]);
    pos++;

    indent_store("<statements>");
    tabcount++;
    Compile_Statements();
    tabcount--;
    indent_store("</statements>");

    if(!check_symbol(v[pos],"}"))
    {
      f_err<<"Expected symbol: '}' after 'else' in "<<currstat<<"-th statement in subroutine '"<<currscope<<"'\n";
      Stop();
    }
    indent_store(v[pos]);
    pos++;

  }

}

void Compiler::Compile_WhileStatement()
{
  indent_store(v[pos]);
  pos++;

  if(!check_symbol(v[pos],"("))
  {
    f_err<<"Expected symbol: '(' after 'while' in "<<currstat<<"-th statement in subroutine '"<<currscope<<"'\n";
    Stop();
  }
  indent_store(v[pos]);
  pos++;

  indent_store("<expression>");
  tabcount++;
  Compile_Expression();
  tabcount--;
  indent_store("</expression>");

  if(!check_symbol(v[pos],")"))
  {
    f_err<<"Expected symbol: ')' after 'while' in "<<currstat<<"-th statement in subroutine '"<<currscope<<"'\n";
    Stop();
  }
  indent_store(v[pos]);
  pos++;

  if(!check_symbol(v[pos],"{"))
  {
    f_err<<"Expected symbol: '{' after 'while' in "<<currstat<<"-th statement in subroutine '"<<currscope<<"'\n";
    Stop();
  }
  indent_store(v[pos]);
  pos++;

  indent_store("<statements>");
  tabcount++;
  Compile_Statements();
  tabcount--;
  indent_store("</statements>");

  if(!check_symbol(v[pos],"}"))
  {
    f_err<<"Expected symbol: '}' after 'while' in "<<currstat<<"-th statement in subroutine '"<<currscope<<"'\n";
    Stop();
  }
  indent_store(v[pos]);
  pos++;

}

void Compiler::Compile_DoStatement()
{
  indent_store(v[pos]);
  pos++;

  indent_store("<subroutineCall>");
  tabcount++;
  Compile_SubroutineCall();
  tabcount--;
  indent_store("</subroutineCall>");

  if(!check_symbol(v[pos],";"))
  {
    f_err<<"Expected symbol: ';' after 'do' in "<<currstat<<"-th statement in subroutine '"<<currscope<<"'\n";
    Stop();
  }
  indent_store(v[pos]);
  pos++;
}

void Compiler::Compile_ReturnStatement()
{
  indent_store(v[pos]);
  pos++;

  if(check_symbol(v[pos],";"))
  {
    indent_store(v[pos]);
    pos++;

    return;
  }
  else
  {
    indent_store("<expression>");
    tabcount++;
    Compile_Expression();
    tabcount--;
    indent_store("</expression>");

    if(!check_symbol(v[pos],";"))
    {
      f_err<<"Expected symbol: ';' after 'return' in "<<currstat<<"-th statement in subroutine '"<<currscope<<"'\n";
      Stop();
    }
    indent_store(v[pos]);
    pos++;
  }
}

void Compiler::Compile_Expression()
{
  indent_store("<term>");
  tabcount++;
  Compile_Term();
  tabcount--;
  indent_store("</term>");

  while(check_symbol(v[pos],"+")||check_symbol(v[pos],"-")||check_symbol(v[pos],"*")||check_symbol(v[pos],"/")||check_symbol(v[pos],"&")||check_symbol(v[pos],"|")||check_symbol(v[pos],"<")||check_symbol(v[pos],">")||check_symbol(v[pos],"="))
  {
    indent_store(v[pos]);
    pos++;

    indent_store("<term>");
    tabcount++;
    Compile_Term();
    tabcount--;
    indent_store("</term>");
  }
}

void Compiler::Compile_Term()
{
  if(check_integerConstant(v[pos])||check_stringConstant(v[pos])||check_keyword(v[pos],"true")||check_keyword(v[pos],"false")||check_keyword(v[pos],"null")||check_keyword(v[pos],"this"))
  {
    indent_store(v[pos]);
    pos++;
  }

  else if(check_identifier(v[pos]))
  {
    if(check_symbol(v[pos+1],"(")||check_symbol(v[pos+1],"."))
    {
      indent_store("<subroutineCall>");
      tabcount++;
      Compile_SubroutineCall();
      tabcount--;
      indent_store("</subroutineCall>");
    }

    else
    {
      indent_store(v[pos]);
      pos++;

      if(check_symbol(v[pos],"["))
      {
        indent_store(v[pos]);
        pos++;

        indent_store("<expression>");
        tabcount++;
        Compile_Expression();
        tabcount--;
        indent_store("</expression>");

        if(!check_symbol(v[pos],"]"))
        {
          f_err<<"Expected symbol: ']' in "<<currstat<<"-th statement in subroutine '"<<currscope<<"'\n";
          Stop();
        }
        indent_store(v[pos]);
        pos++;
      }
    }
  }

  else if(check_symbol(v[pos],"("))
  {
    indent_store(v[pos]);
    pos++;

    indent_store("<expression>");
    tabcount++;
    Compile_Expression();
    tabcount--;
    indent_store("</expression>");

    if(!check_symbol(v[pos],")"))
    {
      f_err<<"Expected symbol: ')' in "<<currstat<<"-th statement in subroutine '"<<currscope<<"'\n";
      Stop();
    }
    indent_store(v[pos]);
    pos++;
  }

  else if(check_symbol(v[pos],"~")||check_symbol(v[pos],"-"))
  {
    indent_store(v[pos]);
    pos++;

    indent_store("<term>");
    tabcount++;
    Compile_Term();
    tabcount--;
    indent_store("</term>");
  }

  else
  {
    f_err<<"Invalid term in "<<currstat<<"-th statement in subroutine '"<<currscope<<"'\n";
    Stop();
  }

}

void Compiler::Compile_SubroutineCall()
{
  if(!check_identifier(v[pos]))
  {
    f_err<<"Missing object/class/subroutine name in subroutine call in "<<currstat<<"-th statement in the subroutine '"<<currscope<<"'\n";
    Stop();
  }
  indent_store(v[pos]);
  pos++;

  if(check_symbol(v[pos],"."))
  {
    indent_store(v[pos]);
    pos++;

    if(!check_identifier(v[pos]))
    {
      f_err<<"Missing subroutine name in subroutine call in "<<currstat<<"-th statement in the subroutine '"<<currscope<<"'\n";
      Stop();
    }
    indent_store(v[pos]);
    pos++;
  }

  if(!check_symbol(v[pos],"("))
  {
    f_err<<"Expected symbol: '(' in subroutine call in "<<currstat<<"-th statement in the subroutine '"<<currscope<<"'\n";
    Stop();
  }
  indent_store(v[pos]);
  pos++;

  indent_store("<expressionList>");
  tabcount++;
  Compile_ExpressionList();
  tabcount--;
  indent_store("</expressionList>");

  if(!check_symbol(v[pos],")"))
  {
    f_err<<"Expected symbol: ')' in subroutine call in "<<currstat<<"-th statement in the subroutine '"<<currscope<<"'\n";
    Stop();
  }
  indent_store(v[pos]);
  pos++;
}

void Compiler::Compile_ExpressionList()
{
  if(check_symbol(v[pos],")"))
    return;

  indent_store("<expression>");
  tabcount++;
  Compile_Expression();
  tabcount--;
  indent_store("</expression>");

  while(check_symbol(v[pos],","))
  {
    indent_store(v[pos]);
    pos++;

    indent_store("<expression>");
    tabcount++;
    Compile_Expression();
    tabcount--;
    indent_store("</expression>");
  }
}

void Compiler::Stop()
{
  f_tok.close();
  f_err.close();
  f_anz.close();
  f_jack.close();
  f_vm.close();
  exit(0);
}

bool Compiler::check_declaration( string curr)
{
  return ((Class_Variables_Field.count(curr)==1)||(Class_Variables_Static.count(curr)==1)||(Subroutine_Variables_Local.count(curr)==1)||(Subroutine_Variables_Argument.count(curr)==1));
}

void Compiler::push_current_variable( string temp)
{
  if(Subroutine_Variables_Local.count(temp)==1)
    f_vm<<"push local "<<Subroutine_Symbol_Table_Local[make_pair(Subroutine_Variables_Local[temp],temp)]<<'\n';
  else if(Subroutine_Variables_Argument.count(temp)==1)
    f_vm<<"push argument "<<Subroutine_Symbol_Table_Argument[make_pair(Subroutine_Variables_Argument[temp],temp)]<<'\n';
  else
  {
    if(Class_Variables_Static.count(temp)==1)
      f_vm<<"push static "<<Class_Symbol_Table_Static[make_pair(Class_Variables_Static[temp],temp)]<<'\n';
    else if(currsubroutinetype=="function")
    {
      f_err<<"Illegal access of class field variables by function '"<<currsubroutinename<<"'\n";
      Stop();
    }
    else if(Class_Variables_Field.count(temp)==1)
      f_vm<<"push this "<<Class_Symbol_Table_Field[make_pair(Class_Variables_Field[temp],temp)]<<'\n';
  }
}

void Compiler::pop_current_variable( string temp)
{
  if(Subroutine_Variables_Local.count(temp)==1)
    f_vm<<"pop local "<<Subroutine_Symbol_Table_Local[make_pair(Subroutine_Variables_Local[temp],temp)]<<'\n';
  else if(Subroutine_Variables_Argument.count(temp)==1)
    f_vm<<"pop argument "<<Subroutine_Symbol_Table_Argument[make_pair(Subroutine_Variables_Argument[temp],temp)]<<'\n';
  else
  {
    if(Class_Variables_Static.count(temp)==1)
      f_vm<<"pop static "<<Class_Symbol_Table_Static[make_pair(Class_Variables_Static[temp],temp)]<<'\n';
    else if(currsubroutinetype=="function")
    {
      f_err<<"Illegal access of class field variables by function '"<<currsubroutinename<<"'\n";
      Stop();
    }
    else if(Class_Variables_Field.count(temp)==1)
        f_vm<<"pop this "<<Class_Symbol_Table_Field[make_pair(Class_Variables_Field[temp],temp)]<<'\n';
  }
}

void Compiler::Code_Generator()
{
  pos=0;
  if(Ltrim(v1[pos])=="class")
  {
    pos++;
    Generate_Class();
  }

  if(Ltrim(v1[pos])!="/class")
  {
    f_err<<"Class declaration incomplete\n";
    Stop();
  }
  pos++;
  f_vm.close();
}

void Compiler::Generate_Class()
{
  pos++;

  currclass = Trim(v1[pos]);

  pos+=2;

  while(Ltrim(v1[pos])=="classVarDec")
  {
    pos++;

    Generate_ClassVarDec();

    if(Ltrim(v1[pos])!="/classVarDec")
    {
      f_err<<"Class variable declaration incomplete\n";
      Stop();
    }
    pos++;
  }

  while(Ltrim(v1[pos])=="subroutineDec")
  {
    pos++;

    Generate_SubroutineDec();

    if(Ltrim(v1[pos])!="/subroutineDec")
    {
      f_err<<"Declaration of subroutine '"<<currsubroutinename<<"' incomplete\n";
      Stop();
    }
    pos++;
  }

  pos++;

}

void Compiler::Generate_ClassVarDec()
{
  bool isStatic = (Trim(v1[pos])=="static");
  pos++;

  pair <string,string> temp;

  temp.ff = Trim(v1[pos]);
  pos++;

  temp.ss = Trim(v1[pos]);
  pos++;

  if(isStatic)
  {
    if(check_declaration(temp.ss))
    {
      f_err<<"Redeclaration of class variable "<<temp.ss<<'\n';
      Stop();
    }
    else
    {
      Class_Symbol_Table_Static[temp]=static_count;
      Class_Variables_Static[temp.ss]=temp.ff;
      static_count++;
    }
  }
  else
  {
    if(check_declaration(temp.ss))
    {
      f_err<<"Redeclaration of class variable "<<temp.ss<<'\n';
      Stop();
    }
    else
    {
      Class_Symbol_Table_Field[temp]=field_count;
      Class_Variables_Field[temp.ss]=temp.ff;
      field_count++;
    }
  }

  while(Trim(v1[pos])==",")
  {
    pos++;

    temp.ss = Trim(v1[pos]);
    pos++;

    if(isStatic)
    {
      if(check_declaration(temp.ss))
      {
        f_err<<"Redeclaration of class variable "<<temp.ss<<'\n';
        Stop();
      }
      else
      {
        Class_Symbol_Table_Static[temp]=static_count;
        Class_Variables_Static[temp.ss]=temp.ff;
        static_count++;
      }
    }
    else
    {
      if(check_declaration(temp.ss))
      {
        f_err<<"Redeclaration of class variable "<<temp.ss<<'\n';
        Stop();
      }
      else
      {
        Class_Symbol_Table_Field[temp]=field_count;
        Class_Variables_Field[temp.ss]=temp.ff;
        field_count++;
      }
    }
  }

  pos++;

}

void Compiler::Generate_SubroutineDec()
{
  argument_count=0;
  local_count=0;
  Subroutine_Symbol_Table_Local.clear();
  Subroutine_Symbol_Table_Argument.clear();
  Subroutine_Variables_Local.clear();
  Subroutine_Variables_Argument.clear();

  currsubroutinetype=Trim(v1[pos]);
  pos++;

  pair <string,string> temp;

  temp.ff = Trim(v1[pos]);
  pos++;

  temp.ss = Trim(v1[pos]);
  pos++;

  if(currsubroutinetype=="constructor")
  {
    if(temp.ff!=currclass)
    {
      f_err<<"Invalid return type for constructor\n";
      Stop();
    }
  }

  else if(currsubroutinetype=="method")
  {
    Subroutine_Symbol_Table_Argument[make_pair(temp.ff,"this")]=argument_count;
    Subroutine_Variables_Argument["this"]=temp.ff;
    argument_count++;
  }

  currsubroutinename=temp.ss;

  pos++;

  if(Ltrim(v1[pos])!="parameterList")
  {
    f_err<<"Missing Parameter List for subroutine '"<<currsubroutinename<<"'\n";
    Stop();
  }
  pos++;

  Generate_ParameterList();

  if(Ltrim(v1[pos])!="/parameterList")
  {
    f_err<<"Parameter List of subroutine '"<<currsubroutinename<<"' incomplete\n";
    Stop();
  }
  pos+=2;

  if(Ltrim(v1[pos])!="subroutineBody")
  {
    f_err<<"Missing subroutine body for '"<<currsubroutinename<<"'\n";
    Stop();
  }
  pos++;

  Generate_SubroutineBody();

  if(Ltrim(v1[pos])!="/subroutineBody")
  {
    f_err<<"Subroutine body of '"<<currsubroutinename<<"' incomplete\n";
    Stop();
  }
  pos++;

}

void Compiler::Generate_ParameterList()
{
  if(Ltrim(v1[pos])=="/parameterList")
    return;

  pair <string,string> temp;

  temp.ff=Trim(v1[pos]);
  pos++;

  temp.ss=Trim(v1[pos]);
  pos++;

  Subroutine_Symbol_Table_Argument[temp]=argument_count;
  Subroutine_Variables_Argument[temp.ss]=temp.ff;
  argument_count++;

  while(Ltrim(v1[pos])=="symbol"&&Trim(v1[pos])==",")
  {
    pos++;

    temp.ff=Trim(v1[pos]);
    pos++;

    temp.ss=Trim(v1[pos]);
    pos++;

    Subroutine_Symbol_Table_Argument[temp]=argument_count;
    Subroutine_Variables_Argument[temp.ss]=temp.ff;
    argument_count++;
  }

}

void Compiler::Generate_SubroutineBody()
{
  pos++;

  while(Ltrim(v1[pos])=="varDec")
  {
    pos++;

    Generate_VarDec();

    if(Ltrim(v1[pos])!="/varDec")
    {
      f_err<<"Incomplete variable declaration in subroutine '"<<currsubroutinename<<"'\n";
      Stop();
    }
    pos++;
  }

  f_vm<<"function "<<currclass<<'.'<<currsubroutinename<<' '<<local_count<<'\n';

  if(currsubroutinetype=="constructor")
    f_vm<<"push constant "<<field_count<<"\ncall Memory.alloc 1\npop pointer 0\n";
  else if(currsubroutinetype=="method")
    f_vm<<"push argument 0\npop pointer 0\n";

  if(Ltrim(v1[pos])!="statements")
  {
    f_err<<"Missing statements in subroutine '"<<currsubroutinename<<"'\n";
    Stop();
  }
  pos++;

  currstat=0;
  Generate_Statements();

  if(Ltrim(v1[pos])!="/statements")
  {
    f_err<<"Incomplete statements in subroutine '"<<currsubroutinename<<"'\n";
    Stop();
  }

  pos+=2;
}

void Compiler::Generate_VarDec()
{
  pair <string,string> temp;

  pos++;

  temp.ff=Trim(v1[pos]);
  pos++;

  temp.ss=Trim(v1[pos]);
  pos++;

  if(check_declaration(temp.ss))
  {
    f_err<<"Redeclaration of the variable '"<<temp.ss<<"'\n";
    Stop();
  }
  else
  {
    Subroutine_Symbol_Table_Local[temp]=local_count;
    Subroutine_Variables_Local[temp.ss]=temp.ff;
    local_count++;
  }

  while(Ltrim(v1[pos])=="symbol"&&Trim(v1[pos])==",")
  {
    pos++;

    temp.ss=Trim(v1[pos]);
    pos++;

    if(check_declaration(temp.ss))
    {
      f_err<<"Redeclaration of the variable '"<<temp.ss<<"'\n";
      Stop();
    }
    else
    {
      Subroutine_Symbol_Table_Local[temp]=local_count;
      Subroutine_Variables_Local[temp.ss]=temp.ff;
      local_count++;
    }
  }

  pos++;

}

void Compiler::Generate_Statements()
{
  while(Ltrim(v1[pos])!="/statements")
  {
    if(Ltrim(v1[pos])!="statement")
    {
      f_err<<"Invalid statement in subroutine '"<<currsubroutinename<<"'\n";
      Stop();
    }
    pos++;

    currstat++;
    Generate_Statement();

    if(Ltrim(v1[pos])!="/statement")
    {
      f_err<<"Incomplete statement in subroutine '"<<currsubroutinename<<"'\n";
      Stop();
    }
    pos++;
  }
}

void Compiler::Generate_Statement()
{
  if(Ltrim(v1[pos])=="letStatement")
  {
    pos++;

    Generate_LetStatement();

    if(Ltrim(v1[pos])!="/letStatement")
    {
      f_err<<"Incomplete let statement in "<<currstat<<"-th statement in subroutine '"<<currsubroutinename<<"'\n";
      Stop();
    }
    pos++;
  }

  else if(Ltrim(v1[pos])=="ifStatement")
  {
    pos++;

    Generate_IfStatement();

    if(Ltrim(v1[pos])!="/ifStatement")
    {
      f_err<<"Incomplete if statement in "<<currstat<<"-th statement in subroutine '"<<currsubroutinename<<"'\n";
      Stop();
    }
    pos++;
  }

  else if(Ltrim(v1[pos])=="whileStatement")
  {
    pos++;

    Generate_WhileStatement();

    if(Ltrim(v1[pos])!="/whileStatement")
    {
      f_err<<"Incomplete while statement in "<<currstat<<"-th statement in subroutine '"<<currsubroutinename<<"'\n";
      Stop();
    }
    pos++;
  }

  else if(Ltrim(v1[pos])=="doStatement")
  {
    pos++;

    Generate_DoStatement();

    if(Ltrim(v1[pos])!="/doStatement")
    {
      f_err<<"Incomplete do statement in "<<currstat<<"-th statement in subroutine '"<<currsubroutinename<<"'\n";
      Stop();
    }
    pos++;
  }

  else if(Ltrim(v1[pos])=="returnStatement")
  {
    pos++;

    Generate_ReturnStatement();

    if(Ltrim(v1[pos])!="/returnStatement")
    {
      f_err<<"Incomplete return statement in "<<currstat<<"-th statement in subroutine '"<<currsubroutinename<<"'\n";
      Stop();
    }
    pos++;
  }
}

void Compiler::Generate_LetStatement()
{
  pos++;

  string temp=Trim(v1[pos]);

  if(!check_declaration(temp))
  {
    f_err<<"Undeclared variable "<<temp<<" in "<<currstat<<"-th statement in subroutine '"<<currsubroutinename<<"'\n";
    Stop();
  }
  pos++;

  bool isArray=false;

  if(Trim(v1[pos])=="[")
  {
    isArray=true;

    pos++;

    if(Ltrim(v1[pos])!="expression")
    {
      f_err<<"Missing expression in "<<currstat<<"-th statement in subroutine '"<<currsubroutinename<<"'\n";
      Stop();
    }
    pos++;

    Generate_Expression();

    if(Ltrim(v1[pos])!="/expression")
    {
      f_err<<"Incomplete expression in "<<currstat<<"-th statement in subroutine '"<<currsubroutinename<<"'\n";
      Stop();
    }
    pos+=2;

    push_current_variable(temp);
    f_vm<<"add\n";

  }

  pos++;

  if(Ltrim(v1[pos])!="expression")
  {
    f_err<<"Missing expression in "<<currstat<<"-th statement in subroutine '"<<currsubroutinename<<"'\n";
    Stop();
  }
  pos++;

  Generate_Expression();

  if(Ltrim(v1[pos])!="/expression")
  {
    f_err<<"Incomplete expression in "<<currstat<<"-th statement in subroutine '"<<currsubroutinename<<"'\n";
    Stop();
  }
  pos+=2;

  if(isArray)
    f_vm<<"pop temp 0\npop pointer 1\npush temp 0\npop that 0\n";
  else
    pop_current_variable(temp);

}

void Compiler::Generate_IfStatement()
{
  ll temp_label=label_count;
  label_count+=2;

  pos+=2;

  if(Ltrim(v1[pos])!="expression")
  {
    f_err<<"Missing expression in "<<currstat<<"-th statement in subroutine '"<<currsubroutinename<<"'\n";
    Stop();
  }
  pos++;

  Generate_Expression();

  if(Ltrim(v1[pos])!="/expression")
  {
    f_err<<"Incomplete expression in "<<currstat<<"-th statement in subroutine '"<<currsubroutinename<<"'\n";
    Stop();
  }
  pos+=3;

  f_vm<<"not\nif-goto "<<currclass<<'.'<<temp_label<<'\n';

  if(Ltrim(v1[pos])!="statements")
  {
    f_err<<"Missing statements in if statement in "<<currstat<<"-th statement in subroutine '"<<currsubroutinename<<"'\n";
    Stop();
  }
  pos++;

  Generate_Statements();

  if(Ltrim(v1[pos])!="/statements")
  {
    f_err<<"Incomplete statements in if statement in "<<currstat<<"-th statement in subroutine '"<<currsubroutinename<<"'\n";
    Stop();
  }
  pos+=2;

  f_vm<<"goto "<<currclass<<'.'<<temp_label+1<<"\nlabel "<<currclass<<'.'<<temp_label<<'\n';

  if(Trim(v1[pos])=="else")
  {
    pos+=2;

    if(Ltrim(v1[pos])!="statements")
    {
      f_err<<"Missing statements in else statement in "<<currstat<<"-th statement in subroutine '"<<currsubroutinename<<"'\n";
      Stop();
    }
    pos++;

    Generate_Statements();

    if(Ltrim(v1[pos])!="/statements")
    {
      f_err<<"Incomplete statements in else statement in "<<currstat<<"-th statement in subroutine '"<<currsubroutinename<<"'\n";
      Stop();
    }
    pos+=2;
  }

  f_vm<<"label "<<currclass<<'.'<<temp_label+1<<'\n';

}

void Compiler::Generate_WhileStatement()
{
  ll temp_label=label_count;
  label_count+=2;

  pos+=2;

  f_vm<<"label "<<currclass<<'.'<<temp_label<<'\n';

  if(Ltrim(v1[pos])!="expression")
  {
    f_err<<"Missing expression in "<<currstat<<"-th statement in subroutine '"<<currsubroutinename<<"'\n";
    Stop();
  }
  pos++;

  Generate_Expression();

  if(Ltrim(v1[pos])!="/expression")
  {
    f_err<<"Incomplete expression in "<<currstat<<"-th statement in subroutine '"<<currsubroutinename<<"'\n";
    Stop();
  }
  pos+=3;

  f_vm<<"not\nif-goto "<<currclass<<'.'<<temp_label+1<<'\n';

  if(Ltrim(v1[pos])!="statements")
  {
    f_err<<"Missing statements in while statement in "<<currstat<<"-th statement in subroutine '"<<currsubroutinename<<"'\n";
    Stop();
  }
  pos++;

  Generate_Statements();

  if(Ltrim(v1[pos])!="/statements")
  {
    f_err<<"Incomplete statements in while statement in "<<currstat<<"-th statement in subroutine '"<<currsubroutinename<<"'\n";
    Stop();
  }
  pos+=2;

  f_vm<<"goto "<<currclass<<'.'<<temp_label<<"\nlabel "<<currclass<<'.'<<temp_label+1<<'\n';
}

void Compiler::Generate_DoStatement()
{
  pos++;

  if(Ltrim(v1[pos])!="subroutineCall")
  {
    f_err<<"Missing Subroutine Call in do statement in "<<currstat<<"-th statement of subroutine '"<<currsubroutinename<<"'\n";
    Stop();
  }
  pos++;

  Generate_SubroutineCall();
  f_vm<<"pop temp 0\n";

  if(Ltrim(v1[pos])!="/subroutineCall")
  {
    f_err<<"Incomplete Subroutine Call in do statement in "<<currstat<<"-th statement of subroutine '"<<currsubroutinename<<"'\n";
    Stop();
  }
  pos+=2;
}

void Compiler::Generate_ReturnStatement()
{
  pos++;
  if(Ltrim(v1[pos])=="expression")
  {
    pos++;

    Generate_Expression();

    if(Ltrim(v1[pos])!="/expression")
    {
      f_err<<"Incomplete expression in "<<currstat<<"-th statement in subroutine '"<<currsubroutinename<<"'\n";
      Stop();
    }
    pos++;

    f_vm<<"return\n";
  }
  else
    f_vm<<"push constant 0\nreturn\n";

  pos++;
}

void Compiler::Generate_Expression()
{
  if(Ltrim(v1[pos])=="term")
  {
    pos++;

    Generate_Term();

    if(Ltrim(v1[pos])!="/term")
    {
      f_err<<"Incomplete term in "<<currstat<<"-th statement in subroutine '"<<currsubroutinename<<"'\n";
      Stop();
    }
    pos++;

    while(Ltrim(v1[pos])!="/expression")
    {
      string temp = Trim(v1[pos]);
      pos++;

      if(Ltrim(v1[pos])!="term")
      {
        f_err<<"Missing term in "<<currstat<<"-th statement in subroutine '"<<currsubroutinename<<"'\n";
        Stop();
      }
      pos++;

      Generate_Term();

      if(Ltrim(v1[pos])!="/term")
      {
        f_err<<"Incomplete term in "<<currstat<<"-th statement in subroutine '"<<currsubroutinename<<"'\n";
        Stop();
      }
      pos++;

      if(temp=="+")
        f_vm<<"add\n";
      else if(temp=="-")
        f_vm<<"sub\n";
      else if(temp=="&")
        f_vm<<"and\n";
      else if(temp=="|")
        f_vm<<"or\n";
      else if(temp==">")
        f_vm<<"gt\n";
      else if(temp=="<")
        f_vm<<"lt\n";
      else if(temp=="=")
        f_vm<<"eq\n";
      else if(temp=="*")
        f_vm<<"call Math.multiply 2\n";
      else if(temp=="/")
        f_vm<<"call Math.divide 2\n";

    }

  }
}

void Compiler::Generate_Term()
{
  if(Ltrim(v1[pos])=="symbol"&&(Trim(v1[pos])=="~"||Trim(v1[pos])=="-"))
  {
    string temp = Trim(v1[pos]);

    pos++;

    if(Ltrim(v1[pos])!="term")
    {
      f_err<<"Missing term in "<<currstat<<"-th statement in subroutine '"<<currsubroutinename<<"'\n";
      Stop();
    }
    pos++;

    Generate_Term();

    if(Ltrim(v1[pos])!="/term")
    {
      f_err<<"Incomplete term in "<<currstat<<"-th statement in subroutine '"<<currsubroutinename<<"'\n";
      Stop();
    }
    pos++;


    if(temp=="~")
      f_vm<<"not\n";
    else
      f_vm<<"neg\n";
  }

  else if(Ltrim(v1[pos])=="symbol"&&Trim(v1[pos])=="(")
  {
    pos++;

    if(Ltrim(v1[pos])!="expression")
    {
      f_err<<"Missing expression in "<<currstat<<"-th statement in subroutine '"<<currsubroutinename<<"'\n";
      Stop();
    }
    pos++;

    Generate_Expression();

    if(Ltrim(v1[pos])!="/expression")
    {
      f_err<<"Incomplete expression in "<<currstat<<"-th statement in subroutine '"<<currsubroutinename<<"'\n";
      Stop();
    }
    pos+=2;
  }

  else if(Ltrim(v1[pos])=="integerConstant")
  {
    f_vm<<"push constant "<<Trim(v1[pos])<<'\n';
    pos++;
  }

  else if(Ltrim(v1[pos])=="keyword")
  {
    if(Trim(v1[pos])=="true")
      f_vm<<"push constant 0\nnot\n";
    else if(Trim(v1[pos])=="false"||Trim(v1[pos])=="null")
      f_vm<<"push constant 0\n";
    else if(Trim(v1[pos])=="this")
      f_vm<<"push pointer 0\n";
    else
    {
      f_err<<"Invalid keyword constant in "<<currstat<<"-th statement in '"<<currsubroutinename<<"'\n";
      Stop();
    }
    pos++;
  }

  else if(Ltrim(v1[pos])=="stringConstant")
  {
    ll string_length=v1[pos].size()-35;

    f_vm<<"push constant "<<string_length<<"\ncall String.new 1\n";
    for(ll i=0;i<string_length;++i)
      f_vm<<"push constant "<<(int)(v1[pos][17+i])<<"\ncall String.appendChar 2\n";

    pos++;
  }

  else if(Ltrim(v1[pos])=="identifier")
  {
    bool isArray=false;

    string temp=Trim(v1[pos]);
    if(!check_declaration(temp))
    {
      f_err<<"Undeclared variable "<<temp<<" in "<<currstat<<"-th statement in subroutine '"<<currsubroutinename<<"'\n";
      Stop();
    }
    pos++;

    if(Ltrim(v1[pos])=="symbol"&&Trim(v1[pos])=="[")
    {
      isArray=true;

      pos++;

      if(Ltrim(v1[pos])!="expression")
      {
        f_err<<"Missing expression in "<<currstat<<"-th statement in subroutine '"<<currsubroutinename<<"'\n";
        Stop();
      }
      pos++;

      Generate_Expression();

      if(Ltrim(v1[pos])!="/expression")
      {
        f_err<<"Incomplete expression in "<<currstat<<"-th statement in subroutine '"<<currsubroutinename<<"'\n";
        Stop();
      }
      pos+=2;
    }

    push_current_variable(temp);

    if(isArray)
      f_vm<<"add\npop pointer 1\npush that 0\n";

  }

  else if(Ltrim(v1[pos])=="subroutineCall")
  {
    pos++;

    Generate_SubroutineCall();

    if(Ltrim(v1[pos])!="/subroutineCall")
    {
      f_err<<"Incomplete Subroutine Call in "<<currstat<<"-th statement of subroutine '"<<currsubroutinename<<"'\n";
      Stop();
    }
    pos++;
  }

  else
  {
    f_err<<"Invalid term in "<<currstat<<"-th statement of subroutine '"<<currsubroutinename<<"'\n";
    Stop();
  }
}

ll Compiler::Generate_ExpressionList()
{
  ll final_ans=0;

  if(Ltrim(v1[pos])=="expression")
  {
    pos++;

    Generate_Expression();
    final_ans++;

    if(Ltrim(v1[pos])!="/expression")
    {
      f_err<<"Incomplete expression in subroutine call in "<<currstat<<"-th statement in subroutine '"<<currsubroutinename<<"'\n";
      Stop();
    }
    pos++;

    while(Trim(v1[pos])==",")
    {
      pos++;

      if(Ltrim(v1[pos])!="expression")
      {
        f_err<<"Missing expression in subroutine call in "<<currstat<<"-th statement in subroutine '"<<currsubroutinename<<"'\n";
        Stop();
      }
      pos++;

      Generate_Expression();
      final_ans++;

      if(Ltrim(v1[pos])!="/expression")
      {
        f_err<<"Incomplete expression in subroutine call in "<<currstat<<"-th statement in subroutine '"<<currsubroutinename<<"'\n";
        Stop();
      }
      pos++;
    }
  }

  return final_ans;
}

void Compiler::Generate_SubroutineCall()
{
  string temp=Trim(v1[pos]),temp1;
  pos++;

  ll No_parameters=0;

  if(Trim(v1[pos])==".")
  {
    pos++;

    temp1=Trim(v1[pos]);
    pos++;

    if(check_declaration(temp))
    {
      push_current_variable(temp);
      No_parameters++;
      if(Subroutine_Variables_Local.count(temp)==1)
        temp=Subroutine_Variables_Local[temp];
      else if(Subroutine_Variables_Argument.count(temp)==1)
        temp=Subroutine_Variables_Argument[temp];
      else
      {
        if(Class_Variables_Static.count(temp)==1)
          temp=Class_Variables_Static[temp];
        else if(currsubroutinetype=="function")
        {
          f_err<<"Illegal access of class field variables by function '"<<currsubroutinename<<"'\n";
          Stop();
        }
        else if(Class_Variables_Field.count(temp)==1)
          temp=Class_Variables_Field[temp];
      }
    }
  }

  else
  {
    temp1=temp;
    temp=currclass;

    f_vm<<"push pointer 0\n";

    No_parameters++;
  }
  pos++;

  if(Ltrim(v1[pos])!="expressionList")
  {
    f_err<<"Missing expression list for subroutine call in "<<currstat<<"-th statement in subroutine '"<<currsubroutinename<<"'\n";
    Stop();
  }
  pos++;

  No_parameters+=Generate_ExpressionList();

  if(Ltrim(v1[pos])!="/expressionList")
  {
    f_err<<"Incomplete expression list for subroutine call in "<<currstat<<"-th statement in subroutine '"<<currsubroutinename<<"'\n";
    Stop();
  }
  pos+=2;

  f_vm<<"call "<<temp<<'.'<<temp1<<' '<<No_parameters<<'\n';

}

int main(int argc, char* argv[])
{
    //FASTIO
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);
    cout.tie(NULL);

    //Declaring and initializing symbols and keywords
    set <string> symbols,keywords;
    keywords.insert("class");
    keywords.insert("constructor");
    keywords.insert("function");
    keywords.insert("method");
    keywords.insert("field");
    keywords.insert("static");
    keywords.insert("var");
    keywords.insert("int");
    keywords.insert("char");
    keywords.insert("boolean");
    keywords.insert("void");
    keywords.insert("true");
    keywords.insert("false");
    keywords.insert("null");
    keywords.insert("this");
    keywords.insert("let");
    keywords.insert("do");
    keywords.insert("if");
    keywords.insert("else");
    keywords.insert("while");
    keywords.insert("return");
    symbols.insert("(");
    symbols.insert(")");
    symbols.insert("{");
    symbols.insert("}");
    symbols.insert("[");
    symbols.insert("]");
    symbols.insert(".");
    symbols.insert(",");
    symbols.insert(";");
    symbols.insert("+");
    symbols.insert("-");
    symbols.insert("*");
    symbols.insert("/");
    symbols.insert("&");
    symbols.insert("|");
    symbols.insert("<");
    symbols.insert(">");
    symbols.insert("=");
    symbols.insert("~");

    //Declaring and initializing compiler
    for(ll i=1;i<argc;++i)
    {
      Compiler random_compiler(symbols,keywords,argv[i]);
      random_compiler.Tokenizer();
      random_compiler.Token_Analyzer();
      random_compiler.Code_Generator();
    }
}
