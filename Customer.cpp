#include <iostream>
#include <string>
#include <iomanip> // Thư viện hỗ trợ định dạng setw, setfill
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

    int getDay() { return day; } // Hàm trả về giá trị ngày
    int getMonth() { return month; } // Hàm trả về giá trị tháng
    int getYear() { return year; } // Hàm trả về giá trị năm

    // So sánh hạn thẻ SV hoặc hạn VIP
    bool operator>=(Date other) { // Nạp chồng toán tử >= để so sánh ngày
        if (year != other.year) return year > other.year; // Năm nào lớn hơn thì ngày đó lớn hơn
        if (month != other.month) return month > other.month; // Cùng năm thì tháng nào lớn hơn sẽ lớn hơn
        return day >= other.day; // Cùng năm cùng tháng thì so sánh ngày
    }

    void display() {
        cout << setfill('0') << setw(2) << day << "/" // In ngày đủ 2 chữ số (vd: 05)
             << setw(2) << month << "/" // In tháng đủ 2 chữ số (vd: 03)
             << setw(4) << year << setfill(' '); // In năm đủ 4 chữ số
    }
};

// 2. LỚP THẺ TÍCH ĐIỂM
class MemberCard {
private:
    string cardId; // Mã thẻ thành viên
    int points; // Điểm tích lũy trên thẻ
    string cardTier; // Hạng thẻ (Đồng, Bạc, Vàng, Kim Cương)

public:
    MemberCard(string id = "CARD-00", int pts = 0) { // Hàm khởi tạo thẻ nhận mã và điểm ban đầu
        cardId = id; // Gán mã thẻ thành viên
        if (pts >= 0) points = pts; // Điểm ban đầu không được âm
        else points = 0; // Nếu điểm âm thì tự động gán về 0
        cardTier = "Dong"; // Mặc định ban đầu là hạng Đồng
        updateTier(); // Tự động cập nhật hạng thẻ theo điểm ban đầu
    }

    string getCardId() { return cardId; } // Hàm trả về mã thẻ thành viên
    int getPoints() { return points; } // Hàm trả về số điểm tích lũy hiện có
    string getCardTier() { return cardTier; } // Hàm trả về hạng thẻ hiện tại

    // Xét nâng hạng thẻ tự động theo mốc điểm
    void updateTier() {
        if (points >= 500) cardTier = "Kim Cuong"; // Từ 500 điểm trở lên: Hạng Kim Cương
        else if (points >= 300) cardTier = "Vang"; // Từ 300 đến 499 điểm: Hạng Vàng
        else if (points >= 100) cardTier = "Bac"; // Từ 100 đến 299 điểm: Hạng Bạc
        else cardTier = "Dong"; // Dưới 100 điểm: Hạng Đồng
    }

    // Tích thêm điểm khi mua hoặc thuê
    void addPoints(int p) {
        if (p > 0) { // Điểm cộng vào phải lớn hơn 0
            points += p; // Cộng dồn điểm vào thẻ
            updateTier(); // Cập nhật lại hạng thẻ sau khi cộng điểm
        }
    }

    // Trừ điểm khi đổi quà tặng
    bool redeemPoints(int p) {
        if (p > 0 && points >= p) { // Điểm trừ phải dương và thẻ phải đủ điểm
            points -= p; // Trừ bớt điểm tích lũy trên thẻ
            updateTier(); // Cập nhật lại hạng thẻ nếu bị tụt mốc
            return true; // Trả về true báo đổi quà thành công
        }
        return false; // Trả về false báo không đủ điểm đổi quà
    }

    void display() {
        cout << "Ma the: " << cardId << " | Diem: " << points << " | Hang the: " << cardTier; // In thông tin thẻ
    }
};

// 3. LỚP KHÁCH HÀNG
class Customer {
protected:
    string customerId; // Mã khách hàng
    string phone; // Số điện thoại
    string email; // Email
    MemberCard* card; // Con trỏ liên kết đến thẻ tích điểm

public:
    Customer(string id, string p, string mail, // Hàm khởi tạo nhận mã KH, SĐT, email
             MemberCard* c = nullptr) { // Nhận con trỏ thẻ (mặc định là null)
        customerId = id; // Gán mã khách hàng
        phone = p; // Gán số điện thoại
        email = mail; // Gán email
        card = c; // Gán con trỏ thẻ thành viên
    }

