#pragma once

// We include tools (libraries) that C++ gives us for free
#include <iostream>   // for printing to screen (cout, cin)
#include <fstream>    // for reading and writing files
#include <sstream>    // for building strings
#include <string>     // for using text (string)
#include <vector>     // for using lists (vector)
#include <map>        // for using key-value pairs (map)
#include <algorithm>  // for sorting
#include <iomanip>    // for formatting numbers
#include <ctime>      // for getting today's date
#include <limits>     // for clearing input buffer

// So we don't have to type "" every time
using namespace std;


// ============================================================
//  SECTION 1: COLORS
//  These are special codes that change text color in terminal.
//  \033[ is how you start a color code in most terminals.
// ============================================================
namespace Color {
    const string RESET  = "\033[0m";    // go back to normal color
    const string RED    = "\033[31m";
    const string GREEN  = "\033[32m";
    const string YELLOW = "\033[33m";
    const string CYAN   = "\033[36m";
    const string WHITE  = "\033[37m";
    const string BOLD   = "\033[1m";
    const string DIM    = "\033[2m";
}


// ============================================================
//  SECTION 2: GLOBAL SETTINGS
//  These are values used across the whole program.
// ============================================================

// The names of the files where we save data
const string USERS_FILE    = "users.csv";
const string EXPENSES_FILE = "expenses.csv";

// How many expenses to show per page
const int PAGE_SIZE = 5;

// The list of allowed expense categories
const vector<string> CATEGORIES = {
    "Food", "Transport", "Shopping", "Health",
    "Entertainment", "Education", "Bills", "Other"
};


// ============================================================
//  SECTION 3: HELPER FUNCTIONS
//  Small reusable functions used throughout the program.
// ============================================================

// Clear the terminal screen
void clearScreen() {
    system("cls");   // "cls" works on Windows
}

// Print a line of dashes (used as a separator)
void printLine(int length = 60) {
    cout << "  ";
    for (int i = 0; i < length; i++) cout << "-";
    cout << "\n";
}

// Print a success message in green
void printSuccess(const string& message) {
    cout << Color::GREEN << "\n  [OK] " << message << Color::RESET << "\n";
}

// Print an error message in red
void printError(const string& message) {
    cout << Color::RED << "\n  [ERROR] " << message << Color::RESET << "\n";
}

// Print a warning message in yellow
void printWarning(const string& message) {
    cout << Color::YELLOW << "\n  [WARN] " << message << Color::RESET << "\n";
}

// Wait for the user to press ENTER before continuing
void pauseScreen() {
    cout << "\n  Press ENTER to continue...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
}

// Ask the user a question and return their answer as a string
string getInput(const string& prompt) {
    cout << Color::YELLOW << "  " << prompt << ": " << Color::RESET;
    string answer;
    getline(cin, answer);  // read the whole line including spaces

    // Remove spaces from the beginning and end of the answer
    size_t start = answer.find_first_not_of(" \t\r\n");
    size_t end   = answer.find_last_not_of(" \t\r\n");
    if (start == string::npos) return "";
    return answer.substr(start, end - start + 1);
}

// Ask yes/no question, return true if user typed "y"
bool getConfirm(const string& question) {
    cout << Color::RED << "  " << question << " (y/n): " << Color::RESET;
    string answer;
    getline(cin, answer);
    return (answer == "y" || answer == "Y");
}

// Convert a string to all lowercase letters
string toLower(string text) {
    for (char& c : text) c = tolower(c);
    return text;
}

// Get today's date as a string like "2026-05-11"
string todayString() {
    time_t now = time(nullptr);
    char buffer[11];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d", localtime(&now));
    return string(buffer);
}

// Get current date AND time as a string like "2026-05-11 14:30:00"
string nowString() {
    time_t now = time(nullptr);
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime(&now));
    return string(buffer);
}

// Format a number as money, e.g. 5.5 becomes "$5.50"
string formatMoney(double amount) {
    ostringstream ss;
    ss << "$" << fixed << setprecision(2) << amount;
    return ss.str();
}

// Simple password hasher - converts password text into a scrambled number
// This means we never save the real password in the file
string hashPassword(const string& password) {
    unsigned long hash = 5381;
    for (char c : password)
        hash = ((hash << 5) + hash) + (unsigned char)c;
    ostringstream ss;
    ss << hex << setw(16) << setfill('0') << hash;
    return ss.str();
}

