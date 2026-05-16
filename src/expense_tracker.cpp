
#include "expense_tracker.hpp"
using namespace std;


class ExpenseTrackerApp {
private:
    UserManager  userMgr;           // handles all user accounts
    ExpenseManager expMgr;          // handles all expenses
    User* currentUser = nullptr;    // who is logged in right now (nullptr = nobody)

    map<string, map<string, double>> budgets;

    double getBudgetLimit(const string& category) {
        auto userIt = budgets.find(currentUser->username);
        if (userIt == budgets.end()) return 0.0;

        auto catIt = userIt->second.find(category);
        if (catIt == userIt->second.end()) return 0.0;

        return catIt->second;
    }


    double getTotalSpent(const string& category) {
        vector<Expense> all = expMgr.getExpenses(*currentUser);
        double total = 0.0;
        for (const Expense& e : all) {
            if (e.category == category) total += e.amount;
        }
        return total;
    }


    bool checkBudgetWarning(const string& category, double newAmount) {
        double limit = getBudgetLimit(category);
        if (limit <= 0.0) return true;  // no limit set, always OK

        double spent   = getTotalSpent(category);
        double afterAdd = spent + newAmount;

        if (afterAdd > limit) {
            // ── OVER BUDGET WARNING ──────────────────────────────
            cout << "\n";
            cout << "  +--------------------------------------------------+\n";
            cout << "  |               *** BUDGET WARNING ***             |\n";
            cout << "  |                                                  |\n";
            cout << "  |  Category : " << left << setw(37) << category << "|\n";
            cout << "  |  Budget   : $" << fixed << setprecision(2)
                      << setw(36) << limit << "|\n";
            cout << "  |  Spent    : $" << setw(36) << spent   << "|\n";
            cout << "  |  This item: $" << setw(36) << newAmount << "|\n";
            cout << "  |  Total    : $" << setw(36) << afterAdd << "|\n";
            cout << "  |                                                  |\n";
            cout << "  |  You will EXCEED your budget by $"
                      << setw(16) << (afterAdd - limit) << "!   |\n";
            cout << "  |  Think carefully before making this purchase.   |\n";
            cout << "  +--------------------------------------------------+\n";
            cout << "\n";

            return getConfirm("Do you still want to add this expense?");

        } else if (afterAdd >= limit * 0.9) {
            // ── NEAR-LIMIT WARNING (within 10% of limit) ─────────
            cout << "\n";
            cout << "  +--------------------------------------------------+\n";
            cout << "  |  *** BUDGET NOTICE ***                           |\n";
            cout << "  |                                                  |\n";
            cout << "  |  Category : " << left << setw(37) << category << "|\n";
            cout << "  |  Budget   : $" << fixed << setprecision(2)
                      << setw(36) << limit << "|\n";
            cout << "  |  After this, you will have spent: $"
                      << setw(14) << afterAdd << "  |\n";
            cout << "  |  Remaining: $" << setw(36) << (limit - afterAdd) << "|\n";
            cout << "  |                                                  |\n";
            cout << "  |  You are approaching your budget limit!          |\n";
            cout << "  +--------------------------------------------------+\n";
            cout << "\n";

            return getConfirm("Do you still want to add this expense?");
        }

        return true;  // well within budget, no warning needed
    }


    void doLogin() {
        clearScreen();
        cout << "\n  LOGIN\n  -----\n";

        string username = getInput("Username");
        string password = getInput("Password");

        User* found = userMgr.authenticate(username, password);

        if (found != nullptr) {
            currentUser = found;
            printSuccess("Welcome, " + currentUser->username + " [" + currentUser->role + "]");
        } else {
            printError("Wrong username or password.");
        }
        pauseScreen();
    }

    void doRegister() {
        clearScreen();
        cout << "\n  REGISTER\n  --------\n";
        cout << Color::DIM << "  Create a new account to start tracking your expenses.\n" << Color::RESET;

        string username = getInput("Choose a username");

        if (username.empty()) {
            printError("Username cannot be empty.");
            pauseScreen();
            return;
        }

        string password = getInput("Choose a password");

        if (password.empty()) {
            printError("Password cannot be empty.");
            pauseScreen();
            return;
        }

        bool success = userMgr.registerUser(username, password, "student");

        if (success) {
            printSuccess("Account created! You can now log in with username: " + username);
        } else {
            printError("The username '" + username + "' is already taken.");
            cout << Color::YELLOW
                      << "  Tip: Try adding a number (e.g. " << username << "2)\n"
                      << "       or use a different name altogether.\n"
                      << Color::RESET;
        }
        pauseScreen();
    }


