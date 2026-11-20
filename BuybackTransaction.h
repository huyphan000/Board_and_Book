// =============================================================================
//  BuybackTransaction.h - Giao dịch MUA LẠI truyện cũ từ khách
//
//  Trách nhiệm: tiếp nhận và định giá truyện cũ khách bán lại cho cửa hàng.
//
//  Công thức (theo đề bài):
//      Giá mua lại = Giá gốc x Tỷ lệ giá trị còn lại
//  Tỷ lệ còn lại theo tình trạng (chính sách giả lập của dự án):
//      Excellent 90% | Good 70% | Fair 50% | Poor 20%
//
//  Cách dùng:
//      BuybackTransaction bb(customer);
//      bb.inspectCondition(comic1, BookCondition::Excellent);   // nhận + kiểm tra từng cuốn
//      bb.inspectCondition(comic2, BookCondition::Fair, "Ố vàng");
//      bb.evaluateBook();                                       // định giá, báo giá cho khách
//      bb.approveBuyback();                                     // khách đồng ý: chi tiền, nhập kho
//  (Khách không đồng ý giá -> bb.cancelTransaction().)
//
//  Mỗi dòng là MỘT cuốn vật lý, vì hai cuốn cùng tên vẫn có thể khác tình trạng.
// =============================================================================
#ifndef BUYBACK_TRANSACTION_H
#define BUYBACK_TRANSACTION_H

#include <cstddef>
#include <string>
#include <vector>

#include "Product.h"
#include "Transaction.h"

// Tình trạng cuốn truyện cũ khi khách bán lại.
enum class BookCondition {
    Excellent,  // Như mới
    Good,       // Tốt
    Fair,       // Trung bình
    Poor        // Kém (cũ, ố vàng, rách nhẹ...)
};

// Một cuốn truyện cũ khách mang đến bán.
struct BuybackItem {
    Product* product;
    BookCondition condition;
    std::string note;      // ghi chú của nhân viên khi kiểm tra
    double originalPrice;  // "Giá gốc" của cuốn này (chốt lúc tiếp nhận)
};

class BuybackTransaction : public Transaction {
public:
    explicit BuybackTransaction(Customer* customer, std::time_t createdDate = std::time(nullptr));

    // ----- Các phương thức theo bảng thiết kế -----

    // Tiếp nhận MỘT cuốn khách mang đến và ghi nhận kết quả kiểm tra tình trạng.
    // Trả về chỉ số (từ 0) của dòng vừa thêm.
    std::size_t inspectCondition(Product* product, BookCondition condition,
                                 const std::string& note = "");

    // Định giá toàn bộ các cuốn đã kiểm tra và chốt báo giá (Pending -> Processing).
    // Sau bước này không nhận thêm cuốn. Trả về tổng tiền cửa hàng đề nghị trả khách.
    double evaluateBook();

    // Tổng số tiền "mất giá" do cũ = Tổng giá gốc - Tổng giá mua lại.
    double calculateDepreciation() const;

    // Tổng giá mua lại = tổng của (Giá gốc x Tỷ lệ còn lại) của từng cuốn.
    double calculateBuybackPrice() const;

    // Khách đồng ý giá: chi tiền, nhập các cuốn vào kho (Processing -> Completed).
    // Bắt buộc đã gọi evaluateBook() trước.
    void approveBuyback();

    // ----- Công thức cho MỘT cuốn (public static để nơi khác tra cứu được) -----
    static double getRemainingRatio(BookCondition condition);  // 0.9 / 0.7 / 0.5 / 0.2
    static double calculateItemPrice(double originalPrice, BookCondition condition);

    // ----- Hàm hỗ trợ -----
    double calculateOriginalValue() const;  // tổng giá gốc của các cuốn
    const std::vector<BuybackItem>& getItems() const { return items_; }

    // ----- Ghi đè từ lớp cha -----
    // "Thực hiện" giao dịch mua lại = (định giá nếu chưa) rồi duyệt luôn.
    void executeTransaction() override;
    double calculateTotal() const override;  // số tiền cửa hàng CHI trả cho khách
    void displayTransaction() const override;
    std::string getStatusText() const override;

private:
    std::vector<BuybackItem> items_;
};

#endif  // BUYBACK_TRANSACTION_H