// Wrap a string in quotes and escape any quotes inside it
// This is needed so commas inside text do not break our CSV files
string csvWrap(const string& text) {
    string result = "\"";
    for (char c : text) {
        if (c == '"') result += "\"\"";  // double quotes escape a quote in CSV
        else          result += c;
    }
    result += "\"";
    return result;
}

// Split one line of a CSV file into a list of fields
// Handles quoted fields that may contain commas
vector<string> csvSplit(const string& line) {
    vector<string> fields;
    string field;
    bool insideQuotes = false;

    for (size_t i = 0; i < line.size(); i++) {
        char c = line[i];
        if (insideQuotes) {
            if (c == '"') {
                // two quotes in a row means a literal quote character
                if (i + 1 < line.size() && line[i+1] == '"') {
                    field += '"';
                    i++;
                } else {
                    insideQuotes = false;
                }
            } else {
                field += c;
            }
        } else {
            if      (c == '"') insideQuotes = true;
            else if (c == ',') { fields.push_back(field); field = ""; }
            else               field += c;
        }
    }
    fields.push_back(field);
    return fields;
}

// Print the program title banner
void printBanner() {
    cout << Color::CYAN;
    cout << "\n";
    cout << "  ============================================================\n";
    cout << "           E X P E N S E   T R A C K E R   S Y S T E M\n";
    cout << "  ============================================================\n";
    cout << Color::RESET << "\n";
}


// ============================================================
//  SECTION 4: USER CLASS
//  Represents one user account (admin or student).
// ============================================================
class User {
public:
    // These are the pieces of data each user has
    string username;      // the login name
    string passwordHash;  // the scrambled (hashed) password - never the real one
    string role;          // either "admin" or "student"
    string created;       // when the account was made

    // Default constructor (creates an empty user)
    User() {}

    // Constructor that sets all the fields at once
    User(string u, string ph, string r, string c) {
        username     = u;
        passwordHash = ph;
        role         = r;
        created      = c;
    }

    // Check if the password the user typed matches the stored hash
    bool checkPassword(const string& password) const {
        return passwordHash == hashPassword(password);
    }

    // Check if this user is an admin
    bool isAdmin() const {
        return role == "admin";
    }

    // Convert user data to one CSV line for saving to file
    string toCSV() const {
        return csvWrap(username) + "," + csvWrap(passwordHash) + ","
             + csvWrap(role)     + "," + csvWrap(created);
    }
};


// ============================================================
//  SECTION 5: EXPENSE CLASS
//  Represents one expense entry (like "Lunch $5.00").
// ============================================================
class Expense {
public:
    int    id;        // unique number to identify this expense
    string owner;     // which user this expense belongs to
    string title;     // short description e.g. "Lunch"
    double amount;    // how much money e.g. 5.50
    string category;  // e.g. "Food"
    string date;      // e.g. "2026-05-11"
    string note;      // optional extra detail
    string created;   // when this expense was added

    // Default constructor
    Expense() {}

    // Constructor that sets all fields
    Expense(int i, string own, string ttl, double amt,
            string cat, string dt, string nt, string crt) {
        id       = i;
        owner    = own;
        title    = ttl;
        amount   = amt;
        category = cat;
        date     = dt;
        note     = nt;
        created  = crt;
    }

    // Convert expense data to one CSV line for saving to file
    string toCSV() const {
        return to_string(id)     + "," + csvWrap(owner)         + ","
             + csvWrap(title)         + "," + to_string(amount) + ","
             + csvWrap(category)      + "," + csvWrap(date)          + ","
             + csvWrap(note)          + "," + csvWrap(created);
    }
};


// ============================================================
//  SECTION 6: USER MANAGER CLASS
//  Handles everything about user accounts:
//  - loading from file
//  - saving to file
//  - login (authenticate)
//  - register new user
//  - delete user
// ============================================================
class UserManager {
private:
    vector<User> users;  // list of all users in memory

    // Read all users from the CSV file into our list
    void loadUsers() {
        users.clear();
        ifstream file(USERS_FILE);
        if (!file.is_open()) return;  // if file does not exist, skip

        string line;
        getline(file, line);  // skip the first line (header row)

        while (getline(file, line)) {
            if (line.empty()) continue;
            vector<string> parts = csvSplit(line);
            if (parts.size() >= 4) {
                users.push_back(User(parts[0], parts[1], parts[2], parts[3]));
            }
        }
    }

