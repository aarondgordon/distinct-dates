#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// Sorts elements of array items into array outItems using count sort algorithm.
// Returns true if sort succeeded.
//
// Allocating adequate memory for items and outItems is the callers responsibility.
//
// Count sort creates a histogram of the frequency of element values in the input list.
// The histogram can then be used to determine the start and end indices of each value
// in the sorted list with the same histogram, i.e. the sorted list with the same frequency
// of element values.
bool CountSort(const int* items, int* outItems, size_t itemCount, size_t maxElemValue)
{
    size_t* histogram = calloc(maxElemValue, sizeof(size_t));  // calloc should initialize memory to 0

    // Build the histogram of element frequencies
    for (size_t i = 0; i < itemCount; i++) {
        int value = items[i];
        if (value < 0 || value > maxElemValue) {  // Value should be in range [0, maxElemValue] or sort will fail
            return false;
        }
        histogram[value]++;
    }

    // Calculate "prefix sums" by summing histogram counts.
    // These become our "end" indices in the sorted list for each value in the histogram.
    for (size_t i = 1; i < maxElemValue; i++) {
        histogram[i] = histogram[i - 1] + histogram[i];
    }

    // Map values from input list to output list using prefix sums.
    // We do this in reverse to make the sort stable.
    for (size_t i = itemCount; i > 0; i--) {
        int value = items[i - 1];
        size_t outIndex = histogram[value] - 1;
        histogram[value]--;  // Decrement this for next instance of value

        outItems[outIndex] = value;
    }

    return true;
}

int main()
{
    const int numValues = 10;
    int values[numValues] = {2, 4, 3, 7, 6, 8, 4, 4, 3, 2};
    int sorted[numValues] = { 0 };

    printf("Unsorted Values:\n");
    for (int i = 0; i < numValues; i++) {
        printf("%d\n", values[i]);
    }

    CountSort(values, sorted, numValues, 9);

    printf("Sorted Values:\n");
    for (int i = 0; i < numValues; i++) {
        printf("%d\n", sorted[i]);
    }

    return 0;
}