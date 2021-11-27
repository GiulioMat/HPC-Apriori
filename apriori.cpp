#include <omp.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <map>
#include <algorithm>
#include <sys/time.h>
using namespace std;

const float MIN_SUPPORT = 0.5;
const float MIN_CONFIDENCE = 1.;

vector< vector<string> > read_file(char file_name[]);
void find_itemsets(vector<string> matrix, vector<string> candidates, map<string,float> &temp_dictionary, int k, int item_idx, string itemset, int current);
void prune_itemsets(map<string,float> &temp_dictionary, vector<string> &candidates, float min_support);
void update_candidates(vector<string> &candidates, vector<string> temp_candidate_items);
void compute_combinations(int offset, int k, vector<string> &elements, vector<string> &items, vector<string> &combinations);
void generate_association_rules(map<string,float> dictionary, float min_confidence);
string create_consequent(string antecedent, vector<string> items);

// ------------------------------------------------------------
// Main
// ------------------------------------------------------------

int main (){
    char file_name[] = "./prova.txt";
    vector< vector<string> > matrix;
    map<string,float> dictionary;
    map<string,float> temp_dictionary;
    vector<string> candidates;
    int n_rows;

    // read file into 2D vector matrix
    matrix = read_file(file_name);
    n_rows = matrix.size();

    struct timeval start, end;
    gettimeofday(&start, NULL);

    // read matrix and insert 1-itemsets in dictionary as key with their frequency as value
    for (int i = 0; i < matrix.size(); i++){
        for (int j = 0; j < matrix[i].size(); j++){
            string item = matrix[i][j];

            if(dictionary.count(item)){ // if key exists
                dictionary[item]++;
            }
            else{
                dictionary.insert(pair<string,float>(item, 1));
            }
        }
    }

    // divide frequency by number of rows to calculate support
    for (map<string, float>::iterator i = dictionary.begin(); i != dictionary.end(); ++i) {
        i->second = i->second/float(n_rows);
    }

    // prune from dictionary 1-itemsets with support < min_support and insert items in candidates vector
    prune_itemsets(dictionary, candidates, MIN_SUPPORT);

    // insert in dictionary all k-itemset
    int n = 2; // starting from 2-itemset
    do{
        temp_dictionary.clear();
        // read matrix and insert n-itemsets in temp_dictionary as key with their frequency as value
        for (int i = 0; i < matrix.size(); i++){
            find_itemsets(matrix[i], candidates, temp_dictionary, n, -1, "", 0);
        }
        // divide frequency by number of rows to calculate support
        for (map<string, float>::iterator i = temp_dictionary.begin(); i != temp_dictionary.end(); ++i) {
            i->second = i->second/float(n_rows);
        }
        // prune from temp_dictionary n-itemsets with support < min_support and insert items in candidates vector
        prune_itemsets(temp_dictionary, candidates, MIN_SUPPORT);
        // append new n-itemsets to main dictionary
        dictionary.insert(temp_dictionary.begin(), temp_dictionary.end());
        n++;
    }while(!temp_dictionary.empty());

    gettimeofday(&end, NULL);

    double elapsed = (end.tv_sec - start.tv_sec) + 
              ((end.tv_usec - start.tv_usec)/1000000.0);
    cout<<"Time passed: "<<elapsed<<endl;

    cout<<"KEY\tVALUE\n";
    for (map<string, float>::iterator itr = dictionary.begin(); itr != dictionary.end(); ++itr) {
        cout << itr->first << '\t' << itr->second << '\n';
    }

    // print out all association rules with confidence >= min_confidence
    // generate_association_rules(dictionary, MIN_CONFIDENCE);

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

void find_itemsets(vector<string> matrix, vector<string> candidates, map<string,float> &temp_dictionary, int k, int item_idx, string itemset, int current){
    if(current == k){
        itemset = itemset.erase(0,1); // remove first space

        // if itemset is a candidate insert it into temp_dictionary to calculate support 
        if(find(candidates.begin(), candidates.end(), itemset) != candidates.end()){
            if(temp_dictionary.count(itemset)){ // if key exists
                temp_dictionary[itemset]++;
            }
            else{
                temp_dictionary.insert(pair<string,float>(itemset, 1));
            }
            return;
        }
        // if itemset is not a candidate discard it
        else{
            return;
        } 
    }
    
    string item;
    for (int j = ++item_idx; j < matrix.size(); j++){
        item = matrix[j];

        find_itemsets(matrix, candidates, temp_dictionary, k, j, itemset + " " + item, current+1);
    }
}

void prune_itemsets(map<string,float> &temp_dictionary, vector<string> &candidates, float min_support){
    vector<string> temp_candidate_items;
    candidates.clear(); // empty candidates to then update it
    for (map<string, float>::iterator it = temp_dictionary.begin(); it != temp_dictionary.end(); ){ // like a while
        if (it->second < MIN_SUPPORT){
            temp_dictionary.erase(it++);
        }
        else{
            temp_candidate_items.push_back(it->first);
            ++it;
        }
        if(it == temp_dictionary.end()){
            if(!temp_dictionary.empty()){
                update_candidates(candidates, temp_candidate_items);
            }
        }
    }
}

void update_candidates(vector<string> &candidates, vector<string> temp_candidate_items){
    string item;
    stringstream to_combine;
    vector<string> items;
    vector<string> elements;

    int common_items;

    for(int i = 0; i < temp_candidate_items.size()-1; i++){
        for(int j = i+1; j < temp_candidate_items.size(); j++){
            common_items = 0;
            items.clear();
            to_combine.clear();

            to_combine << temp_candidate_items[i] + ' ' + temp_candidate_items[j];
            while(getline (to_combine, item, ' ')) {
                if(!(find(items.begin(), items.end(), item) != items.end())){
                    items.push_back(item);
                }
                else{
                    common_items++;
                }
            }

            // if the statement is true than we can create combinations as candidates
            // else we created all correct combianations and we pass to the next itemset 
            if(common_items == items.size()-2){
                compute_combinations(0, items.size(), elements, items, candidates);
            }
            else{
                break;
            }
            
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

void generate_association_rules(map<string,float> dictionary, float min_confidence){
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
    for (map<string, float>::iterator i = dictionary.begin(); i != dictionary.end(); ++i) {
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