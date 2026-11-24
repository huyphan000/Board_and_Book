#ifndef CLASS_CUSTOMER_CPP
#define CLASS_CUSTOMER_CPP

#include <iostream>
#include <string>
#include <iomanip>

using namespace std;

// LỚP QUẢN LÝ NGÀY THÁNG (Date)
class Date {
private:
    int day;   // Ngày
    int month; // Tháng
    int year;  // Năm

    // Kiểm tra năm nhuận để tính đúng số ngày của tháng 2
    static bool isLeapYear(int y) {
        return ((y % 4 == 0 && y % 100 != 0) || (y % 400 == 0));
    }

    // Trả về số ngày tối đa của 1 tháng
    static int daysInMonth(int m, int y) {
        if (m == 4 || m == 6 || m == 9 || m == 11) {
            return 30;
        }

        if (m == 2) {
            if (isLeapYear(y)) {
                return 29;
            } else {
                return 28;
            }
        }

        return 31;
    }

public:
    // Hàm khởi tạo ngày tháng
    Date(int d = 1, int m = 1, int y = 2026) {
        if (y > 0) {
            year = y;
        } else {
            year = 2026;
        }

        if (m >= 1 && m <= 12) {
            month = m;
        } else {
            month = 1;
        }

        if (d >= 1 && d <= daysInMonth(month, year)) {
            day = d;
        } else {
            day = 1;
        }
    }

    // Lấy giá trị ngày
    int getDay() const {
        return day;
    }

    // Lấy giá trị tháng
    int getMonth() const {
        return month;
    }

    // Lấy giá trị năm
    int getYear() const {
        return year;
    }

    // So sánh xem hạn thẻ/VIP có lớn hơn hoặc bằng ngày hiện tại (còn hạn) không
    bool operator>=(const Date& other) const {
        if (year != other.year) {
            return year > other.year;
        }

        if (month != other.month) {
            return month > other.month;
        }

        return day >= other.day;
    }

    // In ngày tháng theo định dạng dd/mm/yyyy
    void display() const {
        cout << setfill('0') << setw(2) << day << "/"
             << setw(2) << month << "/"
             << setw(4) << year << setfill(' ');
    }
};

// LỚP GỐC: KHÁCH HÀNG CHUNG (Customer)
class Customer {
private:
    string customerId;        // Mã khách hàng (VD: KH01)
    int currentBorrowedCount; // Số lượng đồ đang mượn hiện tại

protected:
    string phone;             // Số điện thoại
    string email;             // Địa chỉ email

    // Hàm khởi tạo thông tin cơ bản cho khách hàng
    Customer(const string& id, const string& p, const string& mail, int borrowed = 0) {
        customerId = id;
        phone = p;
        email = mail;

        if (borrowed >= 0) {
            currentBorrowedCount = borrowed;
        } else {
            currentBorrowedCount = 0;
        }
    }

public:
    // Lấy mã khách hàng
    string getCustomerId() const {
        return customerId;
    }

    // Lấy số lượng đồ khách đang mượn
    int getCurrentBorrowedCount() const {
        return currentBorrowedCount;
    }

    // Lấy tên phân khúc/loại khách hàng
    virtual string getCustomerType() const = 0;

    // Lấy họ tên (khách cá nhân) hoặc tên tổ chức (đối tác)
    virtual string getName() const = 0;

    // Tính % giảm giá mặc định theo phân khúc
    virtual double calculateDiscount(const Date& today) const = 0;

    // Tính số đồ được phép mượn tối đa
    virtual int getRentalLimit(const Date& today) const = 0;

    // Tính mức giảm giá cuối cùng thực tế áp dụng
    virtual double getFinalDiscount(const Date& today) const {
        return calculateDiscount(today);
    }

    // Kiểm tra xem khách có mượn vượt quá hạn mức cho phép không
    bool canRent(int quantity, const Date& today) const {
        return (quantity > 0) && (currentBorrowedCount + quantity <= getRentalLimit(today));
    }

