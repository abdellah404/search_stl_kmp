#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>
#include <filesystem>
#include <stdexcept>
#include <algorithm>
using namespace std;
namespace fs = std::filesystem;

string readFile(const string& file) {
    ifstream in(file, ios::binary);
    if (!in) throw runtime_error("Impossible d'ouvrir " + file);
    return string(istreambuf_iterator<char>(in), istreambuf_iterator<char>());
}
// Ecrit d'abord dans un fichier temporaire du meme dossier, puis remplace l'original.
void overwriteFile(const string& file, const string& data) {
    fs::path target(file);
    fs::path temp = target;
    temp += ".tmp";
    {
        ofstream out(temp, ios::binary | ios::trunc);
        if (!out || !out.write(data.data(), static_cast<streamsize>(data.size())))
            throw runtime_error("Erreur d'ecriture temporaire " + temp.string());
        out.close();
        if (!out) throw runtime_error("Erreur de fermeture " + temp.string());
    }
    fs::rename(temp, target);
}
vector<size_t> lpsOf(const string& p) {
    vector<size_t> lps(p.size(),0);
    for (size_t i=1,len=0;i<p.size();) {
        if (p[i]==p[len]) lps[i++]=++len;
        else if (len) len=lps[len-1];
        else lps[i++]=0;
    }
    return lps;
}
vector<size_t> kmp(const string& s,const string& p) {
    vector<size_t> positions;
    if (p.empty()) return positions;
    auto lps=lpsOf(p);
    for(size_t i=0,j=0;i<s.size();) {
        if(s[i]==p[j]) { ++i; ++j; }
        if(j==p.size()) { positions.push_back(i-j); j=lps[j-1]; }
        else if(i<s.size() && s[i]!=p[j]) {
            if(j) j=lps[j-1]; else ++i;
        }
    }
    return positions;
}
vector<size_t> findSTL(const string& s,const string& p) {
    vector<size_t> positions;
    if(p.empty()) return positions;
    size_t pos=0;
    while((pos=s.find(p,pos))!=string::npos) {
        positions.push_back(pos);
        ++pos;
    }
    return positions;
}
// Remplacement non chevauchant : toutes les occurrences dans le texte ORIGINAL.
string buildReplacement(const string& s,const string& p,const string& r,const vector<size_t>& positions,size_t& count) {
    string out;
    out.reserve(s.size());
    size_t last=0;
    count=0;
    for(size_t pos:positions) {
        if(pos<last) continue;
        out.append(s,last,pos-last);
        out+=r;
        last=pos+p.size();
        ++count;
    }
    out.append(s,last,string::npos);
    return out;
}
string replaceKMP(const string& s,const string& p,const string& r,size_t& count) {
    return buildReplacement(s,p,r,kmp(s,p),count);
}
string replaceSTL(const string& s,const string& p,const string& r,size_t& count) {
    return buildReplacement(s,p,r,findSTL(s,p),count);
}
void benchmarkSearch(const string& s,const string& p) {
    using clock=chrono::steady_clock;
    constexpr int rounds=5;
    cout << "\nBenchmark RECHERCHE (" << rounds << " repetitions):\n";
    for(int method=0;method<2;++method) {
        double total=0;
        size_t count=0;
        for(int i=0;i<rounds;++i) {
            auto start=clock::now();
            auto positions=method==0?kmp(s,p):findSTL(s,p);
            auto stop=clock::now();
            count=positions.size();
            total+=chrono::duration<double,milli>(stop-start).count();
        }
        cout<<(method==0?"KMP":"STL find")<<" : "<<total/rounds<<" ms | "<<count<<" occurrences\n";
    }
    cout<<"Lecture, ecriture et affichage exclus.\n";
}
void benchmarkReplacement(const string& s,const string& p,const string& r) {
    using clock=chrono::steady_clock;
    constexpr int rounds=5;
    cout << "\nBenchmark REMPLACEMENT (" << rounds << " repetitions):\n";
    string reference;
    size_t referenceCount=0;
    for(int method=0;method<2;++method) {
        double total=0;
        size_t count=0;
        for(int i=0;i<rounds;++i) {
            auto start=clock::now();
            string result=method==0?replaceKMP(s,p,r,count):replaceSTL(s,p,r,count);
            auto stop=clock::now();
            total+=chrono::duration<double,milli>(stop-start).count();
            if(method==0 && i==0) { reference=result; referenceCount=count; }
            else if(result!=reference || count!=referenceCount)
                throw runtime_error("Resultats differents entre KMP et STL !");
        }
        cout<<(method==0?"KMP":"STL find")<<" : "<<total/rounds
            <<" ms | "<<count<<" remplacements\n";
    }
    cout<<"Recherche + construction du nouveau texte mesurees; E/S disque exclues.\n";
}
int main(int argc,char** argv) {
    try {
        const string input=(argc>1)?argv[1]:"texte.txt";
        const string texte=readFile(input);
        cout<<"Fichier: "<<input<<" ("<<texte.size()<<" octets)\n";
        cout<<"1. Rechercher une sous-chaine\n2. Remplacer une sous-chaine\nChoix: ";
        string choice,motif,repl;
        getline(cin,choice);
        if(choice!="1" && choice!="2") { cerr<<"Choix invalide\n"; return 1; }
        cout<<"Sous-chaine: "; getline(cin,motif);
        if(motif.empty()) { cerr<<"Sous-chaine vide interdite\n"; return 1; }
        if(choice=="1") {
            auto positions=kmp(texte,motif);
            cout<<"Occurrences: "<<positions.size()<<"\n";
            cout<<"10 premieres positions (index 0): ";
            for(size_t i=0;i<min(size_t(10),positions.size());++i) cout<<positions[i]<<" ";
            cout<<"\n";
            istringstream words(texte);
            ofstream found("mots_trouves.txt");
            if(!found) throw runtime_error("Impossible de creer mots_trouves.txt");
            string word;
            size_t wordCount=0;
            while(words>>word) {
                if(word.find(motif)!=string::npos) {
                    found<<word<<'\n';
                    ++wordCount;
                    if(wordCount<=10) cout<<"Mot: "<<word<<'\n';
                }
            }
            cout<<"Mots correspondants: "<<wordCount<<" (liste complete: mots_trouves.txt)\n";
            benchmarkSearch(texte,motif);
        } else {
            cout<<"Remplacer par: "; getline(cin,repl);
            benchmarkReplacement(texte,motif,repl);
            size_t count=0;
            const string output=replaceKMP(texte,motif,repl,count);
            overwriteFile(input,output);
            cout<<count<<" remplacements dans le fichier original: "<<input<<"\n";
        }
    } catch(const exception& e) { cerr<<e.what()<<'\n'; return 1; }
}
