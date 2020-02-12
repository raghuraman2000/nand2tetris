/*

Author : R Raghu Raman
This is a compiler for the JACK programming Language
Usage:
./HACK_Assembler <file1.asm> <file1.asmir> <file1.hack>
Output:
<file1.asmir> <file1.hack>

*/
#include<bits/stdc++.h>

using namespace std;

void vprint( vector <string> v, fstream &fin1)
{
    for(int i=0;i<v.size();++i)
    {
        //cout<<v[i]<<'\n';
        fin1<<v[i]<<'\n';
    }
}

void mprint ( map <string,int> list)
{
    map <string,int> ::iterator it=list.begin();
    //cout<<'\n'<<"Symbol Table:"<<'\n'<<'\n';
    //for(;it!=list.end();++it)
    //    cout<<it->first<<' '<<it->second<<'\n';
}

void init_Symbol_Table ( map <string,int> &list)
{
    list["SP"]=0;
    list["LCL"]=1;
    list["ARG"]=2;
    list["THIS"]=3;
    list["THAT"]=4;
    list["R0"]=0;
    list["R1"]=1;
    list["R2"]=2;
    list["R3"]=3;
    list["R4"]=4;
    list["R5"]=5;
    list["R6"]=6;
    list["R7"]=7;
    list["R8"]=8;
    list["R9"]=9;
    list["R10"]=10;
    list["R11"]=11;
    list["R12"]=12;
    list["R13"]=13;
    list["R14"]=14;
    list["R15"]=15;
    list["KBD"]=0x6000;
    list["SCREEN"]=0x4000;
}

void binaryconv( int val, string &w)
{
    for(int i=15;i>=1;--i)
    {
        w[i]=(char)('0'+(val%2));
        val/=2;
    }
}

string dectostring(map <string,int> &list,string u, int val, int &top)
{
    string s;
    if(val==-1)
    {
        val=top;
        list[u]=top;
        top++;
    }
    while(val>0)
    {
        s=(char)('0'+(val%10))+s;
        val/=10;
    }
    return s;
}

bool ALU( string &w, string exp)
{
    for(int i=0;i<exp.size();++i)
    {
        if(exp[i]=='M')
        {
            w[3]='1';
            exp[i]='A';
        }
    }
    string c;
    c.resize(6);
    if(exp=="0")
        c="101010";
    else if(exp=="1")
        c="111111";
    else if(exp=="-1")
        c="111010";
    else if(exp=="D")
        c="001100";
    else if(exp=="A")
        c="110000";
    else if(exp=="!D")
        c="001101";
    else if(exp=="!A")
        c="110001";
    else if(exp=="-D")
        c="001111";
    else if(exp=="-A")
        c="110011";
    else if(exp=="D+1"||exp=="1+D")
        c="011111";
    else if(exp=="A+1"||exp=="1+A")
        c="110111";
    else if(exp=="D-1")
        c="001110";
    else if(exp=="A-1")
        c="110010";
    else if(exp=="D+A"||exp=="A+D")
        c="000010";
    else if(exp=="D-A")
        c="010011";
    else if(exp=="A-D")
        c="000111";
    else if(exp=="A&D"||exp=="D&A")
        c="000000";
    else if(exp=="A|D"||exp=="D|A")
        c="010101";
    else
        return false;
    for(int i=0;i<6;++i)
        w[4+i]=c[i];
    return true;
}

void Pass1 ( vector <string> &v, map <string,int> &Symbol_Table, fstream &fin1)
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
        if(v[i][0]=='@'&&(!isdigit(v[i][1])))
        {
            string t=v[i].substr(1,v[i].size()-1);
            if(Symbol_Table.count(t)==0)
             Symbol_Table.insert(make_pair(t,-1));
        }
        else if(v[i][0]=='(')
        {
            int closebrac=0;
            for(;closebrac<v[i].size();++closebrac)
                if(v[i][closebrac]==')')
                    break;
            if(closebrac==v[i].size())
            {
                cerr<<'\n'<<i+1<<':'<<1<<": Expected ')'"<<'\n';
                return;
            }
            string t=v[i].substr(1,closebrac-1);
            if(Symbol_Table.count(t)==0)
                Symbol_Table.insert(make_pair(t,i));
            else
                Symbol_Table[t]=i;
            v[i].erase(v[i].begin(),v[i].begin()+closebrac+1);
        }
        if(v[i]=="")
        {
            v.erase(v.begin()+i);
            i--;
        }
    }
    //cout<<'\n'<<"Assembler Pass 1:"<<'\n'<<'\n';
    if(flag==true)
    {
        cerr<<'\n'<<compos[0]<<':'<<compos[1]<<": Expected '*/'"<<'\n';
        return;
    }
    else
    {
        vprint(v,fin1);
        mprint(Symbol_Table);
    }
}

