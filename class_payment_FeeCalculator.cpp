#include <iostream>
#include <windows.h>
#include <string>
#include "class_Customer_and_Rental.cpp"
using namespace std;

// Phương thức thanh toán
enum PaymentMethod {
	CASH, // Thanh toán tiền mặt
    BANK_TRANSFER, // Thanh toán chuyển khoản
    OTHER // Phương thức thanh toán khác
};
// Trạng thái thanh toán
enum PaymentStatus {
    PENDING, // Đang chờ thanh toán
    COMPLETED, // Đã thanh toán
    CANCELLED // Đã hủy thanh toán
};

// Lớp tính các khoản phí
class FeeCalculator {
public:
    // Tính tiền phí thuê truyện theo ngày
    double calculateRentalFee(double pricePerDay, int days) {
        if (pricePerDay < 0 || days < 0) // Kiểm tra thông tin dữ liệu
            return 0;
        return pricePerDay * days;
    }
    // Tính tiền phạt trả trễ
    double calculateLateFee(double feePerDay, int lateDays) {
        if (feePerDay < 0 || lateDays < 0) // Kiểm tra thông tin dữ liệu
            return 0;
        return feePerDay * lateDays;
    }
    // Tính tiền bồi thường hư hỏng
    double calculateDamageFee(double damageFee) {
        if (damageFee < 0) // Kiểm tra dữ liệu phí hư hỏng
            return 0;
        return damageFee;
    }
    // Tính tổng tiền thuê và các khoản phạt
    double calculateTotalFee(double rentalFee, double lateFee, double damageFee) {
        return rentalFee + lateFee + damageFee;
    }
    // Tính giá của hàng mua lại bằng 70% giá gốc
    double calculateBuybackPrice(double originalPrice) {
        if (originalPrice < 0) // Kiểm tra giá gốc
            return 0;
        return originalPrice * 0.70;
    }
};

// Lớp quản lý thanh toán
class Payment {
private:
    string paymentId; // Mã giao dịch
    string rentalId; // Mã thuê
    double amount; // Số tiền trước phí
    PaymentMethod method; // Phương thức thanh toán
    string paymentDate; // Ngày thanh toán
    PaymentStatus status; // Trạng thái thanh toán
public:
	// Khởi tạo thanh toán mặc định
    Payment() {
        paymentId = ""; // Khởi tạo mã thanh toán rỗng
        rentalId = ""; // Khởi tạo mã thuê rỗng
        amount = 0;  // Khởi tạo số tiền bằng 0
        method = CASH; // Mặc định thanh toán tiền mặt
        paymentDate = ""; // Khởi tạo ngày thanh toán rỗng
        status = PENDING;  // Mặc định đang chờ thanh toán
    }
    // Khởi tạo thanh toán có thông tin
    Payment(string pId, string rId, double money,
        PaymentMethod m, string date) {
        paymentId = pId; // Gán mã thanh toán
        rentalId = rId; // Gán mã phiếu thuê
        amount = (money >= 0) ? money : 0; // Kiểm tra số tiền
        method = m; // Gán phương thức thanh toán
        paymentDate = date;  // Gán ngày thanh toán
        status = PENDING; // Đặt trạng thái cho thanh toán
    }
    // Tính phí VAT chuyển khoản 3%
    double calculateExtraFee() const {
        if (method == BANK_TRANSFER) // Nếu khách chuyển khoản
            return amount * 0.03;  // Tính phí VAT 3%
        return 0; 
    }
    // Tính tổng tiền cần thanh toán 
    double calculateFinalAmount() const {
        return amount + calculateExtraFee();
    }
    // Xác nhận giao dịch thành công
    void completePayment() {
        status = COMPLETED;// Cập nhật trạng thái
    }
    // Hủy giao dịch
    void cancelPayment() {
        status = CANCELLED; // Cập nhật trạng thái hủy
    }
    // Hàm trả về mã thanh toán
    string getPaymentId() const {
        return paymentId;
    }
    // Hàm trả về mã phiếu thuê
    string getRentalId() const {
        return rentalId;
    }
    // Hàm trả về số tiền ban đầu
    double getAmount() const {
        return amount;
    }
    // Hàm trả về phí thanh toán
    double getExtraFee() const {
        return calculateExtraFee();
    }
    // Hàm trả về tổng tiền thanh toán
    double getFinalAmount() const {
        return calculateFinalAmount();
    }
    // Hàm trả về phương thức thanh toán
    PaymentMethod getMethod() const {
        return method;
    }
    // Hàm trả về trạng thái thanh toán
    PaymentStatus getStatus() const {
        return status;
    }
    // Hàm trả về ngày thanh toán
    string getPaymentDate() const {
        return paymentDate;
    }
    // Hiển thị thông tin thanh toán
    void displayPayment() const {
        cout << "\n    THÔNG TIN THANH TOÁN\n"; // Tiêu đề
        cout << "Mã thanh toán: " << paymentId << endl; // In mã
        cout << "Mã phiếu thuê: " << rentalId << endl;  // In mã thuê
        cout << "Ngày thanh toán: " << paymentDate << endl; // In ngày
        cout << "Số tiền ban đầu: " << amount << " VND\n"; // In tiền gốc
        cout << "Phụ phí: " << calculateExtraFee() << " VND\n"; // In phí
        cout << "Tổng tiền: " << calculateFinalAmount() << " VND\n"; // In tổng

		cout << "Phương thức: "; // In phương thức thanh toán
		if (method == CASH)         // Thanh toán tiền mặt
            cout << "Tiền mặt";
		else if (method == BANK_TRANSFER) // Thanh toán chuyển khoản
            cout << "Chuyển khoản";
		else                   // Thanh toán phương thức khác
            cout << "Khác";

        cout << "\nTrang thái: ";// In tiêu đề trạng thái
		if (status == PENDING)   // Trạng thái đang chờ thanh toán
            cout << "Chờ thanh toán";
		else if (status == COMPLETED) // Trạng thái đã thanh toán
            cout << "Đã thanh toán";
        else          // Hủy giao dịch
            cout << "Đã hủy";
        cout << endl;
    }
};

