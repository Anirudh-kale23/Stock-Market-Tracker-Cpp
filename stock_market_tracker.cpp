#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <thread> 
#include <chrono> 
#include <unordered_map>
#include <random>
using namespace std;


//THE STOCK CLASS REPRESENT THE SINGLE STOCK.
class Stock {
private:
    string name;
    double currentPrice;
    vector<double> priceHistory; //IN SINGLE DAY HOW MANY TIME STOCK PRICE CHANGED.
    double heighestDayPrice;
    double lowestDayPrice;

public:
    Stock(string name, double initialPrice) : name(name), currentPrice(initialPrice) {
        priceHistory.push_back(initialPrice);
        heighestDayPrice = lowestDayPrice = initialPrice;
    }

    Stock() : name(""), currentPrice(0) {
        priceHistory.push_back(0);
    }

    //GETTER AND SETTER METHODS
    string getName() const {
        return name;
    }

    double getCurrentPrice() const {
        return currentPrice;
    }

    const vector<double>& getPriceHistory() const {
        return priceHistory;
    }

    //UPDATE PRICE OF THE STOCK
    void updatePrice(double newPrice) {
        currentPrice = newPrice;
        priceHistory.push_back(newPrice);

        if (lowestDayPrice > newPrice) {
            lowestDayPrice = newPrice;
        }
        else if (newPrice > heighestDayPrice) {
            heighestDayPrice = newPrice;
        }
    }

    //DISPLAY STOCK NAME, CURRENT PRICE AND DAY HEIGHEST AND LOWEST PRICE OF STOCK
    void displayStockInfo() const {
        cout << "Stock: " << name << endl;
        cout << "Current Price: Rs " << currentPrice << endl;
        cout << "HeighestDayPrice: Rs " << heighestDayPrice << "  LowestDayPrice: Rs " << lowestDayPrice << endl;
        cout << endl;
    }
};


// THE PORTFOLIO CLASS TO MANAGE USER PORTFOLIO  -------------------------------------------------------
class Portfolio {
private:
    unordered_map<string, int> stockQuantities;
    unordered_map<string, double> stockPrices;

public:
    //ADD STOCK IN PORTFOLIO
    void addStock(const string& name, int quantity, double price) {
        if (stockQuantities.find(name) != stockQuantities.end()) {
            int oldQuantity = stockQuantities[name];
            double oldPrice = stockPrices[name];
            double newAveragePrice = ((oldQuantity * oldPrice) + (quantity * price)) / (oldQuantity + quantity);

            stockQuantities[name] += quantity;
            stockPrices[name] = newAveragePrice;
        }
        else {
            stockQuantities[name] = quantity;
            stockPrices[name] = price;
        }
    }
    //SELL STOCK FROM PORTFOLIO
    bool sellStock(const string& name, int quantity) {

        if (stockQuantities[name] >= quantity) {
            stockQuantities[name] -= quantity;
            if (stockQuantities[name] == 0) {
                stockQuantities.erase(name);
                stockPrices.erase(name);
            }
            return true;
        }
        return false;
    }
    //DISPLAY ALL STOCKS IN USER PORTFOLIO
    void displayPortfolio(const unordered_map<string, Stock>& stockTracker) const {
        double totalInvested = 0.0;
        double totalCurrentValue = 0.0;

        cout << endl << "--- Portfolio Details ---" << endl;
        for (const auto& entry : stockQuantities) {
            const string& name = entry.first;
            int quantity = entry.second;
            double currentPrice = stockTracker.at(name).getCurrentPrice();
            double buyPrice = stockPrices.at(name);

            double investedAmount = buyPrice * quantity;
            double currentAmount = currentPrice * quantity;

            totalInvested += investedAmount;
            totalCurrentValue += currentAmount;

            cout << "Stock: " << name << " | Quantity: " << quantity << " | Buy Price: Rs " << buyPrice << " | Current Price: Rs " << currentPrice << endl;
        }

        cout << endl << "----- Portfolio Summary -----" << endl;
        cout << "Total Invested: Rs " << totalInvested << endl;
        cout << "Total Current Value: Rs " << totalCurrentValue << endl;
        cout << "Portfolio Profit: Rs " << totalCurrentValue - totalInvested << endl;
    }
};


