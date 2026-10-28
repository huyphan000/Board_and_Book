#include <iostream>
#include <string>    
#include <windows.h> 
#include "class_book_and_boardgame_docfile.cpp"
#include "class_Customer_and_Rental.cpp"
using namespace std; 
// Khai báo các phương thức thanh toán
enum PaymentMethod {
    CASH, // Thanh toán bằng tiền mặt
    BANK_TRANSFER, // Thanh toán bằng chuyển khoản
    OTHER_METHOD // Thanh toán bằng phương thức khác
};

// Khai báo các trạng thái thanh toán                   
enum PaymentStatus {
    UNPAID, // Chưa thanh toán
    PAID, // Đã thanh toán
    REFUNDED // Đã hoàn tiền
};
class FeeCalculator {
private:
    double bankTransferVATRate; // Tỷ lệ VAT khi chuyển khoản, mặc định 3%
    double buybackRate;  // Tỷ lệ của hàng mua lại truyền tranh, mặc định 70%

public:
    // Hàm khởi tạo: tự động chạy khi tạo đối tượng FeeCalculator
    FeeCalculator() {
        bankTransferVATRate = 0.03; buybackRate = 0.70;         
    }
    // Tính tiền thuê dựa trên sản phẩm được truyền vào
    // duration là số tháng thuê truyền tranh hoặc số giờ thuê board game
    double calculateRentalFee(Product* product, int duration) const {
        if (product == nullptr || duration <= 0)
            return 0;
        // Gọi hàm tính tiền thuê của sản phẩm cụ thể
        return product->calculateRentalFee(duration);
    }

    // Tính phí trễ hạn
    // lateDays: số ngày trễ    
    // feePerDay: phí phạt cho mỗi ngày trễ
    double calculateLateFee(int lateDays, double feePerDay) const {
        if (lateDays <= 0 || feePerDay <= 0)
            return 0;
        return lateDays * feePerDay;
    }

    // Tính phí hư hỏng
    // damageFee là mức phí hư hỏng đã được xác định trước
    double calculateDamageFee(double damageFee) const {
        if (damageFee <= 0)
            return 0;
        return damageFee;
    }
    // Tính tổng phí thuê, phí trễ hạn và phí hư hỏng
    double calculateTotalFee(double rentalFee, double lateFee,
        double damageFee) const {
        if (rentalFee < 0) rentalFee = 0;
        if (lateFee < 0) lateFee = 0;
        if (damageFee < 0) damageFee = 0;

        return rentalFee + lateFee + damageFee;
    }
    // Tính tiền mua sản phẩm theo số lượng
    double calculatePurchaseFee(Product* product, int quantity) const {
        if (product == nullptr || quantity <= 0)
            return 0;

        // Gọi hàm tính tiền mua của sản phẩm cụ thể
        return product->calculatePurchaseFee(quantity);
    }
    // Tính giá của hàng mua lại truyền tranh
    double calculateBuybackPrice(double originalPrice) const {
        if (originalPrice <= 0)
            return 0;

        // Giá mua lại = giá gốc * 70%
        return originalPrice * buybackRate;
    }
    // Tính VAT dựa trên phương thức thanh toán
    double calculateVAT(double amount, PaymentMethod method) const {
        if (amount <= 0)
            return 0;

        // Nếu chuyển khoản thì tính VAT 3%
        if (method == BANK_TRANSFER)
            return amount * bankTransferVATRate;
       // Các phương thức khác không tính VAT theo quy ước này
        return 0;
    }
    // Lấy tỷ lệ VAT hiện tại
    double getVATRate() const {
        return bankTransferVATRate;
    }
    // Lấy tỷ lệ mua lại hiện tại
    double getBuybackRate() const {
        return buybackRate;
    }
};
class Payment {
private:
    string paymentId; // Mã thanh toán
    string rentalId; // Mã giao dịch thuê liên quan
    double amount; // Số tiền gốc, chưa tính VAT
    PaymentMethod method; // Phương thức thanh toán
    Date paymentDate; // Ngày thanh toán
    PaymentStatus status; // Trạng thái thanh toán
    double vatAmount; // Số tiền VAT

public:
    // Hàm khởi tạo đối tượng Payment
    Payment(string pId, string rId, double money,
        PaymentMethod m, Date pDate,
        PaymentStatus s = UNPAID) {
        paymentId = pId; // Gán mã thanh toán
        rentalId = rId; // Gán mã giao dịch thuê

        // Nếu số tiền không âm thì nhận số tiền đó, ngược lại gán bằng 0
        amount = (money >= 0) ? money : 0;

        method = m; // Lưu phương thức thanh toán
        paymentDate = pDate; // Lưu ngày thanh toán
        status = s; // Lưu trạng thái ban đầu
        vatAmount = 0; // Ban đầu chưa tính VAT
    }

