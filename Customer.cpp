#ifndef CLASS_CUSTOMER_CPP
#define CLASS_CUSTOMER_CPP

#include <iostream>
#include <string>
#include <iomanip> 
#include <sstream> 
using namespace std;

// 1. LỚP QUẢN LÝ NGÀY THÁNG NĂM
class Date {
private:
    int day, month, year; // Các biến ngày, tháng, năm

    // Kiểm tra năm nhuận
    static bool isLeapYear(int y) {
        return ((y % 4 == 0 && y % 100 != 0) || (y % 400 == 0)); // Điều kiện năm nhuận
    }

    // Lấy số ngày tối đa trong tháng để chống nhập ngày sai thực tế
    static int daysInMonth(int m, int y) {
        if (m == 4 || m == 6 || m == 9 || m == 11) return 30; // Các tháng 4, 6, 9, 11 có 30 ngày
        if (m == 2) { // Riêng với tháng 2
            if (isLeapYear(y)) return 29; // Năm nhuận tháng 2 có 29 ngày
            return 28; // Năm thường tháng 2 có 28 ngày
        }
        return 31; // Các tháng còn lại có 31 ngày
    }

public:
    Date(int d = 1, int m = 1, int y = 2026) { // Hàm khởi tạo ngày tháng năm mặc định 01/01/2026
        setDate(d, m, y);
    }

    // Tự động ép về ngày hợp lệ nếu người dùng nhập sai
    void setDate(int d, int m, int y) {
        if (y > 0) year = y;
        else year = 2026;

        if (m >= 1 && m <= 12) month = m;
        else month = 1;

        if (d >= 1 && d <= daysInMonth(month, year)) day = d;
        else day = 1;
    }

    int getDay() const { return day; } // Hàm trả về giá trị ngày
    int getMonth() const { return month; } // Hàm trả về giá trị tháng
    int getYear() const { return year; } // Hàm trả về giá trị năm

    // So sánh hạn thẻ SV hoặc hạn VIP
    bool operator>=(const Date& other) const { // Nạp chồng toán tử >= để so sánh ngày
        if (year != other.year) return year > other.year; // Năm nào lớn hơn thì ngày đó lớn hơn
        if (month != other.month) return month > other.month; // Cùng năm thì tháng nào lớn hơn sẽ lớn hơn
        return day >= other.day; // Cùng năm cùng tháng thì so sánh ngày
    }

    // Chuyển ngày sang chuỗi dd/mm/yyyy để in hoặc xuất CSV
    string toString() const {
        ostringstream oss;
        oss << setfill('0') << setw(2) << day << "/"
            << setw(2) << month << "/"
            << setw(4) << year;
        return oss.str();
    }

    // Chuyển chuỗi "dd/mm/yyyy" đọc từ file CSV thành đối tượng Date
    static Date fromString(const string& dateStr) {
        if (dateStr.empty()) return Date(1, 1, 2026);
        stringstream ss(dateStr);
        string part;
        int d = 1, m = 1, y = 2026;

        try {
            if (getline(ss, part, '/') && !part.empty()) d = stoi(part);
            if (getline(ss, part, '/') && !part.empty()) m = stoi(part);
            if (getline(ss, part, '/') && !part.empty()) y = stoi(part);
        } catch (...) {
            return Date(1, 1, 2026);
        }

        return Date(d, m, y);
    }

    void display() const {
        cout << toString(); // In ngày định dạng dd/mm/yyyy
    }
};

// 2. LỚP THẺ TÍCH ĐIỂM
class MemberCard {
private:
    string cardId; // Mã thẻ thành viên
    int lifetimePoints; // Tổng điểm tích lũy để xét hạng
    int availablePoints; // Điểm khả dụng dùng để đổi quà / voucher
    string cardTier; // Hạng thẻ (Dong, Bac, Vang, Kim Cuong)

public:
    