//THE STOCK TRACKER CLASS WHICH MANAGE ALL THE STOCK LISTED IN STOCK MARCKET -----------------------------------------------------
class StockTracker {
private:
    unordered_map<string, Stock> stocks;//MAP TO STORE THE NAME OF STOCK AND OBJECT

    //GENERATE RANDOM VALUE OF STOCK PRICES. ENSURE NATURAL FLUCTUATIONS IN STOCK PRICES
    double generateRandomChange(double currentPrice) {
        static random_device rd;
        static mt19937 gen(rd());
        uniform_real_distribution<> dis(0, 0.1 * currentPrice);

        double valueToAdd = dis(gen);
        double valueToSubtract = dis(gen);

        return currentPrice + valueToAdd - valueToSubtract;
    }

public:
    //Add stock in stock market with its name and initial stock price
    void addStock(const string& name, double initialPrice) {

        if (stocks.find(name) != stocks.end()) {
            cout << "Stock " << name << " already exists in the tracker with a price of Rs "
                << stocks[name].getCurrentPrice() << endl;
        }
        else {
            stocks[name] = Stock(name, initialPrice);
            cout << "Stock " << name << " added with an initial price of Rs " << initialPrice << endl;
        }
    }

    //CALL GENERATERANDOMCHANGE FUNCTION FOR EVERY STOCK IN STOCK MARKET AND CHECK THRESHOLDS VALUE FOR PARTICULAR STOCK
    void priceChanges(unordered_map<string, pair<double, double>>& thresholds) {
        for (auto& entry : stocks) {
            Stock& stock = entry.second;
            double randomChange = generateRandomChange(stock.getCurrentPrice());

            if (randomChange > 0) {
                stock.updatePrice(randomChange);
            }
        }
        //CHECKS IF THE STOCK PRICE MEETS USER-DEFINED THRESHOLDS (MIN AND MAX).
        for (auto it = thresholds.begin(); it != thresholds.end(); ) {
            const string& stockName = it->first;
            const pair<double, double>& thresholdValue = it->second;

            if (stocks.find(stockName) != stocks.end()) {
                const Stock& stock = stocks[stockName];
                double currentPrice = stock.getCurrentPrice();

                if (currentPrice >= thresholdValue.first && currentPrice <= thresholdValue.second) {
                    cout << endl << "*** ALERT ***" << endl;
                    cout << "Stock " << stockName << " has reached the threshold price of Rs " << thresholdValue.first << " and Rs " << thresholdValue.second << endl;

                    it = thresholds.erase(it);
                    continue;
                }
            }

            ++it;
        }
    }


    const unordered_map<string, Stock>& getStocks() const {
        return stocks;
    }

    //DISPLAY ALL STOCKS IN STOCK MARKET
    void displayAllStocks() const {
        cout << endl << "--- Stock Market ---" << endl;
        for (const auto& entry : stocks) {
            entry.second.displayStockInfo();
        }
    }
};

//RUNS IN A SEPARATE THREAD TO PERIODICALLY UPDATE STOCK PRICES
//AND MONITOR USER-DEFINED THRESHOLDS.
void priceUpdates(StockTracker& tracker, unordered_map<string, pair<double, double>>& thresholds) {
    while (true) {
        tracker.priceChanges(thresholds);
        this_thread::sleep_for(chrono::seconds(20));
    }
}

//THE USER CLASS REPRESENTS AS INDIVIDUAL USER, MANAGING THEIR PORTFOLIO AND ACCOUNT BALANCE
//AND STORES USER CREDENTIALS FOR LOGIN PURPOSES.
class User {
private:
    string name;
    Portfolio portfolio;
    string loginId;
    string password;
    double balance = 0;
public:

    User() : name(""), loginId(""), password() {}

    User(string name, string login, string password) : name(name), loginId(login), password(password) {}

    string getName() {
        return name;
    }
    string getLoginId() {
        return loginId;
    }
    string getPassword() {
        return password;
    }

    Portfolio& getPortfolio() {
        return portfolio;
    }

    double getBalance() {
        return balance;
    }
    //IT ALLOWS FOR BALANCE UPDATES.
    void addBalance(double amount) {
        this->balance += amount;
        cout << "Amount added successfully" << endl;
    }

