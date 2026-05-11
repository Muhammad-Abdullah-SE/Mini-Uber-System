#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <ctime>
#include <sstream>

using namespace std;

// -------- DATA MODELS --------
struct Rider {
    int id;
    string name;
    string contact;
    string password; 
    int totalBookings = 0;
};

struct Driver {
    int id;
    string name;
    string contact;
    string password; 
    bool isFree = true;
    double rating = 5.0;
    double earnings = 0.0;
    int totalRides = 0;
};

struct Ride {
    int id, riderID, driverID;
    double distance, fare;
    string status; // Pending, Ongoing, Completed, Cancellede;
    string startTime;
    string endTime;
};

// -------- INPUT VALIDATION --------
int getIntInput(const string& prompt) {
    int value;
    while (true) {
        cout << prompt;
        if (cin >> value) {
            cin.ignore();
            return value;
        } else {
            cout << " Please enter a valid integer.\n";
            cin.clear();
            cin.ignore(10000, '\n');
        }
    }
}

string getValidatedContact(const string& prompt) {
    string contact;
    while (true) {
        cout << prompt;
        getline(cin, contact);
        bool valid = true;
        for (char c : contact) if (!isdigit(c)) { valid = false; break; }
        if (valid && !contact.empty()) return contact;
        cout << " Please enter digits only.\n";
    }
}

string getValidatedName(const string& prompt) {
    string name;
    while (true) {
        cout << prompt;
        getline(cin, name);
        bool valid = true;
        for (char c : name) if (!isalpha(c) && c != ' ') { valid = false; break; }
        if (valid && !name.empty()) return name;
        cout << " Please enter letters and spaces only.\n";
    }
}

double getValidatedRating(const string& prompt) {
    double rating;
    while (true) {
        cout << prompt;
        if (cin >> rating && rating >= 1 && rating <= 5) {
            cin.ignore();
            return rating;
        } else {
            cout << " Please enter a rating between 1 and 5.\n";
            cin.clear();
            cin.ignore(10000, '\n');
        }
    }
}

int getMenuChoice(const string& prompt, int minChoice, int maxChoice) {
    int choice;
    while (true) {
        cout << prompt;
        if (cin >> choice && choice >= minChoice && choice <= maxChoice) {
            cin.ignore();
            return choice;
        } else {
            cout << " Please enter a number between " << minChoice << " and " << maxChoice << ".\n";
            cin.clear();
            cin.ignore(10000, '\n');
        }
    }
}

// -------- MAIN CLASS --------
class MiniUber {
private:
    vector<Driver> drivers;
    vector<Rider> riders;
    queue<Ride> requests;
    vector<Ride> activeRides;
    vector<Ride> history;
    vector<Ride> cancellations;

    // Helper functions for Binary Search
    int binarySearchDriver(int id) {
        int left = 0, right = (int)drivers.size() - 1;
        while (left <= right) {
            int mid = left + (right - left) / 2;
            if (drivers[mid].id == id) return mid;
            if (drivers[mid].id < id) left = mid + 1;
            else right = mid - 1;
        }
        return -1;
    }

    int binarySearchRider(int id) {
        int left = 0, right = (int)riders.size() - 1;
        while (left <= right) {
            int mid = left + (right - left) / 2;
            if (riders[mid].id == id) return mid;
            if (riders[mid].id < id) left = mid + 1;
            else right = mid - 1;
        }
        return -1;
    }

public:
    MiniUber() { loadData(); }

