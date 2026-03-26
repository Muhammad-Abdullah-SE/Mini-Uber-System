# 🚗 MiniUber: File-Based Ride-Sharing System

> A robust, terminal-based ride-sharing management system implemented in C++. This project demonstrates the practical application of **Data Structures & Algorithms (DSA)**, **Object-Oriented Programming (OOP)**, and **persistent file handling** to manage a complex ecosystem of riders and drivers.

---

## ✨ Key Features

### 🔐 Multi-Role Access
Dedicated portals for **Admins**, **Drivers**, and **Riders** — each featuring secure login authentication and password recovery.

### 🚕 Ride Lifecycle Management
- **Riders:** Book rides with real-time fare calculation, cancel pending requests, and rate drivers after arrival.
- **Admins:** Manage the central queue of pending requests and assign them to the nearest/available drivers.
- **Drivers:** Personal dashboards to track total earnings, average ratings, and trip history.

### 📊 Advanced Analytics & Reporting
- **Financials:** Track total system earnings and average trip distances.
- **Performance:** Identify top-rated drivers and the most active riders.
- **Queue Status:** Real-time statistics on pending vs. completed rides.

### 💾 Persistent Storage
Data is automatically serialized to and from `.csv` files, ensuring all user profiles and ride histories are preserved between sessions.

---

## 🛠️ Technical Highlights

### Data Structures & Algorithms

| Concept | Usage |
|---|---|
| `std::vector` | Dynamic management of user records |
| `std::queue` | FIFO handling of ride requests |
| Binary Search | O(log n) user lookup by ID |
| Bubble Sort + Lambdas | Sorting drivers by rating or name |

### Robust Engineering
- **Input Validation:** Strict handling to ensure phone numbers, names, and ratings (1–5) meet system requirements.
- **Time Integration:** Uses the `<chrono>` library to timestamp every lifecycle event of a ride.

---

## 📂 File Structure

The system maintains four database files to ensure modularity:

```
📁 MiniUber/
├── drivers.csv          # Credentials, availability, ratings, and earnings
├── riders.csv           # Rider credentials and booking history
├── completed_rides.csv  # Historical log of all successful transactions
└── cancelled_rides.csv  # Audit records of rider-cancelled rides
```

---

## 💻 How to Run

**1. Clone the repository:**
```bash
git clone https://github.com/Muhammad-Abdullah-SE/Mini-Uber-System.git
cd MiniUber
```

**2. Compile the source code:**
```bash
g++ main.cpp -o MiniUber
```

**3. Execute the program:**
```bash
./MiniUber
```

---

## 🔑 Default Credentials

Use the following credentials to access the **Admin Dashboard**:

| Field | Value |
|---|---|
| Username | `admin` |
| Password | `1234567` |

---

## 📌 Note

This is my 3rd semester project, built to practice and apply DSA and OOP concepts in a real-world simulation. Feel free to share feedback or suggestions!