    void withdrawAmount(double amount) {
        if (amount > balance) { cout << "You don't have enough money to withdraw" << endl; return; }
        this->balance -= amount;
    }

    //UPDATE BALANCE AFTER BUYING STOCK
    void updateAccBalance(double amount) {
        this->balance += amount;
    }

    void displayUserInfo() {
        cout << "Name: " << name << " | Login ID: " << loginId << endl;
    }
};

//THE USERLIST CLASS MANAGES A LIST OF REGISTERED USERS  ---------------------------------------------------------------
class UserList {

public:
    unordered_map<string, User> users;

    bool registerUser(const string& name, const string& loginId, const string& password) {
        /*if (users.find(loginId) != users.end()) {
            cout << "User with login ID " << loginId << " already exists " << endl;
            return false;
        }*/
        users[loginId] = User(name, loginId, password);
        cout << "User " << name << " registered successfully" << endl;
        return true;
    }

    //LOGIN USER TO CHECK LOGINID AND PASSWORD IS CORRECT OR NOT
    User* loginUser(const string& loginId, const string& password) {
        auto it = users.find(loginId);
        if (it != users.end() && it->second.getPassword() == password) {
            cout << "Login successfull Welcome, " << it->second.getName() << endl;
            return &it->second;
        }
        cout << "Invalid login ID or password" << endl;
        return nullptr;
    }

    void displayAllUsers() {
        cout << endl << "--- Registered Users ---" << endl;
        for (auto& entry : users) {
            entry.second.displayUserInfo();
        }
    }
};