    MemberCard(const string& id = "CARD-00", int lifetimePts = 0, int availPts = -1) {
        cardId = id; // Gán mã thẻ thành viên

        if (lifetimePts >= 0) lifetimePoints = lifetimePts; // Điểm tích lũy không âm
        else lifetimePoints = 0;

        if (availPts == -1) {
            availablePoints = lifetimePoints; // Mặc định điểm khả dụng bằng điểm tích lũy ban đầu
        } else if (availPts >= 0) {
            availablePoints = availPts; // Gán điểm khả dụng hợp lệ
        } else {
            availablePoints = 0; // Điểm khả dụng không âm
        }

        cardTier = "Dong"; // Mặc định ban đầu là hạng Đồng
        updateTier(); // Tự động cập nhật hạng thẻ theo điểm tích lũy
    }

    string getCardId() const { return cardId; } // Trả về mã thẻ thành viên
    int getLifetimePoints() const { return lifetimePoints; } // Trả về tổng điểm tích lũy xét hạng
    int getAvailablePoints() const { return availablePoints; } // Trả về điểm khả dụng đổi quà
    string getCardTier() const { return cardTier; } // Trả về hạng thẻ hiện tại

    // Xét nâng hạng thẻ tự động theo tổng điểm tích lũy
    void updateTier() {
        if (lifetimePoints >= 500) cardTier = "Kim Cuong"; // Từ 500 điểm trở lên: Hạng Kim Cương
        else if (lifetimePoints >= 300) cardTier = "Vang"; // Từ 300 đến 499 điểm: Hạng Vàng
        else if (lifetimePoints >= 100) cardTier = "Bac"; // Từ 100 đến 299 điểm: Hạng Bạc
        else cardTier = "Dong"; // Dưới 100 điểm: Hạng Đồng
    }

    // Tích thêm điểm khi mua hoặc thuê
    void addPoints(int p) {
        if (p > 0) { // Điểm cộng vào phải lớn hơn 0
            lifetimePoints += p; // Cộng dồn vào điểm xét hạng
            availablePoints += p; // Cộng dồn vào điểm khả dụng
            updateTier(); // Cập nhật lại hạng thẻ sau khi cộng điểm
        }
    }

    // Trừ điểm khả dụng khi đổi quà (không làm tụt hạng thẻ)
    bool redeemPoints(int p) {
        if (p > 0 && availablePoints >= p) { // Điểm trừ dương và đủ điểm khả dụng
            availablePoints -= p; // Chỉ trừ điểm khả dụng
            return true; // Đổi quà thành công
        }
        return false; // Không đủ điểm đổi quà
    }

    void display() const {
        cout << "Ma the: " << cardId
             << " | Hang: " << cardTier
             << " | Diem tich luy: " << lifetimePoints
             << " | Diem doi qua: " << availablePoints;
    }

    string toCSV() const {
        return cardId + "," + to_string(lifetimePoints) + "," + to_string(availablePoints) + "," + cardTier;
    }
};

// 3. LỚP KHÁCH HÀNG (TRỪU TƯỢNG)
class Customer {
protected:
    string customerId; // Mã khách hàng
    string phone; // Số điện thoại
    string email; // Email
    MemberCard* card; // Con trỏ liên kết đến thẻ tích điểm
    int currentBorrowedCount; // Số lượng sách / boardgame hiện đang mượn

    // Xuất 9 cột chung đầu tiên (cột 0 -> 8)
    string baseToCSV() const {
        string cardData;
        if (card != nullptr) {
            cardData = card->toCSV();
        } else {
            cardData = "NONE,0,0,NONE";
        }
        return getCustomerType() + "," + customerId + "," + phone + "," + email + ","
               + to_string(currentBorrowedCount) + "," + cardData;
    }

public:
    Customer(const string& id, const string& p, const string& mail,
             MemberCard* c = nullptr, int borrowed = 0) {
        customerId = id; // Gán mã khách hàng
        phone = p; // Gán số điện thoại
        email = mail; // Gán email
        card = c; // Gán con trỏ thẻ thành viên

        if (borrowed >= 0) currentBorrowedCount = borrowed; // Số món đang mượn không âm
        else currentBorrowedCount = 0;
    }

