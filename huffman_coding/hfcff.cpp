#include <iostream>
#include <vector>
#include <queue>
#include <chrono>
#include <unordered_map>
#include <fstream>
// #include <ff/ff.hpp>
#include "../../fastflow/ff/svector.hpp"
#include "../../fastflow/ff/pipeline.hpp"
#include <ff/pipeline.hpp>
#include <ff/farm.hpp>
#include "../../fastflow/ff/farm.hpp"

using namespace std;
using namespace std::chrono;
using namespace ff;

#define MAX_TREE_HT 50

struct MinHNode
{
    unsigned freq;
    char item;
    struct MinHNode *left, *right;
};

struct compare
{
    bool operator()(MinHNode *left, MinHNode *right)
    {
        return (left->freq > right->freq);
    }
};

// Creating Huffman tree node
struct MinHNode *newNode(char item, unsigned freq)
{
    struct MinHNode *temp = new MinHNode;

    temp->left = temp->right = NULL;
    temp->item = item;
    temp->freq = freq;

    return temp;
}

void printArray(vector<int> &arr, int n)
{
    int i;
    for (i = 0; i < n; ++i)
        cout << arr[i];

    cout << "\n";
}

void printHCodes(struct MinHNode *root, vector<int> &arr, int top, ofstream &outputFile)
{
    if (root->left)
    {
        arr[top] = 0;
        printHCodes(root->left, arr, top + 1, outputFile);
    }

    if (root->right)
    {
        arr[top] = 1;
        printHCodes(root->right, arr, top + 1, outputFile);
    }

    if (!root->left && !root->right)
    {
        outputFile << root->item << "  | ";
        for (int i = 0; i < top; ++i)
        {
            outputFile << arr[i];
        }
        outputFile << "\n";
    }
}

void HuffmanCodes(char item[], int freq[], int size, ofstream &outputFile)
{
    struct MinHNode *left, *right, *top;

    priority_queue<MinHNode *, vector<MinHNode *>, compare> minHeap;
    for (int i = 0; i < size; ++i)
        minHeap.push(newNode(item[i], freq[i]));

    while (minHeap.size() != 1)
    {
        left = minHeap.top();
        minHeap.pop();

        right = minHeap.top();
        minHeap.pop();

        top = newNode('$', left->freq + right->freq);
        top->left = left;
        top->right = right;

        minHeap.push(top);
    }

    vector<int> arr(MAX_TREE_HT);
    printHCodes(minHeap.top(), arr, 0, outputFile);
}

struct FrequencyMapWorker : ff_node_t<string, unordered_map<char, int>>
{
    unordered_map<char, int> svc(string &inputString) override
    {
        unordered_map<char, int> frequencyMap;
        for (char c : inputString)
        {
            frequencyMap[c]++;
        }
        return frequencyMap;
    }
};

struct CodeGenerationWorker : ff_node_t<unordered_map<char, int>, pair<vector<char>, vector<int>>>
{
    pair<vector<char>, vector<int>> svc(unordered_map<char, int> &frequencyMap) override
    {
        vector<char> arr;
        vector<int> freq;
        for (auto entry : frequencyMap)
        {
            arr.push_back(entry.first);
            freq.push_back(entry.second);
        }
        return make_pair(arr, freq);
    }
};

struct HuffmanWorker : ff_node_t<pair<vector<char>, vector<int>>, bool>
{
    ofstream &outputFile;
    HuffmanWorker(ofstream &outputFile) : outputFile(outputFile) {}

    bool svc(pair<vector<char>, vector<int>> &data) override
    {
        vector<char> &arr = data.first;
        vector<int> &freq = data.second;

        HuffmanCodes(arr.data(), freq.data(), arr.size(), outputFile);

        return true;
    }
};

int main(int argc, char *argv[])
{
    auto start = high_resolution_clock::now();
    if (argc < 3)
    {
        cout << "Usage: " << argv[0] << " <input_file> <output_file>" << endl;
        return 1;
    }

    string inputFilePath = argv[1];
    string outputFilePath = argv[2];

    // Read input file
    ifstream inputFile(inputFilePath);
    if (!inputFile)
    {
        cout << "Error opening input file: " << inputFilePath << endl;
        return 1;
    }

    string inputString((istreambuf_iterator<char>(inputFile)), (istreambuf_iterator<char>()));
    inputFile.close();

    ofstream outputFile(outputFilePath);
    if (!outputFile)
    {
        cout << "Error opening output file: " << outputFilePath << endl;
        return 1;
    }

    FrequencyMapWorker freqWorker;
    CodeGenerationWorker codeGenWorker;
    HuffmanWorker huffmanWorker(outputFile);

    ff_pipeline pipe(freqWorker, codeGenWorker, huffmanWorker);

    pipe.wrap_around(); // Enable feedback

    pipe.run_and_wait_end();

    outputFile.close();

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);

    cout << "Execution time: " << duration.count() << " milliseconds" << endl;

    return 0;
}