    // -------- FILE HANDLING --------
void loadData() {
    string line, token;

    // ---------- LOAD DRIVERS ----------
    ifstream dFile("drivers.csv");
    while (getline(dFile, line)) {
        if (line.empty() || line.find("ID") != string::npos) continue;

        stringstream ss(line);
        Driver d;

        getline(ss, token, ','); d.id = stoi(token);
        getline(ss, d.name, ',');
        getline(ss, d.contact, ',');
        
        // We read next 'token'. If it's 0/1, it's the old 'Free' field.
        getline(ss, token, ',');
        if (token == "0" || token == "1") {
            // Old format detection
            d.password = "1234"; 
            d.isFree = (token == "1");
            getline(ss, token, ','); d.rating = stod(token);
            getline(ss, token, ','); d.earnings = stod(token);
            getline(ss, token);      d.totalRides = stoi(token);
        } else {
            // New format
            d.password = token;
            getline(ss, token, ','); d.isFree = (token == "1");
            getline(ss, token, ','); d.rating = stod(token);
            getline(ss, token, ','); d.earnings = stod(token);
            getline(ss, token);      d.totalRides = stoi(token);
        }

        drivers.push_back(d);
    }

    // ---------- LOAD RIDERS ----------
    ifstream rFile("riders.csv");
    while (getline(rFile, line)) {
        if (line.empty() || line.find("ID") != string::npos) continue;

        stringstream ss(line);
        Rider r;

        getline(ss, token, ','); r.id = stoi(token);
        getline(ss, r.name, ',');
        getline(ss, r.contact, ',');
        

        getline(ss, token, ','); 
     
        string nextToken;
        if (getline(ss, nextToken)) {
            // We have more data, so 'token' was password, 'nextToken' is totalBookings (maybe with \r)
            r.password = token;
            r.totalBookings = stoi(nextToken);
        } else {
            // 'token' was the last field (TotalBookings)
            r.password = "1234";
            r.totalBookings = stoi(token);
        }

        riders.push_back(r);
    }

    // ---------- LOAD COMPLETED RIDES ----------
    ifstream cFile("completed_rides.csv");
    while (getline(cFile, line)) {
        if (line.empty() || line.find("ID") != string::npos) continue;

        stringstream ss(line);
        Ride r;

        getline(ss, token, ','); r.id = stoi(token);
        getline(ss, token, ','); r.riderID = stoi(token);
        getline(ss, token, ','); r.driverID = stoi(token);
        getline(ss, token, ','); r.distance = stod(token);
        getline(ss, token, ','); r.fare = stod(token);
        getline(ss, r.status, ',');
        getline(ss, r.startTime, ',');
        getline(ss, r.endTime);

        history.push_back(r);
    }

    // ---------- LOAD CANCELLED RIDES ----------
    ifstream xFile("cancelled_rides.csv");
    while (getline(xFile, line)) {
        if (line.empty() || line.find("ID") != string::npos) continue;

        stringstream ss(line);
        Ride r;

        getline(ss, token, ','); r.id = stoi(token);
        getline(ss, token, ','); r.riderID = stoi(token);
        getline(ss, token, ','); r.driverID = stoi(token);
        getline(ss, token, ','); r.distance = stod(token);
        getline(ss, token, ','); r.fare = stod(token);
        getline(ss, r.status, ',');
        getline(ss, r.startTime, ',');
        getline(ss, r.endTime);

        cancellations.push_back(r);
    }

    // Sort data for Binary Search
    sort(drivers.begin(), drivers.end(), [](const Driver& a, const Driver& b) { return a.id < b.id; });
    sort(riders.begin(), riders.end(), [](const Rider& a, const Rider& b) { return a.id < b.id; });
}


    void saveData() {
        ofstream dFile("drivers.csv");
        dFile << "ID,Name,Contact,Password,Free,Rating,Earnings,TotalRides\n";
        for (auto& d : drivers) {
            dFile << d.id << "," << d.name << "," << d.contact << "," << d.password << ","
                  << (d.isFree ? "1" : "0") << "," 
                  << d.rating << "," << d.earnings << "," << d.totalRides << "\n";
        }

        ofstream rFile("riders.csv");
        rFile << "ID,Name,Contact,Password,TotalBookings\n";
        for (auto& r : riders) {
            rFile << r.id << "," << r.name << "," << r.contact << "," << r.password << "," << r.totalBookings << "\n";
        }

        ofstream rideFile("completed_rides.csv");
        rideFile << "ID,RiderID,DriverID,Distance,Fare,Status,StartTime,EndTime\n";
        for (auto& r : history) {
            rideFile << r.id << "," << r.riderID << "," << r.driverID << ","
                     << r.distance << "," << r.fare << "," << r.status << "," 
                     << r.startTime << "," << r.endTime << "\n";
        }

        ofstream cancelFile("cancelled_rides.csv");
        cancelFile << "ID,RiderID,DriverID,Distance,Fare,Status,StartTime,EndTime\n";
        for (auto& r : cancellations) {
            cancelFile << r.id << "," << r.riderID << "," << r.driverID << ","
                       << r.distance << "," << r.fare << "," << r.status << "," 
                       << r.startTime << "," << r.endTime << "\n";
        }

        cout << "\n  [System] Data saved successfully.\n";
    }