    // Các hàm getter dùng để đọc giá trị thuộc tính private
    string getPaymentId() const { return paymentId; } // Lấy mã thanh toán
    string getRentalId() const { return rentalId; } // Lấy mã giao dịch thuê
    double getAmount() const { return amount; } // Lấy tiền gốc
    double getVATAmount() const { return vatAmount; } // Lấy tiền VAT
    PaymentMethod getMethod() const { return method; } // Lấy phương thức
    PaymentStatus getStatus() const { return status; } // Lấy trạng thái

    // Tính tổng số tiền phải thanh toán
    double getTotalAmount() const {
        return amount + vatAmount; // Tiền gốc + VAT
    }

    // Thực hiện thanh toán
    void makePayment(const FeeCalculator& calculator) {
        // Nhớ FeeCalculator tính VAT theo số tiền và phương thức
        vatAmount = calculator.calculateVAT(amount, method);

        // Cập nhật trạng thái thành đã thanh toán
        status = PAID;
    }
    // Cập nhật trạng thái giao dịch thành đã hoàn tiền
    void refundPayment() {
        status = REFUNDED;
    }
    // Hiển thị thông tin thanh toán    
    void display() const {
        cout << "\nTHÔNG TIN THANH TOÁN:" << endl;
        cout << "Mã thanh toán: " << paymentId << endl; // In mã thanh toán
		cout << "Mã giao dịch: " << rentalId << endl; // In mã giao dịch thuê
        cout << "Tiền gốc: " << amount << " VND" << endl; // In tiền gốc
        // Hiển thị phương thức thanh toán
        cout << "Phương thức: ";
        if (method == CASH)
            cout << "Tiền mặt" << endl;
        else if (method == BANK_TRANSFER)
            cout << "Chuyển khoản" << endl;
        else
            cout << "Phương thức khác" << endl;
        // Hiển thị tiền VAT và tổng tiền
        cout << "VAT: " << vatAmount << " VND" << endl;
        cout << "Tổng thanh toán: " << getTotalAmount() << " VND" << endl;
        // Hiển thị ngày thanh toán
        cout << "Ngày thanh toán: ";
        paymentDate.display();
        cout << endl;
        // Hiển thị trạng thái thanh toán
        cout << "Trạng thái: ";
        if (status == UNPAID)
            cout << "Chưa thanh toán" << endl;
        else if (status == PAID)
            cout << "Đã thanh toán" << endl;
        else
            cout << "Đã hoàn tiền" << endl;
    }
};
class PaymentStatistics {
private:
    int transactionCount; // Số giao dịch đã thanh toán
    double totalBaseAmount; // Tổng tiền gốc trước VAT
    double totalVAT; // Tổng tiền VAT
    double totalCollected; // Tổng tiền thu được cả VAT
    double cashCollected; // Tổng tiền thu bằng tiền mặt
    double bankTransferCollected; // Tổng tiền thu bằng chuyển khoản
    double otherCollected; // Tổng tiền thu bằng cách khác

public:
    // Hàm khởi tạo: ban đầu tất cả số liệu bằng 0
    PaymentStatistics() {
        transactionCount = 0;
        totalBaseAmount = 0;
        totalVAT = 0;
        totalCollected = 0;
        cashCollected = 0;
        bankTransferCollected = 0;
        otherCollected = 0;
    }

    // Ghi nhận một giao dịch vao thống kê
    void recordPayment(const Payment& payment) {
        // Nếu giao dịch chưa thanh toán hoặc đã hoàn tiền thì bỏ qua
        if (payment.getStatus() != PAID)
            return;
        // Tăng số giao dịch đã thanh toán
        transactionCount++;

        // Cộng tiền gốc vào tổng tiền trước VAT
        totalBaseAmount += payment.getAmount();

        // Cộng VAT vào tổng VAT
        totalVAT += payment.getVATAmount();

        // Cộng tổng tiền đã thu, bao gồm VAT
        totalCollected += payment.getTotalAmount();

        // Cộng tiền vào nhóm phương thức thanh toán tương ứng
        if (payment.getMethod() == CASH)
            cashCollected += payment.getTotalAmount();
        else if (payment.getMethod() == BANK_TRANSFER)
            bankTransferCollected += payment.getTotalAmount();
        else
            otherCollected += payment.getTotalAmount();
    }
    // Hiển thị báo cáo thống kê
    void displayStatistics() const {
        cout << "\nTHÔNG KÊ THANH TOÁN:" << endl;
        cout << "Số giao dịch đã thanh toán: "<< transactionCount << endl;
        cout << "Tổng tiền trước VAT: " << totalBaseAmount << " VND" << endl;
        cout << "Tổng VAT: " << totalVAT << " VND" << endl;
        cout << "Tổng tiền thu được: " << totalCollected << " VND" << endl;
        cout << "Thu bằng tiền mặt: " << cashCollected << " VND" << endl;
        cout << "Thu bằng chuyển khoản: "<< bankTransferCollected << " VND" << endl;
        cout << "Thu bằng cách khác: " << otherCollected << " VND" << endl;
    }
};