    // Cập nhật cộng thêm số đồ đang mượn khi khách thuê đồ
    bool borrowItems(int quantity, const Date& today) {
        if (canRent(quantity, today)) {
            currentBorrowedCount += quantity;
            return true;
        } else {
            return false;
        }
    }

    // Cập nhật trừ bớt số đồ đang mượn khi khách trả đồ
    bool returnItems(int quantity) {
        if (quantity > 0 && quantity <= currentBorrowedCount) {
            currentBorrowedCount -= quantity;
            return true;
        } else {
            return false;
        }
    }

    // In các thông tin của khách hàng
    virtual void display() const {
        cout << "------------------------------------------\n";
        cout << "Ma KH: " << customerId << " | SDT: " << phone << "\n";
        cout << "Email: " << email << " | Dang muon: " << currentBorrowedCount << " mon\n";
    }

    // Hàm hủy ảo để xóa sạch bộ nhớ
    virtual ~Customer() {}
};

// LỚP KHÁCH CÁ NHÂN (IndividualCustomer)
class IndividualCustomer : public Customer {
private:
    Date dateOfBirth; // Ngày tháng năm sinh
    string gender;    // Giới tính

protected:
    string fullName;  // Họ và tên khách hàng
    string address;   // Địa chỉ cư trú

    // Hàm khởi tạo thông tin chung cho khách cá nhân
    IndividualCustomer(const string& id, const string& p, const string& mail,
                       const string& name, const Date& dob, const string& gen,
                       const string& addr, int borrowed = 0)
        : Customer(id, p, mail, borrowed), dateOfBirth(dob) {
        fullName = name;
        gender = gen;
        address = addr;
    }

public:
    // Trả về họ tên khách hàng
    string getName() const override {
        return fullName;
    }

    // Ưu tiên giảm 35% nếu trùng tháng sinh nhật, ngược lại tính theo phân khúc
    double getFinalDiscount(const Date& today) const override {
        if (dateOfBirth.getMonth() == today.getMonth()) {
            return 0.35;
        } else {
            return calculateDiscount(today);
        }
    }

    // In thông tin khách hàng cá nhân
    void display() const override {
        Customer::display();
        cout << "Ho ten: " << fullName << " | Gioi tinh: " << gender << "\n";
        cout << "Ngay sinh: ";
        dateOfBirth.display();
        cout << " | Dia chi: " << address << "\n";
    }
};

// Khách thường (RegularCustomer)
class RegularCustomer : public IndividualCustomer {
public:
    // Hàm khởi tạo khách thường
    RegularCustomer(const string& id, const string& p, const string& mail,
                    const string& name, const Date& dob, const string& gen,
                    const string& addr, int borrowed = 0)
        : IndividualCustomer(id, p, mail, name, dob, gen, addr, borrowed) {}

    // Trả về tên loại khách "Regular"
    string getCustomerType() const override {
        return "Regular";
    }

    // Trả về hạn mức mượn tối đa của khách thường (3 món)
    int getRentalLimit(const Date& today) const override {
        (void)today;
        return 3;
    }

    // Trả về mức giảm giá mặc định của khách thường (0%)
    double calculateDiscount(const Date& today) const override {
        (void)today;
        return 0.0;
    }

    // In thông tin khách thường
    void display() const override {
        IndividualCustomer::display();
        cout << "Phan khuc: Khach thuong | Han muc: 3 mon | Giam mac dinh: 0%\n";
    }
};

// Khách Học sinh - Sinh viên (StudentCustomer)
class StudentCustomer : public IndividualCustomer {
private:
    string studentID;  // Mã số học sinh / sinh viên (MSSV)
    string schoolName; // Tên trường đang theo học
    Date cardExpiry;   // Ngày hết hạn thẻ HSSV

public:
    // Hàm khởi tạo khách hàng HSSV
    StudentCustomer(const string& id, const string& p, const string& mail,
                    const string& name, const Date& dob, const string& gen,
                    const string& addr, const string& sId, const string& school,
                    const Date& expiry, int borrowed = 0)
        : IndividualCustomer(id, p, mail, name, dob, gen, addr, borrowed), cardExpiry(expiry) {
        studentID = sId;
        schoolName = school;
    }