    // -------- REGISTRATION --------
void registerUser(int type) {
    cout << "\n--------------------------------------------------\n";
    cout << (type == 1 ? "           DRIVER REGISTRATION\n"
                       : "           RIDER REGISTRATION\n");
    cout << "--------------------------------------------------\n";

    int id;
    if (type == 1) { // Driver
        id = drivers.empty() ? 101 : drivers.back().id + 1;
    } else { // Rider
        id = riders.empty() ? 201 : riders.back().id + 1;
    }

    cout << " [System] Assigning New ID: " << id << "\n";

    string name = getValidatedName("Enter Name: ");
    string contact = getValidatedContact("Enter Contact: ");
    
    string password;
    while (true) {
        cout << "Enter Password (min 6 chars): ";
        cin >> password;
        if (password.length() >= 6) {
            cin.ignore(); // Clear buffer
            break;
        }
        cout << " ? Password too short! Must be at least 6 characters.\n";
    }

    if (type == 1) {
        drivers.push_back({id, name, contact, password, true, 5.0, 0.0, 0});
        sort(drivers.begin(), drivers.end(), [](const Driver& a, const Driver& b) { return a.id < b.id; });
    } else {
        riders.push_back({id, name, contact, password, 0});
        sort(riders.begin(), riders.end(), [](const Rider& a, const Rider& b) { return a.id < b.id; });
    }

    cout << "\n? Registration successful! Your ID is " << id << ". Please remember it for login.\n";
}


    // -------- SEARCH (Linear Search by ID) --------
    void searchUserByID() {
        cout << "\n--------------------------------------------------\n";
        cout << "            SEARCH USER\n";
        cout << "--------------------------------------------------\n";
        
        int type = getMenuChoice("1. Search Driver\n2. Search Rider\n\nChoice: ",1,2);
        int id = getIntInput("\nEnter ID to search: ");

        if (type == 1) {
            int idx = binarySearchDriver(id);
            if (idx != -1) {
                Driver& d = drivers[idx];
                cout << "\n--------------------------------------------------\n";
                cout << "           DRIVER FOUND\n";
                cout << "--------------------------------------------------\n";
                cout << left << setw(10) << "ID" 
                        << setw(20) << "Name" 
                        << setw(15) << "Contact"
                        << setw(10) << "Rating" 
                        << setw(12) << "Earnings"
                        << setw(10) << "Rides" << "\n";
                cout << string(77, '-') << "\n";
                cout << left << setw(10) << d.id 
                        << setw(20) << d.name
                        << setw(15) << d.contact
                        << setw(10) << d.rating
                        << setw(12) << d.earnings
                        << setw(10) << d.totalRides << "\n";
                return;
            }
            cout << "\n Driver not found.\n";
        } else {
            int idx = binarySearchRider(id);
            if (idx != -1) {
                Rider& r = riders[idx];
                cout << "\n--------------------------------------------------\n";
                cout << "           RIDER FOUND\n";
                cout << "--------------------------------------------------\n";
                cout << left << setw(10) << "ID" 
                        << setw(20) << "Name" 
                        << setw(15) << "Contact"
                        << setw(15) << "Total Bookings" << "\n";
                cout << string(60, '-') << "\n";
                cout << left << setw(10) << r.id 
                        << setw(20) << r.name 
                        << setw(15) << r.contact
                        << setw(15) << r.totalBookings << "\n";
                return;
            }
            cout << "\n Rider not found.\n";
        }
    }

    bool verifyUser(int type, int id, string pass = "") {
        if (type == 2) {
            int idx = binarySearchDriver(id);
            if (idx == -1) {
                cout << "\n Error: Driver Account not found with ID " << id << ".\n";
                return false;
            }
            if (pass != "" && drivers[idx].password != pass) {
                cout << "\n Error: Incorrect Password. Please try again.\n";
                return false;
            }
            return true;
        } else {
            int idx = binarySearchRider(id);
            if (idx == -1) {
                cout << "\n Error: Rider Account not found with ID " << id << ".\n";
                return false;
            }
            if (pass != "" && riders[idx].password != pass) {
                 cout << "\n Error: Incorrect Password. Please try again.\n";
                 return false;
            }
            return true;
        }
    }

