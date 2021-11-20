#include <omp.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
using namespace std;

int main (){
    ifstream myfile ("../prova.txt");

    vector< vector<string> > matrix;
    vector<string> row;  
    
    string line;
    stringstream ss;
    string item;

    while(getline (myfile, line)){
        ss << line;

        while(getline (ss, item, ' ')) {
            row.push_back(item);
        }

        matrix.push_back(row);

        ss.clear();
        row.clear();
    }


    myfile.close();
    

    for (int i = 0; i < matrix.size(); i++){
        for (int j = 0; j < matrix[i].size(); j++){
            cout << matrix[i][j]<<' ';
        }
        cout << "\n";
    }


    return 0;
}