    // Write all users from memory into the CSV file
    void saveUsers() const {
        ofstream file(USERS_FILE);
        file << "username,password_hash,role,created\n";  // write header
        for (const User& u : users) {
            file << u.toCSV() << "\n";
        }
    }

    // Create default accounts if no users file exists yet
    void createDefaultAccounts() {
        ifstream file(USERS_FILE);
        if (file.good()) {
            // File already exists, just load it
            loadUsers();
            return;
        }
        // File does not exist - create two default accounts
        users.push_back(User("admin",    hashPassword("admin123"), "admin",   nowString()));
        users.push_back(User("student1", hashPassword("pass123"),  "student", nowString()));
        saveUsers();
    }

public:
    // Constructor - runs when we create a UserManager object
    UserManager() {
        createDefaultAccounts();
    }

    // Try to log in - returns a pointer to the user if successful, nullptr if not
    User* authenticate(const string& username, const string& password) {
        for (User& u : users) {
            if (u.username == username && u.checkPassword(password)) {
                return &u;  // found the user - return their address
            }
        }
        return nullptr;  // not found
    }

    // Create a new user account
    // Returns true if successful, false if username already taken
    bool registerUser(const string& username, const string& password, const string& role) {
        // Check if username already exists
        for (const User& u : users) {
            if (u.username == username) return false;
        }
        users.push_back(User(username, hashPassword(password), role, nowString()));
        saveUsers();
        return true;
    }

    // Delete a user account by username
    // Returns true if deleted, false if not found
    bool deleteUser(const string& username) {
        for (int i = 0; i < (int)users.size(); i++) {
            if (users[i].username == username) {
                users.erase(users.begin() + i);  // remove from list
                saveUsers();
                return true;
            }
        }
        return false;
    }

    // Return the full list of users (used by admin to view all accounts)
    const vector<User>& getAllUsers() const {
        return users;
    }
};


// ============================================================
//  SECTION 7: EXPENSE MANAGER CLASS
//  Handles everything about expenses:
//  - loading from file
//  - saving to file
//  - adding new expense
//  - deleting expense
//  - getting list of expenses
//  - searching/filtering
//  - sorting
// ============================================================
class ExpenseManager {
private:
    vector<Expense> expenses;  // all expenses in memory
    int nextId = 1;            // the ID number for the next new expense

    // Read all expenses from the CSV file into memory
    void loadExpenses() {
        expenses.clear();
        nextId = 1;

        ifstream file(EXPENSES_FILE);
        if (!file.is_open()) return;

        string line;
        getline(file, line);  // skip header row

        while (getline(file, line)) {
            if (line.empty()) continue;
            vector<string> parts = csvSplit(line);
            if (parts.size() >= 8) {
                int    id     = stoi(parts[0]);
                double amount = stod(parts[3]);
                expenses.push_back(Expense(id, parts[1], parts[2], amount,
                                           parts[4], parts[5], parts[6], parts[7]));
                // Keep track of the highest ID so far
                if (id >= nextId) nextId = id + 1;
            }
        }
    }

    // Write all expenses from memory into the CSV file
    void saveExpenses() const {
        ofstream file(EXPENSES_FILE);
        file << "id,owner,title,amount,category,date,note,created\n";
        for (const Expense& e : expenses) {
            file << e.toCSV() << "\n";
        }
    }

public:
    // Constructor - load expenses when program starts
    ExpenseManager() {
        loadExpenses();
    }

    // Add a new expense to the list and save it
    void addExpense(const string& owner, const string& title, double amount,
                    const string& category, const string& date, const string& note) {
        expenses.push_back(Expense(nextId++, owner, title, amount,
                                   category, date, note, nowString()));
        saveExpenses();
    }

    // Delete an expense by ID
    // Regular users can only delete their own. Admins can delete any.
    bool deleteExpense(int id, const User& currentUser) {
        for (int i = 0; i < (int)expenses.size(); i++) {
            if (expenses[i].id == id) {
                // Check permission
                if (!currentUser.isAdmin() && expenses[i].owner != currentUser.username) {
                    return false;  // not allowed
                }
                expenses.erase(expenses.begin() + i);
                saveExpenses();
                return true;
            }
        }
        return false;  // expense not found
    }