    void forgotPassword(int Role) {
         // Role: 1=Admin, 2=Driver, 3=Rider
         cout << "\n--------------------------------------------------\n";
         cout << "             PASSWORD RECOVERY\n";
         cout << "--------------------------------------------------\n";

         if (Role == 1) {
             string recoveryKey;
             cout << "Enter System Recovery Key: ";
             cin >> recoveryKey;
             if (recoveryKey == "root") { // Hardcoded recovery for Admin
                 cout << "\n[System] Admin Password is: 12345678\n";
             } else {
                 cout << "\n? Invalid Key. Contact Developers.\n";
             }
         } else {
             int id = getIntInput("Enter your ID: ");
             string contact = getValidatedContact("Enter your registered Contact: ");
             
             if (Role == 2) {
                 int idx = binarySearchDriver(id);
                 if (idx != -1 && drivers[idx].contact == contact) {
                     cout << "\n[System] Your Password: " << drivers[idx].password << "\n";
                 } else {
                     cout << "\n? Verification Failed. User not found or details mismatch.\n";
                 }
             } else {
                 int idx = binarySearchRider(id);
                 if (idx != -1 && riders[idx].contact == contact) {
                     cout << "\n[System] Your Password: " << riders[idx].password << "\n";
                 } else {
                      cout << "\n? Verification Failed. User not found or details mismatch.\n";
                 }
             }
         }
    }

    // -------- UPDATE USER INFO --------
    void updateUserInfo() {
        cout << "\n--------------------------------------------------\n";
        cout << "           UPDATE USER INFO\n";
        cout << "--------------------------------------------------\n";
        
        int type = getMenuChoice("1. Update Driver\n2. Update Rider\n\nChoice: ",1,2);
        int id = getIntInput("\nEnter ID to update: ");

        if (type == 1) {
            int idx = binarySearchDriver(id);
            if (idx != -1) {
                Driver& d = drivers[idx];
                cout << "\n-- Updating Driver Information --\n";
                string name = getValidatedName("Enter new Name: ");
                string contact = getValidatedContact("Enter new Contact: ");
                int statusChoice = getMenuChoice("Status (1-Free, 0-Busy): ",0,1);
                double rating = getValidatedRating("Enter Rating (1-5): ");

                d.name = name;
                d.contact = contact;
                d.isFree = (statusChoice == 1);
                d.rating = rating;

                cout << "\n Driver info updated successfully.\n";
                return;
            }
            cout << "\n Driver not found.\n";
        } else {
            int idx = binarySearchRider(id);
            if (idx != -1) {
                Rider& r = riders[idx];
                cout << "\n-- Updating Rider Information --\n";
                string name = getValidatedName("Enter new Name: ");
                string contact = getValidatedContact("Enter new Contact: ");
                r.name = name;
                r.contact = contact;
                cout << "\n Rider info updated successfully.\n";
                return;
            }
            cout << "\n Rider not found.\n";
        }
    }

    // -------- SORTING --------
    void bubbleSortDriversByRating() {
        vector<Driver> v = drivers;
        for (size_t i = 0; i < v.size(); i++)
            for (size_t j = 0; j < v.size() - i - 1; j++)
                if (v[j].rating < v[j+1].rating)
                    swap(v[j], v[j+1]);

        cout << "\n--------------------------------------------------\n";
        cout << "       DRIVERS BY RATING (HIGHEST)\n";
        cout << "--------------------------------------------------\n";
        cout << left << setw(10) << "ID" 
             << setw(20) << "Name" 
             << setw(15) << "Contact"
             << setw(10) << "Rating" 
             << setw(12) << "Earnings"
             << setw(10) << "Rides" << "\n";
        cout << string(77, '-') << "\n";
        for (auto& d : v)
            cout << left << setw(10) << d.id 
                 << setw(20) << d.name
                 << setw(15) << d.contact
                 << setw(10) << d.rating
                 << setw(12) << d.earnings
                 << setw(10) << d.totalRides << "\n";
    }

