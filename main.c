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

bool CopyDigits(char* dst, const char* src, size_t start, size_t length, size_t* outPos)
{
    for (size_t i = 0; i < length; i++) {
        char c = src[i + start];

        if (c == '\0' || c < '0' || c > '9') {
            return false;
        }

        dst[i] = c;
    }

    if (outPos) {
        *outPos = start + length;
    }

    return true;
}

bool TestCopyDigits()
{
    printf("Testing CopyDigits()...\n");

    const char* digits = "1234-5678";
    char lhs[5] = { '\0' };
    char rhs[5] = { '\0' };
    char err[5] = { '\0' };

    size_t pos = 0;

    if (!CopyDigits(lhs, digits, pos, 4, &pos)){
        return false;
    }

    pos++; // consume '-'

    if (!CopyDigits(rhs, digits, pos, 4, &pos)) {
        return false;
    }

    if (strcmp(lhs, "1234") != 0) {
        return false;
    }

    if (strcmp(rhs, "5678") != 0) {
        return false;
    }

    // Should fail if encounters non-digit
    if (CopyDigits(err, digits, 0, 5, &pos)) {
        return false;
    }

    // Should fail if string isn't long enough
    if (CopyDigits(err, digits, 5, 5, &pos)) {
        return false;
    }

    printf("lhs:%s\n", lhs);
    printf("rhs:%s\n", rhs);

    return true;
}

bool IntFromChars(unsigned int* dst, char* src, size_t n)
{
    if (!dst || !src) {
        return false;
    }

    *dst = 0;

    for (size_t i = 0; i < n; i++) {
        if (src[i] == '\0' || src[i] < '0' || src['i'] > '9') {
            return false;
        }
        *dst = *dst * 10 + (src[i] - '0');  // C specification guarantees 0-9 are represented by contiguous values
    }

    return true;
}

bool ExpectChar(const char* src, size_t offset, char val)
{
    return src[offset] == val;
}

// ISO 8601 date-time format is YYYY-MM-DDThh:mm:ssTZD
bool PopulateDateTimeFromIsoString(const char* isoString, DateTime* dateTime)
{
    if (!dateTime) {
        return false;
    }

    const size_t maxIsoLength = 25;
    size_t seekPos = 0;

    char year[4];      // Four digit year
    char month[2];     // [1, 12]
    char day[2];       // [1, 31]
    char hour[2];      // [0, 23]
    char minute[2];    // [0, 59]
    char second[2];    // [0. 59]

    // Read year
    if (!CopyDigits(year, isoString, seekPos, 4, &seekPos)) {
        return false;
    }
    IntFromChars(&(dateTime->year), year, 4);

    // Consume '-'
    if (!ExpectChar(isoString, seekPos++, '-')) {
        return false;
    }

    // Read month
    if (!CopyDigits(month, isoString, seekPos, 2, &seekPos)) {
        return false;
    }
    IntFromChars(&(dateTime->month), month, 2);

    // Consume '-'
    if (!ExpectChar(isoString, seekPos++, '-')) {
        return false;
    }

    // Read day
    if (!CopyDigits(day, isoString, seekPos, 2, &seekPos)) {
        return false;
    }
    IntFromChars(&(dateTime->day), day, 2);

    // Consume 'T'
    if (!ExpectChar(isoString, seekPos++, 'T')) {
        return false;
    }

    // Read hour
    if (!CopyDigits(hour, isoString, seekPos, 2, &seekPos)) {
        return false;
    }
    IntFromChars(&(dateTime->hour), hour, 2);

    // Consume ':'
    if (!ExpectChar(isoString, seekPos++, ':')) {
        return false;
    }

    // Read minute
    if (!CopyDigits(minute, isoString, seekPos, 2, &seekPos)) {
        return false;
    }
    IntFromChars(&(dateTime->minute), minute, 2);

    // Consume ':'
    if (!ExpectChar(isoString, seekPos++, ':')) {
        return false;
    }

    // Read second
    if (!CopyDigits(second, isoString, seekPos, 2, &seekPos)) {
        return false;
    }
    IntFromChars(&(dateTime->second), second, 2);

    // TODO: Time zone support

    return true;
}

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
    PopulateDateTimeFromIsoString("2056-00-00T00:00:00", &date);

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
    size_t sortedKeys[numDates] = { 0 };

    PopulateDateTimeFromIsoString("2066-00-00T00:00:00", &dates[0]);
    PopulateDateTimeFromIsoString("2032-00-00T00:00:00", &dates[1]);
    PopulateDateTimeFromIsoString("2180-00-00T00:00:00", &dates[2]);
    PopulateDateTimeFromIsoString("1432-00-00T00:00:00", &dates[3]);
    PopulateDateTimeFromIsoString("1970-00-00T00:00:00", &dates[4]);
    
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
    printf("%s\n", TestCopyDigits() ? "Passed" : "Failed");

    return 0;
}