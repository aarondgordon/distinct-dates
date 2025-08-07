#include <ctype.h>
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
        return false;
    }

    printf("Sorted Values:\n");
    for (int i = 0; i < numValues; i++) {
        printf("%d\n", values[sortedKeys[i]]);

        if (values[sortedKeys[i]] != expected[i]) {
            return false;
        }
    }

    return true;
}

// Returns true if the given value is within the range [min, max]
bool InRange(unsigned int value, unsigned int min, unsigned int max)
{
    return value >= min && value <= max;
}

// Structure for storing ISO 8601 DateTimes
typedef struct dateTime {
    unsigned int year;      // Four digit year
    unsigned int month;     // [1, 12]
    unsigned int day;       // [1, 31]
    unsigned int hour;      // [0, 23]
    unsigned int minute;    // [0, 59]
    unsigned int second;    // [0, 59]
} DateTime;

// Returns true if all fields of the given DateTime are within
// valid ranges.
//
// This function does not validate if date is semantically accurate
// i.e., it does nottest  if the represented date exists on the calendar.
bool IsDateTimeValid(DateTime* dateTime)
{
    return dateTime
        && InRange(dateTime->year, 0, 9999)
        && InRange(dateTime->month, 1, 12)
        && InRange(dateTime->day, 1, 31)
        && InRange(dateTime->hour, 0, 23)
        && InRange(dateTime->minute, 0, 59)
        && InRange(dateTime->second, 0, 59);
}

// Prints the given DateTime to stdout in ISO 8601 format
void PrintDateTime(DateTime* pDateTime)
{
    if (pDateTime) {
        printf("%04d-%02d-%02dT%02d:%02d:%02dZ\n",
            pDateTime->year,
            pDateTime->month,
            pDateTime->day,
            pDateTime->hour,
            pDateTime->minute,
            pDateTime->second);
    }
}

// Prints the given DateTime to the given file stream in ISO 8601 format
void FPrintDateTime(FILE* stream, DateTime* pDateTime)
{
    if (pDateTime) {
        fprintf(stream, "%04d-%02d-%02dT%02d:%02d:%02dZ\n",
            pDateTime->year,
            pDateTime->month,
            pDateTime->day,
            pDateTime->hour,
            pDateTime->minute,
            pDateTime->second);
    }
}

// Returns true if the given DateTimes are equal.
bool DateTimesEqual(const DateTime* lhs, const DateTime* rhs)
{
    if (!lhs || !rhs) {
        return false;
    }

    return lhs->year == rhs->year
        && lhs->month == rhs->month
        && lhs->day == rhs->day
        && lhs->hour == rhs->hour
        && lhs->minute == rhs->minute
        && lhs->second == rhs->second;
}

// Returns true iff the first given DateTime is smaller than the second.
//
// Note that this function is intended for validating results during testing (i.e., not
// for use in a comparison-based sort).
bool DateTimeLessThan(const DateTime* lhs, const DateTime* rhs)
{
    if (!lhs || !rhs) {
        return false;
    }

    if (lhs->year < rhs->year) {
        return true;
    }
    else if (lhs->year > rhs->year) {
        return false;
    }

    if (lhs->month < rhs->month) {
        return true;
    }
    else if (lhs->month > rhs->month) {
        return false;
    }

    if (lhs->day < rhs->day) {
        return true;
    }
    else if (lhs->day > rhs->day) {
        return false;
    }

    if (lhs->hour < rhs->hour) {
        return true;
    }
    else if (lhs->hour > rhs->hour) {
        return false;
    }

    if (lhs->minute < rhs->minute) {
        return true;
    }
    else if (lhs->minute > rhs->minute) {
        return false;
    }

    return (lhs->second < rhs->second);
}

