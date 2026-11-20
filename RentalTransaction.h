// =============================================================================
//  RentalTransaction.h - Giao dịch THUÊ sản phẩm (truyện / boardgame)
//
//  Trách nhiệm: quản lý thời gian thuê, hạn trả, việc trả hàng và phí phát sinh
//  (phí trễ hạn + phí hư hại).
//
//  Cách dùng:
//      RentalTransaction rt(customer, 7);        // thuê 7 ngày, tính từ hôm nay
//      rt.rentProduct(comic);                    // thêm từng sản phẩm vào phiếu
//      rt.rentProduct(game);
//      rt.executeTransaction();                  // giao hàng cho khách
//      rt.extendRental(3);                       // (tuỳ chọn) gia hạn thêm 3 ngày
//      rt.reportDamage(1, DamageLevel::Minor);   // (tuỳ chọn) ghi nhận hư hại khi trả
//      rt.returnProduct();                       // nhận trả hàng, chốt phí phát sinh
//
//  Mỗi dòng của phiếu là MỘT bản (muốn thuê 2 bản thì rentProduct() 2 lần)
//  để mỗi bản có tình trạng hư hại riêng.
//
//  Ngày là std::time_t. Có thể truyền ngày cụ thể để thử nghiệm, ví dụ:
//      rt.returnProduct(Transaction::addDays(rt.getDueDate(), 4));   // trả trễ 4 ngày
// =============================================================================
#ifndef RENTAL_TRANSACTION_H
#define RENTAL_TRANSACTION_H

#include <cstddef>
#include <string>
#include <vector>

#include "Product.h"
#include "Transaction.h"

// Mức hư hại khi khách trả hàng.
enum class DamageLevel {
    None,   // Nguyên vẹn
    Minor,  // Hư hại nhẹ (bẩn, nhàu bìa, thiếu vài quân bài...)
    Major,  // Hư hại nặng (rách trang, mất nhiều linh kiện...)
    Lost    // Mất / hỏng hoàn toàn
};

// Một dòng (một bản) trong phiếu thuê.
struct RentalItem {
    Product* product;
    double price;        // giá bán lúc thuê: dùng tính phí hư hại và trần phí trễ
    double dailyFee;     // phí thuê 1 ngày của bản này (chốt lúc thêm vào phiếu)
    DamageLevel damage;  // mức hư hại khi trả (mặc định None)
};

class RentalTransaction : public Transaction {
public:
    // rentalDays: số ngày thuê ban đầu, từ 1 đến 30. Ngày thuê = createdDate (mặc định hôm nay).
    explicit RentalTransaction(Customer* customer, int rentalDays = 7,
                               std::time_t createdDate = std::time(nullptr));

    // ----- Các phương thức theo bảng thiết kế -----

    // Thêm một bản sản phẩm vào phiếu thuê (có kiểm tra còn hàng).
    // Trả về chỉ số (từ 0) của dòng vừa thêm.
    std::size_t rentProduct(Product* product);

    // Gia hạn thêm `extraDays` ngày. Trả về phí thuê phát sinh thêm.
    // Quy định: tối đa 2 lần, tổng không quá 30 ngày, không gia hạn khi đã quá hạn.
    double extendRental(int extraDays, std::time_t requestDate = std::time(nullptr));

    // Nhận trả hàng vào ngày `returnDate`: chốt phí trễ + phí hư hại, hoàn kho.
    // Trả về "phí phát sinh" khách phải trả thêm (= phí trễ + phí hư hại).
    double returnProduct(std::time_t returnDate = std::time(nullptr));

    // Số ngày trễ hạn = max(0, ngày trả - hạn trả).
    // Nếu CHƯA trả hàng thì tính đến `atDate` (mặc định hôm nay), coi như tạm tính.
    int calculateLateDays(std::time_t atDate = std::time(nullptr)) const;
    double calculateLateFee(std::time_t atDate = std::time(nullptr)) const;  // phí trễ hạn
    double calculateDamageFee() const;                                       // phí hư hại

    // ----- Hàm hỗ trợ -----
    // Ghi nhận mức hư hại của dòng thứ `itemIndex` (từ 0) trước khi trả hàng.
    void reportDamage(std::size_t itemIndex, DamageLevel level);
    double calculateRentalFee() const;  // phí thuê cơ bản (đã gồm các ngày gia hạn)
    bool isOverdue(std::time_t atDate = std::time(nullptr)) const;

    const std::vector<RentalItem>& getItems() const { return items_; }
    std::time_t getRentDate() const { return getCreatedDate(); }
    std::time_t getDueDate() const { return dueDate_; }
    std::time_t getReturnDate() const { return returnDate_; }
    int getRentalDays() const { return rentalDays_; }  // tổng số ngày thuê (đã cộng gia hạn)
    int getExtensionCount() const { return extensionCount_; }
    bool isReturned() const { return returned_; }

    // ----- Ghi đè từ lớp cha -----
    void executeTransaction() override;  // giao hàng cho khách (trừ kho)
    double calculateTotal() const override;
    void cancelTransaction() override;   // đang cho thuê mà hủy -> hoàn kho, miễn phí
    void displayTransaction() const override;
    std::string getStatusText() const override;

private:
    int countInSlip(Product* product) const;
    void restockItems();

    std::vector<RentalItem> items_;
    int rentalDays_;
    int extensionCount_;
    std::time_t dueDate_;
    std::time_t returnDate_;
    bool returned_;
};

#endif  // RENTAL_TRANSACTION_H
