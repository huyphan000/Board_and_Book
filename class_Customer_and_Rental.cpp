#include<iostream>
#include<string>
#include<windows.h>
#include "class_book_and_boardgame_docfile.cpp"
using namespace std;

// Khai báo các trạng thái của giao dịch thuê / mua
enum RentalStatus{
    RENTING, // Đang thuê
    RETURNED, // Đã trả
    OVERDUE, // Quá hạn
    PURCHASED // Đã mua
};

// Lớp Date đại diện cho ngày tháng năm
class Date{
    private:
        int day; // Ngày
        int month; // Tháng
        int year; // Năm
    public:
        // Hàm khởi tạo ngày tháng năm
        Date(int d = 0, int m = 0, int y = 0){
            day = d; // Gán ngày
            month = m; // Gán tháng
            year = y; // Gán năm
        }
    int getDay() const { return day; } // Hàm trả về ngày
    int getMonth() const { return month; }// Hàm trả về tháng
    int getYear() const { return year; }// Hàm trả về năm
    
    // Hàm hiển thị ngày tháng năm
    void display() const{
        cout << day << "/" << month << "/" << year;
    }
};

// Lớp cha Customer đại diện cho khách hàng
class Customer{
    private:
        string customerId; // Mã khách hàng
        string fullName; // Họ tên khách hàng
        string phoneNumber; // Số điện thoại
        string email; // Địa chỉ email
    public:
        // Hàm khởi tạo
        Customer(string id, string name, string phone, string mail){
            customerId = id;// Gán mã khách hàng
            fullName = name; // Gán họ tên
            phoneNumber = phone; // Gán số điện thoại
            email = mail;// Gán email
        }

        // Hàm trả về mã khách hàng
        string getCustomerId() const{
            return customerId;
        }
        // Hàm trả về họ tên khách hàng
        string getFullName() const{
            return fullName;
        }
        // Hàm trả về số điện thoại
        string getPhoneNumber() const{
            return phoneNumber;
        }
        // Hàm trả về email
        string getEmail() const{
            return email;
        }

    // Cập nhật thông tin khách hàng
    void setCustomerInfo(string name, string phone, string mail){
        fullName = name; // Cập nhật lại họ tên
        phoneNumber = phone; // Cập nhật lại số điện thoại
        email = mail; // Cập nhật lại email
    }

    // Hàm ảo hiển thị thông tin khách hàng
    virtual void display() const{
        cout << " Mã khách hàng : " << customerId << endl;// In mã khách hàng
        cout << " Họ tên        : " << fullName << endl;// In họ tên
        cout << " Số điện thoại : " << phoneNumber << endl;// In số điện thoại
        cout << " Email         : " << email << endl;// In email
    }

    virtual ~Customer(){}// Hàm hủy
};

// Lớp con VIPCustomer kế thừa từ lớp Customer, đại diện cho khách hàng thân thiết
class VIPCustomer : public Customer{
    private:
        int membershipPoints; // Điểm tích lũy của khách hàng VIP
    public:
        // Hàm khởi tạo cho khách hàng VIP
        VIPCustomer(string id, string name, string phone, string mail, int points) 
            : Customer(id, name, phone, mail){
            membershipPoints = points; // Gán điểm tích lũy
        }
        
        // Hàm trả về điểm tích lũy
        int getMembershipPoints() const {
            return membershipPoints; 
        }

        // Cộng điểm tích lũy sau giao dịch
        void addPoints(int points){
            if (points > 0){
                membershipPoints += points;// Cộng dồn điểm mới vào điểm hiện có
            }
        }

        // Ghi đè hàm hiển thị thông tin
        void display() const override{
            Customer::display();// Hiển thị thông tin chung của khách hàng
            cout << " Hạng khách    : VIP" << endl;// In hạng khách hàng
            cout << " Điểm tích lũy : " << membershipPoints << " điểm" << endl;// In điểm tích lũy
        }
};

// Lớp Rental quản lý phiếu giao dịch (Sử dụng con trỏ Customer* và Product* để liên kết chặt chẽ)
class Rental{
    private:
        string rentalId; // Mã phiếu giao dịch
        Customer* customer; // Con trỏ liên kết đến khách hàng (có thể là khách thường hoặc VIP)
        Product* product; // Con trỏ liên kết đến sản phẩm (ComicBook hoặc BoardGame)
        Date rentalDate; // Ngày thuê / mua
        Date dueDate; // Ngày hẹn trả
        Date returnDate; // Ngày trả thực tế
        RentalStatus status; // Trạng thái giao dịch
    public:
        // Hàm khởi tạo nhận vào đối tượng khách hàng và sản phẩm
        Rental(string rId, Customer* c, Product* p, Date rDate, Date dDate, RentalStatus s){
            rentalId = rId; // Gán mã phiếu giao dịch
            customer = c;// Lưu con trỏ liên kết với đối tượng khách hàng
            product = p;// Lưu con trỏ liên kết với đối tượng sản phẩm
            rentalDate = rDate;// Gán ngày bắt đầu thuê hoặc mua
            dueDate = dDate;// Gán ngày hẹn trả
            returnDate = Date(0, 0, 0);// Khi mới tạo phiếu thì ngày trả thực tế mặc định là 0/0/0
            status = s;// Gán trạng thái giao dịch ban đầu
        }