    // Cấm sao chép trực tiếp đối tượng Customer để tránh lỗi double-free con trỏ card
    Customer(const Customer&) = delete;
    Customer& operator=(const Customer&) = delete;

    string getCustomerId() const { return customerId; } // Trả về mã khách hàng
    string getPhoneNumber() const { return phone; } // Trả về số điện thoại
    string getEmail() const { return email; } // Trả về địa chỉ email
    MemberCard* getMemberCard() const { return card; } // Trả về con trỏ thẻ thành viên
    int getCurrentBorrowedCount() const { return currentBorrowedCount; } // Trả về số món đang mượn

    void setPhoneNumber(const string& p) { phone = p; } // Cập nhật số điện thoại
    void setEmail(const string& mail) { email = mail; } // Cập nhật email

    // Đăng ký mới hoặc thay đổi thẻ thành viên sau khi đã tạo tài khoản
    void registerMemberCard(MemberCard* newCard) {
        if (card == newCard) return; // Tránh tự giải phóng nếu gán lại chính thẻ cũ
        if (card != nullptr) delete card; // Giải phóng thẻ cũ nếu tồn tại
        card = newCard; // Gán con trỏ thẻ mới
    }

    // Các hàm thuần ảo: lớp con bắt buộc tự định nghĩa theo phân khúc
    virtual string getCustomerType() const = 0; // Trả về tên loại khách hàng
    virtual string getName() const = 0; // Trả về họ tên (cá nhân) hoặc tên công ty (doanh nghiệp)
    virtual double calculateDiscount(const Date& today) const = 0; // Tính mức giảm giá cơ bản (có xét hạn thẻ)
    virtual int getRentalLimit(const Date& today) const = 0; // Lấy giới hạn số món mượn tối đa
    virtual double getDepositRate(const Date& today) const = 0; // Lấy tỷ lệ tiền cọc (0.0 -> 1.0)
    virtual string toCSV() const = 0; // Xuất dữ liệu khách hàng sang đúng 21 cột CSV

    // Hàm ảo trung tâm tính mức giảm giá cuối cùng để bên hóa đơn gọi đa hình qua Customer*
    virtual double getFinalDiscount(const Date& today, int quantity = 1) const {
        (void)quantity;
        return calculateDiscount(today); // Mặc định dùng mức giảm cơ bản
    }

    // Kiểm tra khách có đủ hạn mức để mượn thêm số lượng món yêu cầu hay không
    bool canRent(int quantity, const Date& today) const {
        return (quantity > 0) && (currentBorrowedCount + quantity <= getRentalLimit(today));
    }

    // Cập nhật tăng số lượng món đang mượn khi lập phiếu thuê
    bool borrowItems(int quantity, const Date& today) {
        if (canRent(quantity, today)) {
            currentBorrowedCount += quantity;
            return true;
        }
        return false;
    }

    // Cập nhật giảm số lượng món đang mượn khi khách trả đồ
    bool returnItems(int quantity) {
        if (quantity > 0 && quantity <= currentBorrowedCount) {
            currentBorrowedCount -= quantity;
            return true;
        }
        return false;
    }

    virtual void display() const {
        cout << "------------------------------------------\n";
        cout << "Ma KH: " << customerId << " | SDT: " << phone << "\n";
        cout << "Email: " << email << " | Dang muon: " << currentBorrowedCount << " mon\n";
        if (card != nullptr) {
            cout << "The TV -> ";
            card->display();
            cout << "\n";
        } else {
            cout << "The TV -> Khong dang ky\n";
        }
    }

    // Hàm hủy ảo đặt ở cuối lớp để thu hồi bộ nhớ con trỏ thẻ khi xóa khách hàng
    virtual ~Customer() {
        if (card != nullptr) {
            delete card;
            card = nullptr;
        }
    }
};

// 4. LỚP KHÁCH HÀNG CÁ NHÂN
class IndividualCustomer : public Customer {
protected:
    string fullName; // Họ và tên
    Date dateOfBirth; // Ngày tháng năm sinh
    string gender; // Giới tính
    string address; // Địa chỉ