    string getCustomerId() { return customerId; } // Hàm trả về mã khách hàng
    string getPhoneNumber() { return phone; } // Hàm trả về số điện thoại
    string getEmail() { return email; } // Hàm trả về địa chỉ email
    MemberCard* getMemberCard() { return card; } // Hàm trả về con trỏ thẻ thành viên

    // Hàm thuần ảo: lớp con phải tự định nghĩa mức giảm giá và giới hạn mượn
    virtual double calculateDiscount() = 0; // Hàm thuần ảo tính giảm giá mặc định
    virtual int getRentalLimit() = 0; // Hàm thuần ảo lấy số lượng đồ mượn tối đa

    virtual void display() {
        cout << "------------------------------------------\n";
        cout << "Ma KH: " << customerId // In mã khách hàng
             << " | SDT: " << phone << "\n"; // In số điện thoại
        cout << "Email: " << email << "\n"; // In địa chỉ email
        if (card != nullptr) { // Nếu khách hàng có đăng ký thẻ
            cout << "The TV -> "; // In nhãn thẻ thành viên
            card->display(); // Gọi hàm in thông tin của thẻ
            cout << "\n";
        } else { // Nếu không có thẻ thành viên
            cout << "The TV -> Khong dang ky\n"; // In thông báo chưa đăng ký thẻ
        }
    }

    // Hàm hủy ảo để thu hồi bộ nhớ của con trỏ thẻ khi xóa khách hàng
    virtual ~Customer() {
        if (card != nullptr) {
            delete card;
            card = nullptr;
        }
    }
};

// 4. LỚP KHÁCH HÀNG CÁ NHÂN
class IndividualCustomer : public Customer { // Lớp IndividualCustomer kế thừa Customer
protected:
    string fullName; // Họ và tên 
    Date dateOfBirth; // Ngày tháng năm sinh
    string gender; // Giới tính
    string address; // Địa chỉ

public:
    IndividualCustomer(string id, string p, string mail, // Constructor nhận mã KH, SĐT, email
                       MemberCard* c, string name, // Nhận thẻ, họ và tên
                       Date dob, string gen, string addr) // Nhận ngày sinh, giới tính, địa chỉ
        : Customer(id, p, mail, c), dateOfBirth(dob) { // Gọi constructor lớp Customer và Date
        fullName = name; // Gán họ và tên
        gender = gen; // Gán giới tính
        address = addr; // Gán địa chỉ
    } 

    string getFullName() { return fullName; } // Hàm trả về họ tên
    Date getDateOfBirth() { return dateOfBirth; } // Hàm trả về ngày sinh

    // Kiểm tra tháng hôm nay có trùng tháng sinh nhật không
    bool isBirthdayMonth(Date today) { // Nhận vào ngày giao dịch hôm nay
        return (dateOfBirth.getMonth() == today.getMonth()); // Trả về true nếu trùng tháng sinh
    }

    // Nếu đúng tháng sinh nhật thì ưu tiên giảm 35%, ngược lại giảm theo từng loại khách
    double getFinalDiscount(Date today) { // Nhận vào ngày giao dịch hôm nay
        if (isBirthdayMonth(today)) { // Nếu giao dịch vào tháng sinh nhật
            return 0.35; // Ưu tiên giảm giá sinh nhật 35%
        }
        return calculateDiscount(); // Nếu không phải sinh nhật thì giảm bình thường
    }

    void display() override { // Ghi đè hàm hiển thị của lớp Customer
        cout << "------------------------------------------\n";
        cout << "Ma KH: " << customerId // In mã khách hàng
             << " | Ho ten: " << fullName << "\n"; // In họ và tên
        cout << "SDT: " << phone << " | Email: " << email << "\n"; // In SĐT và email
        cout << "Gioi tinh: " << gender << " | Ngay sinh: "; // In giới tính và nhãn ngày sinh
        dateOfBirth.display(); // Gọi display của Date để in ngày sinh
        cout << "\nDia chi: " << address << "\n"; // In địa chỉ nhà
        if (card != nullptr) { // Nếu khách có thẻ thành viên
            cout << "The TV -> "; // In nhãn thẻ
            card->display(); // In chi tiết thẻ thành viên
            cout << "\n"; // Xuống dòng
        } else { // Nếu không có thẻ thành viên
            cout << "The TV -> Khong dang ky\n"; // In thông báo chưa đăng ký
        }
    }
};

