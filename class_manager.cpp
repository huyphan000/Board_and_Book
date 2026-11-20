#ifndef CLASS_PERSON5_MANAGERS_H
#define CLASS_PERSON5_MANAGERS_H

// ============================================================
// CAC LOP DANH CHO THANH VIEN 5 - BOARD AND BOOK
// Dieu kien: include file nay SAU khi cac lop Product, Customer,
// Rental, Payment, Date, FeeCalculator va cac enum lien quan da duoc khai bao.
// Khong dinh nghia lai cac lop nghiep vu cua thanh vien 1-4.
// ============================================================

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// ------------------------------------------------------------
// 1. LOP CHA TRUU TUONG BaseManager
// ------------------------------------------------------------
class BaseManager {
protected:
    std::string managerName;
    bool initialized;

public:
    explicit BaseManager(const std::string& name)
        : managerName(name), initialized(false) {}

    virtual void initialize() { initialized = true; }
    bool isInitialized() const { return initialized; }
    std::string getManagerName() const { return managerName; }

    virtual void showMenu() const = 0;
    virtual void handleChoice(int choice) = 0;
    virtual void displaySummary() const = 0;

    virtual ~BaseManager() = default;
};

// ------------------------------------------------------------
// 2. CSVHandler - lop tien ich doc/ghi CSV dung chung
// Lop nay khong ke thua BaseManager vi no la lop tien ich luu tru.
// ------------------------------------------------------------
class CSVHandler {
private:
    static std::string escapeField(const std::string& value) {
        bool needsQuotes = value.find(',') != std::string::npos ||
                           value.find('"') != std::string::npos ||
                           value.find('\n') != std::string::npos ||
                           value.find('\r') != std::string::npos;
        if (!needsQuotes) return value;

        std::string escaped;
        escaped.reserve(value.size() + 2);
        escaped.push_back('"');
        for (char ch : value) {
            if (ch == '"') escaped.push_back('"');
            escaped.push_back(ch);
        }
        escaped.push_back('"');
        return escaped;
    }

public:
    typedef std::vector<std::string> Row;
    typedef std::vector<Row> Table;

    // Tach mot dong CSV, ho tro dau phay trong ngoac kep va "".
    static Row splitCSVLine(const std::string& line) {
        Row fields;
        std::string field;
        bool insideQuotes = false;

        for (std::size_t i = 0; i < line.size(); ++i) {
            const char ch = line[i];
            if (ch == '"') {
                if (insideQuotes && i + 1 < line.size() && line[i + 1] == '"') {
                    field.push_back('"');
                    ++i;
                } else {
                    insideQuotes = !insideQuotes;
                }
            } else if (ch == ',' && !insideQuotes) {
                fields.push_back(field);
                field.clear();
            } else {
                field.push_back(ch);
            }
        }
        fields.push_back(field);
        return fields;
    }

    // Doc toan bo CSV vao bang cac dong/cot.
    static bool readCSV(const std::string& fileName, Table& rows) {
        std::ifstream file(fileName.c_str(), std::ios::in);
        if (!file.is_open()) return false;

        rows.clear();
        std::string line;
        bool firstLine = true;
        while (std::getline(file, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (firstLine && line.size() >= 3 &&
                static_cast<unsigned char>(line[0]) == 0xEF &&
                static_cast<unsigned char>(line[1]) == 0xBB &&
                static_cast<unsigned char>(line[2]) == 0xBF) {
                line.erase(0, 3); // Bo UTF-8 BOM
            }
            firstLine = false;
            if (!line.empty()) rows.push_back(splitCSVLine(line));
        }
        return !file.bad();
    }

    // Ghi moi bang CSV (ghi de noi dung cu).
    static bool writeCSV(const std::string& fileName, const Table& rows) {
        std::ofstream file(fileName.c_str(), std::ios::out | std::ios::trunc);
        if (!file.is_open()) return false;

        for (std::size_t r = 0; r < rows.size(); ++r) {
            for (std::size_t c = 0; c < rows[r].size(); ++c) {
                if (c > 0) file << ',';
                file << escapeField(rows[r][c]);
            }
            file << '\n';
        }
        return file.good();
    }

    // Them mot dong vao cuoi file CSV.
    static bool appendCSVRow(const std::string& fileName, const Row& row) {
        std::ofstream file(fileName.c_str(), std::ios::out | std::ios::app);
        if (!file.is_open()) return false;

        for (std::size_t c = 0; c < row.size(); ++c) {
            if (c > 0) file << ',';
            file << escapeField(row[c]);
        }
        file << '\n';
        return file.good();
    }

    static bool fileExists(const std::string& fileName) {
        std::ifstream file(fileName.c_str());
        return file.good();
    }
};

// ------------------------------------------------------------
// 3. ProductManager - quan ly danh sach Product*
// ------------------------------------------------------------
class ProductManager : public BaseManager {
private:
    std::vector<Product*>& products;

public:
    explicit ProductManager(std::vector<Product*>& productList)
        : BaseManager("Product Manager"), products(productList) {}

