#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <math.h>
#include <chrono>

#define BLOCK_SIZE  85 //bytes
#define DATA_RECORD_SIZE 17 //bytes
#define INDEX_RECORD_SIZE 8 //4 + 4 bytes (two integers)
using namespace std;


//the file directory into the filePath (file should be comma seperated)
string filePath = "records.txt";
ifstream fin(filePath);
ofstream fout("indexing.txt");

class Record {

private:

    int id;
    string name;
    int salary;

public:

    Record(int id, string name, int salary) {
        this->id = id;
        this->name = name;
        this->salary = salary;
    }

    int getID() const {
        return id;
    }

    void print() const {
        fout << id << ", " << name << ", " << salary << ". " << endl;
    }
};

class Block {

public:
    vector<Record> records;
    void addRecord(Record R) {
        records.push_back(R);
    }

    void print() const {

        for (Record r : records)
            r.print();

        fout << "---------" << endl;
    }



};


class IndexRecord {

private:
    int value;
    int pointer;

public:

    IndexRecord(int value, int pointer) {
        this->value = value;
        this->pointer = pointer;
    }
    void print() {
        fout << value << ", " << pointer << endl;
    }
    int getVal() const {
        return value;
    }
    int getPointer() const {
        return pointer;
    }
};

class BlockIndex {

public:
    vector<IndexRecord> indexRecords;

    void addRecord(IndexRecord R) {
        indexRecords.push_back(R);
    }

    void print() {
        for (IndexRecord r : indexRecords)
            r.print();
        fout << "-------" << endl;
    }
};

vector<string> splitString(const string& input, char delimiter) {
    vector<string> tokens;
    stringstream ss(input);
    string token;

    while (getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }

    return tokens;
}
Record findLinear(vector<Record> records, int id);
Record findSingle(const vector<vector<BlockIndex>>& levels, int id, const vector<Block>& dataFile);
Record find(const vector<vector<BlockIndex>>& levels, int id, const vector<Block>& dataFile);
int main() {
    // filling records vector from the file:


    vector<Record> records;

    string line;
    while (getline(fin, line)) {
        vector<string> tempVec = splitString(line, ',');

        if (tempVec.size() == 3) {
            Record r(stoi(tempVec[0]), tempVec[1], stoi(tempVec[2]));
            records.push_back(r);
        }
    }

    fin.close();

    /*for (const Record& r : records) {
       r.print();
    }*/


    // data file calculations
    int df_Size = records.size() * DATA_RECORD_SIZE;
    int df_RecordsPerBlock = BLOCK_SIZE / DATA_RECORD_SIZE; // 85/17=5
    int df_Blocks = ceil((float)records.size() / df_RecordsPerBlock);

    vector<Block> dataFile;

    fout << df_Blocks << endl;

    int j = 0;
    for (int i = 0; i < df_Blocks; i++) {

        Block b;

        for (int counter = 0; counter < df_RecordsPerBlock && j < records.size(); counter++) {
            b.addRecord(records[j]);
            j++;
        }

        dataFile.push_back(b);
    }

    // for(Block b : blocks)
    //     b.print();


    // indexing file calculations:
    int if_RecordsPerBlock = BLOCK_SIZE / INDEX_RECORD_SIZE; //10.625 = 10
    int if_Blocks = ceil((float)df_Blocks / if_RecordsPerBlock);


    vector<BlockIndex> indexBlocks;

    int s = 0;
    for (int i = 0; i < if_Blocks; i++) {

        BlockIndex b;

        for (int counter = 0; s < df_Blocks && counter < if_RecordsPerBlock; counter++) {

            int value = dataFile[s].records[0].getID();
            int pointer = s;

            IndexRecord tempIndexRecord(value, pointer);
            b.addRecord(tempIndexRecord);

            s++;

        }

        indexBlocks.push_back(b);
    }

    /*for (BlockIndex b : indexBlocks)
        b.print();*/


    int tempNoBlocks = if_Blocks;
    vector<vector<BlockIndex>> levels;
    levels.push_back(indexBlocks);

    int k = 0;
    bool isOne = false;

    while (tempNoBlocks >= 1 && !isOne) {

        tempNoBlocks = ceil((float)tempNoBlocks / if_RecordsPerBlock);
        if (tempNoBlocks == 1) isOne = true;

        vector<BlockIndex> currentLevel;
        int s = 0;
        for (int i = 0; i < tempNoBlocks; i++) {

            BlockIndex b;

            for (int counter = 0; s < levels[k].size() && counter < if_RecordsPerBlock; counter++) {

                int value = levels[k][s].indexRecords[0].getVal();
                int pointer = s;

                IndexRecord tempIndexRecord(value, pointer);
                b.addRecord(tempIndexRecord);

                s++;
            }
            currentLevel.push_back(b);
        }
        k++;
        levels.push_back(currentLevel);
    }

    int i = 0;
    for (vector<BlockIndex> b : levels) {
        fout << "level " << i << endl;
        for (BlockIndex b2 : b) {
            b2.print();
        }
        fout << "\n\n\n";
        i++;
    }

    int id_to_search;
    cin >> id_to_search;
    fout << "Results for searching for record with id = "<< id_to_search << ", file with " << records.size() << " records: " << endl;

    // linear search directly from the data file
    auto start3 = std::chrono::high_resolution_clock::now();
    Record r3 = findLinear(records, id_to_search);
    auto end3 = std::chrono::high_resolution_clock::now();
    auto duration3 = std::chrono::duration_cast<std::chrono::microseconds>(end3 - start3);
    fout << "\n\nExecution time linear search directly from data file: " << duration3.count() << " microseconds" << endl;
    fout << "\n\n";
    r3.print();

    // single level search
    auto start = std::chrono::high_resolution_clock::now(); 
    Record r2 = findSingle(levels, id_to_search, dataFile);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    fout << "\n\nExecution time for single level search: " << duration.count() << " microseconds" << endl;
    fout << "\n\n";
    r2.print();


    // multi level search
    auto start2 = std::chrono::high_resolution_clock::now();
    Record r = find(levels, id_to_search, dataFile);
    auto end2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2);
    fout << "\n\nExecution time for multi level search: " << duration2.count() << " microseconds" << endl;
    fout << "\n\n";
    r.print();


    


    
}
 