        // Hàm trả về mã phiếu thuê
        string getRentalId() const{
            return rentalId;
        }
        // Hàm trả về khách hàng
        Customer* getCustomer() const{
            return customer;
        }
        // Hàm trả về sản phẩm
        Product* getProduct() const{
            return product;
        }
        // Hàm trả về ngày thuê
        Date getRentalDate() const{
            return rentalDate;
        }
        // Hàm trả về hạn trả
        Date getDueDate() const{
            return dueDate;
        }
        // Hàm trả về ngày trả thực tế
        Date getReturnDate() const{
            return returnDate;
        }
        // Hàm trả về trạng thái giao dịch
        RentalStatus getStatus() const{
            return status;
        }

    // Chức năng Mua sản phẩm
    void buyProduct(Date buyDate){
        rentalDate = buyDate;// Ghi nhận ngày mua
        status = PURCHASED;// Cập nhật trạng thái thành đã mua
        cout << " -> Khách hàng " << customer->getFullName() << " đã mua thành công sản phẩm [" << product->getProductID() << "] " << product->getName() << endl; // In thông báo mua
    }

    // Chức năng Thuê sản phẩm
    void rentProduct(Date rDate, Date dDate){
        rentalDate = rDate; // Ghi nhận ngày bắt đầu thuê
        dueDate = dDate;// Ghi nhận ngày hẹn trả
        status = RENTING;// Cập nhật trạng thái thành đang thuê
        cout << " -> Khách hàng " << customer->getFullName() << " đã thuê thành công sản phẩm [" << product->getProductID() << "] " << product->getName() << endl; // In thông báo thuê
    }

    // Chức năng Trả sản phẩm
    void returnProduct(Date actualReturnDate){
        returnDate = actualReturnDate; // Ghi nhận ngày trả thực tế
        status = RETURNED;// Cập nhật trạng thái thành đã trả
        cout << " -> Khách hàng " << customer->getFullName() << " đã trả sản phẩm [" << product->getProductID() << "] " << product->getName() << endl; // In thông báo trả
    }

    // Đổi trạng thái quá hạn
    void setOverdue(){
        if (status == RENTING){
            status = OVERDUE;// Đổi trạng thái từ đang thuê sang quá hạn
            cout << " -> Giao dịch " << rentalId << " của khách hàng " << customer->getCustomerId() << " đã bị quá hạn!" << endl;
        }
    }

    // Hiển thị thông tin phiếu giao dịch kèm chi tiết khách và sản phẩm
    void display() const{
        cout << " Mã giao dịch  : " << rentalId << endl; // In mã giao dịch
        cout << "[THÔNG TIN KHÁCH HÀNG]" << endl;
        if (customer != nullptr) {
            customer->display(); // Gọi đa hình (in được cả khách thường lẫn VIP)
        }
        cout << "[THÔNG TIN SẢN PHẨM]" << endl;
        if (product != nullptr) {
            product->display(); // Gọi hàm display của sản phẩm
        }
        cout << " Ngày giao dịch: "; rentalDate.display(); cout << endl;// In ngày thuê hoặc mua

        //Chỉ in Hạn trả và Ngày trả thực tế nếu đây là giao dịch Thuê (không phải Mua đứt)
        if (status != PURCHASED){
            cout << " Hạn trả       : "; dueDate.display(); cout << endl;// In ngày hẹn trả
            if (status == RETURNED){
                cout << " Ngày trả thực tế : "; returnDate.display(); cout << endl; // Chỉ in ngày trả thực tế khi khách đã trả
            }
        }
        // Kiểm tra và in trạng thái giao dịch
        cout << " Trạng thái    : ";
        if (status == RENTING)
            cout << "Đang thuê" << endl; // Nếu là RENTING thì in Đang thuê
        else if (status == RETURNED)
            cout << "Đã trả" << endl; // Nếu là RETURNED thì in Đã trả
        else if (status == OVERDUE)
            cout << "Quá hạn" << endl; // Nếu là OVERDUE thì in Quá hạn
        else if (status == PURCHASED) 
            cout << "Đã mua" << endl; // Nếu là PURCHASED thì in Đã mua
    }
};