// 5. LỚP KHÁCH HÀNG THÔNG THƯỜNG
class RegularCustomer : public IndividualCustomer { // Kế thừa từ IndividualCustomer
private:
    string regularPolicy; // Chính sách cọc áp dụng cho khách thường

public:
    RegularCustomer(string id, string p, string mail, // Constructor nhận thông tin liên lạc
                    MemberCard* c, string name, Date dob, // Nhận thẻ, họ tên, ngày sinh
                    string gen, string addr) // Nhận giới tính và địa chỉ
        : IndividualCustomer(id, p, mail, c, name, dob, gen, addr) { // Gọi constructor của lớp cha
        regularPolicy = "Tieu chuan - Coc 100%"; // Gán chính sách cọc mặc định 100%
    }

    int getRentalLimit() override { // Ghi đè hàm lấy số lượng mượn tối đa
        return 3; // Khách thường mượn tối đa 3 món
    }

    // Giảm giá theo bậc thẻ tích điểm: Kim Cương 15%, Vàng 10%, Bạc 5%
    double calculateDiscount() override { // Ghi đè hàm tính tỷ lệ giảm giá
        if (card != nullptr) { // Nếu khách có đăng ký thẻ tích điểm
            if (card->getCardTier() == "Kim Cuong") return 0.15; // Hạng Kim Cương giảm 15%
            if (card->getCardTier() == "Vang") return 0.10; // Hạng Vàng giảm 10%
            if (card->getCardTier() == "Bac") return 0.05; // Hạng Bạc giảm 5%
        }
        return 0.0; // Hạng Đồng hoặc chưa có thẻ giảm 0%
    }

    void display() override { // Ghi đè hàm hiển thị
        IndividualCustomer::display(); // Gọi hiển thị thông tin cá nhân
        cout << "Phan khuc: Khach thuong | Chinh sach: " // In phân khúc khách hàng
             << regularPolicy << "\n"; // In chính sách cọc
    }
};

//6. KHÁCH HỌC SINH, SINH VIÊN
class StudentCustomer : public IndividualCustomer { // Kế thừa từ IndividualCustomer
private:
    string studentID; // Mã số sinh viên (MSSV)
    string schoolName; // Tên trường đại học / trường học
    Date cardExpiry; // Ngày hết hạn thẻ học sinh / sinh viên

public:
    StudentCustomer(string id, string p, string mail, // Constructor nhận thông tin liên lạc
                    MemberCard* c, string name, Date dob, // Nhận thẻ, họ tên, ngày sinh
                    string gen, string addr, string sId, // Nhận giới tính, địa chỉ, MSSV
                    string school, Date expiry) // Nhận tên trường và hạn thẻ SV
        : IndividualCustomer(id, p, mail, c, name, dob, gen, addr), // Gọi constructor của lớp cha
          cardExpiry(expiry) { // Khởi tạo đối tượng ngày hết hạn thẻ SV
        studentID = sId; // Gán mã số sinh viên
        schoolName = school; // Gán tên trường học
    }

    // Kiểm tra thẻ sinh viên còn hạn hay không
    bool isStudentCardValid(Date today) { // Nhận vào ngày hôm nay để đối chiếu
        return cardExpiry >= today; // Còn hạn nếu ngày hết hạn >= hôm nay
    }

    int getRentalLimit() override { // Ghi đè hàm lấy số lượng mượn tối đa
        return 5; // HSSV được mượn tối đa 5 món
    }

    double calculateDiscount() override { // Ghi đè hàm tính tỷ lệ giảm giá
        return 0.15; // Trợ giá sinh viên cố định 15%
    }

    void display() override { // Ghi đè hàm hiển thị
        IndividualCustomer::display(); // Gọi hiển thị thông tin cá nhân
        cout << "Phan khuc: HSSV | Ma HSSV: " << studentID // In phân khúc và mã số sinh viên
             << " | Truong: " << schoolName << "\n"; // In tên trường học
        cout << "Han the HSSV: "; // In nhãn hạn thẻ sinh viên
        cardExpiry.display(); // Gọi display của Date để in ngày
        cout << "\n";
    }
};