Record findLinear(vector<Record> records, int id) {

    for(Record record : records) {
        if(record.getID() == id)
            return record;
    }

    return Record(-1, "", -1);
}

Record findSingle(const vector<vector<BlockIndex>>& levels, int id, const vector<Block>& dataFile) {
   
   vector<BlockIndex> firstLevel = levels[0];

   int pointer = 0;

   for(int i = 0; i < firstLevel.size(); i++) {

        for(int j = 0; j < firstLevel[i].indexRecords.size() - 1; j++) {

            if(id >= firstLevel[i].indexRecords[j].getVal() && id < firstLevel[i].indexRecords[j + 1].getVal()) {
                pointer = firstLevel[i].indexRecords[j].getPointer();
                break;
            }
        }
   }

   for(int i = 0; i < dataFile[pointer].records.size(); i++) {
        if(id == dataFile[pointer].records[i].getID())
            return dataFile[pointer].records[i];
   }

   return Record(-1, "", -1);


}



Record find(const vector<vector<BlockIndex>>& levels, int id, const vector<Block>& dataFile) {
    int levelIndex = levels.size() - 1; // Start from the highest level

    while (levelIndex >= 0) {
        const vector<BlockIndex>& currentLevel = levels[levelIndex];

        for (const BlockIndex& blockIndex : currentLevel) {
            const vector<IndexRecord>& indexRecords = blockIndex.indexRecords;
            for (const IndexRecord& indexRecord : indexRecords) {
                int blockStartID = indexRecord.getVal();
                int blockEndID = blockStartID + dataFile[indexRecord.getPointer()].records.size() - 1;

                if (id >= blockStartID && id <= blockEndID) {
                    const Block& block = dataFile[indexRecord.getPointer()];
                    for (const Record& record : block.records) {
                        if (record.getID() == id) {
                            return record;
                        }
                    }
                }
            }
        }

        levelIndex--;
    }

    // Return an empty record if the ID was not found
    return Record(-1, "", -1);
}