// Adds or subtracts the given offset from the given val, wrapping the result if
// outside the range [min, max].
// Returns signed carryover, i.e., number of wraps performed.
int OffsetAndWrap(unsigned int* val, int offset, unsigned int min, unsigned int max)
{
    if (val == NULL) {
        return 0;
    }

    // If starting value isn't in [min, max] range, wrapping is undefined
    if (!InRange(*val, min, max)) {
        return 0;
    }

    int carry = 0;

    // Remap range to [0, (max - min)]
    int tempVal = (int)(*val - min);
    int tempMax = (int)(max - min);

    int newVal = tempVal + offset;
    if (!InRange(newVal, 0, tempMax)) {

        if (newVal < 0) {
            carry = -1;
            carry -= abs(newVal) / (tempMax + 1);
        }
        else {
            carry += newVal / (tempMax + 1);
        }

        newVal = newVal % (tempMax + 1);
        if (newVal < 0) {
            newVal = (tempMax + 1) - abs(newVal);
        }
    }
    
    // Unmap range
    newVal += min;
    
    *val = newVal;

    return carry;
}

bool DoOffsetAndWrapTest(
    unsigned int* val,
    int* carry,
    unsigned int startVal,
    int offset,
    unsigned int min,
    unsigned int max,
    unsigned int expectedVal,
    int expectedCarry)
{
    if (val == NULL || carry == NULL) {
        return false;
    }

    *val = startVal;
    *carry = OffsetAndWrap(val, offset, min, max);
    printf("%d + %d = %d (carry %d)\n", startVal, offset, *val, *carry);

    return (*val == expectedVal) && (*carry == expectedCarry);
}

bool TestOffsetAndWrap()
{
    unsigned int val = 0;
    int carry = 0;

    if (!DoOffsetAndWrapTest(&val, &carry, 8, 1, 0, 9, 9, 0)) {
        return false;
    }

    if (!DoOffsetAndWrapTest(&val, &carry, 8, 4, 0, 9, 2, 1)) {
        return false;
    }

    if (!DoOffsetAndWrapTest(&val, &carry, 8, -8, 0, 9, 0, 0)) {
        return false;
    }

    if (!DoOffsetAndWrapTest(&val, &carry, 8, -10, 0, 9, 8, -1)) {
        return false;
    }

    if (!DoOffsetAndWrapTest(&val, &carry, 10, 6, 1, 12, 4, 1)) {
        return false;
    }
    
    if (!DoOffsetAndWrapTest(&val, &carry, 2, -4, 1, 12, 10, -1)) {
        return false;
    }

    if (!DoOffsetAndWrapTest(&val, &carry, 10, 24, 1, 12, 10, 2)) {
        return false;
    }

    return true;
}

// Applies the given hour and minute offsets to the given DateTime.
// Overflow is handled automatically.
// Returns true if the resulting DateTime is still valid.
bool OffsetDateTime(DateTime* dateTime, int hours, int minutes)
{
    if (dateTime == NULL) {
        return false;
    }

    int days = 0;
    int months = 0;
    int years = 0;

    hours += OffsetAndWrap(&dateTime->minute, minutes, 0, 59);
    days += OffsetAndWrap(&dateTime->hour, hours, 0, 23);
    months += OffsetAndWrap(&dateTime->day, days, 1, 31);
    years += OffsetAndWrap(&dateTime->month, months, 1, 12);
    OffsetAndWrap(&dateTime->year, years, 0, 9999);
    
    return IsDateTimeValid(dateTime);
}

// Copies length number of characters at the given start position from the given source
// buffer into the given destination buffer, provided all encountered characters are digits.
// Returns true if the given length of digits was copied.
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

// Converts the string representation of a number in the given source
// buffer into an integer at the given destination.
bool IntFromChars(unsigned int* dst, char* src, size_t n)
{
    if (!dst || !src) {
        return false;
    }

    *dst = 0;

    for (size_t i = 0; i < n; i++) {
        if (src[i] == '\0' || src[i] < '0' || src[i] > '9') {
            return false;
        }
        *dst = *dst * 10 + (src[i] - '0');  // C specification guarantees 0-9 are represented by contiguous values
    }

    return true;
}

// Returns true if the given value appears at the given position in the given soruce buffer.
bool ExpectChar(const char* src, size_t offset, char val)
{
    return src[offset] == val;
}