    void bubbleSortDriversByName() {
        vector<Driver> v = drivers;
        for (size_t i = 0; i < v.size(); i++)
            for (size_t j = 0; j < v.size() - i - 1; j++)
                if (v[j].name > v[j+1].name)
                    swap(v[j], v[j+1]);

        cout << "\n--------------------------------------------------\n";
        cout << "       DRIVERS ALPHABETICALLY (A-Z)\n";
        cout << "--------------------------------------------------\n";
        cout << left << setw(10) << "ID" 
             << setw(20) << "Name" 
             << setw(15) << "Contact"
             << setw(10) << "Rating" 
             << setw(12) << "Earnings"
             << setw(10) << "Rides" << "\n";
        cout << string(77, '-') << "\n";
        for (auto& d : v)
            cout << left << setw(10) << d.id 
                 << setw(20) << d.name
                 << setw(15) << d.contact
                 << setw(10) << d.rating
                 << setw(12) << d.earnings
                 << setw(10) << d.totalRides << "\n";
    }

    // -------- RIDES --------
    void bookRide(int rid) {
        cout << "\n--------------------------------------------------\n";
        cout << "             Enter Distance\n";
        cout << "--------------------------------------------------\n";
        
        double dist;
        cout << "Enter distance (km): ";
        while (!(cin >> dist) || dist <= 0) {
            cout << "? Please enter a valid distance.\n";
            cin.clear();
            cin.ignore(10000, '\n');
        }
        cin.ignore();

        Ride r = {(int)(history.size() + activeRides.size() + requests.size() + 1), rid, -1, dist, dist*50.0, "Pending", "", ""};
        requests.push(r);

        for (auto& rObj : riders)
            if (rObj.id == rid) rObj.totalBookings++;

        cout << "\n Ride booked successfully!\n";
        cout << " Ride Details:\n";
        cout << "    Ride ID: " << r.id << "\n";
        cout << "    Distance: " << r.distance << " km\n";
        cout << "    Fare: $" << r.fare << "\n";
        cout << "    Status: " << r.status << "\n";
    }

    void cancelRide(int riderID) {
        cout << "\n--------------------------------------------------\n";
        cout << "            CANCEL RIDE\n";
        cout << "--------------------------------------------------\n";
        
        queue<Ride> temp;
        bool cancelled = false;

        while (!requests.empty()) {
            Ride r = requests.front();
            requests.pop();
            if (!cancelled && r.riderID == riderID) {
                r.status = "Cancelled";

                time_t now = time(0);
                r.endTime = ctime(&now);
                r.endTime.pop_back(); // remove newline

                cancellations.push_back(r);
                cancelled = true;
            } else temp.push(r);
        }
        requests = temp;

        if (cancelled) {
            cout << "\n Ride cancelled successfully.\n";
        } else {
            cout << "\n  No pending ride found to cancel.\n";
        }
    }

    void assignRideToDriver() {
        cout << "\n--------------------------------------------------\n";
        cout << "         ASSIGN RIDE TO DRIVER\n";
        cout << "--------------------------------------------------\n";
        
        if (requests.empty()) { 
            cout << "\n   No pending ride requests.\n"; 
            return; 
        }

        for (auto& d : drivers) {
            if (d.isFree) {
                Ride r = requests.front(); 
                requests.pop();
                
                r.driverID = d.id;
                r.status = "Ongoing";
                
                time_t now = time(0);
                r.startTime = ctime(&now);
                r.startTime.pop_back(); // remove newline
                
                d.isFree = false; 
                
                activeRides.push_back(r);
                cout << "\n Ride assigned successfully!\n";
                cout << " 	Driver Details:\n";
                cout << "    Driver: " << d.name << "\n";
                cout << "    Driver ID: " << d.id << "\n";
                cout << "    Ride Status: " << r.status << "\n";
                cout << "    Start Time: " << r.startTime << "\n";
                return;
            }
        }
        cout << "\n No drivers available at the moment.\n";
    }