void Pass2( vector <string> v, map <string,int> &Symbol_Table, fstream &fin2)
{
    vector <string> ans;
    int top=16;
    string w;
    w.resize(16);
    //cout<<'\n'<<"Assembler Pass 2:"<<'\n'<<'\n';
    for(int i=0;i<v.size();++i)
    {
        if(v[i][0]=='@')
        {
            string u=v[i].substr(1,v[i].size()-1);
            if(!isdigit(u[0]))
                u=dectostring(Symbol_Table,u,Symbol_Table[u],top);
            else
            {
                bool flag=true;
                for(int j=0;j<u.size();++j)
                    flag&=isdigit(u[j]);
                if(flag==false)
                {
                    cerr<<'\n'<<i+1<<':'<<2<<": Invalid Address "<<u<<'\n';
                    return;
                }
            }
            int val=0;
            for(int j=0;j<u.size();++j)
            {
                val*=10;
                val+=(int)(u[j]-'0');
            }
            w[0]='0';
            binaryconv(val,w);
            ans.push_back(w);
            //cout<<val<<'\n';
            //cout<<w<<'\n';
            fin2<<w<<'\n';
        }
        else
        {
            w[0]=w[1]=w[2]='1';
            for(int j=3;j<16;++j)
                w[j]='0';
            bool assign=false,semicolon=false;
            int pos=0;
            for(int j=0;j<v[i].size();++j)
            {
                if(v[i][j]=='=')
                {
                    assign=true;
                    pos=j;
                }
                else if(v[i][j]==';')
                {
                    semicolon=true;
                    pos=j;
                }
            }
            if(assign==true)
            {
                if(semicolon==true)
                {
                    cerr<<'\n'<<i+1<<':'<<pos+1<<": Unexpected "<<v[i][pos]<<'\n';
                    return;
                }
                else
                {
                    if(pos==0)
                    {
                        cerr<<'\n'<<i+1<<':'<<pos+1<<": Expected before '=' "<<'\n';
                        return;
                    }
                    else if(pos==v[i].size()-1)
                    {
                        cerr<<'\n'<<i+1<<':'<<pos+1<<": Expected after '=' "<<'\n';
                        return;
                    }
                    string u=v[i].substr(0,pos),t=v[i].substr(pos+1,v[i].size()-pos);
                    //cout<<u<<' '<<t<<'\n';
                    if(u=="D")
                        w[11]='1';
                    else if(u=="M")
                        w[12]='1';
                    else if(u=="A")
                        w[10]='1';
                    else if(u=="DM"||u=="MD")
                        w[11]=w[12]='1';
                    else if(u=="DA"||u=="AD")
                        w[11]=w[10]='1';
                    else if(u=="AM"||u=="MA")
                        w[12]=w[10]='1';
                    else if(u=="ADM"||u=="AMD"||u=="DMA"||u=="DAM"||u=="MAD"||u=="MDA")
                        w[10]=w[11]=w[12]='1';
                    else if(u=="null")
                        ;
                    else
                    {
                        cerr<<'\n'<<i+1<<':'<<1<<": Invalid input before '=' "<<'\n';
                        return;
                    }
                    if(ALU(w,t)==false)
                    {
                        cerr<<'\n'<<i+1<<':'<<pos+1<<": Invalid Expression after '=' "<<'\n';
                        return;
                    }
                }
            }
            else
            {
                if(semicolon==true)
                {
                    if(pos==0)
                    {
                        cerr<<'\n'<<i+1<<':'<<pos+1<<": Expected before ';' "<<'\n';
                        return;
                    }
                    else if(pos==v[i].size()-1)
                    {
                        cerr<<'\n'<<i+1<<':'<<pos+1<<": Expected after ';' "<<'\n';
                        return;
                    }
                    string t=v[i].substr(0,pos),u=v[i].substr(pos+1,v[i].size()-pos);
                    //cout<<u<<' '<<t<<'\n';
                    if(u=="JGT")
                        w[15]='1';
                    else if(u=="JEQ")
                        w[14]='1';
                    else if(u=="JGE")
                        w[14]=w[15]='1';
                    else if(u=="JLT")
                        w[13]='1';
                    else if(u=="JNE")
                        w[13]=w[15]='1';
                    else if(u=="JLE")
                        w[13]=w[14]='1';
                    else if(u=="JMP")
                        w[13]=w[14]=w[15]='1';
                    else if(u=="null")
                        ;
                    else
                    {
                        cerr<<'\n'<<i+1<<':'<<1<<": Invalid input after ';' "<<'\n';
                        return;
                    }
                    if(ALU(w,t)==false)
                    {
                        cerr<<'\n'<<i+1<<':'<<1<<": Invalid Expression before ';' "<<'\n';
                        return;
                    }
                }
                else
                {
                    cerr<<'\n'<<i+1<<':'<<pos+1<<": Invalid Expression "<<'\n';
                    return;
                }
            }
            ans.push_back(w);
            //cout<<w<<'\n';
            fin2<<w<<'\n';
        }
    }
    mprint(Symbol_Table);
}

void Assemble( vector <string> v, fstream &fin1, fstream &fin2)
{
    map <string,int> Symbol_Table;
    init_Symbol_Table(Symbol_Table);
    Pass1(v,Symbol_Table,fin1);
    Pass2(v,Symbol_Table,fin2);
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
    fin2.open(argv[3],ios::out);
    vector <string> v;
    while(!fin.eof())
    {
        getline(fin,s,'\n');
        v.push_back(s);
    }
    //cout<<"Source Code: "<<'\n'<<'\n';
    vprint(v,fin);
    Assemble(v,fin1,fin2);
}