// Initializes the given DateTime using the given, null-terminated ISO 8601 date string.
// Returns true if the DateTime is left in a valid state.
//
// ISO 8601 date-time format is YYYY-MM-DDThh:mm:ss[Z | +hh:mm | -hh:mm]
//
// Trailing whitespace at the end of the string is allowed, but the string must still
// end with a null terminator.
bool PopulateDateTimeFromIsoString(const char* isoString, DateTime* dateTime)
{
    if (!dateTime) {
        return false;
    }

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

    // Read time zone
    char tzd = isoString[seekPos++];

    if (tzd == '+' || tzd == '-') {
        unsigned int tzHourOffset = 0;
        unsigned int tzMinuteOffset = 0;
        char tzdHour[2];      // [0, 23]
        char tzdMinute[2];    // [0, 59]

        // Read hour
        if (!CopyDigits(tzdHour, isoString, seekPos, 2, &seekPos)) {
            return false;
        }

        if (!IntFromChars(&tzHourOffset, tzdHour, 2) || !InRange(tzHourOffset, 0, 23)) {
            return false;
        }

        // Consume ':'
        if (!ExpectChar(isoString, seekPos++, ':')) {
            return false;
        }

        // Read minute
        if (!CopyDigits(tzdMinute, isoString, seekPos, 2, &seekPos)) {
            return false;
        }

        if (!IntFromChars(&tzMinuteOffset, tzdMinute, 2) || !InRange(tzMinuteOffset, 0, 59)) {
            return false;
        }

        if (tzd == '-') {
            tzHourOffset *= -1;
            tzMinuteOffset *= -1;
        }
        
        if (!OffsetDateTime(dateTime, tzHourOffset, tzMinuteOffset)) {
            return false;
        }
    }
    else if (tzd != 'Z') { // 'Z' denotes GMT
        return false;
    }

    // Consume trailing whitespace
    while (isspace(isoString[seekPos])) {
        seekPos++;
    }

    // Expect end of string
    if (!ExpectChar(isoString, seekPos++, '\0')) {
        return false;
    }

    return IsDateTimeValid(dateTime);
}

bool TestPopulateDateTimeFromIsoString()
{
    DateTime date;
    DateTime expected;

    expected.year = 2085;
    expected.month = 9;
    expected.day = 28;
    expected.hour = 20;
    expected.minute = 33;
    expected.second = 29;

    bool success = false;

    // Empty string
    success = PopulateDateTimeFromIsoString("", &date);
    if (success != false) {
        return false;
    }

    // Not a date
    success = PopulateDateTimeFromIsoString("Hello, world!", &date);
    if (success != false) {
        return false;
    }

    // Partial date
    success = PopulateDateTimeFromIsoString("2085-09-28", &date);
    if (success != false) {
        return false;
    }

    // Missing TZD
    success = PopulateDateTimeFromIsoString("2085-09-28T20:33:29", &date);
    if (success != false) {
        return false;
    }

    // GMT
    success = PopulateDateTimeFromIsoString("2085-09-28T20:33:29Z", &date);
    if (success == false || !DateTimesEqual(&date, &expected)) {
        return false;
    }

    // Positive TZD
    success = PopulateDateTimeFromIsoString("2085-09-28T08:03:29+12:30", &date);
    if (success == false || !DateTimesEqual(&date, &expected)) {
        return false;
    }

    // Negative TZD
    success = PopulateDateTimeFromIsoString("2085-09-28T22:53:29-02:20", &date);
    if (success == false || !DateTimesEqual(&date, &expected)) {
        return false;
    }

    // Overflow TZD
    success = PopulateDateTimeFromIsoString("2085-09-27T20:34:29+23:59", &date);
    if (success == false || !DateTimesEqual(&date, &expected)) {
        return false;
    }

    return true;
}

// The following selectors allow sorting of a DateTime with CountSort
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
    DateTime date;
    PopulateDateTimeFromIsoString("2056-00-00T00:00:00", &date);

    if (YearLSDSelector((void*)&date, 0) != 6) {
        return false;
    }

    if (YearDecadeSelector((void*)&date, 0) != 5) {
        return false;
    }

    if (YearCenturySelector((void*)&date, 0) != 0) {
        return false;
    }

    if (YearMilleniumSelector((void*)&date, 0) != 2) {
        return false;
    }

    return true;
}

