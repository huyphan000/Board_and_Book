// =============================================================================
//  RentalTransaction.cpp
// =============================================================================
#include "RentalTransaction.h"

#include <iostream>
#include <sstream>

// ----------------------------- CHÍNH SÁCH (giả lập) ---------------------------
// Muốn đổi quy định thuê chỉ cần sửa các hằng số này.
namespace {
const double DAILY_RATE = 0.05;        // phí thuê 1 ngày = 5% giá bán của sản phẩm
const int MAX_RENTAL_DAYS = 30;        // tổng số ngày thuê tối đa (kể cả gia hạn)
const int MAX_EXTENSIONS = 2;          // số lần gia hạn tối đa
const double LATE_FEE_RATE = 1.5;      // phí trễ 1 ngày = 150% phí thuê 1 ngày
                                       // (tổng phí trễ của 1 sản phẩm không vượt quá giá bán)
const double DAMAGE_MINOR_RATE = 0.2;  // hư hại nhẹ: đền 20% giá bán
const double DAMAGE_MAJOR_RATE = 0.5;  // hư hại nặng: đền 50% giá bán
const double DAMAGE_LOST_RATE = 1.0;   // mất / hỏng hoàn toàn: đền 100% giá bán

int g_nextNumber = 1;  // số thứ tự của mã giao dịch RT-0001, RT-0002, ...

// Kiểm tra tham số TRƯỚC khi cấp số thứ tự, để tạo phiếu thất bại (khách null, số ngày
// thuê sai...) không làm mã giao dịch bị "mất số" (RT-0001 -> RT-0003).
int takeNextNumber(const Customer* customer, int rentalDays) {
    if (customer == nullptr) {
        throw TransactionException("Giao dịch phải gắn với một khách hàng hợp lệ");
    }
    if (rentalDays < 1 || rentalDays > MAX_RENTAL_DAYS) {
        std::ostringstream msg;
        msg << "Số ngày thuê phải từ 1 đến " << MAX_RENTAL_DAYS << " ngày";
        throw TransactionException(msg.str());
    }
    return g_nextNumber++;
}

double damageRate(DamageLevel level) {
    switch (level) {
        case DamageLevel::None:  return 0;
        case DamageLevel::Minor: return DAMAGE_MINOR_RATE;
        case DamageLevel::Major: return DAMAGE_MAJOR_RATE;
        case DamageLevel::Lost:  return DAMAGE_LOST_RATE;
    }
    return 0;
}

std::string damageText(DamageLevel level) {
    switch (level) {
        case DamageLevel::None:  return "Không";
        case DamageLevel::Minor: return "Nhẹ";
        case DamageLevel::Major: return "Nặng";
        case DamageLevel::Lost:  return "Mất";
    }
    return "Không";
}

double damageFeeOf(const RentalItem& item) {
    return Transaction::roundMoney(item.price * damageRate(item.damage));
}
}  // namespace

// ------------------------------------------------------------------ khởi tạo
RentalTransaction::RentalTransaction(Customer* customer, int rentalDays, std::time_t createdDate)
    : Transaction(makeId("RT", takeNextNumber(customer, rentalDays)), customer, createdDate),
      rentalDays_(rentalDays),
      extensionCount_(0),
      dueDate_(0),
      returnDate_(0),
      returned_(false) {
    // (customer và rentalDays đã được kiểm tra trong takeNextNumber)
    dueDate_ = addDays(createdDate, rentalDays);  // hạn trả = ngày thuê + số ngày thuê
}

// Đếm số dòng của cùng một sản phẩm trong phiếu (để so với tồn kho).
int RentalTransaction::countInSlip(Product* product) const {
    int count = 0;
    for (std::size_t i = 0; i < items_.size(); ++i) {
        if (items_[i].product->getId() == product->getId()) {
            ++count;
        }
    }
    return count;
}

// Hoàn kho các bản được trả về. Bản bị MẤT (Lost) thì không còn để hoàn.
void RentalTransaction::restockItems() {
    for (std::size_t i = 0; i < items_.size(); ++i) {
        if (items_[i].damage != DamageLevel::Lost) {
            items_[i].product->increaseStock(1);
        }
    }
}

