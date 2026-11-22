#ifndef CUSTOMER_MANAGER_CPP
#define CUSTOMER_MANAGER_CPP

#include "Class_Customer.cpp"
#include <fstream> // Đọc và ghi file CSV
#include <vector> // Lưu danh sách khách hàng và tách cột CSV
#include <cctype> // Hỗ trợ chuyển chữ thường khi tìm kiếm

// LỚP QUẢN LÝ DANH SÁCH KHÁCH HÀNG VÀ FILE CSV
class CustomerManager {
private:
    vector<Customer*> customers; // Danh sách con trỏ khách hàng (đa hình)
    string filePath; // Đường dẫn file CSV lưu dữ liệu

    // Chuyển chuỗi sang chữ thường để tìm kiếm không phân biệt hoa/thường
    static string toLowerStr(const string& s) {
        string res = s;
        for (size_t i = 0; i < res.size(); i++) {
            res[i] = static_cast<char>(tolower(static_cast<unsigned char>(res[i])));
        }
        return res;
    }

    // Thay dấu chấm phẩy (nếu người dùng lỡ nhập) bằng dấu gạch ngang để không lệch cột CSV
    static string cleanCSVText(const string& s) {
        string res = s;
        for (size_t i = 0; i < res.size(); i++) {
            if (res[i] == ';') {
                res[i] = '-';
            }
        }
        return res;
    }

    // Tách 1 dòng CSV thành danh sách các cột (Cắt bằng dấu chấm phẩy)
    static vector<string> splitCSVLine(const string& line) {
        vector<string> cols;
        stringstream ss(line);
        string item;
        while (getline(ss, item, ';')) {
            cols.push_back(item);
        }
        if (!line.empty() && line.back() == ';') {
            cols.push_back("");
        }
        return cols;
    }

    // Hỗ trợ nhập chuỗi có khoảng trắng mà không bị trôi lệnh getline
    static string inputLine(const string& prompt) {
        cout << prompt;
        string s;
        getline(cin >> ws, s);
        return cleanCSVText(s);
    }

    // Hỗ trợ nhập ngày tháng năm từ bàn phím
    static Date inputDate(const string& prompt) {
        int d = 1, m = 1, y = 2026;
        cout << prompt << " (nhap ngay thang nam): ";
        cin >> d >> m >> y;
        if (cin.fail()) {
            cin.clear();
            cin.ignore(10000, '\n');
            return Date(1, 1, 2026);
        }
        return Date(d, m, y);
    }

public:
    CustomerManager(const string& path = "customers.csv") {
        filePath = path;
    }

    // Lấy toàn bộ danh sách khách hàng để bên hóa đơn / menu sử dụng
    const vector<Customer*>& getAllCustomers() const {
        return customers;
    }

    int getCount() const {
        return static_cast<int>(customers.size());
    }

    // Giải phóng bộ nhớ danh sách hiện tại
    void clear() {
        for (size_t i = 0; i < customers.size(); i++) {
            if (customers[i] != nullptr) {
                delete customers[i];
            }
        }
        customers.clear();
    }

    // 1. TỰ ĐỘNG SINH MÃ KHÁCH HÀNG TIẾP THEO (KH01 -> KH99 -> KH100) KHÔNG TRÙNG
    string generateNextCustomerId() const {
        int maxNum = 0;
        for (size_t i = 0; i < customers.size(); i++) {
            if (customers[i] == nullptr) continue;
            string id = customers[i]->getCustomerId();
            if (id.size() >= 3 && (id[0] == 'K' || id[0] == 'k') && (id[1] == 'H' || id[1] == 'h')) {
                try {
                    int num = stoi(id.substr(2));
                    if (num > maxNum) {
                        maxNum = num;
                    }
                } catch (...) {
                    // Bỏ qua nếu mã không theo dạng KH + số
                }
            }
        }
        int nextNum = maxNum + 1;
        if (nextNum < 10) {
            return "KH0" + to_string(nextNum);
        }
        return "KH" + to_string(nextNum);
    }

    // 2. TỰ ĐỘNG SINH MÃ THẺ THÀNH VIÊN TIẾP THEO (CARD-01 -> CARD-85 -> CARD-86)
    string generateNextCardId() const {
        int maxNum = 0;
        for (size_t i = 0; i < customers.size(); i++) {
            if (customers[i] == nullptr) continue;
            MemberCard* c = customers[i]->getMemberCard();
            if (c == nullptr) continue;
            string cid = c->getCardId();
            if (cid.size() >= 6 && cid.substr(0, 5) == "CARD-") {
                try {
                    int num = stoi(cid.substr(5));
                    if (num > maxNum) {
                        maxNum = num;
                    }
                } catch (...) {
                    // Bỏ qua nếu mã thẻ không theo dạng CARD- + số
                }
            }
        }
        int nextNum = maxNum + 1;
        if (nextNum < 10) {
            return "CARD-0" + to_string(nextNum);
        }
        return "CARD-" + to_string(nextNum);
    }

