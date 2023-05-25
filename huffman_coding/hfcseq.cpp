#include <iostream>
#include <vector>
#include <queue>
using namespace std;

#define MAX_TREE_HT 50

struct MinHNode
{
    unsigned freq;
    char item;
    struct MinHNode *left, *right;
};

struct MinH
{
    unsigned size;
    unsigned capacity;
    vector<MinHNode *> array;
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

// Print the array
void printArray(vector<int> &arr)
{
    for (int i = 0; i < arr.size(); ++i)
        cout << arr[i];

    cout << "\n";
}

// Swap function
void swapMinHNode(struct MinHNode **a, struct MinHNode **b)
{
    struct MinHNode *t = *a;
    *a = *b;
    *b = t;
}

// Heapify
void minHeapify(struct MinH *minHeap, int idx)
{
    int smallest = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;

    if (left < minHeap->size && minHeap->array[left]->freq < minHeap->array[smallest]->freq)
        smallest = left;

    if (right < minHeap->size && minHeap->array[right]->freq < minHeap->array[smallest]->freq)
        smallest = right;

    if (smallest != idx)
    {
        swapMinHNode(&minHeap->array[smallest], &minHeap->array[idx]);
        minHeapify(minHeap, smallest);
    }
}

// Check if size is 1
int checkSizeOne(struct MinH *minHeap)
{
    return (minHeap->size == 1);
}

// Extract the min
struct MinHNode *extractMin(struct MinH *minHeap)
{
    struct MinHNode *temp = minHeap->array[0];
    minHeap->array[0] = minHeap->array[minHeap->size - 1];

    --minHeap->size;
    minHeapify(minHeap, 0);

    return temp;
}

// Insertion
void insertMinHeap(struct MinH *minHeap, struct MinHNode *minHeapNode)
{
    ++minHeap->size;
    int i = minHeap->size - 1;

    while (i && minHeapNode->freq < minHeap->array[(i - 1) / 2]->freq)
    {
        minHeap->array[i] = minHeap->array[(i - 1) / 2];
        i = (i - 1) / 2;
    }

    minHeap->array[i] = minHeapNode;
}

// Build min heap
void buildMinHeap(struct MinH *minHeap)
{
    int n = minHeap->size - 1;
    int i;

    for (i = (n - 1) / 2; i >= 0; --i)
        minHeapify(minHeap, i);
}

int isLeaf(struct MinHNode *root)
{
    return !(root->left) && !(root->right);
}

struct MinH *createAndBuildMinHeap(vector<char> &item, vector<int> &freq, int size)
{
    struct MinH *minHeap = new MinH;

    minHeap->size = 0;
    minHeap->capacity = size;
    minHeap->array.resize(minHeap->capacity);

    for (int i = 0; i < size; ++i)
        minHeap->array[i] = newNode(item[i], freq[i]);

    minHeap->size = size;
    buildMinHeap(minHeap);

    return minHeap;
}

struct MinHNode *buildHfTree(vector<char> &item, vector<int> &freq, int size)
{
    struct MinHNode *left, *right, *top;
    struct MinH *minHeap = createAndBuildMinHeap(item, freq, size);

    while (!checkSizeOne(minHeap))
    {
        left = extractMin(minHeap);
        right = extractMin(minHeap);

        top = newNode('$', left->freq + right->freq);

        top->left = left;
        top->right = right;

        insertMinHeap(minHeap, top);
    }
    return extractMin(minHeap);
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
    if (isLeaf(root))
    {
        cout << root->item << "  | ";
        printArray(arr);
    }
}

// Wrapper function
void HuffmanCodes(vector<char> &item, vector<int> &freq, int size)
{
    struct MinHNode *root = buildHfTree(item, freq, size);

    vector<int> arr(MAX_TREE_HT);
    int top = 0;

    printHCodes(root, arr, top);
}

int main()
{
    vector<char> arr = {'A', 'B', 'C', 'D'};
    vector<int> freq = {5, 1, 6, 3};

    int size = arr.size();

    cout << "Char | Huffman code ";
    cout << "\n----------------------\n";
    HuffmanCodes(arr, freq, size);
}
