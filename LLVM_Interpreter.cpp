/*

Author : R Raghu Raman
This is a compiler for the JACK programming Language
Usage:
./LLVM_Interpreter <file1.vm> <file1.vmir> <file1.asm>
Output:
<file1.vmir> <file1.asm>

*/
#include<bits/stdc++.h>

using namespace std;

//M=x,D=y
#define binop "@SP\nAM=M-1\nD=M\nA=A-1\n"
//M=x
#define unop "@SP\nA=M-1\n"
//D=val
#define compush "@SP\nAM=M+1\nA=A-1\nM=D\n"
//D=index,M=base addr
#define comaddr "A=D+M\nD=M\n"
//R13=addr
#define compop "@SP\nAM=M-1\nD=D+M\nA=D-M\nM=D-A\n"

void vprint( vector <string> v, fstream &fin1)
{
    for(int i=0;i<v.size();++i)
    {
        //cout<<v[i]<<'\n';
        fin1<<v[i]<<'\n';
    }
}

void Pass1 ( vector <string> &v, fstream &fin1)
{
    bool flag=false;
    int compos[2];
    for(int i=0;i<v.size();++i)
    {
        if(flag==true)
        {
            int pos=v[i].size();
            for(int j=0;j<v[i].size()-1;++j)
            {
                if(v[i][j]=='*'&&v[i][j+1]=='/')
                {
                    pos=j+2;
                    flag=false;
                    break;
                }
            }
            v[i].erase(v[i].begin(),v[i].begin()+pos);
        }
        for(int j=0;j<v[i].size();++j)
        {
            if(v[i][j]==' ')
            {
                v[i].erase(v[i].begin()+j);
                j--;
                if(j!=-1&&j!=v[i].size()-1&&v[i][j]!=' '&&v[i][j+1]!=' ')
                {
                  string temp=v[i].substr(j+1,v[i].size()-1);
                  v[i].erase(v[i].begin()+j+1,v[i].end());
                  v.insert(v.begin()+i+1,temp);
                }
            }
            else if(j<v[i].size()-1)
            {
                if(v[i][j]=='/'&&v[i][j+1]=='/')
                {
                    v[i].erase(v[i].begin()+j,v[i].end());
                    break;
                }
                else if(v[i][j]=='/'&&v[i][j+1]=='*')
                {
                    flag=true;
                    compos[0]=i+1;
                    compos[1]=j+1;
                    int pos=v[i].size();
                    for(int k=j+2;k<v[i].size()-1;++k)
                    {
                        if(v[i][k]=='*'&&v[i][k+1]=='/')
                        {
                            pos=k+2;
                            flag=false;
                            break;
                        }
                    }
                    v[i].erase(v[i].begin()+j,v[i].begin()+pos);
                    j--;
                }
            }
        }
      if(v[i]=="")
      {
        v.erase(v.begin()+i);
        i--;
      }
    }
    //cout<<'\n'<<"Assembler Pass 1:\n\n";
    if(flag==true)
    {
      cerr<<'\n'<<compos[0]<<':'<<compos[1]<<": Expected '*/'\n";
      return;
    }
    else
      vprint(v,fin1);
}

