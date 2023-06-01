#include <iostream>
#include <vector>
#include <queue>
#include <chrono>
#include <unordered_map>
#include <fstream>
#include <thread>
#include <mutex>
#include <atomic>
#include "utimer.cpp"

using namespace std;
using namespace std::chrono;

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

void parallelPrintHCodes(struct MinHNode *root, vector<int> &arr, int top, ofstream &outputFile, int numThreads)
{
    int numChars = 256; // Assuming ASCII characters
    int charsPerThread = numChars / numThreads;

    vector<thread> threads(numThreads);

    for (int i = 0; i < numThreads; ++i)
    {
        char startChar = i * charsPerThread;
        char endChar = startChar + charsPerThread - 1;

        if (i == numThreads - 1)
            endChar = numChars - 1;

        threads[i] = thread(printHCodesRange, root, ref(arr), top, ref(outputFile), startChar, endChar);
    }

    for (int i = 0; i < numThreads; ++i)
    {
        threads[i].join();
    }
}

void HuffmanCodes(char item[], int freq[], int size, ofstream &outputFile, int numThreads)
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
    parallelPrintHCodes(minHeap.top(), arr, 0, outputFile, numThreads);
}

void computeFrequencies(const string &inputString, unordered_map<char, int> &frequencyMap, int start, int end)
{
    for (int i = start; i < end; ++i)
    {
        char c = inputString[i];
        frequencyMap[c]++;
    }
}

void parallelComputeFrequencies(const string &inputString, unordered_map<char, int> &frequencyMap, int numThreads)
{
    int inputSize = inputString.size();
    vector<thread> threads(numThreads);
    vector<unordered_map<char, int>> partialMaps(numThreads);

    int chunkSize = inputSize / numThreads;
    int remainingSize = inputSize % numThreads;

    // Create threads to compute frequencies in parallel
    for (int i = 0; i < numThreads; ++i)
    {
        int start = i * chunkSize;
        int end = start + chunkSize;

        if (i == numThreads - 1)
            end += remainingSize;

        threads[i] = thread(computeFrequencies, ref(inputString), ref(partialMaps[i]), start, end);
    }

    // Join threads and combine partial frequency maps
    for (int i = 0; i < numThreads; ++i)
    {
        threads[i].join();

        for (auto entry : partialMaps[i])
        {
            char c = entry.first;
            int freq = entry.second;
            frequencyMap[c] += freq;
        }
    }
}

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

    // Compute frequencies in parallel
    parallelComputeFrequencies(inputString, frequencyMap, numThreads);

    vector<char> arr;
    vector<int> freq;

    // Convert the frequency map into separate character and frequency arrays
    for (auto entry : frequencyMap)
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

    HuffmanCodes(arr.data(), freq.data(), size, outputFile, numThreads);
    outputFile.close();

    // auto stop = high_resolution_clock::now();
    // auto duration = duration_cast<milliseconds>(stop - start);

    // cout << "Execution time: " << duration.count() << " milliseconds" << endl;

    return 0;
}

/**
 * In this modified code, the parallelComputeFrequencies function is introduced to compute frequencies in parallel.
 * It splits the input string into chunks and creates multiple threads to process each chunk concurrently.
 * Each thread updates its own partial frequency map, and then the main thread combines the partial maps to obtain the final frequency map.

To use parallelization, run the program with an additional command-line argument specifying the number of threads to be used.

Please note that the efficiency of parallelization may vary depending on the characteristics of the input data and the hardware used.
Additionally, make sure to compile the code with proper support for C++11 or later to use the std::thread library.
*/