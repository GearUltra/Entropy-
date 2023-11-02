#include <unistd.h>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cmath> 
#include <map>
#include <vector>
#include <sys/wait.h>
#include <sstream>
using namespace std;

//Input Struct
struct in{
  float currFreq=0;
  float currh=0;
  float extrafreq=0;
  map<string,float> freq;

  string charnow;//The new var
  vector<string> currset;//Set[i] segemented 
  vector<float> printH;//set of h for print 
};

//Output Struct
struct out{
  float h=0;
  float Nfreq=0;
};

//Caluclates h
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

//Iterating the currset 
void* work(void* arg){
  in* input = static_cast<in*>(arg);
  out out;

  for(int i=0;i<input->currset.size();i=i+2){
      input->extrafreq=stoi(input->currset[i+1]);
      input->currh=out.h;
      input->currFreq =out.Nfreq;

      out=entropy(*input,input->currset[i]);
      input->printH.push_back(out.h);
      input->freq[input->currset[i]]+=input->extrafreq;
    }

  return nullptr;
}

//Deals with while(true)
void fireman(int){
   while (waitpid(-1, NULL, WNOHANG) > 0)
    ;
}

//changes the h vector back into string for client 
std::string vectostring(const std::vector<float>& floatVector) {
    std::stringstream ss;
    // Iterate through the vector and convert each float to a string with a space
    for (const float& value : floatVector) {
        ss << value << " ";
    }
    // Convert the stringstream to a string
    std::string result = ss.str();

    return result;
}

int main(int argc, char *argv[])
{
   int sockfd, newsockfd, portno, clilen;
   struct sockaddr_in serv_addr, cli_addr;

   // Check the commandline arguments
   if (argc != 2){
      std::cerr << "Port not provided" << std::endl;
      exit(0);
   }

   // Create the socket
   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   if (sockfd < 0) {
      std::cerr << "Error opening socket" << std::endl;
      exit(0);
   }

   // Populate the sockaddr_in structure
   memset(&serv_addr, 0, sizeof(serv_addr));

   portno = atoi(argv[1]);
   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr.s_addr = INADDR_ANY;
   serv_addr.sin_port = htons(portno);

   // Bind the socket with the sockaddr_in structure
   if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
      std::cerr << "Error binding" << std::endl;
      exit(0);
   }

   // Set the max number of concurrent connections
   listen(sockfd, 5);
   clilen = sizeof(cli_addr);

   // Accept a new connection
   signal(SIGCHLD, fireman);
   while(true) {
     newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, (socklen_t *)&clilen);
     
     if(fork() == 0){
       if (newsockfd < 0){
          std::cerr << "Error accepting new connections" << std::endl;
          exit(0);
       }
       int n, msgSize = 0;
       n = read(newsockfd, &msgSize, sizeof(int));
       if (n < 0){
          std::cerr << "Error reading from socket" << std::endl;
          exit(0);
       }
       char *tempBuffer = new char[msgSize + 1];
       memset(tempBuffer, 0, msgSize + 1);
    
       n = read(newsockfd, tempBuffer, msgSize + 1);
       if (n < 0){
          std::cerr << "Error reading from socket" << std::endl;
          exit(0);
       }
       
       // Convert the received data string to vector
       string buffer=tempBuffer;
       
       vector<string> segmentbuf;
       in input;
       work(&input);
       string newbuffer=vectostring(input.printH);
       
       stringstream s;
       string letter, num;
       s.clear();
       s<<buffer;
       while(s>>letter>>num){
           //segmented set
          segmentbuf.push_back(letter);
          segmentbuf.push_back(num);

           //Null map
           input.freq.insert({letter,0});
          }
       input.currset=segmentbuf;
       
       //puts input to do all the calculations
       
       
       //sets output to client back into string 
       
       delete[] tempBuffer;
       
       msgSize = newbuffer.size();
       n = write(newsockfd, &msgSize, sizeof(int));
       if (n < 0){
          std::cerr << "Error writing to socket" << std::endl;
          exit(0);
       }
       n = write(newsockfd, newbuffer.c_str(), msgSize);
       if (n < 0){
          std::cerr << "Error writing to socket" << std::endl;
          exit(0);
       }
       exit(0);
        }
     wait(0);
   } 
   close(newsockfd);
   close(sockfd);
  
   return 0;
}

}