    // Trả về tên loại khách "Student"
    string getCustomerType() const override {
        return "Student";
    }

    // Thẻ còn hạn cho mượn 5 món, hết hạn tụt về 3 món như khách thường
    int getRentalLimit(const Date& today) const override {
        if (cardExpiry >= today) {
            return 5;
        } else {
            return 3;
        }
    }

    // Thẻ còn hạn giảm 15%, hết hạn không giảm (0%)
    double calculateDiscount(const Date& today) const override {
        if (cardExpiry >= today) {
            return 0.15;
        } else {
            return 0.0;
        }
    }

    // In chi tiết thông tin khách HSSV
    void display() const override {
        IndividualCustomer::display();
        cout << "Phan khuc: HSSV | MSSV: " << studentID << " | Truong: " << schoolName << "\n";
        cout << "Han the: ";
        cardExpiry.display();
        cout << "\n";
    }
};

// Khách VIP (VIPCustomer)
class VIPCustomer : public IndividualCustomer {
private:
    Date vipExpiry;   // Ngày hết hạn gói VIP
    double annualFee; // Phí duy trì VIP hằng năm

public:
    // Hàm khởi tạo khách hàng VIP
    VIPCustomer(const string& id, const string& p, const string& mail,
                const string& name, const Date& dob, const string& gen,
                const string& addr, const Date& expiry, double fee = 120000.0, int borrowed = 0)
        : IndividualCustomer(id, p, mail, name, dob, gen, addr, borrowed), vipExpiry(expiry) {
        annualFee = fee;
    }

    // Trả về tên loại khách "VIP"
    string getCustomerType() const override {
        return "VIP";
    }

    // VIP còn hạn mượn 10 món, hết hạn tụt về 3 món
    int getRentalLimit(const Date& today) const override {
        if (vipExpiry >= today) {
            return 10;
        } else {
            return 3;
        }
    }

    // VIP còn hạn giảm 25%, hết hạn không giảm (0%)
    double calculateDiscount(const Date& today) const override {
        if (vipExpiry >= today) {
            return 0.25;
        } else {
            return 0.0;
        }
    }

    // In chi tiết thông tin khách VIP
    void display() const override {
        IndividualCustomer::display();
        cout << "Phan khuc: VIP | Phi thuong nien: " << fixed << setprecision(0) << (long long)annualFee << " VND | Han VIP: ";
        vipExpiry.display();
        cout << "\n";
    }
};

// LỚP TỔ CHỨC / ĐỐI TÁC (PartnerCustomer)
class PartnerCustomer : public Customer {
private:
    string orgName; // Tên tổ chức / quán cafe / câu lạc bộ / đơn vị sự kiện

protected:
    string address; // Địa chỉ của tổ chức

    // Hàm khởi tạo thông tin chung cho khách tổ chức
    PartnerCustomer(const string& id, const string& p, const string& mail,
                    const string& org, const string& addr, int borrowed = 0)
        : Customer(id, p, mail, borrowed) {
        orgName = org;
        address = addr;
    }

public:
    // Trả về tên tổ chức
    string getName() const override {
        return orgName;
    }

    // In thông tin cơ bản của tổ chức
    void display() const override {
        Customer::display();
        cout << "To chuc: " << orgName << " | Dia chi: " << address << "\n";
    }
};

// Đối tác Quán Cafe (CafePartnerCustomer)
class CafePartnerCustomer : public PartnerCustomer {
private:
    int contractMonths; // Thời hạn hợp đồng (tính theo tháng)

public:
    // Hàm khởi tạo đối tác Quán Cafe
    CafePartnerCustomer(const string& id, const string& p, const string& mail,
                        const string& org, const string& addr, int months, int borrowed = 0)
        : PartnerCustomer(id, p, mail, org, addr, borrowed) {
        if (months > 0) {
            contractMonths = months;
        } else {
            contractMonths = 1;
        }
    }

