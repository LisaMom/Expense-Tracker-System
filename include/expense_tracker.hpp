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



using namespace std;



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



// The name of the file where we save user accounts
const string USERS_FILE = "users.csv";

string expenseFileFor(const string& username) {
    return "expenses_" + username + ".csv";
}

// How many expenses to show per page
const int PAGE_SIZE = 5;

// The list of allowed expense categories
const vector<string> CATEGORIES = {
    "Food", "Transport", "Shopping", "Health",
    "Entertainment", "Education", "Bills", "Other"
};


void clearScreen() {
#if defined(_WIN32)
    (void)system("cls");
#else
    (void)system("clear");
#endif
}

bool openCSVInExcel(const string& filename) {
    // Check the file actually exists before trying to open it
    ifstream check(filename);
    if (!check.good()) return false;
    check.close();

#if defined(_WIN32)

    HINSTANCE result = ShellExecuteA(
        nullptr,          // no parent window
        "open",           // action: open
        filename.c_str(), // the file path
        nullptr,          // no extra parameters
        nullptr,          // default working directory
        SW_SHOWNORMAL     // show the window normally
    );
    // ShellExecute returns a value > 32 on success (Windows API quirk)
    return (reinterpret_cast<INT_PTR>(result) > 32);

#elif defined(__APPLE__)

    string cmd = "open \"" + filename + "\" 2>/dev/null";
    return (system(cmd.c_str()) == 0);

#else
    string cmd = "xdg-open \"" + filename + "\" 2>/dev/null &";
    return (system(cmd.c_str()) == 0);
#endif
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

// Get today's date as a string in DD-MM-YYYY format, e.g. "12-05-2026"
string todayString() {
    time_t now = time(nullptr);
    char buffer[11];
    strftime(buffer, sizeof(buffer), "%d-%m-%Y", localtime(&now));
    return string(buffer);
}

// Get current date AND time as a string, e.g. "12-05-2026 14:30:00"
string nowString() {
    time_t now = time(nullptr);
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%d-%m-%Y %H:%M:%S", localtime(&now));
    return string(buffer);
}

// Format a number as money, e.g. 5.5 becomes "$5.50"
string formatMoney(double amount) {
    ostringstream ss;
    ss << "$" << fixed << setprecision(2) << amount;
    return ss.str();
}

string dateToComparable(const string& d) {
    // Expects exactly "DD-MM-YYYY" (10 characters)
    if (d.size() != 10) return d;
    return d.substr(6, 4) + "-" + d.substr(3, 2) + "-" + d.substr(0, 2);
}

string hashPassword(const string& password) {
    unsigned long hash = 5381;
    for (char c : password)
        hash = ((hash << 5) + hash) + (unsigned char)c;
    ostringstream ss;
    ss << hex << setw(16) << setfill('0') << hash;
    return ss.str();
}

string csvWrap(const string& text) {
    string result = "\"";
    for (char c : text) {
        if (c == '"') result += "\"\"";  // double quotes escape a quote in CSV
        else          result += c;
    }
    result += "\"";
    return result;
}

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
        string lowerInput = toLower(username);
        for (User& u : users) {
            // Compare case-insensitively so "Admin" and "admin" match the same account
            if (toLower(u.username) == lowerInput && u.checkPassword(password)) {
                return &u;  // found the user - return their address
            }
        }
        return nullptr;  // not found
    }

    // Create a new user account
    // Returns true if successful, false if username already taken
    bool registerUser(const string& username, const string& password, const string& role) {
        // Check if username already exists (case-insensitive: "John" and "john" are the same)
        string lowerNew = toLower(username);
        for (const User& u : users) {
            if (toLower(u.username) == lowerNew) return false;
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


class ExpenseManager {
private:

    map<string, vector<Expense>> userExpenses;  // username -> list of their expenses
    map<string, int>             userNextId;    // username -> next 
    void loadUserExpenses(const string& username) {
        // Start fresh for this user
        userExpenses[username].clear();
        userNextId[username] = 1;

        string filename = expenseFileFor(username);
        ifstream file(filename);
        if (!file.is_open()) return;  // file doesn't exist yet = no expenses, that's fine

        string line;
        getline(file, line);  // skip the header row (id,owner,title,...)

        while (getline(file, line)) {
            if (line.empty()) continue;

            vector<string> parts = csvSplit(line);
            if (parts.size() >= 8) {
                int    id     = stoi(parts[0]);
                double amount = stod(parts[3]);

                userExpenses[username].push_back(
                    Expense(id, parts[1], parts[2], amount,
                            parts[4], parts[5], parts[6], parts[7])
                );

                // Track the highest ID so the next one is always higher
                if (id >= userNextId[username]) {
                    userNextId[username] = id + 1;
                }
            }
        }
    }

    void saveUserExpenses(const string& username) {
        vector<Expense>& list = userExpenses[username];

        // Renumber: go through every expense and assign a new ID
        // starting from 1, going up by 1 each time
        for (int i = 0; i < (int)list.size(); i++) {
            list[i].id = i + 1;  // i+1 because we want 1,2,3... not 0,1,2...
        }

        // Update the next available ID to be one after the last item
        userNextId[username] = (int)list.size() + 1;

        // Now write to file
        string filename = expenseFileFor(username);
        ofstream file(filename);
        file << "id,owner,title,amount,category,date,note,created\n";
        for (const Expense& e : list) {
            file << e.toCSV() << "\n";
        }
    }

    void ensureLoaded(const string& username) {
        // If username is NOT in our map yet, load their file now
        if (userExpenses.find(username) == userExpenses.end()) {
            loadUserExpenses(username);
        }
    }


public:
    // Constructor - nothing to load yet, we load on demand
    ExpenseManager() {}


    void addExpense(const string& owner, const string& title, double amount,
                    const string& category, const string& date, const string& note) {
        ensureLoaded(owner);

        int newId = userNextId[owner]++;  // grab the next ID, then increase the counter

        userExpenses[owner].push_back(
            Expense(newId, owner, title, amount, category, date, note, nowString())
        );

        saveUserExpenses(owner);
    }

    bool deleteExpense(int id, const User& currentUser) {
        ensureLoaded(currentUser.username);

        if (currentUser.isAdmin()) {
            for (auto& pair : userExpenses) {
                const string& username = pair.first;
                vector<Expense>& list  = pair.second;

                for (int i = 0; i < (int)list.size(); i++) {
                    if (list[i].id == id) {
                        list.erase(list.begin() + i);  // remove it (no renumbering)
                        saveUserExpenses(username);
                        return true;
                    }
                }
            }
            return false;  // not found in any loaded user's list
        }

        // Regular user: only look in their own list
        vector<Expense>& list = userExpenses[currentUser.username];
        for (int i = 0; i < (int)list.size(); i++) {
            if (list[i].id == id) {
                list.erase(list.begin() + i);  // remove it (no renumbering)
                saveUserExpenses(currentUser.username);
                return true;
            }
        }
        return false;  // expense not found
    }

    vector<Expense> getExpenses(const User& currentUser) const {

        ExpenseManager* self = const_cast<ExpenseManager*>(this);

        if (!currentUser.isAdmin()) {
            // Regular user: just load and return their own data
            self->ensureLoaded(currentUser.username);
            return userExpenses.at(currentUser.username);
        }

        self->ensureLoaded(currentUser.username);

        vector<Expense> all;
        for (const auto& pair : userExpenses) {
            for (const Expense& e : pair.second) {
                all.push_back(e);
            }
        }
        return all;
    }

    vector<Expense> searchExpenses(const User& currentUser,
                                   const string& keyword,
                                   const string& category,
                                   const string& dateFrom,
                                   const string& dateTo) const {
        vector<Expense> results  = getExpenses(currentUser);
        vector<Expense> filtered;

        for (const Expense& e : results) {

            // -- Check if the keyword appears in title or note --
            bool keywordMatch = true;
            if (!keyword.empty()) {
                string kw    = toLower(keyword);
                string title = toLower(e.title);
                string note  = toLower(e.note);
                keywordMatch = (title.find(kw) != string::npos ||
                                note.find(kw)  != string::npos);
            }

            // -- Check category --
            bool categoryMatch = category.empty() || (e.category == category);

            // -- Check date range (convert DD-MM-YYYY to YYYY-MM-DD for comparison) --
            bool dateFromMatch = dateFrom.empty() || (dateToComparable(e.date) >= dateToComparable(dateFrom));
            bool dateToMatch   = dateTo.empty()   || (dateToComparable(e.date) <= dateToComparable(dateTo));

            // Only include this expense if ALL conditions match
            if (keywordMatch && categoryMatch && dateFromMatch && dateToMatch) {
                filtered.push_back(e);
            }
        }
        return filtered;
    }

    void sortExpenses(vector<Expense>& list, const string& key, bool descending) const {
        // A "lambda" is a mini-function we define right here inline.
        // It tells sort how to compare two expenses.
        auto compare = [&](const Expense& a, const Expense& b) -> bool {
            bool result;
            if      (key == "amount")   result = a.amount < b.amount;
            else if (key == "title")    result = toLower(a.title) < toLower(b.title);
            else if (key == "category") result = toLower(a.category) < toLower(b.category);
            else                        result = dateToComparable(a.date) < dateToComparable(b.date);
            return descending ? !result : result;
        };
        sort(list.begin(), list.end(), compare);
    }

    map<string, double> groupByCategory(const vector<Expense>& list) const {
        map<string, double> totals;
        for (const Expense& e : list) {
            totals[e.category] += e.amount;
        }
        return totals;
    }
};

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
                  << setw(20) << "Category"
                  << setw(15) << "Date"
                  << setw(30) << "Note"
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
                      << setw(20) << e.category
                      << setw(15) << e.date
                      << Color::DIM    << noteText
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