    void doLogout() {
        cout << "\n  Goodbye, " << currentUser->username << "!\n";
        currentUser = nullptr;
        pauseScreen();
    }


    void addExpense() {
        clearScreen();
        cout << "\n  ADD EXPENSE\n  -----------\n";

        // Step 1: Get the title
        string title = getInput("Item/Product Name");
        if (title.empty()) {
            printError("Title cannot be empty.");
            pauseScreen();
            return;
        }

        // Step 2: Get the amount (must be a positive number)
        double amount = 0;
        while (true) {
            string amountText = getInput("Amount $");
            try {
                amount = stod(amountText);
                if (amount <= 0) throw invalid_argument("must be positive");
                break;
            } catch (...) {
                printError("Please enter a valid positive number.");
            }
        }

        // Step 3: Choose a category
        cout << "\n  Categories:\n";
        for (int i = 0; i < (int)CATEGORIES.size(); i++) {
            cout << "    " << (i + 1) << ". " << CATEGORIES[i];

            // Show the current budget limit next to each category (if set)
            double limit = getBudgetLimit(CATEGORIES[i]);
            if (limit > 0.0) {
                double spent = getTotalSpent(CATEGORIES[i]);
                cout << "  [Budget: $" << fixed << setprecision(2)
                          << limit << "  Spent: $" << spent << "]";
            }
            cout << "\n";
        }

        string category;
        while (true) {
            string choice = getInput("Enter category number");
            try {
                int index = stoi(choice) - 1;
                if (index < 0 || index >= (int)CATEGORIES.size()) {
                    throw out_of_range("out of range");
                }
                category = CATEGORIES[index];
                break;
            } catch (...) {
                printError("Invalid choice. Enter a number from the list.");
            }
        }

        // Step 4: Get the date (or use today)
        string date = getInput("Date (DD-MM-YYYY) or press ENTER for today");
        if (date.empty()) date = todayString();

        // Step 5: Optional note
        string note = getInput("Note (optional, press ENTER to skip)");

        // Step 6: Budget warning check BEFORE saving
        if (!checkBudgetWarning(category, amount)) {
            printWarning("Expense cancelled.");
            pauseScreen();
            return;
        }

        // Save the expense
        expMgr.addExpense(currentUser->username, title, amount, category, date, note);
        printSuccess("Expense saved!");
        pauseScreen();
    }