    void rateDriver(int currentRiderID) {
        cout << "\n--------------------------------------------------\n";
        cout << "        COMPLETE RIDE & RATE DRIVER\n";
        cout << "--------------------------------------------------\n";
        
        int foundIndex = -1;
        for (size_t i = 0; i < activeRides.size(); i++) {
            if (activeRides[i].riderID == currentRiderID) {
                foundIndex = i;
                break;
            }
        }

        if (foundIndex == -1) {
            cout << "\n   You have no ongoing rides to rate.\n";
            return;
        }

        Ride r = activeRides[foundIndex]; 
        int dIdx = binarySearchDriver(r.driverID);
        if (dIdx != -1) {
            Driver& d = drivers[dIdx];
                d.earnings += r.fare;
                d.totalRides++;
                double rating = getValidatedRating("\nRide Completed! Rate driver " + d.name + " (1-5): ");
                d.rating = (d.rating + rating)/2.0;
                d.isFree = true;
            }

        r.status = "Completed";

        time_t now = time(0);
        r.endTime = ctime(&now);
        r.endTime.pop_back(); // remove newline

        history.push_back(r);
        activeRides.erase(activeRides.begin() + foundIndex);

        cout << "\n Thank you for your rating! Ride marked as completed.\n";
        cout << " Ride Details:\n";
        cout << "    Start Time: " << r.startTime << "\n";
        cout << "    End Time: " << r.endTime << "\n";
    }

    void showReports() {
        cout << "\n--------------------------------------------------\n";
        cout << "                   SYSTEM REPORTS\n";
        cout << "--------------------------------------------------\n";
        
        cout << "\n RIDE STATISTICS:\n";
        cout << "    Pending Requests: " << requests.size() << "\n";
        cout << "    Ongoing Rides: " << activeRides.size() << "\n";
        cout << "    Completed Rides: " << history.size() << "\n";
        cout << "    Cancelled Rides: " << cancellations.size() << "\n";

        if (!history.empty()) {
            double totalDistance = 0, totalFare = 0;
            for (auto& r : history) { totalDistance += r.distance; totalFare += r.fare; }
            cout << "\n FINANCIAL SUMMARY:\n";
            cout << "    Total Earnings: $" << totalFare << "\n";
            cout << "    Average Distance: " << (totalDistance / history.size()) << " km\n";
            
            // Display recent completed rides with timestamps
            cout << "\n--------------------------------------------------\n";
            cout << "           RECENT COMPLETED RIDES (Last 5)\n";
            cout << "--------------------------------------------------\n";
            cout << left << setw(8) << "ID" 
                 << setw(10) << "RiderID" 
                 << setw(10) << "DriverID"
                 << setw(10) << "Distance" 
                 << setw(10) << "Fare"
                 << setw(25) << "Start Time"
                 << setw(25) << "End Time" << "\n";
            cout << string(98, '-') << "\n";
            
            // Show last 5 completed rides (or fewer if less than 5)
            int startIdx = max(0, (int)history.size() - 5);
            for (int i = startIdx; i < history.size(); i++) {
                auto& r = history[i];
                cout << left << setw(8) << r.id 
                     << setw(10) << r.riderID 
                     << setw(10) << r.driverID
                     << setw(10) << r.distance 
                     << setw(10) << r.fare
                     << setw(25) << r.startTime
                     << setw(25) << r.endTime << "\n";
            }
        }

        if (!drivers.empty()) {
            double avgRating = 0;
            for (auto& d : drivers) avgRating += d.rating;
            cout << "\n  DRIVER PERFORMANCE:\n";
            cout << "    Average Driver Rating: " << fixed << setprecision(1) << (avgRating / drivers.size()) << "/5.0\n";
        }
    }