void Pass2( vector <string> v, fstream &fin2, char * filename)
{
  int lc1=0,lc2=0,lc3=0,ret=0;
  for(int i=0;i<v.size();++i)
  {
    //Arithmetic
    if(v[i]=="add")
      fin2<<binop<<"M=M+D\n";
    else if(v[i]=="sub")
      fin2<<binop<<"M=M-D\n";
    else if(v[i]=="and")
      fin2<<binop<<"M=M&D\n";
    else if(v[i]=="or")
      fin2<<binop<<"M=M|D\n";
    else if(v[i]=="not")
      fin2<<unop<<"M=!M\n";
    else if(v[i]=="neg")
      fin2<<unop<<"M=-M\n";
    else if(v[i]=="eq")
      fin2<<binop<<"D=M-D\nD=D-1\nM=-1\n@IFEQ_"<<filename<<'_'<<lc1++<<'\n'<<"D+1;JEQ\n@SP\nA=M-1\nM=0\n(IFEQ_"<<filename<<'_'<<lc1-1<<')'<<'\n';
    else if(v[i]=="lt")
      fin2<<binop<<"D=M-D\nD=D-1\nM=-1\n@IFLT_"<<filename<<'_'<<lc2++<<'\n'<<"D+1;JLT\n@SP\nA=M-1\nM=0\n(IFLT_"<<filename<<'_'<<lc2-1<<')'<<'\n';
    else if(v[i]=="gt")
      fin2<<binop<<"D=M-D\nD=D-1\nM=-1\n@IFGT_"<<filename<<'_'<<lc3++<<'\n'<<"D+1;JGT\n@SP\nA=M-1\nM=0\n(IFGT_"<<filename<<'_'<<lc3-1<<')'<<'\n';

    //Memory Access
    else if(v[i]=="push")
    {
      i+=2;
      if(i>=v.size())
      {
        cerr<<'\n'<<i-2<<": Insufficient arguments for "<<v[i-2]<<'\n';
        return;
      }
      if(v[i-1]=="constant")
        fin2<<"@"<<v[i]<<'\n'<<"D=A\n"<<compush;
      else if(v[i-1]=="pointer")
        fin2<<"@"<<v[i]<<'\n'<<"D=A\n@3\nA=A+D\nD=M\n"<<compush;
      else if(v[i-1]=="temp")
        fin2<<"@"<<v[i]<<'\n'<<"D=A\n@5\nA=A+D\nD=M\n"<<compush;
      else if(v[i-1]=="static")
        fin2<<"@"<<filename<<'.'<<v[i]<<'\n'<<"D=M\n"<<compush;
      else if(v[i-1]=="argument")
        fin2<<"@"<<v[i]<<'\n'<<"D=A\n@ARG\n"<<comaddr<<compush;
      else if(v[i-1]=="local")
        fin2<<"@"<<v[i]<<'\n'<<"D=A\n@LCL\n"<<comaddr<<compush;
      else if(v[i-1]=="this")
        fin2<<"@"<<v[i]<<'\n'<<"D=A\n@THIS\n"<<comaddr<<compush;
      else if(v[i-1]=="that")
        fin2<<"@"<<v[i]<<'\n'<<"D=A\n@THAT\n"<<comaddr<<compush;
      else
      {
        cerr<<'\n'<<i-2<<": Invalid segment "<<v[i-1]<<" for "<<v[i]<<'\n';
        return;
      }
    }
    else if(v[i]=="pop")
    {
      i+=2;
      if(i>=v.size())
      {
        cerr<<'\n'<<i-2<<": Insufficient arguments for "<<v[i-2]<<'\n';
        return;
      }
      if(v[i-1]=="pointer")
        fin2<<"@"<<v[i]<<'\n'<<"D=A\n@3\nD=A+D\n"<<compop;
      else if(v[i-1]=="temp")
        fin2<<"@"<<v[i]<<'\n'<<"D=A\n@5\nD=A+D\n"<<compop;
      else if(v[i-1]=="static")
        fin2<<"@SP\nAM=M-1\nD=M\n@"<<filename<<'.'<<v[i]<<'\n'<<"M=D\n";
      else if(v[i-1]=="argument")
        fin2<<"@"<<v[i]<<'\n'<<"D=A\n@ARG\nD=D+M\n"<<compop;
      else if(v[i-1]=="local")
        fin2<<"@"<<v[i]<<'\n'<<"D=A\n@LCL\nD=D+M\n"<<compop;
      else if(v[i-1]=="this")
        fin2<<"@"<<v[i]<<'\n'<<"D=A\n@THIS\nD=D+M\n"<<compop;
      else if(v[i-1]=="that")
        fin2<<"@"<<v[i]<<'\n'<<"D=A\n@THAT\nD=D+M\n"<<compop;
      else
      {
        cerr<<'\n'<<i-2<<": Invalid segment "<<v[i-1]<<" for "<<v[i]<<'\n';
        return;
      }
    }

    //Program Flow
    else if(v[i]=="label")
    {
      i++;
      if(i>=v.size())
      {
        cerr<<'\n'<<i-1<<": Insufficient arguments for "<<v[i-2]<<'\n';
        return;
      }
      fin2<<'('<<filename<<'_'<<v[i]<<')'<<'\n';
    }
    else if(v[i]=="goto")
    {
      i++;
      if(i>=v.size())
      {
        cerr<<'\n'<<i-1<<": Insufficient arguments for "<<v[i]<<'\n';
        return;
      }
      fin2<<'@'<<filename<<'_'<<v[i]<<'\n'<<"0;JMP\n";
    }
    else if(v[i]=="if-goto")
    {
      i++;
      if(i>=v.size())
      {
        cerr<<'\n'<<i-1<<": Insufficient arguments for "<<v[i]<<'\n';
        return;
      }
      fin2<<"@SP\nAM=M-1\nD=M\n@"<<filename<<'_'<<v[i]<<'\n'<<"D;JNE\n";
    }
    //Function call
    else if(v[i]=="function")
    {
      i+=2;
      if(i>=v.size())
      {
        cerr<<'\n'<<i-2<<": Insufficient arguments for "<<v[i-2]<<'\n';
        return;
      }
      fin2<<'('<<v[i-1]<<')'<<'\n'<<'@'<<v[i]<<'\n'<<"D=A\n@SP\nM=M+D\n@"<<filename<<'_'<<v[i-1]<<"_LOOP_COND\n0;JMP\n";
      fin2<<'('<<filename<<'_'<<v[i-1]<<"_LOOP)"<<'\n'<<"@SP\nA=M-D\nM=0\nD=D-1\n"<<'('<<filename<<'_'<<v[i-1]<<"_LOOP_COND)"<<'\n'<<'@'<<filename<<'_'<<v[i-1]<<"_LOOP\nD;JGT\n";
    }
    else if(v[i]=="call")
    {
      i+=2;
      if(i>=v.size())
      {
        cerr<<'\n'<<i-2<<": Insufficient arguments for "<<v[i-2]<<'\n';
        return;
      }
      fin2<<'@'<<filename<<"_RETURN_"<<++ret<<'\n'<<"D=A\n@SP\nAM=M+1\nA=A-1\nM=D\n";
      fin2<<"@LCL\nD=M\n@SP\nAM=M+1\nA=A-1\nM=D\n";
      fin2<<"@ARG\nD=M\n@SP\nAM=M+1\nA=A-1\nM=D\n";
      fin2<<"@THIS\nD=M\n@SP\nAM=M+1\nA=A-1\nM=D\n";
      fin2<<"@THAT\nD=M\n@SP\nAM=M+1\nA=A-1\nM=D\n";
      fin2<<"@SP\nD=M\n@LCL\nM=D\n";
      fin2<<'@'<<v[i]<<'\n'<<"D=D-A\n@5\nD=D-A\n@ARG\nM=D\n";
      fin2<<'@'<<v[i-1]<<'\n'<<"0;JMP\n";
      fin2<<'('<<filename<<"_RETURN_"<<ret<<')'<<'\n';
    }
    else if(v[i]=="return")
    {
      fin2<<"@R13\nD=A\n"<<compop<<"@ARG\nD=M\n@SP\nM=D\n";
      fin2<<"@THAT\nD=A\n@LCL\nAM=M-1\nD=D+M\nA=D-M\nM=D-A\n";
      fin2<<"@THIS\nD=A\n@LCL\nAM=M-1\nD=D+M\nA=D-M\nM=D-A\n";
      fin2<<"@ARG\nD=A\n@LCL\nAM=M-1\nD=D+M\nA=D-M\nM=D-A\n";
      fin2<<"@R14\nD=A\n@LCL\nM=M-1\nAM=M-1\nD=D+M\nA=D-M\nM=D-A\n";
      fin2<<"@LCL\nAM=M+1\nD=M\n@LCL\nM=D\n";
      fin2<< "@R13\nD=M\n@SP\nAM=M+1\nA=A-1\nM=D\n";
      fin2<<"@R14\nA=M\n0;JMP\n";
    }
    else
    {
      cerr<<'\n'<<i<<": Invalid command "<<v[i]<<'\n';
      return;
    }
  }
}

void VM( vector <string> v, fstream &fin1, fstream &fin2, char * filename)
{
    Pass1(v,fin1);
    Pass2(v,fin2,filename);
}

int main(int argc, char* argv[])
{
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);
    cout.tie(NULL);
    string s;
    fstream fin,fin1,fin2;
    fin.open(argv[1],ios::in);
    fin1.open(argv[2],ios::out);
    fin2.open(argv[3],ios::app);
    vector <string> v;
    while(!fin.eof())
    {
        getline(fin,s,'\n');
        v.push_back(s);
    }
    //cout<<"Source Code: \n\n";
    vprint(v,fin);
    VM(v,fin1,fin2,argv[1]);
}