    Product* findProduct(const std::string& productID) const {
        for (Product* product : products) {
            if (product != nullptr && product->getProductID() == productID)
                return product;
        }
        return nullptr;
    }

    bool addProduct(Product* product) {
        if (product == nullptr || findProduct(product->getProductID()) != nullptr)
            return false;
        products.push_back(product);
        return true;
    }

    void sortProductsByName() {
        std::stable_sort(products.begin(), products.end(),
            [](const Product* a, const Product* b) {
                if (a == nullptr) return false;
                if (b == nullptr) return true;
                return a->getName() < b->getName();
            });
    }

    void displayProducts() const {
        if (products.empty()) {
            std::cout << "Danh sach san pham dang rong.\n";
            return;
        }
        for (const Product* product : products) {
            if (product != nullptr) {
                std::cout << "------------------------------\n";
                product->display();
            }
        }
    }

    int countAvailableProducts() const {
        int count = 0;
        for (const Product* product : products) {
            if (product != nullptr && product->getQuantity() > 0 &&
                product->getStatus() != DAMAGED) {
                ++count;
            }
        }
        return count;
    }

    void showMenu() const override {
        std::cout << "\n=== PRODUCT MANAGER ===\n"
                  << "1. Hien thi san pham\n"
                  << "2. Sap xep theo ten\n"
                  << "3. Thong ke san pham\n";
    }

    void handleChoice(int choice) override {
        switch (choice) {
            case 1: displayProducts(); break;
            case 2: sortProductsByName(); displayProducts(); break;
            case 3: displaySummary(); break;
            default: std::cout << "Lua chon Product Manager khong hop le.\n";
        }
    }

    void displaySummary() const override {
        std::cout << "Tong so mat hang: " << products.size() << '\n'
                  << "Mat hang con co the cap phat: "
                  << countAvailableProducts() << '\n';
    }
};

// ------------------------------------------------------------
// 4. CustomerManager - quan ly danh sach Customer*
// ------------------------------------------------------------
class CustomerManager : public BaseManager {
private:
    std::vector<Customer*>& customers;

public:
    explicit CustomerManager(std::vector<Customer*>& customerList)
        : BaseManager("Customer Manager"), customers(customerList) {}

    Customer* findCustomer(const std::string& customerID) const {
        for (Customer* customer : customers) {
            if (customer != nullptr && customer->getCustomerId() == customerID)
                return customer;
        }
        return nullptr;
    }

    bool addRegularCustomer(const std::string& id, const std::string& name,
                            const std::string& phone, const std::string& email) {
        if (id.empty() || name.empty() || findCustomer(id) != nullptr)
            return false;
        customers.push_back(new Customer(id, name, phone, email));
        return true;
    }

    bool addVIPCustomer(const std::string& id, const std::string& name,
                        const std::string& phone, const std::string& email,
                        int initialPoints = 0) {
        if (id.empty() || name.empty() || findCustomer(id) != nullptr || initialPoints < 0)
            return false;
        customers.push_back(new VIPCustomer(id, name, phone, email, initialPoints));
        return true;
    }