    // Trả về tên loại khách "Cafe Partner"
    string getCustomerType() const override {
        return "Cafe Partner";
    }

    // Hạn mức mượn sỉ cho quán cafe (100 món)
    int getRentalLimit(const Date& today) const override {
        (void)today;
        return 100;
    }

    // Hợp đồng >= 6 tháng giảm 40%, bình thường giảm 30%
    double calculateDiscount(const Date& today) const override {
        (void)today;

        if (contractMonths >= 6) {
            return 0.40;
        } else {
            return 0.30;
        }
    }

    // In chi tiết thông tin đối tác Quán Cafe
    void display() const override {
        PartnerCustomer::display();
        cout << "Phan khuc: Quan Cafe | Hop dong: " << contractMonths << " thang\n";
    }
};

// Câu lạc bộ (ClubCustomer)
class ClubCustomer : public PartnerCustomer {
private:
    string clubType; // Loại hình CLB (VD: Cong dong, Truong hoc, Cong ty)
    int memberCount; // Số lượng thành viên của CLB

public:
    // Hàm khởi tạo khách Câu lạc bộ
    ClubCustomer(const string& id, const string& p, const string& mail,
                 const string& org, const string& addr, const string& type,
                 int members, int borrowed = 0)
        : PartnerCustomer(id, p, mail, org, addr, borrowed) {
        clubType = type;
        if (members > 0) {
            memberCount = members;
        } else {
            memberCount = 1;
        }
    }

    // Trả về tên loại khách "Club"
    string getCustomerType() const override {
        return "Club";
    }

    // Hạn mức mượn cho câu lạc bộ (20 món)
    int getRentalLimit(const Date& today) const override {
        (void)today;
        return 20;
    }

    // CLB trên 40 thành viên giảm 25%, bình thường giảm 20%
    double calculateDiscount(const Date& today) const override {
        (void)today;

        if (memberCount > 40) {
            return 0.25;
        } else {
            return 0.20;
        }
    }

    // In chi tiết thông tin Câu lạc bộ
    void display() const override {
        PartnerCustomer::display();
        cout << "Phan khuc: Cau lac bo | Loai hinh: " << clubType
             << " | T.Vien: " << memberCount << "\n";
    }
};

// Đối tác Sự kiện (EventPartnerCustomer)
class EventPartnerCustomer : public PartnerCustomer {
private:
    string eventName; // Tên sự kiện / chương trình (VD: Workshop, Teambuilding)
    int eventDays;    // Số ngày diễn ra sự kiện

public:
    // Hàm khởi tạo đối tác Sự kiện
    EventPartnerCustomer(const string& id, const string& p, const string& mail,
                         const string& org, const string& addr, const string& eName,
                         int days, int borrowed = 0)
        : PartnerCustomer(id, p, mail, org, addr, borrowed) {
        eventName = eName;
        if (days > 0) {
            eventDays = days;
        } else {
            eventDays = 1;
        }
    }

    // Trả về tên loại khách
    string getCustomerType() const override {
        return "Event Partner";
    }

    // Hạn mức mượn cho sự kiện (50 món)
    int getRentalLimit(const Date& today) const override {
        (void)today;
        return 50;
    }

    // Sự kiện >= 3 ngày giảm 30%, dưới 3 ngày giảm 20%
    double calculateDiscount(const Date& today) const override {
        (void)today;

        if (eventDays >= 3) {
            return 0.30;
        } else {
            return 0.20;
        }
    }

    // In chi tiết thông tin đối tác Sự kiện
    void display() const override {
        PartnerCustomer::display();
        cout << "Phan khuc: Su kien | Ten SK: " << eventName
             << " | Thoi luong: " << eventDays << " ngay\n";
    }
};

#endif