    void setBudgetLimit() {
        clearScreen();
        cout << "\n  BUDGET LIMITS\n  -------------\n";
        cout << "  Set a maximum amount you want to spend per category.\n";
        cout << "  You will be warned when a new expense would exceed the limit.\n\n";

        // Show all categories with their current budget and spending
        vector<Expense> all = expMgr.getExpenses(*currentUser);
        map<string, double> totals = expMgr.groupByCategory(all);

        cout << "  " << left
                  << setw(3)  << "#"
                  << setw(16) << "Category"
                  << setw(14) << "Spent"
                  << setw(14) << "Limit"
                  << "Status\n";
        cout << "  " << string(60, '-') << "\n";

        for (int i = 0; i < (int)CATEGORIES.size(); i++) {
            const string& cat = CATEGORIES[i];
            double spent = (totals.count(cat) ? totals[cat] : 0.0);
            double limit = getBudgetLimit(cat);

            // Build status label
            string status = "No limit";
            if (limit > 0.0) {
                if (spent > limit) {
                    status = "OVER BUDGET";
                } else if (spent >= limit * 0.9) {
                    status = "Near limit";
                } else {
                    double pct = (spent / limit) * 100.0;
                    status = to_string((int)pct) + "% used";
                }
            }

            cout << "  " << left
                      << setw(3)  << (i + 1)
                      << setw(16) << cat
                      << "$" << fixed << setprecision(2)
                      << setw(13) << spent;

            if (limit > 0.0) {
                cout << "$" << setw(13) << limit;
            } else {
                cout << setw(14) << "--";
            }

            cout << status << "\n";
        }

        cout << "\n";

        // Pick a category to set/update/remove its limit
        string choice;
        int index = -1;
        while (true) {
            choice = getInput("Enter category number to set/update its limit (or 0 to go back)");
            if (choice == "0") return;
            try {
                index = stoi(choice) - 1;
                if (index < 0 || index >= (int)CATEGORIES.size()) {
                    throw out_of_range("out of range");
                }
                break;
            } catch (...) {
                printError("Please enter a valid number from the list.");
            }
        }

        const string& selectedCat = CATEGORIES[index];
        double currentLimit = getBudgetLimit(selectedCat);

        cout << "\n  Category : " << selectedCat << "\n";
        if (currentLimit > 0.0) {
            cout << "  Current limit: $" << fixed << setprecision(2)
                      << currentLimit << "\n";
        } else {
            cout << "  Current limit: (none)\n";
        }

        cout << "\n  Options:\n";
        cout << "    1. Set / Update limit\n";
        if (currentLimit > 0.0) {
            cout << "    2. Remove limit\n";
        }
        cout << "    0. Cancel\n";

        string action = getInput("Choose");

        if (action == "1") {
            // Set a new limit
            double newLimit = 0.0;
            while (true) {
                string limitText = getInput("Enter budget limit (e.g. 200.00)");
                try {
                    newLimit = stod(limitText);
                    if (newLimit <= 0) throw invalid_argument("must be positive");
                    break;
                } catch (...) {
                    printError("Please enter a valid positive number.");
                }
            }
            budgets[currentUser->username][selectedCat] = newLimit;
            printSuccess("Budget limit for '" + selectedCat + "' set to $"
                         + to_string(newLimit));

        } else if (action == "2" && currentLimit > 0.0) {
            // Remove the limit
            if (getConfirm("Remove the budget limit for '" + selectedCat + "'?")) {
                budgets[currentUser->username].erase(selectedCat);
                printSuccess("Budget limit removed for '" + selectedCat + "'.");
            }
        } else {
            cout << "  Cancelled.\n";
        }

        pauseScreen();
    }

    void viewExpenses() {
        vector<Expense> list = expMgr.getExpenses(*currentUser);
        showWithPages(list, "All Expenses");
    }


    void searchExpenses() {
        clearScreen();
        cout << "\n  SEARCH / FILTER\n  ---------------\n";

        string keyword = getInput("Search keyword (or ENTER to skip)");

        cout << "  Categories: ";
        for (const string& c : CATEGORIES) cout << c << "  ";
        cout << "\n";

        string category = getInput("Category (or ENTER to skip)");

        if (!category.empty()) {
            bool valid = false;
            for (const string& c : CATEGORIES) {
                if (c == category) { valid = true; break; }
            }
            if (!valid) {
                printWarning("Category not found, ignoring.");
                category = "";
            }
        }

        string dateFrom = getInput("From date DD-MM-YYYY (or ENTER to skip)");
        string dateTo   = getInput("To date DD-MM-YYYY (or ENTER to skip)");

        vector<Expense> results = expMgr.searchExpenses(*currentUser, keyword, category, dateFrom, dateTo);

        string resultTitle = "Search Results (" + to_string(results.size()) + " found)";
        showWithPages(results, resultTitle);
    }

    void sortExpenses() {
        clearScreen();
        cout << "\n  SORT EXPENSES\n  -------------\n";
        cout << "  Sort by:\n";
        cout << "    1. Date\n";
        cout << "    2. Amount\n";
        cout << "    3. Title\n";
        cout << "    4. Category\n";

        string choice = getInput("Choose (1-4)");

        string key = "date";
        if      (choice == "1") key = "date";
        else if (choice == "2") key = "amount";
        else if (choice == "3") key = "title";
        else if (choice == "4") key = "category";

        cout << "  Order:\n";
        cout << "    1. Ascending (low to high)\n";
        cout << "    2. Descending (high to low)\n";
        bool descending = (getInput("Choose (1-2)") == "2");

        vector<Expense> list = expMgr.getExpenses(*currentUser);
        expMgr.sortExpenses(list, key, descending);

        string order = descending ? " (high to low)" : " (low to high)";
        showWithPages(list, "Sorted by " + key + order);
    }

