#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;
int main(int argc, char *argv[]) {

        ofstream myfile;
        myfile.open ("test.txt", std::ofstream::out | std::ofstream::trunc);
        if (argc >= 2) {
                std::istringstream iss( argv[1] );
                int val;

                if (iss >> val){
                        for (int i = 2 ; i <= val; i++){
                                if (i == val){
                                        myfile<< i;
                                        break; 
                                }
                                myfile<< i << "\n";
                        }
                }
        }
        myfile.close();
        return 0;
}