    bool updateCustomer(const std::string& id, const std::string& name,
                        const std::string& phone, const std::string& email) {
        Customer* customer = findCustomer(id);
        if (customer == nullptr || name.empty()) return false;
        customer->setCustomerInfo(name, phone, email);
        return true;
    }

    void displayCustomers() const {
        if (customers.empty()) {
            std::cout << "Danh sach khach hang dang rong.\n";
            return;
        }
        for (const Customer* customer : customers) {
            if (customer != nullptr) {
                std::cout << "------------------------------\n";
                customer->display(); // Da hinh: Customer hoac VIPCustomer
            }
        }
    }

    void showMenu() const override {
        std::cout << "\n=== CUSTOMER MANAGER ===\n"
                  << "1. Hien thi khach hang\n"
                  << "2. Thong ke khach hang\n";
    }

    void handleChoice(int choice) override {
        switch (choice) {
            case 1: displayCustomers(); break;
            case 2: displaySummary(); break;
            default: std::cout << "Lua chon Customer Manager khong hop le.\n";
        }
    }

    void displaySummary() const override {
        int vipCount = 0;
        for (const Customer* customer : customers) {
            if (dynamic_cast<const VIPCustomer*>(customer) != nullptr)
                ++vipCount;
        }
        std::cout << "Tong so khach hang: " << customers.size() << '\n'
                  << "Khach hang VIP: " << vipCount << '\n'
                  << "Khach hang thuong: " << customers.size() - vipCount << '\n';
    }
};

// ------------------------------------------------------------
// 5. TransactionManager - quan ly danh sach Rental*
// ------------------------------------------------------------
class TransactionManager : public BaseManager {
private:
    std::vector<Rental*>& transactions;

    static bool dateIsBefore(const Date& left, const Date& right) {
        if (left.getYear() != right.getYear())
            return left.getYear() < right.getYear();
        if (left.getMonth() != right.getMonth())
            return left.getMonth() < right.getMonth();
        return left.getDay() < right.getDay();
    }

public:
    explicit TransactionManager(std::vector<Rental*>& transactionList)
        : BaseManager("Transaction Manager"), transactions(transactionList) {}

    Rental* findTransaction(const std::string& transactionID) const {
        for (Rental* transaction : transactions) {
            if (transaction != nullptr && transaction->getRentalId() == transactionID)
                return transaction;
        }
        return nullptr;
    }

    Rental* createRental(const std::string& id, Customer* customer,
                         Product* product, const Date& rentalDate,
                         const Date& dueDate) {
        if (id.empty() || customer == nullptr || product == nullptr ||
            findTransaction(id) != nullptr || product->getQuantity() <= 0 ||
            product->getStatus() == DAMAGED) {
            return nullptr;
        }

        // Luu y: Product trong code hien tai chua co setQuantity/decreaseStock.
        // Vi vay method nay tao phieu va lien ket doi tuong, chua tu dong tru kho.
        // Sau khi thanh vien 1 bo sung decreaseStock(), hay goi method do tai day.
        Rental* rental = new Rental(id, customer, product, rentalDate, dueDate, RENTING);
        transactions.push_back(rental);
        return rental;
    }

    bool returnRental(const std::string& id, const Date& actualReturnDate) {
        Rental* rental = findTransaction(id);
        if (rental == nullptr ||
            (rental->getStatus() != RENTING && rental->getStatus() != OVERDUE)) {
            return false;
        }
        rental->returnProduct(actualReturnDate);
        // Tuong tu, can tang ton kho o day khi Product co increaseStock().
        return true;
    }

    int updateOverdueTransactions(const Date& today) {
        int updated = 0;
        for (Rental* rental : transactions) {
            if (rental != nullptr && rental->getStatus() == RENTING &&
                dateIsBefore(rental->getDueDate(), today)) {
                rental->setOverdue();
                ++updated;
            }
        }
        return updated;
    }

    void displayTransactions() const {
        if (transactions.empty()) {
            std::cout << "Chua co giao dich nao.\n";
            return;
        }
        for (const Rental* rental : transactions) {
            if (rental != nullptr) {
                std::cout << "------------------------------\n";
                rental->display();
            }
        }
    }

