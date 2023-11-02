#include <iostream>
#include <sstream>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>     
#include <cmath> 
#include <iomanip>
#include <vector>
#include <map>


#include <fstream>
using namespace std;

//Input struct
struct in{
  float currFreq=0;
  float currh=0;
  float extrafreq=0;
  map<string,float> freq;
};

//Output Struct
struct out{
  float h=0;
  float Nfreq=0;
};

//Print change set vector to map 
void print(int x,vector<float> arr,vector<string> set){
  cout<<"CPU "<<x<<endl;
  cout<<"Task scheduling information: ";
  
  for(int i=0; i<set.size()-2;i=i+2){
    cout<<set[i]<<"("<<set[i+1]<<"), ";
  }
  cout<<set[set.size()-2]<<"("<<set[set.size()-1]<<")"<<endl;
  
  cout<<"Entropy for CPU "<<x<<endl;
  for(int i=0; i<arr.size();i++){
    cout<<fixed<<setprecision(2)<<abs(arr[i])<<" ";
  }
  cout<<endl;
}

//Should work now just need to change main
out entropy(in input, string i){
  out output;
  float newTerm=0,currTerm=0;
  

  output.Nfreq=input.currFreq +input.extrafreq;

  
  if(output.Nfreq==input.extrafreq){output.h=0;}
    
  else{
    
    if(input.freq[i]==0){currTerm=0;}

    
    else{currTerm = input.freq[i] * log2(input.freq[i]);}
    
  //NewTerm  
  newTerm =(input.freq[i] +input.extrafreq ) * log2(input.freq[i]+input.extrafreq);
    
  //H 
  output.h=log2(output.Nfreq) - ((log2 (input.currFreq)-input.currh)*(input.currFreq)-currTerm+newTerm)/output.Nfreq;

   
  }
  
  
  return output;
}

int main() {
  in input;
  out out;
  
  vector<string> set;//whole input
  vector<string> currset;//curr input segmented

  vector<float> printH;//set of h for print 

  //stuff for input
  stringstream s;
  string line;
  string letter;
  string num;
  
  fstream fin("t4.txt");

  //Get all of the lines from input
  while(getline(fin,line)){
    set.push_back(line);
  }

  
  //Segment each line one by one
  for(int j=0; j<set.size();j++){
    
    s.clear();
    s<<set[j];
    while(s>>letter>>num){
     //segmented set
     currset.push_back(letter);
     currset.push_back(num);
     //Null map
     input.freq.insert({letter,0});
    }
    
  
    
    //entropy loop 
    for(int i=0;i<currset.size();i=i+2){
      input.extrafreq=stoi(currset[i+1]);
      input.currh=out.h;
      input.currFreq =out.Nfreq;
      
      out=entropy(input,currset[i]);
      printH.push_back(out.h);
      input.freq[currset[i]]+=input.extrafreq;
    }
    
    //Printing everything 
    print(j+1,printH,currset);
    cout<<endl;

    
    //Clearing/setting everything back to 0
    input.currFreq=0;
    input.currh=0;
    input.extrafreq=0;
    out.h=0;
    out.Nfreq=0;
    input.freq.clear();
    currset.clear();
    printH.clear();
    
  }
  

  return 0;
}