    // Tính giảm giá theo hạng thẻ tích điểm (dùng chung cho khách thường hoặc khi HSSV/VIP hết hạn)
    double getCardTierDiscount() const {
        if (card != nullptr) {
            if (card->getCardTier() == "Kim Cuong") return 0.15; // Hạng Kim Cương giảm 15%
            if (card->getCardTier() == "Vang") return 0.10; // Hạng Vàng giảm 10%
            if (card->getCardTier() == "Bac") return 0.05; // Hạng Bạc giảm 5%
        }
        return 0.0; // Hạng Đồng hoặc chưa có thẻ giảm 0%
    }

    // Xuất 13 cột đầu cho khách cá nhân (cột 0 -> 12)
    string individualToCSV() const {
        return baseToCSV() + "," + fullName + "," + dateOfBirth.toString() + "," + gender + "," + address;
    }

public:
    IndividualCustomer(const string& id, const string& p, const string& mail,
                       MemberCard* c, const string& name,
                       const Date& dob, const string& gen, const string& addr,
                       int borrowed = 0)
        : Customer(id, p, mail, c, borrowed), dateOfBirth(dob) {
        fullName = name; // Gán họ và tên
        gender = gen; // Gán giới tính
        address = addr; // Gán địa chỉ
    }

    string getName() const override { return fullName; } // Trả về họ tên qua giao diện chung Customer
    string getFullName() const { return fullName; } // Trả về họ tên
    Date getDateOfBirth() const { return dateOfBirth; } // Trả về ngày sinh
    string getGender() const { return gender; } // Trả về giới tính
    string getAddress() const { return address; } // Trả về địa chỉ

    void setAddress(const string& addr) { address = addr; } // Cập nhật địa chỉ nhà

    // Kiểm tra tháng hôm nay có trùng tháng sinh nhật không
    bool isBirthdayMonth(const Date& today) const {
        return (dateOfBirth.getMonth() == today.getMonth());
    }

    // Ghi đè hàm đa hình: đúng tháng sinh nhật ưu tiên giảm 35%, ngược lại giảm theo phân khúc
    double getFinalDiscount(const Date& today, int quantity = 1) const override {
        (void)quantity;
        if (isBirthdayMonth(today)) {
            return 0.35; // Ưu tiên giảm giá sinh nhật 35%
        }
        return calculateDiscount(today); // Giảm giá thông thường theo loại khách
    }

    void display() const override {
        cout << "------------------------------------------\n";
        cout << "Ma KH: " << customerId << " | Ho ten: " << fullName << "\n";
        cout << "SDT: " << phone << " | Email: " << email << "\n";
        cout << "Gioi tinh: " << gender << " | Ngay sinh: ";
        dateOfBirth.display();
        cout << "\nDia chi: " << address << " | Dang muon: " << currentBorrowedCount << " mon\n";
        if (card != nullptr) {
            cout << "The TV -> ";
            card->display();
            cout << "\n";
        } else {
            cout << "The TV -> Khong dang ky\n";
        }
    }
};

// 5. LỚP KHÁCH HÀNG THÔNG THƯỜNG
class RegularCustomer : public IndividualCustomer {
public:
    RegularCustomer(const string& id, const string& p, const string& mail,
                    MemberCard* c, const string& name, const Date& dob,
                    const string& gen, const string& addr, int borrowed = 0)
        : IndividualCustomer(id, p, mail, c, name, dob, gen, addr, borrowed) {}

    string getCustomerType() const override { return "Regular"; }

    int getRentalLimit(const Date& today) const override {
        (void)today;
        return 3; // Khách thường mượn tối đa 3 món
    }

    double getDepositRate(const Date& today) const override {
        (void)today;
        return 1.0; // Chính sách tiêu chuẩn: cọc 100% giá trị
    }

    double calculateDiscount(const Date& today) const override {
        (void)today;
        return getCardTierDiscount(); // Giảm theo bậc thẻ tích điểm
    }

    void display() const override {
        IndividualCustomer::display();
        cout << "Phan khuc: Khach thuong | Coc: 100% | Han muc: 3 mon\n";
    }

