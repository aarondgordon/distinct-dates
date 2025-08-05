#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Sorts entries of array keys into array outKeys per the count sort algorithm.
//
// A key's value for sorting is determined by the valueSelector callback, which receives
// the provided values pointer and a key and should return an integer between zero and
// maxValue (inclusive).
//
// Allocating and initializing memory for keys and outKeys is the callers responsibility.
//
// Returns true if sort succeeded.
//
// Count sort creates a histogram of the frequency of element values in the input list.
// The histogram can then be used to determine the start and end indices of each value
// in the sorted list with the same histogram, i.e. the sorted list with the same frequency
// of element values.
bool CountSort(const void* values, unsigned int(*valueSelector)(const void*, size_t), unsigned int maxValue, size_t keyCount, const size_t* keys, size_t* outKeys)
{
    if (values == NULL || valueSelector == NULL) {
        return false;
    }

    size_t* histogram = calloc(maxValue + 1, sizeof(size_t));  // calloc should initialize memory to 0

    // Build the histogram of element frequencies
    for (size_t i = 0; i < keyCount; i++) {
        size_t key = keys[i];
        unsigned int value = valueSelector(values, key);

        if (value > maxValue) {  // Value should be in range [0, maxElemValue] or sort will fail
            free(histogram);
            return false;
        }
        histogram[value]++;
    }

    // Calculate "prefix sums" by summing histogram counts.
    // These become our "end" indices in the sorted list for each value in the histogram.
    for (size_t i = 1; i <= maxValue; i++) {
        histogram[i] = histogram[i - 1] + histogram[i];
    }

    // Map values from input list to output list using prefix sums.
    // We do this in reverse to make the sort stable.
    for (size_t i = keyCount; i > 0; i--) {
        size_t key = keys[i - 1];
        unsigned int value = valueSelector(values, key);

        size_t outIndex = histogram[value] - 1;
        histogram[value]--;  // Decrement this for next instance of value

        outKeys[outIndex] = key;
    }

    free(histogram);
    return true;
}

unsigned int TestValueSelector(const void* values, size_t key)
{
    return ((const unsigned int*)values)[key];
}

bool TestCountSort()
{
    printf("Testing Count Sort...\n");

    const size_t numValues = 10;
    unsigned int values[numValues] = {2, 0, 3, 7, 6, 9, 4, 4, 3, 2};
    unsigned int expected[numValues] = {0, 2, 2, 3, 3, 4, 4, 6, 7, 9};

    size_t keys[numValues] = { 0 };
    size_t sortedKeys[numValues] = { 0 };
    
    for (int i = 0; i < numValues; i++) {
        keys[i] = i;
    }

    printf("Unsorted Values:\n");
    for (int i = 0; i < numValues; i++) {
        printf("%d\n", values[keys[i]]);
    }

    if (!CountSort((void*)values, TestValueSelector, 9, numValues, keys, sortedKeys)) {
        printf("Failed\n");
        return false;
    }

    printf("Sorted Values:\n");
    for (int i = 0; i < numValues; i++) {
        printf("%d\n", values[sortedKeys[i]]);

        if (values[sortedKeys[i]] != expected[i]) {
            printf("Failed\n");
            return false;
        }
    }

    printf("Passed\n");
    return true;
}


typedef struct dateTime {
    unsigned int year;      // Four digit year
    unsigned int month;     // [1, 12]
    unsigned int day;       // [1, 31]
    unsigned int hour;      // [0, 23]
    unsigned int minute;    // [0, 59]
    unsigned int second;    // [0. 59]
} DateTime;

unsigned int SecondSelector(const void* dateTimeValues, size_t key)
{
    return ((const DateTime*)dateTimeValues)[key].second;
}

unsigned int MinuteSelector(const void* dateTimeValues, size_t key)
{
    return ((const DateTime*)dateTimeValues)[key].minute;
}

unsigned int HourSelector(const void* dateTimeValues, size_t key)
{
    return ((const DateTime*)dateTimeValues)[key].hour;
}

unsigned int DaySelector(const void* dateTimeValues, size_t key)
{
    return ((const DateTime*)dateTimeValues)[key].day;
}

unsigned int MonthSelector(const void* dateTimeValues, size_t key)
{
    return ((const DateTime*)dateTimeValues)[key].month;
}

unsigned int YearLSDSelector(const void* dateTimeValues, size_t key)
{
    unsigned int year = ((const DateTime*)dateTimeValues)[key].year;
    return year % 10;
}

unsigned int YearDecadeSelector(const void* dateTimeValues, size_t key)
{
    unsigned int year = ((const DateTime*)dateTimeValues)[key].year;
    return (year / 10) % 10;
}

unsigned int YearCenturySelector(const void* dateTimeValues, size_t key)
{
    unsigned int year = ((const DateTime*)dateTimeValues)[key].year;
    return (year / 100) % 10;
}