    void deleteExpense() {
        // Keep looping until the user decides to leave
        while (true) {
            clearScreen();
            cout << "\n  DELETE EXPENSE\n  --------------\n";
            cout << Color::DIM
                      << "  Tip: After deleting, items below will shift up to fill the gap.\n"
                      << "       Example: delete #2 -> item #3 becomes #2, #4 becomes #3...\n"
                      << Color::RESET << "\n";

            // Show the user's expenses so they can see the IDs
            vector<Expense> list = expMgr.getExpenses(*currentUser);

            if (list.empty()) {
                printWarning("You have no expenses to delete.");
                pauseScreen();
                return;  // nothing to do, go back to menu
            }

            // Show all expenses (up to 10 at a time to keep the screen clean)
            int previewCount = min((int)list.size(), 10);
            vector<Expense> preview(list.begin(), list.begin() + previewCount);
            Display::showExpenses(preview, "Your Expenses (showing up to 10)");

            if ((int)list.size() > 10) {
                cout << Color::DIM << "  ... and " << (list.size() - 10)
                          << " more. Use View Expenses to see all.\n" << Color::RESET;
            }

            cout << "\n";
            cout << Color::YELLOW
                      << "  Enter the ID number to delete, or type 0 to go back: "
                      << Color::RESET;

            string idText;
            getline(cin, idText);

            // Trim spaces
            size_t s = idText.find_first_not_of(" \t\r\n");
            size_t e = idText.find_last_not_of(" \t\r\n");
            if (s != string::npos) idText = idText.substr(s, e - s + 1);
            else                   idText = "";

            // If they typed 0 or nothing, exit the delete screen
            if (idText == "0" || idText.empty()) {
                cout << "\n  Returning to menu...\n";
                break;  // exit the while(true) loop
            }

            // Try to convert what they typed into a number
            int id;
            try {
                id = stoi(idText);
            } catch (...) {
                printError("Please enter a valid number (e.g. 3), or 0 to go back.");
                pauseScreen();
                continue;  // loop again
            }

            // Ask for confirmation before deleting
            if (getConfirm("Are you sure you want to delete expense #" + to_string(id) + "?")) {
                bool deleted = expMgr.deleteExpense(id, *currentUser);
                if (deleted) {
                    printSuccess("Expense #" + to_string(id) + " has been deleted.");
                    cout << Color::DIM
                              << "  Note: The items below have shifted up to fill the gap.\n"
                              << Color::RESET;
                } else {
                    printError("Could not find expense #" + to_string(id) + ", or you do not have permission.");
                }
            } else {
                cout << "  Cancelled. Nothing was deleted.\n";
            }

            // Small pause so the user can read the result, then loop again
            pauseScreen();

        }  // end while(true) - goes back to the top to show the list again
    }

    void manageUsers() {
        if (!currentUser->isAdmin()) {
            printError("Access denied. Admins only.");
            pauseScreen();
            return;
        }

        while (true) {
            clearScreen();
            cout << "\n  USER MANAGEMENT\n  ---------------\n";

            Display::showUsers(userMgr.getAllUsers());

            cout << "\n  1. Add a new user\n";
            cout << "  2. Delete a user\n";
            cout << "  0. Back to menu\n\n";

            string choice = getInput("Choose");

            if (choice == "1") {
                string username = getInput("New username");
                string password = getInput("Password");
                cout << "  Role:\n    1. Student\n    2. Admin\n";
                string roleChoice = getInput("Choose role (1 or 2)");
                string role = (roleChoice == "2") ? "admin" : "student";

                if (userMgr.registerUser(username, password, role)) {
                    printSuccess("User '" + username + "' created as " + role + ".");
                } else {
                    printError("The username '" + username + "' is already taken. Please choose a different one.");
                }
                pauseScreen();

            } else if (choice == "2") {
                string username = getInput("Enter username to delete");

                if (username == currentUser->username) {
                    printError("You cannot delete your own account.");
                } else if (getConfirm("Delete user '" + username + "'?")) {
                    if (userMgr.deleteUser(username)) {
                        printSuccess("User deleted.");
                    } else {
                        printError("User not found.");
                    }
                }
                pauseScreen();
            } else if (choice == "0") {
                break;
            }
        }
    }