    // 3. ĐỌC DỮ LIỆU TỪ FILE CSV CHUẨN 21 CỘT
    bool loadFromCSV() {
        ifstream inFile(filePath);
        if (!inFile.is_open()) {
            return false; // Chưa có file hoặc không mở được file
        }

        clear(); // Xóa dữ liệu cũ trong RAM trước khi nạp mới

        string line;
        while (getline(inFile, line)) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back(); // Xóa ký tự xuống dòng Windows nếu có
            }
            if (line.empty()) continue;
            if (line.substr(0, 4) == "sep=") continue; // Bỏ qua dòng cấu hình chia cột của Excel

            vector<string> cols = splitCSVLine(line);
            if (cols.size() > 0 && cols.size() < 21) {
                cols.resize(21, ""); 
            }

            if (cols.size() < 21) continue;

            string type = cols[0];
            if (type != "Regular" && type != "Student" && type != "VIP" && type != "Corporate") {
                continue; // Bỏ qua dòng tiêu đề (header)
            }

            MemberCard* card = nullptr;
            try {
                string id = cols[1];
                string phone = cols[2];
                string email = cols[3];
                int borrowed = stoi(cols[4]);

                if (cols[5] != "NONE" && !cols[5].empty()) {
                    string cardId = cols[5];
                    int lifetimePts = stoi(cols[6]);
                    int availPts = stoi(cols[7]);
                    card = new MemberCard(cardId, lifetimePts, availPts);
                }

                if (type == "Regular") {
                    Date dob = Date::fromString(cols[10]);
                    customers.push_back(new RegularCustomer(id, phone, email, card, cols[9], dob, cols[11], cols[12], borrowed));
                } else if (type == "Student") {
                    Date dob = Date::fromString(cols[10]);
                    Date expiry = Date::fromString(cols[15]);
                    customers.push_back(new StudentCustomer(id, phone, email, card, cols[9], dob, cols[11], cols[12], cols[13], cols[14], expiry, borrowed));
                } else if (type == "VIP") {
                    Date dob = Date::fromString(cols[10]);
                    Date expiry = Date::fromString(cols[16]);
                    double fee = stod(cols[17]);
                    customers.push_back(new VIPCustomer(id, phone, email, card, cols[9], dob, cols[11], cols[12], expiry, fee, borrowed));
                } else if (type == "Corporate") {
                    customers.push_back(new CorporateCustomer(id, phone, email, card, cols[18], cols[19], cols[20], cols[12], borrowed));
                } else {
                    if (card != nullptr) delete card;
                }
            } catch (...) {
                if (card != nullptr) delete card;
            }
        }

        inFile.close();
        return true;
    }

    // 4. GHI TOÀN BỘ DANH SÁCH RA FILE CSV (21 CỘT) BẰNG DẤU CHẤM PHẨY
    bool saveToCSV() const {
        ofstream outFile(filePath);
        if (!outFile.is_open()) {
            return false; // Lỗi thường gặp nếu file CSV đang mở bằng Excel
        }

        // Lệnh cấu hình bắt Excel tự động chia cột bằng dấu chấm phẩy
        outFile << "sep=;\n";

        // Ghi tiêu đề bằng dấu chấm phẩy
        outFile << "Type;CustomerID;Phone;Email;Borrowed;CardID;LifetimePts;AvailPts;CardTier;"
                << "FullName;DOB;Gender;Address;StudentID;SchoolName;StudentExpiry;"
                << "VipExpiry;AnnualFee;CompanyName;TaxCode;ContractID\n";

        for (size_t i = 0; i < customers.size(); i++) {
            if (customers[i] != nullptr) {
                // Ép chuỗi gốc từ file Class_Customer chuyển hết phẩy sang chấm phẩy
                string data = customers[i]->toCSV();
                for (char& ch : data) {
                    if (ch == ',') ch = ';';
                }
                outFile << data << "\n";
            }
        }

        outFile.close();
        return true;
    }

    // 5. DÒ KHÁCH HÀNG THEO MÃ ID (KHÔNG PHÂN BIỆT HOA/THƯỜNG)
    Customer* findById(const string& id) const {
        string target = toLowerStr(id);
        for (size_t i = 0; i < customers.size(); i++) {
            if (customers[i] != nullptr && toLowerStr(customers[i]->getCustomerId()) == target) {
                return customers[i];
            }
        }
        return nullptr;
    }

    // 6. DÒ KHÁCH HÀNG THEO TÊN (TÌM GẦN ĐÚNG CẢ TÊN CÁ NHÂN LẪN TÊN CÔNG TY)
    vector<Customer*> searchByName(const string& keyword) const {
        vector<Customer*> matches;
        string key = toLowerStr(keyword);
        if (key.empty()) return matches;

        for (size_t i = 0; i < customers.size(); i++) {
            if (customers[i] != nullptr) {
                string nameLower = toLowerStr(customers[i]->getName());
                if (nameLower.find(key) != string::npos) {
                    matches.push_back(customers[i]);
                }
            }
        }
        return matches;
    }

    // Giao diện tìm theo ID
    void searchCustomerByIdUI() const {
        string id = inputLine("Nhap ma khach hang can tim (vd: KH04): ");
        Customer* found = findById(id);
        if (found != nullptr) {
            cout << "\n>>> TIM THAY KHACH HANG:\n";
            found->display();
        } else {
            cout << "\n>>> Khong tim thay khach hang co ma: " << id << "\n";
        }
    }

    // Giao diện tìm theo Tên
    void searchCustomerByNameUI() const {
        string keyword = inputLine("Nhap ten khach hang hoac ten cong ty can tim: ");
        vector<Customer*> results = searchByName(keyword);
        if (results.empty()) {
            cout << "\n>>> Khong tim thay khach hang nao chua tu khoa: " << keyword << "\n";
            return;
        }
        cout << "\n>>> TIM THAY " << results.size() << " KET QUA:\n";
        for (size_t i = 0; i < results.size(); i++) {
            results[i]->display();
        }
    }

    // 7. ĐĂNG KÝ KHÁCH HÀNG MỚI -> TỰ SINH MÃ TIẾP THEO -> LƯU THẲNG VÀO CSV
    void registerNewCustomerUI(const Date& today) {
        cout << "\n========== DANG KY KHACH HANG MOI ==========\n";
        cout << "1. Khach thong thuong (Regular)\n";
        cout << "2. Hoc sinh - Sinh vien (Student)\n";
        cout << "3. Khach hang VIP (VIP)\n";
        cout << "4. Khach doanh nghiep (Corporate)\n";
        cout << "Chon loai khach hang (1-4): ";

        int choice = 0;
        cin >> choice;
        if (cin.fail() || choice < 1 || choice > 4) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << ">>> Lua chon khong hop le. Huy dang ky!\n";
            return;
        }

        string newId = generateNextCustomerId();
        cout << ">>> Ma khach hang tu dong cap: " << newId << "\n";

        string phone = inputLine("Nhap so dien thoai: ");
        string email = inputLine("Nhap email: ");

        // Hỏi đăng ký thẻ thành viên
        MemberCard* card = nullptr;
        string wantCard = inputLine("Dang ky the thanh vien tich diem (y/n): ");
        if (wantCard == "y" || wantCard == "Y") {
            string newCardId = generateNextCardId();
            card = new MemberCard(newCardId, 0, 0);
            cout << ">>> Da cap the thanh vien moi: " << newCardId << " (Hang Dong - 0 diem)\n";
        }

        Customer* newCustomer = nullptr;

        if (choice == 4) {
            string compName = inputLine("Nhap ten cong ty / CLB: ");
            string taxCode = inputLine("Nhap ma so thue: ");
            string contractId = inputLine("Nhap ma hop dong: ");
            string address = inputLine("Nhap dia chi tru so: ");
            newCustomer = new CorporateCustomer(newId, phone, email, card, compName, taxCode, contractId, address, 0);
        } else {
            string fullName = inputLine("Nhap ho va ten (khong dau): ");
            Date dob = inputDate("Nhap ngay sinh");
            string gender = inputLine("Nhap gioi tinh (Nam/Nu): ");
            string address = inputLine("Nhap dia chi: ");

            if (choice == 1) {
                newCustomer = new RegularCustomer(newId, phone, email, card, fullName, dob, gender, address, 0);
            } else if (choice == 2) {
                string studentId = inputLine("Nhap ma so sinh vien (MSSV): ");
                string schoolName = inputLine("Nhap ten truong: ");
                Date expiry = inputDate("Nhap ngay het han the HSSV");
                newCustomer = new StudentCustomer(newId, phone, email, card, fullName, dob, gender, address, studentId, schoolName, expiry, 0);
            } else if (choice == 3) {
                Date vipExpiry(today.getDay(), today.getMonth(), today.getYear() + 1);
                cout << ">>> Goi VIP co hieu luc 1 nam den ngay: " << vipExpiry.toString() << " (Phi: 120.000 VND)\n";
                newCustomer = new VIPCustomer(newId, phone, email, card, fullName, dob, gender, address, vipExpiry, 120000.0, 0);
            }
        }

        if (newCustomer != nullptr) {
            customers.push_back(newCustomer);
            if (saveToCSV()) {
                cout << ">>> DANG KY THANH CONG! Da luu " << newId << " vao file " << filePath << "\n";
            } else {
                cout << ">>> Da them " << newId << " vao bo nho, NHUNG LOI GHI FILE (hay dong file Excel neu dang mo)!\n";
            }
        }
    }

    // In toàn bộ danh sách khách hàng
    void displayAllCustomers() const {
        cout << "\n===== DANH SACH KHACH HANG (" << customers.size() << " KHACH) =====\n";
        for (size_t i = 0; i < customers.size(); i++) {
            if (customers[i] != nullptr) {
                customers[i]->display();
            }
        }
    }

    ~CustomerManager() {
        clear();
    }
};

#endif