unsigned int YearMilleniumSelector(const void* dateTimeValues, size_t key)
{
    unsigned int year = ((const DateTime*)dateTimeValues)[key].year;
    return (year / 1000) % 10;
}

bool TestYearSelectors()
{
    printf("Testing Year Selectors...\n");

    DateTime date;
    date.year = 2056;

    if (YearLSDSelector((void*)&date, 0) != 6) {
        printf("Failed\n");
        return false;
    }

    if (YearDecadeSelector((void*)&date, 0) != 5) {
        printf("Failed\n");
        return false;
    }

    if (YearCenturySelector((void*)&date, 0) != 0) {
        printf("Failed\n");
        return false;
    }

    if (YearMilleniumSelector((void*)&date, 0) != 2) {
        printf("Failed\n");
        return false;
    }

    printf("Passed\n");
    return true;
}


bool SortDateTimes(const DateTime* dateTimes, size_t count, size_t* outKeys)
{
    // Initialize key array that we'll be sorting
    size_t* keys = calloc(count, sizeof(size_t));  // calloc should initialize memory to 0
    for (size_t i = 0; i < count; i++) {
        keys[i] = i;
    }

    if (!CountSort((void*)dateTimes, SecondSelector, 59, count, keys, outKeys)) {
        free(keys);
        return false;
    }
    memcpy(keys, outKeys, count * sizeof(size_t));
    
    if (!CountSort((void*)dateTimes, MinuteSelector, 59, count, keys, outKeys)) {
        free(keys);
        return false;
    }
    memcpy(keys, outKeys, count * sizeof(size_t));

    if (!CountSort((void*)dateTimes, HourSelector, 23, count, keys, outKeys)) {
        free(keys);
        return false;
    }
    memcpy(keys, outKeys, count * sizeof(size_t));

    if (!CountSort((void*)dateTimes, DaySelector, 31, count, keys, outKeys)) {
        free(keys);
        return false;
    }
    memcpy(keys, outKeys, count * sizeof(size_t));

    if (!CountSort((void*)dateTimes, MonthSelector, 12, count, keys, outKeys)) {
        free(keys);
        return false;
    }
    memcpy(keys, outKeys, count * sizeof(size_t));

    if (!CountSort((void*)dateTimes, YearLSDSelector, 9, count, keys, outKeys)) {
        free(keys);
        return false;
    }
    memcpy(keys, outKeys, count * sizeof(size_t));

    if (!CountSort((void*)dateTimes, YearDecadeSelector, 9, count, keys, outKeys)) {
        free(keys);
        return false;
    }
    memcpy(keys, outKeys, count * sizeof(size_t));

    if (!CountSort((void*)dateTimes, YearCenturySelector, 9, count, keys, outKeys)) {
        free(keys);
        return false;
    }
    memcpy(keys, outKeys, count * sizeof(size_t));

    if (!CountSort((void*)dateTimes, YearMilleniumSelector, 9, count, keys, outKeys)) {
        free(keys);
        return false;
    }

    free(keys);
    return true;
}

bool TestSortDateTimes()
{
    printf("Testing Sort DateTime...\n");

    const size_t numDates = 5;
    DateTime dates[numDates];

    dates[0].second = 0;
    dates[0].minute = 0;
    dates[0].hour = 0;
    dates[0].month = 1;
    dates[0].day = 1;
    dates[0].year = 2066;

    dates[1].second = 0;
    dates[1].minute = 0;
    dates[1].hour = 0;
    dates[1].month = 1;
    dates[1].day = 1;
    dates[1].year = 2032;

    dates[2].second = 0;
    dates[2].minute = 0;
    dates[2].hour = 0;
    dates[2].month = 1;
    dates[2].day = 1;
    dates[2].year = 2180;

    dates[3].second = 0;
    dates[3].minute = 0;
    dates[3].hour = 0;
    dates[3].month = 1;
    dates[3].day = 1;
    dates[3].year = 1432;

    dates[4].second = 0;
    dates[4].minute = 0;
    dates[4].hour = 0;
    dates[4].month = 1;
    dates[4].day = 1;
    dates[4].year = 1970;

    size_t sortedKeys[numDates] = { 0 };

    if (!SortDateTimes(dates, numDates, sortedKeys)) {
        printf("Failed\n");
        return false;
    }

    printf("Sorted Dates:\n");
    for (int i = 0; i < numDates; i++) {
        printf("%d\n", dates[sortedKeys[i]].year);

        if (i > 0 && dates[sortedKeys[i]].year < dates[sortedKeys[i - 1]].year) {
            printf("Failed\n");
            return false;
        }
    }

    printf("Passed\n");
    return true;
}

int main()
{
    TestCountSort();
    TestYearSelectors();
    TestSortDateTimes();

    return 0;
}