//--------------------------------------------------------------------------------------------------
int main() {
    StockTracker stockTracker;
    UserList userList;

    //ADDING INITIAL STOCKS TO THE SYSTEM.
    stockTracker.addStock("X", 150.0);
    stockTracker.addStock("GOOG", 280.0);
    stockTracker.addStock("AMZN", 300.0);
    stockTracker.addStock("X", 150.0);

    User* currentUser = nullptr; //POINTER TO TRACK THE CURRENTLY LOGGED-IN USER.
    unordered_map<string, pair<double, double>> thresholds;

    //RUNS IN A SEPARATE THREAD TO PERIODICALLY UPDATE STOCK PRICES.
    thread priceUpdateThread(priceUpdates, ref(stockTracker), ref(thresholds));
    priceUpdateThread.detach();

    while (true) {

        if (!currentUser) {  //IF NO USER IS LOGGED IN, SHOW REGISTRATION/LOGIN OPTIONS.
            system("cls");
            cout << "Welcome to the Stock Management System" << endl;
            cout << "1. Register" << endl;
            cout << "2. Login" << endl;
            cout << "3. Exit" << endl;
            cout << "Enter your choice: ";
            string choice;
            cin >> choice;

            if (choice == "1") {
                //REGISTER A NEW USER
                string name, loginId, password;
                cout << "Enter your name: ";
                cin.ignore();
                getline(cin, name);
                /*cin >> name;*/
                // Validate name length
                if (name.size() < 4 || name.size() > 24) {
                    cout << "Invalide Name. The Name must be between 4 and 12 characters. Your Name has " << name.size() << " characters.\n";
                    cin.ignore();
                    cin.get();
                    continue;
                }
                cout << "Enter a login ID: ";
                cin >> loginId;
                // VALIDATE LOGINID LENGTH
                if (loginId.size() < 4 || loginId.size() > 8) {
                    cout << "Invalide Login Id. The Login Id must be between 4 and 8 characters. Your Login It has " << loginId.size() << " characters.\n";
                    cin.ignore();
                    cin.get();
                    continue;
                }

                if (userList.users.find(loginId) != userList.users.end()) {
                    cout << "User with login ID " << loginId << " already exists " << endl;
                    cin.ignore();
                    cin.get();
                    continue;
                }
                cout << "Enter a password: ";
                cin >> password;
                //VALIDATE PASSWORD LENGTH
                if (password.size() < 4 || password.size() > 8) {
                    cout << "Invalide Password. The Password must be between 4 and 8 characters. Your Password has " << name.size() << " characters.\n";
                    cin.ignore();
                    cin.get();
                    continue;
                }
                userList.registerUser(name, loginId, password);
            }
            else if (choice == "2") {
                //LOGIN AN EXISTING USER
                string loginId, password;
                cout << "Enter your login ID: ";
                cin >> loginId;
                cout << "Enter your password: ";
                cin >> password;
                currentUser = userList.loginUser(loginId, password);
            }
            else if (choice == "3") {
                //EXIT THE APPLICATION
                cout << "Exiting the system. Goodbye" << endl;
                break;
            }
            else {
                cout << "Invalid choice. Please try again." << endl;
            }

            cin.ignore();
            cin.get();
        }
        else {
            //USER IS LOGGED IN, SHOW MAIN MENU OPTIONS
            system("cls");
            cout << "Welcome, " << currentUser->getName() << endl;
            cout << endl << "Current Balance : " << currentUser->getBalance() << endl;
            stockTracker.displayAllStocks(); //SHOW ALL AVAILABLE STOCKS.

            cout << endl << "Choose an option:" << endl;
            cout << "1. View Portfolio" << endl;
            cout << "2. Buy Stock" << endl;
            cout << "3. Sell Stock" << endl;
            cout << "4. Set Threshold for Stock" << endl;
            cout << "5. View Stocks Available in the Market" << endl;
            cout << "6. Withdraw Amount" << endl;
            cout << "7. Add Balance" << endl;
            cout << "8. Logout" << endl;
            cout << "Enter your choice: ";
            int choice;
            //VALIDATE USER INPUT FOR NUMERIC MENU CHOICES.
            if (!(cin >> choice) || choice <= 0) {
                cin.clear(); //CLEAR THE ERROR FLAG.
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Invalid Choice. Please enter a positive number." << endl;
                cin.get();
                continue;
            }

            Portfolio& portfolio = currentUser->getPortfolio();

            switch (choice) {
            case 1: {
                //VIEW USER'S PORTFOLIO.
                portfolio.displayPortfolio(stockTracker.getStocks());
                break;
            }
            case 2: {
                //BUY STOCKS
                string stockName;
                int quantity;
                cout << "Enter the stock name to buy: ";
                cin >> stockName;

                if (stockTracker.getStocks().find(stockName) != stockTracker.getStocks().end()) {
                    //GET THE CURRENT PRICE OF THE STOCK.
                    double buyPrice = stockTracker.getStocks().at(stockName).getCurrentPrice();
                    cout << "Current price of " << stockName << " is Rs " << buyPrice << endl;

                    cout << "Enter the quantity to buy: ";
                    //VALIDATE QUANTITY INPUT.
                    if (!(cin >> quantity) || quantity <= 0) {
                        cin.clear(); //CLEAR THE ERROR FLAG.
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        cout << "Invalid quantity. Please enter a positive number." << endl;
                        cin.get();
                        continue;
                    }

                    buyPrice = stockTracker.getStocks().at(stockName).getCurrentPrice();
                    //CHECK IF USER HAS SUFFICIENT BALANCE.
                    if (currentUser->getBalance() < quantity * buyPrice) {
                        cout << "You don't have enough money to buy stock. Please add money." << endl;
                        break;
                    }
                    currentUser->updateAccBalance(-quantity * buyPrice);
                    portfolio.addStock(stockName, quantity, buyPrice);
                    cout << "Bought " << quantity << " of " << stockName << " at Rs " << buyPrice << endl;
                }
                else {
                    cout << "Stock not found in the tracker" << endl;
                }
                break;
            }
            case 3: {
                // SELL STOCKS
                string stockName;
                int quantity;
                cout << "Enter the stock name to sell: ";
                cin >> stockName;

                if (stockTracker.getStocks().find(stockName) != stockTracker.getStocks().end()) {
                    //GET THE CURRENT PRICE OF THE STOCK.
                    double currentPrice = stockTracker.getStocks().at(stockName).getCurrentPrice();
                    cout << "Current price of " << stockName << " is Rs " << currentPrice << endl;

                    cout << "Enter the quantity to sell: ";
                    //VALIDATE QUANTITY INPUT.
                    if (!(cin >> quantity) || quantity <= 0) {
                        cin.clear(); //CLEAR THE ERROR FLAG.
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        cout << "Invalid quantity. Please enter a positive number." << endl;
                        /*cin.ignore();*/
                        cin.get();
                        continue;
                    }


                    if (portfolio.sellStock(stockName, quantity)) {
                        currentPrice = stockTracker.getStocks().at(stockName).getCurrentPrice();
                        currentUser->updateAccBalance(quantity * currentPrice);
                        cout << "Sold " << quantity << " of " << stockName << endl;
                    }
                    else {
                        cout << "Not enough quantity to sell" << endl;
                    }
                }
                else {
                    cout << "Stock not found in the tracker" << endl;
                }
                break;
            }
            case 4: {
                //SET PRICE THRESHOLD FOR STOCKS.
                string stockName;
                int minThreshold, maxThreshold;
                cout << "Enter the stock name to set a threshold: ";
                cin >> stockName;

                if (stockTracker.getStocks().find(stockName) != stockTracker.getStocks().end()) {
                    cout << "Enter the minimum threshold price for " << stockName << ": ";
                    //VALIDATE MINIMUM THRESHOLD.
                    if (!(cin >> minThreshold) || minThreshold <= 0) {
                        cin.clear(); //CLEAR THE ERROR FLAG.
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        cout << "Invalid maxThreshold. Please enter a positive number." << endl;
                        /*cin.ignore();*/
                        cin.get();
                        continue;
                    }

                    string str = to_string(minThreshold);
                    if (str.size() > 9) {
                        cout << "Invalide MinThreshold. Please Enter Valid MinThreshold";
                        break;
                    }

                    cout << "Enter the maximum threshold price for " << stockName << ": ";
                    //VALIDATE MAXIMUM THRESHOLD.
                    if (!(cin >> maxThreshold) || maxThreshold <= 0) {
                        cin.clear(); //CLEAR THE ERROR FLAG.
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        cout << "Invalid minThreshold. Please enter a positive number." << endl;
                        /*cin.ignore();*/
                        cin.get();
                        continue;
                    }

                    str = to_string(maxThreshold);
                    if (str.size() > 9 && maxThreshold > minThreshold) {
                        cout << "Invalide MaxThreshold. Please Enter Valid MaxThreshold";
                        break;
                    }
                    thresholds[stockName] = { minThreshold, maxThreshold };
                    cout << "Thresholds set for " << stockName << ": Min = Rs " << minThreshold << ", Max = Rs " << maxThreshold << endl;
                }
                else {
                    cout << "Stock not found in the tracker" << endl;
                }
                break;
            }
            case 5: {
                //VIEW ALL AVAILABLE STOCKS.
                stockTracker.displayAllStocks();
                break;
            }
            case 6: {
                //WITHDRAW AMOUNT FROM USER BALANCE.
                int amount;
                cout << "Please enter amount : ";

                //VALIDATE AMOUNT.
                if (!(cin >> amount) || amount <= 0) {
                    cin.clear(); //CLEAR THE ERROR FLAG/FAIL STATE.
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    cout << "Invalid amount. Please enter a positive number." << endl;
                    cin.get();
                    continue;
                }

                string str = to_string(amount);
                if (str.size() > 9) {
                    cout << "Invalid Amount. Please Enter Valid Amount";
                    break;
                }
                if (currentUser->getBalance() < amount) {
                    cout << "Invalide amount" << endl;
                    break;
                }
                currentUser->withdrawAmount(amount);
                cout << "Transaction is successful" << endl;
                break;

            }
            case 7: {
                //ADD BALANCE TO USER ACCOUNT.
                int balance;
                cout << "Please enter amount : ";

                //VALIDATE BALANCE INPUT.
                if (!(cin >> balance) || balance <= 0) {
                    cin.clear(); //CLEAR THE ERROR FLAG/FAIL STATE.
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    cout << "Invalid amount. Please enter a valid number." << endl;
                    cin.get();
                    continue;
                }


                string str = to_string(balance);
                if (str.size() >= 9 || str.size() <= 0) {
                    cout << "Invalid Amount. Please Enter Valid Amount";
                    break;
                }
                currentUser->addBalance(balance);
                break;
            }
            case 8: {
                //LOGOUT USERS
                currentUser = nullptr;
                cout << "Logged out successfully." << endl;
                thresholds.clear(); //CLEAR THREASHOLD AFTER LOGOUT
                break;
            }
            default: {
                cout << "Invalid option. Please choose a valid option." << endl;
                break;
            }
            }

            cin.ignore();
            cin.get();
        }
    }

    return 0;
}