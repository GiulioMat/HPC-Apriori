#include <omp.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <map>
#include <algorithm>
using namespace std;

const int MIN_SUPPORT = 2;
const float MIN_CONFIDENCE = 1.;

vector< vector<string> > read_file(char file_name[]);
void find_itemsets(vector<string> matrix, vector<string> candidates, map<string,int> &temp_dictionary, int k, int item_idx, string itemset, int current);
void prune_itemsets(map<string,int> &temp_dictionary, vector<string> &candidates, int min_support);
void update_candidates(vector<string> &candidates, string itemset);
void compute_combinations(int offset, int k, vector<string> &elements, vector<string> &items, vector<string> &combinations);
void generate_association_rules(map<string,int> dictionary, float min_confidence);
string create_consequent(string antecedent, vector<string> items);

// ------------------------------------------------------------
// Main
// ------------------------------------------------------------

int main (){
    char file_name[] = "./prova.txt";
    vector< vector<string> > matrix;
    map<string,int> dictionary;
    map<string,int> temp_dictionary;
    vector<string> candidates;

    // read file into 2D vector matrix
    matrix = read_file(file_name);

    // read matrix and insert 1-itemsets in dictionary as key with their support as value
    for (int i = 0; i < matrix.size(); i++){
        for (int j = 0; j < matrix[i].size(); j++){
            string item = matrix[i][j];

            if(dictionary.count(item)){ // if key exists
                dictionary[item]++;
            }
            else{
                dictionary.insert(pair<string,int>(item, 1));
            }
        }
    }

    // prune from dictionary 1-itemsets with support < min_support and insert items in candidates vector
    prune_itemsets(dictionary, candidates, MIN_SUPPORT);

    // insert in dictionary all k-itemset
    int n = 2; // starting from 2-itemset
    do{
        temp_dictionary.clear();
        // read matrix and insert n-itemsets in temp_dictionary as key with their support as value
        for (int i = 0; i < matrix.size(); i++){
            find_itemsets(matrix[i], candidates, temp_dictionary, n, -1, "", 0);
        }
        // prune from temp_dictionary n-itemsets with support < min_support and insert items in candidates vector
        prune_itemsets(temp_dictionary, candidates, MIN_SUPPORT);
        // append new n-itemsets to main dictionary
        dictionary.insert(temp_dictionary.begin(), temp_dictionary.end());
        n++;
    }while(!temp_dictionary.empty());

    // cout<<"KEY\tVALUE\n";
    // for (map<string, int>::iterator itr = dictionary.begin(); itr != dictionary.end(); ++itr) {
    //     cout << itr->first << '\t' << itr->second << '\n';
    // }

    // print out all association rules with confidence >= min_confidence
    generate_association_rules(dictionary, MIN_CONFIDENCE);


    return 0;
}

// ------------------------------------------------------------
// Functions
// ------------------------------------------------------------

vector< vector<string> > read_file(char file_name[]){
    ifstream myfile (file_name);

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

        sort(row.begin(), row.end());
        matrix.push_back(row);

        ss.clear();
        row.clear();
    }

    myfile.close();

    return matrix;
}

void find_itemsets(vector<string> matrix, vector<string> candidates, map<string,int> &temp_dictionary, int k, int item_idx, string itemset, int current){
    if(current == k){
        itemset = itemset.erase(0,1); // remove first space
        if(temp_dictionary.count(itemset)){ // if key exists
                temp_dictionary[itemset]++;
        }
        else{
            temp_dictionary.insert(pair<string,int>(itemset, 1));
        }
        return;
    }
    
    string item;
    for (int j = ++item_idx; j < matrix.size(); j++){
        item = matrix[j];

        if(find(candidates.begin(), candidates.end(), item) != candidates.end()){
            find_itemsets(matrix, candidates, temp_dictionary, k, j, itemset + " " + item, current+1);
        }
    }
}

void prune_itemsets(map<string,int> &temp_dictionary, vector<string> &candidates, int min_support){
    candidates.clear(); // empty candidates to then update it
    for (map<string, int>::iterator it = temp_dictionary.begin(); it != temp_dictionary.end(); ){
        if (it->second < MIN_SUPPORT){
            temp_dictionary.erase(it++);
        }
        else{
            update_candidates(candidates, it->first);
            ++it;
        }
    }
}

void update_candidates(vector<string> &candidates, string itemset){
    stringstream ss;
    string item;
    ss << itemset;
    while(getline (ss, item, ' ')) {
        if(!(find(candidates.begin(), candidates.end(), item) != candidates.end())){
            candidates.push_back(item);
        }
    }
}

// https://stackoverflow.com/questions/12991758/creating-all-possible-k-combinations-of-n-items-in-c/28698654
void compute_combinations(int offset, int k, vector<string> &elements, vector<string> &items, vector<string> &combinations) {
    if (k == 0){
        string temp;
        for (int i = 0; i < elements.size(); ++i){
            temp += " " + elements[i];
        }
        temp.erase(0,1); // remove first space
        combinations.push_back(temp);
        return;
    }
    for (int i = offset; i <= items.size() - k; ++i) {
        elements.push_back(items[i]);
        compute_combinations(i+1, k-1, elements, items, combinations);
        elements.pop_back();
    }
}

void generate_association_rules(map<string,int> dictionary, float min_confidence){
    stringstream ss;
    vector<string> items;
    vector<string> elements;
    vector<string> combinations;
    string item;
    string antecedent;
    string consequent;
    float conf;

    vector<string>::iterator it_items;
    
    // iterate through all frequent itemsets
    for (map<string, int>::iterator i = dictionary.begin(); i != dictionary.end(); ++i) {
        if(!(i->first.find(' ') != string::npos)) continue; // if itemset has just 1 element continue

        cout<<"---------------"<<endl;
        cout<<"\e[1m"<<i->first<<"\e[0m"<<endl;
        cout<<"---------------"<<endl;

        // insert all items of the itemset into a vector
        ss << i->first;
        while(getline (ss, item, ' ')) {
            items.push_back(item);
        }

        // compute all combinations (which will be the antecedents) of size from k-1 to 1 among items of the itemsets
        for (int idx = items.size()-1; idx>0; idx--){
            compute_combinations(0, idx, elements, items, combinations);
        }

        for (vector<string>::iterator itr = combinations.begin(); itr != combinations.end(); ++itr){
            antecedent = *itr;
            consequent = create_consequent(antecedent, items);

            conf = float(dictionary[i->first])/float(dictionary[antecedent]);
            if(conf >= min_confidence){
                cout<<antecedent<<" -> "<<consequent<<" : "<<conf<<endl;
            }
        }

        ss.clear();
        items.clear();
        combinations.clear();
    }
}

string create_consequent(string antecedent, vector<string> items){
    stringstream ss;
    string item;
    string consequent;
    vector<string>::iterator itr;

    ss << antecedent;

    while(getline (ss, item, ' ')) {
        itr = find(items.begin(), items.end(), item);

        if(itr != items.end()){
            items.erase(itr);
        }
    }
    
    for (itr = items.begin(); itr != items.end(); ++itr){
        consequent += " " + *itr;
    }
    consequent.erase(0,1);

    return consequent;
}