    // Cột 0 -> 12 có dữ liệu, bỏ trống 8 cột cuối (13 -> 20)
    string toCSV() const override {
        return individualToCSV() + ",,,,,,,,";
    }
};

// 6. LỚP KHÁCH HỌC SINH, SINH VIÊN
class StudentCustomer : public IndividualCustomer {
private:
    string studentID; // Mã số sinh viên (MSSV)
    string schoolName; // Tên trường học
    Date cardExpiry; // Ngày hết hạn thẻ học sinh / sinh viên

public:
    StudentCustomer(const string& id, const string& p, const string& mail,
                    MemberCard* c, const string& name, const Date& dob,
                    const string& gen, const string& addr, const string& sId,
                    const string& school, const Date& expiry, int borrowed = 0)
        : IndividualCustomer(id, p, mail, c, name, dob, gen, addr, borrowed),
          cardExpiry(expiry) {
        studentID = sId; // Gán MSSV
        schoolName = school; // Gán tên trường
    }

    string getStudentID() const { return studentID; }
    string getSchoolName() const { return schoolName; }
    Date getCardExpiry() const { return cardExpiry; }

    // Kiểm tra thẻ sinh viên còn hạn hay không
    bool isStudentCardValid(const Date& today) const {
        return cardExpiry >= today;
    }

    string getCustomerType() const override { return "Student"; }

    int getRentalLimit(const Date& today) const override {
        if (isStudentCardValid(today)) return 5; // Còn hạn thẻ được mượn tối đa 5 món
        else return 3; // Hết hạn thẻ quay về hạn mức khách thường
    }

    double getDepositRate(const Date& today) const override {
        if (isStudentCardValid(today)) return 0.5; // Còn hạn cọc 50%
        else return 1.0; // Hết hạn cọc 100%
    }

    double calculateDiscount(const Date& today) const override {
        if (isStudentCardValid(today)) {
            return 0.15; // Trợ giá sinh viên 15% khi thẻ còn hạn
        }
        return getCardTierDiscount(); // Hết hạn thẻ tính giảm theo hạng thẻ tích điểm
    }

    void display() const override {
        IndividualCustomer::display();
        cout << "Phan khuc: HSSV | Ma HSSV: " << studentID
             << " | Truong: " << schoolName << "\n";
        cout << "Han the HSSV: ";
        cardExpiry.display();
        cout << " | Coc uu dai: 50% | Han muc: 5 mon\n";
    }

    // Điền cột 13, 14, 15 (Student), bỏ trống 5 cột cuối (16 -> 20)
    string toCSV() const override {
        return individualToCSV() + "," + studentID + "," + schoolName + "," + cardExpiry.toString() + ",,,,,";
    }
};

// 7. LỚP KHÁCH HÀNG VIP
class VIPCustomer : public IndividualCustomer {
private:
    Date vipExpiry; // Ngày hết hạn của gói VIP
    double annualFee; // Phí duy trì gói VIP 120.000 VNĐ / năm

public:
    VIPCustomer(const string& id, const string& p, const string& mail, MemberCard* c,
                const string& name, const Date& dob, const string& gen, const string& addr,
                const Date& expiry, double fee = 120000.0, int borrowed = 0)
        : IndividualCustomer(id, p, mail, c, name, dob, gen, addr, borrowed),
          vipExpiry(expiry) {
        if (fee > 0) annualFee = fee; // Gán mức phí duy trì hàng năm
        else annualFee = 120000.0; // Mặc định 120.000 VNĐ nếu truyền sai
    }

    Date getVipExpiry() const { return vipExpiry; }
    double getAnnualFee() const { return annualFee; }

    bool isVipActive(const Date& today) const {
        return vipExpiry >= today; // Còn hạn nếu hạn VIP >= ngày hôm nay
    }

    // Đóng phí thường niên để gia hạn VIP
    bool payAnnualFee(double amountPaid, const Date& newExpiry) {
        if (amountPaid >= annualFee) {
            vipExpiry = newExpiry;
            return true;
        }
        return false;
    }