// Lớp thống kê các khoản thu chi
class MoneyStatistics {
private:
    double rentalIncome; // Tổng tiền thuê
	double lateIncome; // Tổng tiền phạt trả trễ
    double damageIncome; // Tổng tiền bồi thường
    double extraFeeIncome; // Tổng phí thanh toán
    double buybackExpense; // Tổng tiền chi mua lại truyện và board game
    int completedPayments; // Số giao dịch thành công
public:
	// Khởi tạo thống kê tiền mặc định
    MoneyStatistics() {
        rentalIncome = 0;// Tiền thuê ban đầu bằng 0
		lateIncome = 0;  // Tiền phạt ban đầu bằng 0
		damageIncome = 0;// Tiền bồi thường ban đầu bằng 0
		extraFeeIncome = 0; // Phụ phí ban đầu bằng 0
        buybackExpense = 0;  // Chi mua lại truyện và board game ban đầu bằng 0
        completedPayments = 0; // Giao dịch thành công ban đầu bằng 0
    }
    // Gọi hàm doanh thu tiền thuê
    void addRentalIncome(double amount) {
        if (amount > 0)   // Kiểm tra số tiền
            rentalIncome += amount;
    }
	// Gọi hàm doanh thu tiền phạt trả trễ
    void addLateIncome(double amount) {
        if (amount > 0)  // Kiểm tra số tiền
            lateIncome += amount; 
    }
    // Gọi hàm doanh thu bồi thường
    void addDamageIncome(double amount) {
        if (amount > 0)  // Kiểm tra số tiền
            damageIncome += amount;
    }
	// Gọi hàm doanh thu phụ phí thanh toán
    void addExtraFeeIncome(double amount) {
        if (amount > 0)  // Kiểm tra số tiền
            extraFeeIncome += amount;
    }
    // Gọi hàm chi phí mua lại truyện
    void addBuybackExpense(double amount) {
        if (amount > 0)  // Kiểm tra số tiền
            buybackExpense += amount;
    }
	// Gọi hàm tăng số giao dịch thành công
    void addCompletedPayment() {
        completedPayments++;
    }
	// Gọi hàm tính tổng các khoản thu
    double calculateTotalIncome() const {
        return rentalIncome + lateIncome + damageIncome + extraFeeIncome;
    }
    // Tính số tiền thu được sau khi trừ tiền mua lại
    double calculateNetIncome() const {
        return calculateTotalIncome() - buybackExpense;
    }
    // Hiển thị báo cáo thu chi
    void displayStatistics() const {
        cout << "\n   THỐNG KÊ THU CHI\n";
		cout << "Tiền thuê: " << rentalIncome << " VND\n"; // In tiền thuê
		cout << "Tiền phạt trả trễ: " << lateIncome << " VND\n"; // In tiền phạt trả trễ
		cout << "Tiền bồi thường: " << damageIncome << " VND\n"; // In tiền bồi thường
		cout << "Phụ phí: " << extraFeeIncome << " VND\n"; // In phụ phí thanh toán
        cout << "Tiền chi mua lại: " << buybackExpense << " VND\n"; // In chi phí mua lại
        cout << "Giao dịch thành công: "<< completedPayments << endl; // In số giao dịch
        cout << "Tổng các khoản thu: "<< calculateTotalIncome() << " VND\n"; // In tổng doanh thu
        cout << "Tổng thu sau khi trừ chi phí mua lại: "<< calculateNetIncome() << " VND\n"; // In tổng tiền thu sau khi trừ chi phí mua lại
    }
};
