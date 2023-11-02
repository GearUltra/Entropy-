#include <unistd.h>
#include <iostream>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
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

//Prints out everything 
void print(int x,vector<float> arr,vector<string> set){
  cout<<"CPU "<<x<<endl;
  cout<<"Task scheduling information: ";

  //Loop with string set
  for(int i=0; i<set.size()-2;i=i+2){
    cout<<set[i]<<"("<<set[i+1]<<"), ";
  }
  cout<<set[set.size()-2]<<"("<<set[set.size()-1]<<")"<<endl;

  cout<<"Entropy for CPU "<<x<<endl;

  //Loop with just numbers 
  for(int i=0; i<arr.size();i++){
    cout<<fixed<<setprecision(2)<<abs(arr[i])<<" ";
  }
  cout<<endl;
}

//Converts sever output into float for print 
vector<float> stringToFloat(string x) {
  vector<float> result;
  float f;
  stringstream s;
  s<<x;
  while(s>>f){
    result.push_back(f);
  }
 return result;
}

//input struct
struct in{
  const char* portno;
  const char* severIP;
  string currset;
  //For print 
  vector<string> currsetarr;
  //Server
  vector<float> printH;
};

void* fuc(void* arg){
  string buffer;
  struct sockaddr_in serv_addr;
  struct hostent *server;
  in* arg_ptr= (in*)arg;
  
  int portno = atoi(arg_ptr->portno);
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  buffer = arg_ptr->currset;
  
  //Creating socket
  if (sockfd < 0) {
      std::cerr << "ERROR opening socket" << std::endl;
      exit(0);
  }
  server = gethostbyname(arg_ptr->severIP);

  if (server == NULL) {
      std::cerr << "ERROR, no such host" << std::endl;
      exit(0);
  }
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, 
       (char *)&serv_addr.sin_addr.s_addr,
       server->h_length);
  serv_addr.sin_port = htons(portno);
  
  // Connect to the server
  if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) {
      std::cerr << "ERROR connecting" << std::endl;
      exit(0);
  }

  //Writing to sever
  int msgSize = buffer.size() * sizeof(std::string);
 
  if (write(sockfd,&msgSize,sizeof(int)) < 0) {
      std::cerr << "ERROR writing to socket" << std::endl;
      exit(0);
  }

  if (write(sockfd,buffer.c_str(),msgSize) < 0){
      std::cerr << "ERROR writing to socket" << std::endl;
      exit(0);
  }


 //reading sever 
  if (read(sockfd,&msgSize,sizeof(int)) < 0) {
      std::cerr << "ERROR reading from socket" << std::endl;
      exit(0);
  }
  
  char *tempBuffer = new char[msgSize+1];
  bzero(tempBuffer,msgSize+1);

  if (read(sockfd,tempBuffer,msgSize) < 0) {
      std::cerr << "ERROR reading from socket" << std::endl;
      exit(0);
  }
  //getting sever output
  buffer=tempBuffer;
  
  //converting output to float
  arg_ptr->printH = stringToFloat(buffer);
  
  delete [] tempBuffer;
  
  close(sockfd);
  return nullptr;
}


int main(int argc, char *argv[]){
  vector<string> set;//whole input

  //stuff for input
  stringstream s;
  string line;
  string letter;
  string num;
  
  
  //Getting input 
  while(getline(cin,line)){
    set.push_back(line);
  }
  
  //setting up threads
  int nthreads;
  nthreads=set.size();

  pthread_t tid[nthreads];
  in input[nthreads];
  
  
  //create 
  for(int i=0; i<nthreads;i++){
    s.clear();
    s<<set[i];
    input[i].severIP=argv[1];
    input[i].portno=argv[2];
    input[i].currset=set[i];
    
    vector<string> currset;//Get all of the lines from input

    //Segements set per char
    while(s>>letter>>num){
     //segmented set for print 
     currset.push_back(letter);
     currset.push_back(num);
    }
    input[i].currsetarr=currset;

      if(pthread_create(&tid[i],nullptr, fuc, &input[i])){
        cout<<"Cannot create the thread"<<endl;
        return 1;
      }
  }
  
  //join 
  for(int i=0; i<nthreads;i++){
    pthread_join(tid[i],nullptr);
  }

  //Print 
  for(int i=0; i<nthreads;i++){
    print(i+1,input[i].printH,input[i].currsetarr);
    cout<<endl;
  }
  
    return 0;
}