// Sorts the given list of DateTimes using a radix sort.
bool SortDateTimes(const DateTime* dateTimes, size_t count, size_t* outKeys)
{
    if (!outKeys) {
        return false;
    }

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
    const size_t numDates = 12;
    DateTime dates[numDates];
    size_t sortedKeys[numDates] = { 0 };

    PopulateDateTimeFromIsoString("0000-01-01T00:01:01", &dates[0]);
    PopulateDateTimeFromIsoString("0000-01-02T01:01:01", &dates[1]);
    PopulateDateTimeFromIsoString("0001-02-02T01:00:00", &dates[2]);
    PopulateDateTimeFromIsoString("0001-02-02T00:00:00", &dates[3]);
    PopulateDateTimeFromIsoString("0000-02-02T01:01:01", &dates[4]);
    PopulateDateTimeFromIsoString("0000-01-01T00:00:01", &dates[5]);
    PopulateDateTimeFromIsoString("0000-01-01T00:00:00", &dates[6]);
    PopulateDateTimeFromIsoString("0000-01-01T01:01:01", &dates[7]);
    PopulateDateTimeFromIsoString("0001-02-01T00:00:00", &dates[8]);
    PopulateDateTimeFromIsoString("0001-02-02T01:01:01", &dates[9]);
    PopulateDateTimeFromIsoString("0001-02-02T01:01:00", &dates[10]);
    PopulateDateTimeFromIsoString("0001-01-01T00:00:00", &dates[11]);
    
    if (!SortDateTimes(dates, numDates, sortedKeys)) {
        return false;
    }

    printf("Sorted Dates:\n");
    for (size_t i = 0; i < numDates; i++) {
        DateTime* pCurDate = &dates[sortedKeys[i]];
        PrintDateTime(pCurDate);

        if (i > 0) {
            DateTime* pPrevDate = &dates[sortedKeys[i-1]];
            if (DateTimeLessThan(pCurDate, pPrevDate)) {
                return false;
            }
        }
    }

    return true;
}

// Finds the set of keys in the given list of DateTimes that correspond to unique entries and places
// them in outKeys.
//
// This uses a radix sort to organize DateTimes in ascending order, then scans the ordered list for
// unique keys. This has two implications:
//
//   1) The algorithm is not stable, i.e., elements in outKeys will not appear in the same order as the input list
//   2) The algorithm scales linearly with the number of DateTimes
//
bool DistinctDateTimes(const DateTime* dateTimes, size_t count, size_t* outKeys, size_t* outNewCount)
{
    if (!outKeys || !outNewCount) {
        return false;
    }

    // First sort the list of dates
    size_t* sortedKeys = calloc(count, sizeof(size_t));  // calloc should initialize memory to 0
    bool success = SortDateTimes(dateTimes, count, sortedKeys);

    size_t newCount = 0;
    if (success) {
        for (size_t i = 0; i < count; i++) {
            // Equal dates are now contiguous; if a date equals the previous date then skip it
            if (i > 0) {
                const DateTime* prevDate = &dateTimes[sortedKeys[i - 1]];
                const DateTime* curDate = &dateTimes[sortedKeys[i]];
                if (DateTimesEqual(prevDate, curDate)) {
                    continue;
                }
            }

            outKeys[newCount] = sortedKeys[i];
            newCount++;
        }
    }

    *outNewCount = newCount;
    free(sortedKeys);

    return success;
}

bool TestDistinctDateTimes()
{
    const size_t numDates = 8;
    DateTime dates[numDates];

    size_t distinctKeys[numDates] = { 0 };
    size_t numDistinctKeys = 0;

    PopulateDateTimeFromIsoString("0000-01-01T00:00:00", &dates[0]);
    PopulateDateTimeFromIsoString("0000-01-01T00:00:01", &dates[1]);
    PopulateDateTimeFromIsoString("0000-01-01T00:01:01", &dates[2]);
    PopulateDateTimeFromIsoString("0000-01-01T00:01:01", &dates[3]); // Copy
    PopulateDateTimeFromIsoString("0000-01-01T00:01:01", &dates[4]); // Copy
    PopulateDateTimeFromIsoString("0000-01-01T01:01:01", &dates[5]);
    PopulateDateTimeFromIsoString("1000-01-01T00:00:00", &dates[6]);
    PopulateDateTimeFromIsoString("1000-01-01T00:00:00", &dates[7]); // Copy

    if (!DistinctDateTimes(dates, numDates, distinctKeys, &numDistinctKeys)) {
        return false;
    }

    // Three dates in the test set are copies and should be removed
    if (numDistinctKeys != 5) {
        return false;
    }

    printf("Distinct Dates:\n");
    for (size_t i = 0; i < numDistinctKeys; i++) {
        DateTime* pCurDate = &dates[distinctKeys[i]];
        PrintDateTime(pCurDate);

        if (i > 0) {
            DateTime* pPrevDate = &dates[distinctKeys[i-1]];
            if (!DateTimeLessThan(pPrevDate, pCurDate)) {
                return false;
            }
        }
    }

    return true;
}