    // Get expenses visible to this user
    // Admins see all. Students see only their own.
    vector<Expense> getExpenses(const User& currentUser) const {
        if (currentUser.isAdmin()) {
            return expenses;  // admin sees everything
        }
        vector<Expense> myExpenses;
        for (const Expense& e : expenses) {
            if (e.owner == currentUser.username) {
                myExpenses.push_back(e);
            }
        }
        return myExpenses;
    }

    // Search expenses by keyword, category, and/or date range
    vector<Expense> searchExpenses(const User& currentUser,
                                   const string& keyword,
                                   const string& category,
                                   const string& dateFrom,
                                   const string& dateTo) const {
        vector<Expense> results = getExpenses(currentUser);
        vector<Expense> filtered;

        for (const Expense& e : results) {
            // Check keyword (looks in title and note)
            bool keywordMatch = true;
            if (!keyword.empty()) {
                string kw    = toLower(keyword);
                string title = toLower(e.title);
                string note  = toLower(e.note);
                keywordMatch = (title.find(kw) != string::npos ||
                                note.find(kw)  != string::npos);
            }

            bool categoryMatch = category.empty() || (e.category == category);
            bool dateFromMatch = dateFrom.empty() || (e.date >= dateFrom);
            bool dateToMatch   = dateTo.empty()   || (e.date <= dateTo);

            if (keywordMatch && categoryMatch && dateFromMatch && dateToMatch) {
                filtered.push_back(e);
            }
        }
        return filtered;
    }

    // Sort a list of expenses by a chosen field
    void sortExpenses(vector<Expense>& list, const string& key, bool descending) const {
        // Lambda = a small function defined inline
        auto compare = [&](const Expense& a, const Expense& b) -> bool {
            bool result;
            if      (key == "amount")   result = a.amount < b.amount;
            else if (key == "title")    result = toLower(a.title) < toLower(b.title);
            else if (key == "category") result = toLower(a.category) < toLower(b.category);
            else                        result = a.date < b.date;  // default: sort by date
            return descending ? !result : result;
        };
        sort(list.begin(), list.end(), compare);
    }

    // Add up expenses grouped by category
    // Returns a map like: { "Food": 25.50, "Transport": 10.00 }
    map<string, double> groupByCategory(const vector<Expense>& list) const {
        map<string, double> totals;
        for (const Expense& e : list) {
            totals[e.category] += e.amount;
        }
        return totals;
    }
};


// ============================================================
//  SECTION 8: DISPLAY CLASS
//  Handles printing tables and summaries to the screen.
// ============================================================
class Display {
public:
    // Print a table of expenses
    static void showExpenses(const vector<Expense>& list, const string& title) {
        if (list.empty()) {
            printWarning("No expenses found.");
            return;
        }

        cout << "\n" << Color::CYAN << "  " << title << Color::RESET << "\n";
        printLine(97);

        // Print header row
        cout << Color::BOLD
                  << "  " << left
                  << setw(5)  << "#"
                  << setw(12) << "Owner"
                  << setw(22) << "Title"
                  << setw(12) << right << "Amount" << left
                  << "  "
                  << setw(15) << "Category"
                  << setw(12) << "Date"
                  << setw(20) << "Note"
                  << Color::RESET << "\n";
        printLine(97);

        // Print each expense as a row
        double total = 0;
        for (int i = 0; i < (int)list.size(); i++) {
            const Expense& e = list[i];
            total += e.amount;

            // Alternate row colors for readability
            string rowColor = (i % 2 == 0) ? Color::WHITE : Color::DIM;
            string noteText = e.note.empty() ? "-" : e.note.substr(0, 19);

            cout << rowColor
                      << "  " << left
                      << setw(5)  << e.id
                      << setw(12) << e.owner.substr(0, 11)
                      << setw(22) << e.title.substr(0, 21)
                      << Color::GREEN  << right << setw(12) << formatMoney(e.amount)
                      << Color::RESET  << rowColor
                      << "  " << left
                      << setw(15) << e.category
                      << setw(12) << e.date
                      << setw(20) << noteText
                      << Color::RESET  << "\n";
        }

        printLine(97);
        cout << Color::GREEN << "  Total: " << formatMoney(total) << Color::RESET
                  << "   " << Color::CYAN << list.size() << " record(s)" << Color::RESET << "\n";
    }

