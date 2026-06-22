#include <iostream>
#include <sstream>
#include <string>

using namespace std;

int main()
{
    string datatype, identifier, equalSign, valueWithSemicolon;

    cout << "Enter declaration: ";
    string input;
    getline(cin, input);

    stringstream ss(input);

    ss >> datatype >> identifier >> equalSign >> valueWithSemicolon;

    // Validation
    if ((datatype != "int" &&
         datatype != "float" &&
         datatype != "char") ||
        equalSign != "=")
    {
        cout << "Syntax Error!" << endl;
        return 0;
    }

    if (valueWithSemicolon.back() != ';')
    {
        cout << "Syntax Error: Missing ';'" << endl;
        return 0;
    }

    string value =
        valueWithSemicolon.substr(
            0,
            valueWithSemicolon.length() - 1);

    cout << "\nParse Tree / AST\n\n";

    cout << "Declaration" << endl;
    cout << "+-- " << datatype << endl;
    cout << "+-- " << identifier << endl;
    cout << "+-- " << value << endl;

    return 0;
}
