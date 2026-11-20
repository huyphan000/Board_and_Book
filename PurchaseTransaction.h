// =============================================================================
//  PurchaseTransaction.h - Giao dịch MUA sản phẩm (truyện / boardgame mới)
//
//  Trách nhiệm: quản lý sản phẩm mua, số lượng, giá và giảm giá.
//
//  Cách dùng:
//      PurchaseTransaction pt(customer);
//      pt.purchaseProduct(comic, 3);                    // thêm vào đơn (có kiểm tra kho)
//      pt.purchaseProduct(game, 1);
//      pt.applyDiscount(pt.suggestDiscountPercent());   // giảm giá
//      pt.executeTransaction();                         // thanh toán, trừ kho
//      std::cout << pt.generateInvoice();               // in hóa đơn
// =============================================================================
#ifndef PURCHASE_TRANSACTION_H
#define PURCHASE_TRANSACTION_H

#include <string>
#include <vector>

#include "Product.h"
#include "Transaction.h"

// Một dòng trong đơn mua.
struct PurchaseItem {
    Product* product;
    int quantity;
    double unitPrice;  // giá chốt tại thời điểm thêm vào đơn

    double lineTotal() const { return unitPrice * quantity; }
};

class PurchaseTransaction : public Transaction {
public:
    explicit PurchaseTransaction(Customer* customer, std::time_t createdDate = std::time(nullptr));

    // ----- Các phương thức theo bảng thiết kế -----

    // Thêm sản phẩm vào đơn. Sản phẩm đã có trong đơn thì cộng dồn số lượng.
    // Ném TransactionException nếu: sai trạng thái, sản phẩm null, số lượng <= 0, không đủ hàng.
    void purchaseProduct(Product* product, int quantity);

    // Kiểm tra tồn kho có đủ `quantity` của `product` không.
    bool checkStock(Product* product, int quantity) const;
    // Kiểm tra tồn kho có đủ cho TOÀN BỘ đơn không.
    bool checkStock() const;

    // Giảm giá `percent`% cho cả đơn (0 đến 30). Gọi lại sẽ thay mức cũ, không cộng dồn.
    void applyDiscount(double percent);

    // Tổng phải trả = Tạm tính - Giảm giá.
    double calculatePurchaseTotal() const;

    // Hóa đơn dạng chuỗi (để in ra màn hình hoặc ghi file).
    // Chưa thanh toán -> "PHIẾU TẠM TÍNH"; đã thanh toán -> "HÓA ĐƠN BÁN HÀNG".
    std::string generateInvoice() const;

    // ----- Hàm hỗ trợ -----
    double suggestDiscountPercent() const;   // mức giảm gợi ý theo số lượng: >=5 món 5%, >=10 món 10%
    double calculateSubtotal() const;        // tạm tính (chưa giảm)
    double calculateDiscountAmount() const;  // số tiền được giảm
    int getTotalQuantity() const;
    const std::vector<PurchaseItem>& getItems() const { return items_; }
    double getDiscountPercent() const { return discountPercent_; }

    // ----- Ghi đè từ lớp cha -----
    void executeTransaction() override;  // thanh toán + trừ kho
    double calculateTotal() const override;
    void displayTransaction() const override;
    std::string getStatusText() const override;

private:
    PurchaseItem* findItem(Product* product);
    std::string itemLines() const;
    std::string totalLines() const;

    std::vector<PurchaseItem> items_;
    double discountPercent_;
};

#endif  // PURCHASE_TRANSACTION_H