    // Print a table of user accounts (admin only)
    static void showUsers(const vector<User>& users) {
        cout << "\n" << Color::CYAN << "  User Accounts" << Color::RESET << "\n";
        printLine(55);
        cout << Color::BOLD
                  << "  " << left
                  << setw(18) << "Username"
                  << setw(12) << "Role"
                  << setw(22) << "Created"
                  << Color::RESET << "\n";
        printLine(55);

        for (const User& u : users) {
            string roleColor = u.isAdmin() ? Color::RED : Color::GREEN;
            cout << "  " << Color::CYAN << left << setw(18) << u.username
                      << roleColor << setw(12) << u.role
                      << Color::DIM << setw(22) << u.created
                      << Color::RESET << "\n";
        }
        printLine(55);
    }

    // Print a summary of spending by category with a simple bar chart
    static void showCategorySummary(const map<string, double>& categoryTotals) {
        if (categoryTotals.empty()) {
            printWarning("No data to show.");
            return;
        }

        // Calculate grand total
        double grandTotal = 0;
        for (const auto& pair : categoryTotals) grandTotal += pair.second;

        // Sort categories from highest to lowest spending
        vector<pair<string, double>> sorted(categoryTotals.begin(), categoryTotals.end());
        sort(sorted.begin(), sorted.end(),
            [](const auto& a, const auto& b) { return a.second > b.second; });

        cout << "\n" << Color::CYAN << "  Category Summary" << Color::RESET << "\n";
        printLine(58);
        cout << Color::BOLD
                  << "  " << left  << setw(16) << "Category"
                  << right << setw(14) << "Amount"
                  << setw(8)  << "Share"
                  << "  Chart"
                  << Color::RESET << "\n";
        printLine(58);

        for (const auto& pair : sorted) {
            string category = pair.first;
            double amount   = pair.second;
            double percent  = (grandTotal > 0) ? (amount / grandTotal * 100.0) : 0;
            int    barLen   = (int)(percent / 4);   // scale bar to fit screen
            string bar(barLen, '#');                 // e.g. "#####"

            cout << "  " << Color::YELLOW << left << setw(16) << category
                      << Color::GREEN  << right << setw(14) << formatMoney(amount)
                      << Color::CYAN   << right << setw(7)
                      << fixed << setprecision(1) << percent << "%"
                      << Color::WHITE  << "  " << bar
                      << Color::RESET  << "\n";
        }

        printLine(58);
        cout << Color::BOLD << "  " << left << setw(16) << "TOTAL"
                  << right << setw(14) << formatMoney(grandTotal)
                  << Color::RESET << "\n";
    }
};


// ============================================================
//  SECTION 9: PAGINATOR CLASS
//  Splits a long list into pages so it does not flood the screen.
//  Example: 20 expenses shown 5 at a time = 4 pages.
// ============================================================
class Paginator {
public:
    vector<Expense> allItems;   // the full list
    int pageSize;               // how many items per page
    int currentPage;            // which page we are on (starts at 0)

    // Constructor
    Paginator(const vector<Expense>& items, int size = PAGE_SIZE) {
        allItems    = items;
        pageSize    = size;
        currentPage = 0;
    }

    // How many pages are there in total?
    int totalPages() const {
        int total = (int)allItems.size();
        if (total == 0) return 1;
        return (total + pageSize - 1) / pageSize;  // round up
    }

    // Get only the items on the current page
    vector<Expense> getCurrentPage() const {
        int start = currentPage * pageSize;
        int end   = min(start + pageSize, (int)allItems.size());
        return vector<Expense>(allItems.begin() + start, allItems.begin() + end);
    }

    // Go to next page. Returns false if already on last page.
    bool nextPage() {
        if (currentPage < totalPages() - 1) {
            currentPage++;
            return true;
        }
        return false;
    }

    // Go to previous page. Returns false if already on first page.
    bool prevPage() {
        if (currentPage > 0) {
            currentPage--;
            return true;
        }
        return false;
    }

    // Get info text like "Page 1 / 4  (20 total)"
    string pageInfo() const {
        return "Page " + to_string(currentPage + 1) +
               " / "   + to_string(totalPages()) +
               "  ("   + to_string(allItems.size()) + " total)";
    }
};