    void showWithPages(const vector<Expense>& list, const string& title) {
        Paginator pages(list);

        while (true) {
            clearScreen();
            cout << "\n  " << title << "  [" << pages.pageInfo() << "]\n\n";

            Display::showExpenses(pages.getCurrentPage(), title);

            if (pages.totalPages() == 1) break;

            cout << "\n  [N] Next page   [P] Previous page   [Q] Quit\n";
            string nav = toLower(getInput("Navigate"));

            if (nav == "n") {
                if (!pages.nextPage()) printWarning("You are already on the last page.");
            } else if (nav == "p") {
                if (!pages.prevPage()) printWarning("You are already on the first page.");
            } else if (nav == "q") {
                break;
            }
        }

        if (pages.totalPages() == 1) pauseScreen();
    }


    void viewMyCSV() {
        clearScreen();
        cout << "\n  OPEN CSV FILE\n  -------------\n";

        string filename = expenseFileFor(currentUser->username);

        cout << "  File: " << Color::CYAN << filename << Color::RESET << "\n\n";

        // Check the file exists first
        ifstream check(filename);
        if (!check.good()) {
            printWarning("No expense file found yet. Add some expenses first.");
            pauseScreen();
            return;
        }
        check.close();

        cout << Color::DIM
                  << "  Opening your expense CSV file...\n"
                  << "  (Your default spreadsheet or CSV viewer will launch)\n\n"
                  << Color::RESET;

        bool opened = openCSVInExcel(filename);

        if (opened) {
            printSuccess("File opened! Check your taskbar / dock for the app.");
        } else {
            printError("Could not open the file automatically.");
            cout << "  You can open it manually at: " << filename << "\n";
        }
        pauseScreen();
    }

    void showLoginScreen() {
        clearScreen();
        printBanner();
        cout << "  1. Login\n";
        cout << "  2. Register\n";
        cout << "  0. Exit\n\n";

        string choice = getInput("Choose");

        if      (choice == "1") doLogin();
        else if (choice == "2") doRegister();
        else if (choice == "0") {
            cout << "\n  Goodbye!\n\n";
            exit(0);
        }
    }


    void showStudentMenu() {
        cout << "\n  MENU\n";
        cout << "  1. Add Expense\n";
        cout << "  2. View My Expenses\n";
        cout << "  3. Search / Filter\n";
        cout << "  4. Sort Expenses\n";
        cout << "  5. Set Budget Limits\n";
        cout << "  6. Delete Expense\n";
        cout << "  7. Open My CSV File\n";
        cout << "  0. Logout\n\n";

        string choice = getInput("Select");

        if      (choice == "1") addExpense();
        else if (choice == "2") viewExpenses();
        else if (choice == "3") searchExpenses();
        else if (choice == "4") sortExpenses();
        else if (choice == "5") setBudgetLimit();
        else if (choice == "6") deleteExpense();
        else if (choice == "7") viewMyCSV();
        else if (choice == "0") doLogout();
        else printError("Invalid option. Please choose from the menu.");
    }


    void showAdminMenu() {
        cout << "\n  ADMIN MENU\n";
        cout << "  1. Add Expense\n";
        cout << "  2. View ALL Expenses\n";
        cout << "  3. Search / Filter\n";
        cout << "  4. Sort Expenses\n";
        cout << "  5. Set Budget Limits\n";
        cout << "  6. Delete Any Expense\n";
        cout << "  7. Manage Users\n";
        cout << "  8. Open My CSV File\n";
        cout << "  0. Logout\n\n";

        string choice = getInput("Select");

        if      (choice == "1") addExpense();
        else if (choice == "2") viewExpenses();
        else if (choice == "3") searchExpenses();
        else if (choice == "4") sortExpenses();
        else if (choice == "5") setBudgetLimit();
        else if (choice == "6") deleteExpense();
        else if (choice == "7") manageUsers();
        else if (choice == "8") viewMyCSV();
        else if (choice == "0") doLogout();
        else printError("Invalid option. Please choose from the menu.");
    }


public:
   
    void run() {
        while (true) {
            if (currentUser == nullptr) {
                showLoginScreen();
                continue;
            }

            clearScreen();
            printBanner();
            cout << "  Logged in as: " << currentUser->username
                      << " [" << currentUser->role << "]\n";

            if (currentUser->isAdmin()) {
                showAdminMenu();
            } else {
                showStudentMenu();
            }
        }
    }
};

int main(int argc, char* argv[]) {
    setDataDir(argc > 0 ? argv[0] : ".");
    ExpenseTrackerApp app;
    app.run();
    return 0;
}