    void showMenu() const override {
        std::cout << "\n=== TRANSACTION MANAGER ===\n"
                  << "1. Hien thi giao dich\n"
                  << "2. Thong ke giao dich\n";
    }

    void handleChoice(int choice) override {
        switch (choice) {
            case 1: displayTransactions(); break;
            case 2: displaySummary(); break;
            default: std::cout << "Lua chon Transaction Manager khong hop le.\n";
        }
    }

    void displaySummary() const override {
        int renting = 0, returned = 0, overdue = 0, purchased = 0;
        for (const Rental* rental : transactions) {
            if (rental == nullptr) continue;
            switch (rental->getStatus()) {
                case RENTING: ++renting; break;
                case RETURNED: ++returned; break;
                case OVERDUE: ++overdue; break;
                case PURCHASED: ++purchased; break;
            }
        }
        std::cout << "Tong giao dich: " << transactions.size() << '\n'
                  << "Dang thue: " << renting << '\n'
                  << "Da tra: " << returned << '\n'
                  << "Qua han: " << overdue << '\n'
                  << "Da mua: " << purchased << '\n';
    }
};

// ------------------------------------------------------------
// 6. PaymentManager - quan ly danh sach Payment*
// ------------------------------------------------------------
class PaymentManager : public BaseManager {
private:
    std::vector<Payment*>& payments;
    FeeCalculator& feeCalculator;

public:
    PaymentManager(std::vector<Payment*>& paymentList, FeeCalculator& calculator)
        : BaseManager("Payment Manager"), payments(paymentList),
          feeCalculator(calculator) {}

    Payment* findPayment(const std::string& paymentID) const {
        for (Payment* payment : payments) {
            if (payment != nullptr && payment->getPaymentId() == paymentID)
                return payment;
        }
        return nullptr;
    }

    Payment* processPayment(const std::string& paymentID,
                            const std::string& transactionID,
                            double amount, PaymentMethod method,
                            const Date& paymentDate) {
        if (paymentID.empty() || transactionID.empty() || amount <= 0 ||
            findPayment(paymentID) != nullptr) {
            return nullptr;
        }

        // Moi giao dich chi tao mot thanh toan trong mo hinh hien tai.
        for (const Payment* existing : payments) {
            if (existing != nullptr && existing->getRentalId() == transactionID &&
                existing->getStatus() != REFUNDED) {
                return nullptr;
            }
        }

        Payment* payment = new Payment(paymentID, transactionID, amount,
                                      method, paymentDate, UNPAID);
        payment->makePayment(feeCalculator);
        payments.push_back(payment);
        return payment;
    }

    bool refundPayment(const std::string& paymentID) {
        Payment* payment = findPayment(paymentID);
        if (payment == nullptr || payment->getStatus() != PAID)
            return false;
        payment->refundPayment();
        return true;
    }

    double calculateCollectedRevenue() const {
        double total = 0.0;
        for (const Payment* payment : payments) {
            if (payment != nullptr && payment->getStatus() == PAID)
                total += payment->getTotalAmount();
        }
        return total;
    }

    void displayPayments() const {
        if (payments.empty()) {
            std::cout << "Chua co thanh toan nao.\n";
            return;
        }
        for (const Payment* payment : payments) {
            if (payment != nullptr) {
                std::cout << "------------------------------\n";
                payment->display();
            }
        }
    }

    void showMenu() const override {
        std::cout << "\n=== PAYMENT MANAGER ===\n"
                  << "1. Hien thi thanh toan\n"
                  << "2. Thong ke doanh thu\n";
    }

    void handleChoice(int choice) override {
        switch (choice) {
            case 1: displayPayments(); break;
            case 2: displaySummary(); break;
            default: std::cout << "Lua chon Payment Manager khong hop le.\n";
        }
    }