// ================================================================== rentProduct()
std::size_t RentalTransaction::rentProduct(Product* product) {
    requireStatus(TransactionStatus::Pending, "thêm sản phẩm vào phiếu thuê");
    if (product == nullptr) {
        throw TransactionException("Sản phẩm không hợp lệ");
    }
    // Cần còn ít nhất (số bản đã có trong phiếu + 1) trong kho.
    if (product->getStock() < countInSlip(product) + 1) {
        throw TransactionException("Sản phẩm \"" + std::string(product->getName()) +
                                   "\" hiện không còn bản nào để cho thuê");
    }

    RentalItem item;
    item.product = product;
    item.price = product->getPrice();
    item.dailyFee = roundMoney(item.price * DAILY_RATE);
    item.damage = DamageLevel::None;
    items_.push_back(item);
    return items_.size() - 1;
}

// ================================================================ reportDamage()
void RentalTransaction::reportDamage(std::size_t itemIndex, DamageLevel level) {
    requireStatus(TransactionStatus::Processing, "ghi nhận hư hại");
    if (itemIndex >= items_.size()) {
        std::ostringstream msg;
        msg << "Phiếu thuê không có sản phẩm thứ " << (itemIndex + 1);
        throw TransactionException(msg.str());
    }
    items_[itemIndex].damage = level;
}

// ============================================================ executeTransaction()
// Với giao dịch thuê, "thực hiện" = giao hàng cho khách (trừ kho).
void RentalTransaction::executeTransaction() {
    requireStatus(TransactionStatus::Pending, "bắt đầu cho thuê");
    if (items_.empty()) {
        throw TransactionException("Phiếu thuê chưa có sản phẩm nào");
    }

    // Kiểm tra lại LẦN CUỐI rồi mới trừ kho -> không bao giờ trừ dở dang.
    for (std::size_t i = 0; i < items_.size(); ++i) {
        if (items_[i].product->getStock() < countInSlip(items_[i].product)) {
            throw TransactionException("Sản phẩm \"" + std::string(items_[i].product->getName()) +
                                       "\" không còn đủ bản để cho thuê");
        }
    }
    for (std::size_t i = 0; i < items_.size(); ++i) {
        items_[i].product->decreaseStock(1);
    }
    setStatus(TransactionStatus::Processing);
}

// ================================================================ extendRental()
double RentalTransaction::extendRental(int extraDays, std::time_t requestDate) {
    requireStatus(TransactionStatus::Processing, "gia hạn");
    if (extraDays <= 0) {
        throw TransactionException("Số ngày gia hạn phải lớn hơn 0");
    }
    if (extensionCount_ >= MAX_EXTENSIONS) {
        std::ostringstream msg;
        msg << "Phiếu đã gia hạn tối đa " << MAX_EXTENSIONS << " lần";
        throw TransactionException(msg.str());
    }
    if (daysBetween(dueDate_, requestDate) > 0) {
        throw TransactionException("Phiếu thuê đã quá hạn (hạn trả " + formatDate(dueDate_) +
                                   "), không thể gia hạn - vui lòng trả hàng");
    }
    if (extraDays > MAX_RENTAL_DAYS - rentalDays_) {  // viết dạng này để không tràn số
        std::ostringstream msg;
        msg << "Tổng thời gian thuê không được vượt quá " << MAX_RENTAL_DAYS << " ngày";
        throw TransactionException(msg.str());
    }

    double extraFee = 0;
    for (std::size_t i = 0; i < items_.size(); ++i) {
        extraFee += items_[i].dailyFee * extraDays;
    }
    rentalDays_ += extraDays;
    dueDate_ = addDays(dueDate_, extraDays);
    ++extensionCount_;
    return extraFee;
}

// ================================================================ returnProduct()
double RentalTransaction::returnProduct(std::time_t returnDate) {
    requireStatus(TransactionStatus::Processing, "nhận trả hàng");
    if (daysBetween(getRentDate(), returnDate) < 0) {
        throw TransactionException("Ngày trả (" + formatDate(returnDate) +
                                   ") không thể trước ngày thuê (" + formatDate(getRentDate()) +
                                   ")");
    }

    returnDate_ = returnDate;
    returned_ = true;
    restockItems();
    setStatus(TransactionStatus::Completed);
    return calculateLateFee() + calculateDamageFee();
}

// ============================================================ các hàm tính phí
int RentalTransaction::calculateLateDays(std::time_t atDate) const {
    if (getStatus() == TransactionStatus::Pending || getStatus() == TransactionStatus::Cancelled) {
        return 0;  // chưa giao hàng hoặc đã hủy thì không có trễ hạn
    }
    const std::time_t reference = returned_ ? returnDate_ : atDate;
    const int late = daysBetween(dueDate_, reference);
    return late > 0 ? late : 0;
}

