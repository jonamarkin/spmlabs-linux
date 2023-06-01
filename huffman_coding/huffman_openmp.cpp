#include <iostream>
#include <vector>
#include <queue>
#include <chrono>
#include <unordered_map>
#include <fstream>
#include <omp.h>

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

int main(int argc, char *argv[])
{
    auto start = high_resolution_clock::now();
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

    ofstream outputFile(outputFilePath);
    if (!outputFile)
    {
        cout << "Error opening output file: " << outputFilePath << endl;
        return 1;
    }

    unordered_map<char, int> frequencyMap;

// Parallel section: Compute frequency map
#pragma omp parallel for num_threads(numThreads) shared(frequencyMap)
    for (int i = 0; i < inputString.size(); ++i)
    {
#pragma omp atomic
        frequencyMap[inputString[i]]++;
    }

    vector<char> arr;
    vector<int> freq;

    // Sequential section: Generate code generation inputs
    for (auto entry : frequencyMap)
    {
        arr.push_back(entry.first);
        freq.push_back(entry.second);
    }

// Parallel section: Generate Huffman codes
#pragma omp parallel num_threads(numThreads)
    {
#pragma omp single
        HuffmanCodes(arr.data(), freq.data(), arr.size(), outputFile);
    }

    outputFile.close();

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);

    cout << "Execution time: " << duration.count() << " milliseconds" << endl;

    return 0;
}

/**
 * Explanation:

The parallel section is added before the computation of the frequency map, where each thread increments the count of characters in the frequency map. The atomic directive ensures that the increments are done safely without race conditions.
The generation of the code generation inputs (array arr and freq) is kept sequential because it involves shared data and adding parallelism here would introduce potential race conditions or synchronization overhead.
The parallel section is added around the HuffmanCodes function call using the single directive. This allows the function to be executed only once by a single thread, ensuring correct results while utilizing multiple threads for parallel execution.
Note that the effectiveness of parallelization may vary depending on the input size, number of threads, and the underlying hardware. It's recommended to experiment and benchmark the performance to find the optimal configuration for your specific use case.
*/