// Reads the given file containing ISO 8601 format date strings on each line into
// a DateTime buffer. If dateTimeBuff is NULL and n is 0 a buffer will be initialized
// for the caller. Regardless, it is the caller's responsibility to free the buffer
// when finished with it.
#define MAX_ISO_DATE_LEN 25;
size_t IngestDateTimes(DateTime** dateTimeBuff, size_t* n, FILE* stream)
{
    if (!dateTimeBuff || !n || !stream) {
        return false;
    }

    // If caller didn't allocate dateTimeBuff (and no size is provided) we can allocate it
    if (*dateTimeBuff == NULL) {
        if (*n == 0) {
            *n = sizeof(DateTime);
            *dateTimeBuff = (DateTime*)calloc(1, *n);
        }
        else { // If user provided a non-zero size but no dateTimeBuff, then fail
            return false;
        }
    }
    
    char* buff;
    size_t buffSize = MAX_ISO_DATE_LEN;

    buff = (char*)malloc(buffSize * sizeof(char));
    if (buff == NULL) {
        return false;
    }

    size_t chars = 0;
    size_t validDateTimes = 0;

    while (!feof(stream)) {
        chars = getline(&buff, &buffSize, stream);
        if (chars > 0) {
            // If we're out of space, allocate more
            const size_t spaceRemaining = *n - (validDateTimes * sizeof(DateTime));
            if (spaceRemaining < sizeof(DateTime)) {
                *n *= 2;
                *dateTimeBuff = (DateTime*)realloc(*dateTimeBuff, *n);
            }

            if (PopulateDateTimeFromIsoString(buff, &(*dateTimeBuff)[validDateTimes])) {
                validDateTimes++;
            }
        }
        else {
            validDateTimes = chars;
            break;
        }
    }

    free(buff);

    return validDateTimes;
}


#define TEST(t) \
    printf("===Running Test %s===\n", #t); \
    printf("%s\n\n", t() ? "Passed" : "Failed") ;

int main()
{
    TEST(TestCountSort);
    TEST(TestCopyDigits);
    TEST(TestPopulateDateTimeFromIsoString);
    TEST(TestYearSelectors);
    TEST(TestSortDateTimes);
    TEST(TestDistinctDateTimes);
    TEST(TestOffsetAndWrap);

    FILE* fileIn;
    FILE* fileOut;
    fileIn = fopen("dates.txt", "r");
    fileOut = fopen("distinct-dates.txt", "w");

    if (fileIn == NULL || fileOut == NULL) {
        return -1;
    }

    DateTime* datesBuffer = NULL;
    size_t datesBufferSize = 0;
    size_t numDates = 0;

    numDates = IngestDateTimes(&datesBuffer, &datesBufferSize, fileIn);

    if (numDates > 0) {
        size_t* distinctKeys;
        size_t numDistinctKeys;

        distinctKeys = (size_t*)malloc(numDates * sizeof(size_t));
        if (distinctKeys == NULL) {
            return -1;
        }

        if (DistinctDateTimes(datesBuffer, numDates, distinctKeys, &numDistinctKeys)) {
            for (size_t i = 0; i < numDistinctKeys; i++) {
                FPrintDateTime(fileOut, &datesBuffer[distinctKeys[i]]);
            }
        }

        free(distinctKeys);
    }

    free(datesBuffer);
    
    fclose(fileOut);
    fclose(fileIn);

    return 0;
}