    void showAnalytics() {
        cout << "\n--------------------------------------------------\n";
        cout << "                   SYSTEM ANALYTICS\n";
        cout << "--------------------------------------------------\n";
        
        if (!drivers.empty()) {
            Driver* mostActive = &drivers[0];
            for (auto& d : drivers) if (d.totalRides > mostActive->totalRides) mostActive = &d;
            cout << "\n MOST ACTIVE DRIVER:\n";
            cout << left << setw(10) << "ID" 
                 << setw(20) << "Name" 
                 << setw(15) << "Contact"
                 << setw(10) << "Rating" 
                 << setw(12) << "Earnings"
                 << setw(10) << "Rides" << "\n";
            cout << string(77, '-') << "\n";
            cout << left << setw(10) << mostActive->id
                 << setw(20) << mostActive->name
                 << setw(15) << mostActive->contact
                 << setw(10) << mostActive->rating
                 << setw(12) << mostActive->earnings
                 << setw(10) << mostActive->totalRides << "\n";
        }

        cout << "\n--------------------------------------------------\n";
        cout << "        DRIVER EARNINGS REPORT\n";
        cout << "--------------------------------------------------\n";
        cout << left << setw(10) << "ID" 
             << setw(20) << "Name" 
             << setw(12) << "Earnings"
             << setw(10) << "Rides" << "\n";
        cout << string(52, '-') << "\n";
        for (auto& d : drivers)
            cout << left << setw(10) << d.id 
                 << setw(20) << d.name 
                 << setw(12) << d.earnings
                 << setw(10) << d.totalRides << "\n";

        if (!riders.empty()) {
            Rider* topRider = &riders[0];
            for (auto& r : riders)
                if (r.totalBookings > topRider->totalBookings)
                    topRider = &r;
            cout << "\n--------------------------------------------------\n";
            cout << "           TOP RIDER\n";
            cout << "--------------------------------------------------\n";
            cout << left << setw(10) << "ID" 
                 << setw(20) << "Name" 
                 << setw(15) << "Contact"
                 << setw(15) << "Total Bookings" << "\n";
            cout << string(60, '-') << "\n";
            cout << left << setw(10) << topRider->id 
                 << setw(20) << topRider->name 
                 << setw(15) << topRider->contact
                 << setw(15) << topRider->totalBookings << "\n";
        }
    }

    void viewDriverProfile(int id) {
        int idx = binarySearchDriver(id);
        if (idx != -1) {
            Driver& d = drivers[idx];
            cout << "\n--------------------------------------------------\n";
            cout << "           DRIVER PROFILE\n";
            cout << "--------------------------------------------------\n";
            cout << left << setw(10) << "ID" 
                    << setw(20) << "Name" 
                    << setw(15) << "Contact"
                    << setw(10) << "Rating" 
                    << setw(12) << "Earnings"
                    << setw(10) << "Rides" << "\n";
            cout << string(77, '-') << "\n";
            cout << left << setw(10) << d.id 
                    << setw(20) << d.name
                    << setw(15) << d.contact
                    << setw(10) << d.rating
                    << setw(12) << d.earnings
                    << setw(10) << d.totalRides << "\n";
            cout << "\n Status: " << (d.isFree ? " Available" : " On Ride") << "\n";
            return;
        }
        cout << "\n Driver not found.\n";
    }
};

