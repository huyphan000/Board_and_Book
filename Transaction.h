// =============================================================================
//  Transaction.h - Lớp cha TRỪU TƯỢNG của hệ thống giao dịch (mục 3.3)
//
//                        Transaction (abstract)
//                                |
//            +-------------------+-------------------+
//            |                   |                   |
//     RentalTransaction   PurchaseTransaction   BuybackTransaction
//
//  Lớp này giữ thông tin CHUNG: mã giao dịch, khách hàng, ngày tạo, trạng thái.
//  Phần RIÊNG của từng loại giao dịch do lớp con cài đặt (tính đa hình).
//
//  ---------------------------------------------------------------------------
//  YÊU CẦU với class của nhóm (Transaction chỉ gọi đúng các hàm này):
//      Customer : getId() , getName()
//      Product  : getId() , getName() , getPrice() , getStock() ,
//                 increaseStock(int) , decreaseStock(int)
//  Transaction chỉ GIỮ CON TRỎ tới Customer / Product, không sở hữu và không delete.
//  Nếu tên hàm của nhóm khác thì chỉ cần sửa lại các chỗ gọi.
//  ---------------------------------------------------------------------------
//
//  Vòng đời trạng thái:
//
//      Pending --executeTransaction()--> Processing --> Completed
//         |                                  |
//         +---------- cancelTransaction() ---+------> Cancelled
//
//  (Mua hàng đi thẳng Pending -> Completed; Thuê và Mua lại đi qua Processing.)
// =============================================================================
#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <ctime>
#include <stdexcept>
#include <string>

#include "Customer.h"

// Lỗi nghiệp vụ: hết hàng, sai trạng thái, tham số không hợp lệ...
// Dùng try { ... } catch (const TransactionException& e) { std::cout << e.what(); }
class TransactionException : public std::runtime_error {
public:
    explicit TransactionException(const std::string& message)
        : std::runtime_error(message) {}
};

enum class TransactionStatus {
    Pending,     // Mới tạo, đang lập phiếu, chưa thực hiện
    Processing,  // Đang thực hiện (thuê: đang cho thuê | mua lại: đã định giá, chờ duyệt)
    Completed,   // Hoàn tất
    Cancelled    // Đã hủy
};

class Transaction {
public:
    virtual ~Transaction() {}  // virtual để delete qua con trỏ lớp cha vẫn đúng

    // Mỗi giao dịch có MỘT mã duy nhất nên không cho phép sao chép.
    Transaction(const Transaction&) = delete;
    Transaction& operator=(const Transaction&) = delete;

    // ===== 4 phương thức của lớp Transaction theo bảng thiết kế =====

    // Thực hiện giao dịch. Thuần ảo vì mỗi loại làm một việc khác nhau:
    //   Mua hàng : thanh toán, trừ kho            -> Completed
    //   Thuê     : giao hàng cho khách thuê       -> Processing
    //   Mua lại  : duyệt, chi tiền, nhập kho      -> Completed
    virtual void executeTransaction() = 0;

    // Tổng tiền của giao dịch (VND). Thuần ảo vì mỗi loại tính một kiểu.
    virtual double calculateTotal() const = 0;

    // Hủy giao dịch. Ném TransactionException nếu giao dịch đã hoàn tất hoặc đã hủy.
    // Lớp con ghi đè nếu cần hoàn tác thêm (vd: phiếu thuê phải hoàn kho) và PHẢI
    // gọi Transaction::cancelTransaction() trước để kiểm tra trạng thái.
    virtual void cancelTransaction();

    // In thông tin chung ra màn hình. Lớp con ghi đè: in thêm phần của mình.
    virtual void displayTransaction() const;

    // Trạng thái bằng chữ tiếng Việt (lớp con ghi đè để gọi tên cho đúng nghiệp vụ).
    virtual std::string getStatusText() const;

    // ===== Getter =====
    const std::string& getTransactionId() const { return transactionId_; }
    Customer* getCustomer() const { return customer_; }
    std::time_t getCreatedDate() const { return createdDate_; }
    TransactionStatus getStatus() const { return status_; }

    // ===== Tiện ích ngày & tiền (static, dùng chung cho các lớp con) =====
    // Ngày được lưu bằng std::time_t. Các hàm dưới chỉ so sánh theo NGÀY, bỏ qua giờ.
    static std::time_t makeDate(int day, int month, int year);  // tạo ngày; ném lỗi nếu không tồn tại
    static std::time_t addDays(std::time_t date, int days);     // cộng (hoặc trừ) số ngày
    static int daysBetween(std::time_t from, std::time_t to);   // số ngày từ `from` đến `to`
    static std::string formatDate(std::time_t date);            // "dd/mm/yyyy"
    static std::string formatMoney(double amount);              // 1250000 -> "1.250.000 VND"
    static double roundMoney(double amount);                    // làm tròn đến 1 đồng

protected:
    // transactionId: do lớp con tạo bằng makeId("PT", số thứ tự).
    // Ném TransactionException nếu customer là nullptr.
    Transaction(const std::string& transactionId, Customer* customer, std::time_t createdDate);

    static std::string makeId(const std::string& prefix, int number);  // ("PT", 7) -> "PT-0007"

    void setStatus(TransactionStatus newStatus) { status_ = newStatus; }

    // Ném TransactionException nếu giao dịch không ở đúng trạng thái yêu cầu.
    // action: việc đang định làm, dùng để tạo thông báo lỗi dễ hiểu.
    void requireStatus(TransactionStatus expected, const std::string& action) const;

private:
    std::string transactionId_;
    Customer* customer_;
    std::time_t createdDate_;
    TransactionStatus status_;
};

#endif  // TRANSACTION_H