double RentalTransaction::calculateLateFee(std::time_t atDate) const {
    const int lateDays = calculateLateDays(atDate);
    if (lateDays == 0) {
        return 0;
    }
    double total = 0;
    for (std::size_t i = 0; i < items_.size(); ++i) {
        const RentalItem& item = items_[i];
        if (item.damage == DamageLevel::Lost) {
            continue;  // bản đã mất bị đền 100% giá rồi, không tính thêm phí trễ
        }
        const double feePerDay = roundMoney(item.dailyFee * LATE_FEE_RATE);
        const double fee = feePerDay * lateDays;
        total += (fee > item.price ? item.price : fee);  // trần = giá bán của sản phẩm
    }
    return total;
}

double RentalTransaction::calculateDamageFee() const {
    double total = 0;
    for (std::size_t i = 0; i < items_.size(); ++i) {
        total += damageFeeOf(items_[i]);
    }
    return total;
}

double RentalTransaction::calculateRentalFee() const {
    double total = 0;
    for (std::size_t i = 0; i < items_.size(); ++i) {
        total += items_[i].dailyFee * rentalDays_;
    }
    return total;
}

double RentalTransaction::calculateTotal() const {
    if (getStatus() == TransactionStatus::Cancelled) {
        return 0;  // hủy phiếu -> miễn phí
    }
    return calculateRentalFee() + calculateLateFee() + calculateDamageFee();
}

bool RentalTransaction::isOverdue(std::time_t atDate) const {
    return getStatus() == TransactionStatus::Processing && daysBetween(dueDate_, atDate) > 0;
}

// ================================================================== cancelTransaction()
void RentalTransaction::cancelTransaction() {
    const bool wasRenting = (getStatus() == TransactionStatus::Processing);
    Transaction::cancelTransaction();  // kiểm tra trạng thái; ném lỗi nếu không hủy được
    if (wasRenting) {
        restockItems();  // hàng đã giao đi thì phải hoàn kho
    }
    // Đang Pending: chưa trừ kho nên không có gì để hoàn.
}

std::string RentalTransaction::getStatusText() const {
    switch (getStatus()) {
        case TransactionStatus::Pending:
            return "Đang lập phiếu thuê";
        case TransactionStatus::Processing: {
            const int late = calculateLateDays();
            if (late > 0) {
                std::ostringstream oss;
                oss << "Đang thuê - QUÁ HẠN " << late << " ngày";
                return oss.str();
            }
            return "Đang thuê";
        }
        case TransactionStatus::Completed:
            return "Đã trả";
        default:
            return Transaction::getStatusText();
    }
}

// ============================================================== in ra màn hình
void RentalTransaction::displayTransaction() const {
    std::cout << "================= PHIẾU THUÊ =================\n";
    Transaction::displayTransaction();
    std::cout << "----------------------------------------------\n"
              << "Ngày thuê : " << formatDate(getRentDate()) << "\n"
              << "Hạn trả   : " << formatDate(dueDate_) << " (" << rentalDays_
              << " ngày, đã gia hạn " << extensionCount_ << " lần)\n";
    if (returned_) {
        std::cout << "Ngày trả  : " << formatDate(returnDate_) << "\n";
    }
    std::cout << "Trễ hạn   : " << calculateLateDays() << " ngày\n"
              << "----------------------------------------------\n";

    if (items_.empty()) {
        std::cout << "(Phiếu thuê chưa có sản phẩm nào)\n"
                  << "==============================================\n";
        return;
    }
    for (std::size_t i = 0; i < items_.size(); ++i) {
        const RentalItem& item = items_[i];
        std::cout << (i + 1) << ". " << item.product->getName() << "\n"
                  << "   Phí thuê/ngày: " << formatMoney(item.dailyFee)
                  << " | Hư hại: " << damageText(item.damage) << " ("
                  << formatMoney(damageFeeOf(item)) << ")\n";
    }
    std::cout << "----------------------------------------------\n";

    if (getStatus() == TransactionStatus::Cancelled) {
        std::cout << "Phiếu thuê đã hủy - miễn phí.\n";
    } else {
        std::cout << "Phí thuê    : " << formatMoney(calculateRentalFee()) << " ("
                  << items_.size() << " sản phẩm x " << rentalDays_ << " ngày)\n"
                  << "Phí trễ hạn : " << formatMoney(calculateLateFee()) << " ("
                  << calculateLateDays() << " ngày"
                  << (getStatus() == TransactionStatus::Processing ? ", tạm tính đến hôm nay" : "")
                  << ")\n"
                  << "Phí hư hại  : " << formatMoney(calculateDamageFee()) << "\n"
                  << "TỔNG CỘNG   : " << formatMoney(calculateTotal()) << "\n";
    }
    std::cout << "==============================================\n";
}