// -------- MAIN --------
int main() {
    MiniUber app;
    int role, id, choice;

    while (true) {
        cout << "\n";
        cout << "==================================================\n";
        cout << "            MINI UBER SYSTEM\n";
        cout << "==================================================\n";
        cout << "1. Admin Login\n";
        cout << "2. Driver Login\n";
        cout << "3. Rider Login\n";
        cout << "4. Save & Exit\n";
        cout << "==================================================\n";
        role = getMenuChoice("\nEnter your choice (1-4): ",1,4);

        if (role == 4) { 
            cout << "\n Saving data...\n";
            app.saveData(); 
            cout << "\n Thank you for using Mini Uber System!\n";
            break; 
        }

        switch (role) {
        case 1:
            // Admin Login Flow with Forgot Password
            {
                cout << "\n--------------------------------------------------\n";
                cout << "            ADMIN LOGIN\n";
                cout << "--------------------------------------------------\n";
                cout << "1. Login\n";
                cout << "2. Forgot Password?\n";
                int admChoice = getMenuChoice("Choice: ", 1, 2);

                if (admChoice == 2) {
                    app.forgotPassword(1);
                    break; // break out of case 1, back to main menu
                }

                // Normal Login
                string uName, uPass;
                cout << "\nUsername: "; 
                cin >> uName; // "admin"
                cout << "Password: ";
                cin >> uPass; // "12345678"

                if (uName != "admin") {
                    cout << "\n Error: Invalid Username.\n";
                } else if (uPass != "12345678") {
                    cout << "\n Error: Incorrect Password.\n";
                } else {
                    cout << "\n[Info] Default admin password is set by the system.\n";
                    do {
                        cout << "\n";
                        cout << "==================================================\n";
                        cout << "            ADMIN DASHBOARD\n";
                        cout << "==================================================\n";
                cout << "1.  Register Driver\n";
                cout << "2.  Register Rider\n";
                cout << "3.  Assign Ride to Driver\n";
                cout << "4.  View Reports\n";
                cout << "5.  Show Drivers by Rating\n";
                cout << "6.  Show Drivers Alphabetically\n";
                cout << "7.  Search User by ID\n";
                cout << "8.  View Analytics\n";
                cout << "9.  Update User Info\n";
                cout << "10. Logout\n";
                cout << "==================================================\n";
                
                choice = getMenuChoice("\nEnter your choice (1-10): ",1,10);

                if (choice == 1) app.registerUser(1);
                else if (choice == 2) app.registerUser(2);
                else if (choice == 3) app.assignRideToDriver();
                else if (choice == 4) app.showReports();
                else if (choice == 5) app.bubbleSortDriversByRating();
                else if (choice == 6) app.bubbleSortDriversByName();
                else if (choice == 7) app.searchUserByID();
                else if (choice == 8) app.showAnalytics();
                else if (choice == 9) app.updateUserInfo();
                    } while (choice != 10);
                    cout << "\n Logging out from Admin Panel...\n";
                }
            }
            break;

        case 2: {
            cout << "\n--------------------------------------------------\n";
            cout << "           DRIVER PORTAL\n";
            cout << "1. Login\n";
            cout << "2. Sign Up\n";
            cout << "3. Forgot Password?\n";
            choice = getMenuChoice("Choice: ", 1, 3);

            if (choice == 2) {
                app.registerUser(1);
                cout << "\nPlease login with your new ID.\n";
            } else if (choice == 3) {
                app.forgotPassword(2);
                break;
            }

            if (choice == 1 || choice == 2) {
                 cout << "\n--- DRIVER LOGIN ---\n";
                 id = getIntInput("Enter Driver ID: ");
                 string pass;
                 cout << "Enter Password: ";
                 cin >> pass;
                 
                 if (app.verifyUser(2, id, pass)) {
                     do {
                    cout << "\n";
                    cout << "==================================================\n";
                    cout << "            DRIVER DASHBOARD\n";
                    cout << "==================================================\n";
                    cout << "1. View Profile\n";
                    cout << "2. Logout\n";
                    cout << "==================================================\n";
                    
                    choice = getMenuChoice("\nEnter your choice (1-2): ",1,2);
                    if (choice == 1) app.viewDriverProfile(id);
                    if (choice == 1) app.viewDriverProfile(id);
                } while (choice != 2);
                cout << "\n Logging out from Driver Panel...\n";
            }
            }
            break;
        }

        case 3: {
            cout << "\n--------------------------------------------------\n";
            cout << "            RIDER PORTAL\n";
            cout << "--------------------------------------------------\n";
            cout << "1. Login\n";
            cout << "2. Sign Up\n";
            cout << "3. Forgot Password?\n";
            choice = getMenuChoice("Choice: ", 1, 3);

            if (choice == 2) {
                app.registerUser(2);
                cout << "\nPlease login with your new ID.\n";
            } else if (choice == 3) {
                 app.forgotPassword(3);
                 break;
            }

            if (choice == 1 || choice == 2) {
                cout << "\n--- RIDER LOGIN ---\n";
                id = getIntInput("Enter Rider ID: ");
                 string pass;
                 cout << "Enter Password: ";
                 cin >> pass;

                if (app.verifyUser(3, id, pass)) {
                    do {
                    cout << "\n";
                    cout << "==================================================\n";
                    cout << "            RIDER DASHBOARD\n";
                    cout << "==================================================\n";
                    cout << "1. Book a Ride\n";
                    cout << "2. Cancel Ride\n";
                    cout << "3. Rate Driver (Complete Ride)\n";
                    cout << "4. Logout\n";
                    cout << "==================================================\n";
                    
                    choice = getMenuChoice("\nEnter your choice (1-4): ",1,4);
                    if (choice == 1) app.bookRide(id);
                    else if (choice == 2) app.cancelRide(id);
                    else if (choice == 3) app.rateDriver(id);
                } while (choice != 4);
                cout << "\n Logging out from Rider Panel...\n";
            }
            }
            break;
        }
        }
    }
    return 0;
}