// 7.LỚP KHÁCH HÀNG VIP
class VIPCustomer : public IndividualCustomer { // Kế thừa từ IndividualCustomer
private:
    Date vipExpiry; // Ngày hết hạn của gói VIP
    double annualFee; // Phí duy trì gói VIP 120.000 VNĐ / năm

public:
    VIPCustomer(string id, string p, string mail, MemberCard* c, // Constructor nhận thông tin cơ bản
                string name, Date dob, string gen, string addr, // Nhận họ tên, ngày sinh, giới tính, địa chỉ
                Date expiry, double fee = 120000.0) // Nhận hạn VIP và phí duy trì 120k
        : IndividualCustomer(id, p, mail, c, name, dob, gen, addr), // Gọi constructor của lớp cha
          vipExpiry(expiry) { // Khởi tạo ngày hết hạn gói VIP
        annualFee = fee; // Gán mức phí duy trì hàng năm
    }

    Date getVipExpiry() { return vipExpiry; } // Hàm trả về ngày hết hạn gói VIP

    bool isVipActive(Date today) { // Nhận vào ngày hôm nay để đối chiếu
        return vipExpiry >= today; // Còn hạn nếu hạn VIP >= ngày hôm nay
    }

    // Đóng phí thường niên 120.000 VNĐ để gia hạn VIP thêm 1 năm
    bool payAnnualFee(double amountPaid, Date newExpiry) { // Nhận số tiền đóng và hạn VIP mới
        if (amountPaid >= annualFee) { // Nếu đóng đủ hoặc thừa 120.000 VNĐ
            vipExpiry = newExpiry; // Cập nhật hạn VIP sang năm mới
            return true; // Trả về true báo gia hạn thành công
        }
        return false; // Trả về false báo đóng thiếu tiền
    }

    int getRentalLimit() override { // Ghi đè hàm lấy số lượng mượn tối đa
        return 10; // Khách VIP mượn tối đa 10 món
    }

    double calculateDiscount() override { // Ghi đè hàm tính tỷ lệ giảm giá
        return 0.25; // Khách VIP giảm giá cố định 25%
    }

    void display() override { // Ghi đè hàm hiển thị
        IndividualCustomer::display(); // Gọi hiển thị thông tin cá nhân
        cout << "Phan khuc: VIP (Giam 25% | Muon 10 mon)\n"; // In quyền lợi VIP
        cout << "Phi duy tri: " << (long long)annualFee // In mức phí duy trì thường niên
             << " VND/nam | Han VIP: "; // In nhãn hạn VIP
        vipExpiry.display(); // In ngày hết hạn gói VIP
        cout << "\n";
    }
};
// 8. KHÁCH DOANH NGHIỆP 
class CorporateCustomer : public Customer { // Kế thừa trực tiếp từ lớp Customer
private:
    string companyName; // Tên công ty / tổ chức
    string taxCode; // Mã số thuế doanh nghiệp
    string contractID; // Mã hợp đồng thuê kinh tế
    string address; // Địa chỉ trụ sở công ty

public:
    CorporateCustomer(string id, string p, string mail, // Constructor nhận mã KH, SĐT, email
                      MemberCard* c, string comp, string tax, // Nhận thẻ, tên công ty, MST
                      string contract, string addr) // Nhận mã hợp đồng và địa chỉ
        : Customer(id, p, mail, c) { // Gọi constructor của Customer
        companyName = comp; // Gán tên công ty
        taxCode = tax; // Gán mã số thuế
        contractID = contract; // Gán mã hợp đồng
        address = addr; // Gán địa chỉ doanh nghiệp
    }

    // Thuê sỉ từ 30 món trở lên giảm 30%, dưới 30 món giảm 20%
    double calculateBulkDiscount(int quantity) { // Nhận vào số lượng món thuê
        if (quantity >= 30) return 0.30; // Thuê từ 30 món trở lên giảm 30%
        return 0.20; // Dưới 30 món giảm 20%
    }

    int getRentalLimit() override { // Ghi đè hàm lấy số lượng mượn tối đa
        return 50; // Doanh nghiệp được mượn số lượng lớn (50 món)
    }

    double calculateDiscount() override { // Ghi đè hàm tính tỷ lệ giảm giá
        return 0.20; // Mặc định giảm 20% theo hợp đồng
    }

    void display() override { // Ghi đè hàm hiển thị
        Customer::display(); // Gọi hiển thị thông tin chung Customer
        cout << "Phan loai: DOANH NGHIEP | Cong ty: " // In loại khách doanh nghiệp
             << companyName << "\n"; // In tên công ty
        cout << "MST: " << taxCode // In mã số thuế
             << " | Ma HD: " << contractID << "\n"; // In mã hợp đồng
        cout << "Dia chi cong ty: " << address << "\n"; // In địa chỉ công ty
    }
};
