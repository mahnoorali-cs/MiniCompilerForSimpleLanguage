#include <iostream>
#include <string>
#include <cstdio>

using namespace std;

int tempCount = 0;
int labelCount = 0;

string newTemp()
{
    tempCount++;
    char temp[20];
    sprintf(temp, "t%d", tempCount);
    return string(temp);
}

string newLabel()
{
    labelCount++;
    char label[20];
    sprintf(label, "L%d", labelCount);
    return string(label);
}

// Assignment
void generateAssignment(string var, string value)
{
    cout << var << " = " << value << endl;
}

// Arithmetic
void generateArithmetic(string result, string op1, string op, string op2)
{
    string temp = newTemp();

    cout << temp << " = " << op1 << " " << op << " " << op2 << endl;
    cout << result << " = " << temp << endl;
}

// IF
void generateIf(string condition)
{
    string endLabel = newLabel();

    cout << "\nIF NOT (" << condition << ") GOTO " << endLabel << endl;
    cout << "    // if body code" << endl;
    cout << endLabel << ":" << endl;
}

// WHILE
void generateWhile(string condition)
{
    string startLabel = newLabel();
    string endLabel = newLabel();

    cout << "\n" << startLabel << ":" << endl;
    cout << "IF NOT (" << condition << ") GOTO " << endLabel << endl;
    cout << "    // loop body code" << endl;
    cout << "GOTO " << startLabel << endl;
    cout << endLabel << ":" << endl;
}

int main()
{
    int choice;

    cout << "===== MINI COMPILER (IMMEDIATE CODE GEN) =====\n";

    while (true)
    {
        cout << "\nChoose Statement:\n";
        cout << "1. Assignment (a = 5)\n";
        cout << "2. Arithmetic (a = b + c)\n";
        cout << "3. IF statement\n";
        cout << "4. WHILE loop\n";
        cout << "5. Exit\n";
        cout << "Enter choice: ";
        cin >> choice;

        if (choice == 1)
        {
            string var, value;
            cout << "Enter variable: ";
            cin >> var;
            cout << "Enter value: ";
            cin >> value;

            generateAssignment(var, value);
        }
        else if (choice == 2)
        {
            string result, op1, op, op2;

            cout << "Enter result variable: ";
            cin >> result;
            cout << "Enter operand 1: ";
            cin >> op1;
            cout << "Enter operator (+,-,*,/): ";
            cin >> op;
            cout << "Enter operand 2: ";
            cin >> op2;

            generateArithmetic(result, op1, op, op2);
        }
        else if (choice == 3)
        {
            string condition;
            cout << "Enter condition (e.g. a > 10): ";
            cin.ignore();
            getline(cin, condition);

            generateIf(condition);
        }
        else if (choice == 4)
        {
            string condition;
            cout << "Enter condition (e.g. a > 0): ";
            cin.ignore();
            getline(cin, condition);

            generateWhile(condition);
        }
        else if (choice == 5)
        {
            cout << "Exiting compiler..." << endl;
            break;
        }
        else
        {
            cout << "Invalid choice!" << endl;
        }
    }

    return 0;
}