    void displaySummary() const override {
        int paid = 0, refunded = 0, unpaid = 0;
        for (const Payment* payment : payments) {
            if (payment == nullptr) continue;
            if (payment->getStatus() == PAID) ++paid;
            else if (payment->getStatus() == REFUNDED) ++refunded;
            else ++unpaid;
        }
        std::cout << "Tong so thanh toan: " << payments.size() << '\n'
                  << "Da thanh toan: " << paid << '\n'
                  << "Da hoan tien: " << refunded << '\n'
                  << "Chua thanh toan: " << unpaid << '\n'
                  << "Doanh thu dang ghi nhan: " << calculateCollectedRevenue()
                  << " VND\n";
    }
};

// ------------------------------------------------------------
// 7. ReportManager - tong hop bao cao tu cac phan he
// ------------------------------------------------------------
class ReportManager : public BaseManager {
private:
    const std::vector<Product*>& products;
    const std::vector<Customer*>& customers;
    const std::vector<Rental*>& transactions;
    const std::vector<Payment*>& payments;

public:
    ReportManager(const std::vector<Product*>& productList,
                  const std::vector<Customer*>& customerList,
                  const std::vector<Rental*>& transactionList,
                  const std::vector<Payment*>& paymentList)
        : BaseManager("Report Manager"), products(productList),
          customers(customerList), transactions(transactionList),
          payments(paymentList) {}

    double calculateRevenue() const {
        double revenue = 0.0;
        for (const Payment* payment : payments) {
            if (payment != nullptr && payment->getStatus() == PAID)
                revenue += payment->getTotalAmount();
        }
        return revenue;
    }

    int countOverdueTransactions() const {
        int count = 0;
        for (const Rental* rental : transactions) {
            if (rental != nullptr && rental->getStatus() == OVERDUE)
                ++count;
        }
        return count;
    }

    int countLowStockProducts(int threshold = 2) const {
        int count = 0;
        for (const Product* product : products) {
            if (product != nullptr && product->getQuantity() <= threshold)
                ++count;
        }
        return count;
    }

    void displaySummary() const override {
        std::cout << "\n========== BAO CAO TONG QUAN ==========\n"
                  << "So mat hang: " << products.size() << '\n'
                  << "So khach hang: " << customers.size() << '\n'
                  << "So giao dich: " << transactions.size() << '\n'
                  << "So thanh toan: " << payments.size() << '\n'
                  << "Giao dich qua han: " << countOverdueTransactions() << '\n'
                  << "Mat hang ton kho thap: " << countLowStockProducts() << '\n'
                  << "Doanh thu dang ghi nhan: " << calculateRevenue() << " VND\n"
                  << "======================================\n";
    }

    bool exportSummaryCSV(const std::string& fileName) const {
        CSVHandler::Table report;
        report.push_back(CSVHandler::Row{"metric", "value"});
        report.push_back(CSVHandler::Row{"product_count", std::to_string(products.size())});
        report.push_back(CSVHandler::Row{"customer_count", std::to_string(customers.size())});
        report.push_back(CSVHandler::Row{"transaction_count", std::to_string(transactions.size())});
        report.push_back(CSVHandler::Row{"payment_count", std::to_string(payments.size())});
        report.push_back(CSVHandler::Row{"overdue_transaction_count", std::to_string(countOverdueTransactions())});
        report.push_back(CSVHandler::Row{"low_stock_product_count", std::to_string(countLowStockProducts())});
        std::ostringstream revenue;
        revenue << calculateRevenue();
        report.push_back(CSVHandler::Row{"recorded_revenue_vnd", revenue.str()});
        return CSVHandler::writeCSV(fileName, report);
    }

    void showMenu() const override {
        std::cout << "\n=== REPORT MANAGER ===\n"
                  << "1. Hien thi bao cao tong quan\n"
                  << "2. Xuat bao cao CSV\n";
    }

    void handleChoice(int choice) override {
        switch (choice) {
            case 1: displaySummary(); break;
            case 2:
                if (exportSummaryCSV("report_summary.csv"))
                    std::cout << "Da xuat report_summary.csv\n";
                else
                    std::cout << "Khong the ghi report_summary.csv\n";
                break;
            default: std::cout << "Lua chon Report Manager khong hop le.\n";
        }
    }
};

#endif // CLASS_PERSON5_MANAGERS_H
