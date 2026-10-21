#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;

// checks required format: optional sign, digits, optional dot and digits, etc
bool isValidNumber(const string& value) {
    if (value.empty()) return false;
    // skip over a leading sign if there is one
    size_t pos = (value[0] == '+' || value[0] == '-') ? 1 : 0;
    // a lone sign with no digits is not a number
    if (pos == value.size()) return false;

    bool hasDot = false;
    bool digitBeforeDot = false;
    bool digitAfterDot = false;
    // walk through each char and track digits on both sides of the dot
    for (; pos < value.size(); ++pos) {
        char ch = value[pos];
        if (ch >= '0' && ch <= '9') {
            if (hasDot) digitAfterDot = true;
            else digitBeforeDot = true;
        } else if (ch == '.' && !hasDot) {
            hasDot = true;
        } else {
            return false;
        }
    }
    // need digits before the dot and if there is a dot digits after it too
    return digitBeforeDot && (!hasDot || digitAfterDot);
}

// removes extra leading zeros w/o changing the rep value
string trimLeadingZeros(const string& digits) {
    size_t first = digits.find_first_not_of('0');
    return first == string::npos ? "0" : digits.substr(first);
}

// holds a parsed number as a sign plus a string of digits
struct Number {
    bool negative;
    string digits; // all decimal digits w decimal point removed
    size_t decimals; // how long (in digits) 
};

// is only called after isValidNumber succeeds
Number parseNumber(const string& input) {
    // remember the sign then strip it off before reading digits
    Number result{!input.empty() && input[0] == '-', "", 0};
    size_t start = (input[0] == '+' || input[0] == '-') ? 1 : 0;
    bool afterDot = false;
    // copy every digit and count how many come after the dot
    for (size_t i = start; i < input.size(); ++i) {
        if (input[i] == '.') {
            afterDot = true;
        } else {
            result.digits += input[i];
            if (afterDot) ++result.decimals;
        }
    }
    result.digits = trimLeadingZeros(result.digits);
    // treat negative zero as plain zero
    if (result.digits == "0") result.negative = false;
    return result;
}

// compares two integer strings, returning -1, 0, or 1 (for integers it needs to be positive)
int compareDigits(const string& a, const string& b) {
    // a longer string is bigger since leading zeros are already trimmed
    if (a.size() != b.size()) return a.size() < b.size() ? -1 : 1;
    if (a == b) return 0;
    // same length so a normal string compare works
    return a < b ? -1 : 1;
}

// add 2 nonnegative integer strings digit by digit
string addDigits(const string& a, const string& b) {
    string reversed;
    int i = static_cast<int>(a.size()) - 1;
    int j = static_cast<int>(b.size()) - 1;
    int carry = 0;
    // go right to left adding digits and carrying over
    while (i >= 0 || j >= 0 || carry) {
        int sum = carry;
        if (i >= 0) sum += a[i--] - '0';
        if (j >= 0) sum += b[j--] - '0';
        reversed += static_cast<char>('0' + sum % 10);
        carry = sum / 10;
    }
    // digits were built backwards so flip them
    reverse(reversed.begin(), reversed.end());
    return trimLeadingZeros(reversed);
}

// sub b from a, assuming a >= b and both are nonnegative integer strings
string subtractDigits(const string& a, const string& b) {
    string reversed;
    int i = static_cast<int>(a.size()) - 1;
    int j = static_cast<int>(b.size()) - 1;
    int borrow = 0;
    // go right to left subtracting and borrowing when needed
    while (i >= 0) {
        int digit = (a[i--] - '0') - borrow;
        if (j >= 0) digit -= b[j--] - '0';
        borrow = 0;
        if (digit < 0) {
            digit += 10;
            borrow = 1;
        }
        reversed += static_cast<char>('0' + digit);
    }
    // digits were built backwards so flip them
    reverse(reversed.begin(), reversed.end());
    return trimLeadingZeros(reversed);
}

// Restore the decimal point, then remove unnecessary zeroes.
string formatResult(string digits, size_t decimalPlaces, bool negative) {
    digits = trimLeadingZeros(digits);
    if (digits == "0") return "0";
    if (decimalPlaces > 0) {
        // pad with zeros so there is at least one digit before the dot
        if (digits.size() <= decimalPlaces) {
            digits.insert(0, decimalPlaces - digits.size() + 1, '0');
        }
        digits.insert(digits.size() - decimalPlaces, 1, '.');
        // drop trailing zeros after the dot and the dot itself if nothing is left
        while (!digits.empty() && digits.back() == '0') digits.pop_back();
        if (!digits.empty() && digits.back() == '.') digits.pop_back();
    }
    return (negative ? "-" : "") + digits;
}

// adds two decimal strings without converting them to floating point
string addNumbers(const string& first, const string& second) {
    Number a = parseNumber(first);
    Number b = parseNumber(second);
    // line up both numbers so they have the same number of decimal places
    size_t places = max(a.decimals, b.decimals);
    a.digits.append(places - a.decimals, '0');
    b.digits.append(places - b.decimals, '0');
    a.digits = trimLeadingZeros(a.digits);
    b.digits = trimLeadingZeros(b.digits);

    string result;
    bool negative = false;
    // same sign so just add the magnitudes and keep that sign
    if (a.negative == b.negative) {
        result = addDigits(a.digits, b.digits);
        negative = a.negative;
    } else {
        // different signs so subtract the smaller magnitude from the bigger one
        int comparison = compareDigits(a.digits, b.digits);
        if (comparison >= 0) {
            result = subtractDigits(a.digits, b.digits);
            negative = a.negative;
        } else {
            result = subtractDigits(b.digits, a.digits);
            negative = b.negative;
        }
    }
    return formatResult(result, places, negative);
}

// process one file line at a time
void processFile(istream& file) {
    string line;
    size_t lineNumber = 0;
    while (getline(file, line)) {
        ++lineNumber;
        istringstream input(line);
        string first, second, extra;
        cout << "line " << lineNumber << ": ";
        // make sure the line has exactly two tokens
        if (!(input >> first >> second) || (input >> extra)) {
            cout << "ERROR: expected exactly two numbers\n";
            continue;
        }
        // check both tokens and report whichever ones are bad
        bool validFirst = isValidNumber(first);
        bool validSecond = isValidNumber(second);
        if (!validFirst || !validSecond) {
            cout << "ERROR: invalid number";
            if (!validFirst) cout << " \"" << first << "\"";
            if (!validSecond) cout << " \"" << second << "\"";
            cout << '\n';
            continue;
        }
        // both numbers are valid so print the sum
        cout << first << " + " << second << " = " << addNumbers(first, second) << '\n';
    }
}

int main(int argc, char* argv[]) {
    string filename;
    // use the filename from the command line or ask for one
    if (argc > 1) {
        filename = argv[1];
    } else {
        cout << "Enter the input filename: ";
        if (!getline(cin, filename)) return 1;
    }
    // open the file and stop if it cannot be read
    ifstream file(filename);
    if (!file) {
        cerr << "ERROR: cannot open file \"" << filename << "\"\n";
        return 1;
    }
    processFile(file);
    return 0;
}