    string getCustomerType() const override { return "VIP"; }

    int getRentalLimit(const Date& today) const override {
        if (isVipActive(today)) return 10; // Còn hạn VIP được mượn tối đa 10 món
        else return 3; // Hết hạn VIP quay về hạn mức 3 món
    }

    double getDepositRate(const Date& today) const override {
        if (isVipActive(today)) return 0.0; // VIP còn hạn miễn cọc (0%)
        else return 1.0; // Hết hạn VIP cọc 100%
    }

    double calculateDiscount(const Date& today) const override {
        if (isVipActive(today)) {
            return 0.25; // Khách VIP còn hạn giảm 25%
        }
        return getCardTierDiscount(); // Hết hạn VIP tính theo hạng thẻ tích điểm
    }

    void display() const override {
        IndividualCustomer::display();
        cout << "Phan khuc: VIP (Giam 25% | Mien coc | Muon 10 mon)\n";
        cout << "Phi duy tri: " << (long long)annualFee
             << " VND/nam | Han VIP: ";
        vipExpiry.display();
        cout << "\n";
    }

    // Bỏ trống cột 13, 14, 15 (Student), điền cột 16, 17 (VIP), bỏ trống cột 18, 19, 20 (Corporate)
    string toCSV() const override {
        return individualToCSV() + ",,,," + vipExpiry.toString() + "," + to_string((long long)annualFee) + ",,,";
    }
};

// 8. LỚP KHÁCH DOANH NGHIỆP
class CorporateCustomer : public Customer {
private:
    string companyName; // Tên công ty / tổ chức
    string taxCode; // Mã số thuế doanh nghiệp
    string contractID; // Mã hợp đồng thuê kinh tế
    string address; // Địa chỉ trụ sở công ty

public:
    CorporateCustomer(const string& id, const string& p, const string& mail,
                      MemberCard* c, const string& comp, const string& tax,
                      const string& contract, const string& addr, int borrowed = 0)
        : Customer(id, p, mail, c, borrowed) {
        companyName = comp; // Gán tên công ty
        taxCode = tax; // Gán mã số thuế
        contractID = contract; // Gán mã hợp đồng
        address = addr; // Gán địa chỉ doanh nghiệp
    }

    string getName() const override { return companyName; } // Trả về tên công ty qua giao diện chung Customer
    string getCompanyName() const { return companyName; }
    string getTaxCode() const { return taxCode; }
    string getContractID() const { return contractID; }
    string getAddress() const { return address; }

    void setAddress(const string& addr) { address = addr; } // Cập nhật địa chỉ công ty

    string getCustomerType() const override { return "Corporate"; }

    int getRentalLimit(const Date& today) const override {
        (void)today;
        return 50; // Doanh nghiệp được mượn tối đa 50 món
    }

    double getDepositRate(const Date& today) const override {
        (void)today;
        return 0.3; // Doanh nghiệp có hợp đồng nên cọc 30%
    }

    double calculateDiscount(const Date& today) const override {
        (void)today;
        return 0.20; // Mặc định giảm 20% theo hợp đồng
    }

    // Ghi đè hàm đa hình: thuê sỉ từ 30 món trở lên giảm 30%, dưới 30 món giảm 20%
    double getFinalDiscount(const Date& today, int quantity = 1) const override {
        if (quantity >= 30) return 0.30;
        return calculateDiscount(today);
    }

    void display() const override {
        Customer::display();
        cout << "Phan loai: DOANH NGHIEP | Cong ty: " << companyName << "\n";
        cout << "MST: " << taxCode << " | Ma HD: " << contractID << "\n";
        cout << "Dia chi cong ty: " << address << " | Coc: 30% | Han muc: 50 mon\n";
    }

    // Bỏ trống 9, 10, 11 (cá nhân), điền cột 12 (Address), bỏ trống 13 -> 17 (Student/VIP), điền 18, 19, 20 (Corporate)
    string toCSV() const override {
        return baseToCSV() + ",,,," + address + ",,,,,," + companyName + "," + taxCode + "," + contractID;
    }
};

#endif
