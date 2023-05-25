#include <iostream>
#include <vector>
#include <queue>
#include <chrono>
#include <unordered_map>
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

void printHCodes(struct MinHNode *root, vector<int> &arr, int top)
{
    if (root->left)
    {
        arr[top] = 0;
        printHCodes(root->left, arr, top + 1);
    }

    if (root->right)
    {
        arr[top] = 1;
        printHCodes(root->right, arr, top + 1);
    }

    if (!root->left && !root->right)
    {
        cout << root->item << "  | ";
        printArray(arr, top);
    }
}

void HuffmanCodes(char item[], int freq[], int size)
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
    printHCodes(minHeap.top(), arr, 0);
}

// int main()
// {
//     char arr[] = {'A', 'B', 'C', 'D'};
//     int freq[] = {5, 1, 6, 3};

//     int size = sizeof(arr) / sizeof(arr[0]);

//     cout << "Char | Huffman code ";
//     cout << "\n----------------------\n";
//     HuffmanCodes(arr, freq, size);

//     return 0;
// }

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        cout << "Usage: " << argv[0] << " <string>" << endl;
        return 1;
    }

    string inputString = argv[1];

    unordered_map<char, int> frequencyMap; // Use unordered_map to store character frequencies

    // Aggregate the frequencies of characters in the input string
    for (char c : inputString)
    {
        frequencyMap[c]++;
    }

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

    auto start = high_resolution_clock::now();

    HuffmanCodes(arr.data(), freq.data(), size);

    auto stop = high_resolution_clock::now();

    // auto duration = duration_cast<milliseconds>(stop.time_since_epoch() - start.time_since_epoch());
    auto duration = (stop.time_since_epoch() - start.time_since_epoch());
    // auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

    // Print start and stop times cast to milliseconds
    cout << "Start: " << duration_cast<milliseconds>(start.time_since_epoch()).count() << " Stop: " << duration_cast<milliseconds>(stop.time_since_epoch()).count() << " Duration: " << duration.count() << endl; // "Start: 0 Stop: 0 Duration: 0

    cout << "Start: " << start.time_since_epoch().count() << " Stop: " << stop.time_since_epoch().count() << " Duration: " << duration.count() << endl; // "Start: 0 Stop: 0 Duration: 0

    cout << "Execution time: " << duration.count() << " milliseconds" << endl;

    return 0;
}
