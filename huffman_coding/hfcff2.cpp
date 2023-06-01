#include <iostream>
#include <vector>
#include <chrono>
#include <unordered_map>
#include <fstream>
#include <fastflow/ff.h>
#include "utimer.cpp"

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

    temp->left = temp->right = nullptr;
    temp->item = item;
    temp->freq = freq;

    return temp;
}

void printArray(vector<int> &arr, int n)
{
    for (int i = 0; i < n; ++i)
        cout << arr[i];

    cout << "\n";
}

void printHCodesRange(struct MinHNode *root, vector<int> &arr, int top, ofstream &outputFile, char startChar, char endChar)
{
    if (root->left)
    {
        arr[top] = 0;
        printHCodesRange(root->left, arr, top + 1, outputFile, startChar, endChar);
    }

    if (root->right)
    {
        arr[top] = 1;
        printHCodesRange(root->right, arr, top + 1, outputFile, startChar, endChar);
    }

    if (!root->left && !root->right)
    {
        if (root->item >= startChar && root->item <= endChar)
        {
            outputFile << root->item << "  | ";
            for (int i = 0; i < top; ++i)
            {
                outputFile << arr[i];
            }
            outputFile << "\n";
        }
    }
}

struct FrequencyTask : ff_node_t<unordered_map<char, int>>
{
    const string &inputString;
    int start;
    int end;

    FrequencyTask(const string &inputString, int start, int end)
        : inputString(inputString), start(start), end(end) {}

    unordered_map<char, int> *svc(unordered_map<char, int> *frequencyMap)
    {
        for (int i = start; i < end; ++i)
        {
            char c = inputString[i];
            (*frequencyMap)[c]++;
        }

        return frequencyMap;
    }
};

struct CombineTask : ff_node_t<unordered_map<char, int>>
{
    unordered_map<char, int> combinedMap;

    unordered_map<char, int> *svc(unordered_map<char, int> *partialMap)
    {
        for (auto entry : *partialMap)
        {
            char c = entry.first;
            int freq = entry.second;
            combinedMap[c] += freq;
        }

        delete partialMap;
        return GO_ON;
    }
};

struct HuffmanTask : ff_node_t<>
{
    char *item;
    int *freq;
    int size;
    ofstream &outputFile;
    int numThreads;

    HuffmanTask(char *item, int *freq, int size, ofstream &outputFile, int numThreads)
        : item(item), freq(freq), size(size), outputFile(outputFile), numThreads(numThreads) {}

    void svc()
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
        int numChars = 256; // Assuming ASCII characters
        int charsPerThread = numChars / numThreads;

        vector<ff_node *> workers;
        for (int i = 0; i < numThreads; ++i)
        {
            char startChar = i * charsPerThread;
            char endChar = startChar + charsPerThread - 1;

            if (i == numThreads - 1)
                endChar = numChars - 1;

            workers.push_back(new printHCodesRange(top, arr, 0, ref(outputFile), startChar, endChar));
        }

        ff_Farm<> farm(std::move(workers));
        farm.remove_collector();
        farm.wrap_around();

        farm.run_and_wait_end();

        delete top;
    }
};

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        cout << "Usage: " << argv[0] << " <input_file> <output_file> <num_threads>" << endl;
        return 1;
    }

    string inputFilePath = argv[1];
    string outputFilePath = argv[2];
    int numThreads = atoi(argv[3]);

    // Read input file
    ifstream inputFile(inputFilePath);
    if (!inputFile)
    {
        cout << "Error opening input file: " << inputFilePath << endl;
        return 1;
    }

    string inputString((istreambuf_iterator<char>(inputFile)), (istreambuf_iterator<char>()));
    inputFile.close();

    unordered_map<char, int> frequencyMap; // Use unordered_map to store character frequencies

    long elapsed_time = 0;
    utimer timer1("Parallel computation", &elapsed_time);
    // auto start = high_resolution_clock::now();

    int inputSize = inputString.size();
    int chunkSize = inputSize / numThreads;
    int remainingSize = inputSize % numThreads;

    ff_Farm<> farm;

    vector<ff_node *> workers;
    for (int i = 0; i < numThreads; ++i)
    {
        int start = i * chunkSize;
        int end = start + chunkSize;

        if (i == numThreads - 1)
            end += remainingSize;

        workers.push_back(new FrequencyTask(inputString, start, end));
    }

    farm.add_workers(workers);
    farm.remove_collector();
    farm.wrap_around();

    farm.run_and_wait_end();

    unordered_map<char, int> combinedMap;

    farm.clear_farm();

    farm.add_workers(new CombineTask);
    farm.remove_collector();
    farm.wrap_around();

    farm.run_and_wait_end();

    vector<char> arr;
    vector<int> freq;

    for (auto entry : combinedMap)
    {
        arr.push_back(entry.first);
        freq.push_back(entry.second);
    }

    int size = arr.size();

    cout << "Char | Huffman code ";
    cout << "\n----------------------\n";

    // Create the Huffman codes and write them to the output file
    ofstream outputFile(outputFilePath);
    if (!outputFile)
    {
        cout << "Error opening output file: " << outputFilePath << endl;
        return 1;
    }

    farm.clear_farm();

    farm.add_workers(new HuffmanTask(arr.data(), freq.data(), size, outputFile, numThreads));
    farm.remove_collector();
    farm.wrap_around();

    farm.run_and_wait_end();

    outputFile.close();

    // auto stop = high_resolution_clock::now();
    // auto duration = duration_cast<milliseconds>(stop - start);

    // cout << "Execution time: " << duration.count() << " milliseconds" << endl;

    return 0;
}
