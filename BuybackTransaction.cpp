// =============================================================================
//  BuybackTransaction.cpp
// =============================================================================
#include "BuybackTransaction.h"

#include <iostream>
#include <sstream>

// ----------------------------- CHÍNH SÁCH (giả lập) ---------------------------
// Tỷ lệ giá trị còn lại theo tình trạng sách. Đây là chính sách giả lập của dự án,
// không phải mức giá thị trường cố định - nhóm có thể tự đổi các con số này.
// Lưu bằng PHẦN TRĂM (số nguyên) rồi mới chia 100: số thực 0.7 không biểu diễn chính xác,
// nên "giá x 0.7" có thể ra x.4999... và làm tròn sai 1 đồng (vd: 45 x 0.7 ra 31 thay vì 32).
namespace {
const int PERCENT_EXCELLENT = 90;
const int PERCENT_GOOD = 70;
const int PERCENT_FAIR = 50;
const int PERCENT_POOR = 20;

int g_nextNumber = 1;  // số thứ tự của mã giao dịch BB-0001, BB-0002, ...

// Cấp số thứ tự kế tiếp. Kiểm tra tham số TRƯỚC khi tăng bộ đếm, để tạo giao dịch
// thất bại (vd: customer == nullptr) không làm mã giao dịch bị "mất số".
int takeNextNumber(const Customer* customer) {
    if (customer == nullptr) {
        throw TransactionException("Giao dịch phải gắn với một khách hàng hợp lệ");
    }
    return g_nextNumber++;
}

int remainingPercent(BookCondition condition) {
    switch (condition) {
        case BookCondition::Excellent: return PERCENT_EXCELLENT;
        case BookCondition::Good:      return PERCENT_GOOD;
        case BookCondition::Fair:      return PERCENT_FAIR;
        case BookCondition::Poor:      return PERCENT_POOR;
    }
    return 0;
}

std::string conditionText(BookCondition condition) {
    switch (condition) {
        case BookCondition::Excellent: return "Excellent";
        case BookCondition::Good:      return "Good";
        case BookCondition::Fair:      return "Fair";
        case BookCondition::Poor:      return "Poor";
    }
    return "Unknown";
}
}  // namespace

BuybackTransaction::BuybackTransaction(Customer* customer, std::time_t createdDate)
    : Transaction(makeId("BB", takeNextNumber(customer)), customer, createdDate) {}

// ================================================================ công thức giá
double BuybackTransaction::getRemainingRatio(BookCondition condition) {
    return remainingPercent(condition) / 100.0;  // 0.9 / 0.7 / 0.5 / 0.2
}

// Giá mua lại = Giá gốc x Tỷ lệ giá trị còn lại (làm tròn đến 1 đồng)
// Nhân với phần trăm (số nguyên) trước rồi mới chia 100 để không bị sai số số thực.
double BuybackTransaction::calculateItemPrice(double originalPrice, BookCondition condition) {
    return roundMoney(originalPrice * remainingPercent(condition) / 100.0);
}

// ============================================================ inspectCondition()
std::size_t BuybackTransaction::inspectCondition(Product* product, BookCondition condition,
                                                 const std::string& note) {
    requireStatus(TransactionStatus::Pending, "tiếp nhận thêm truyện");
    if (product == nullptr) {
        throw TransactionException("Sản phẩm không hợp lệ");
    }

    // Đề bài: chỉ thu mua TRUYỆN cũ. Nếu nhóm có class Comic (kế thừa Product),
    // bỏ comment 3 dòng dưới (và #include file của Comic) để chặn boardgame:
    //
    // if (dynamic_cast<Comic*>(product) == nullptr) {
    //     throw TransactionException("Cửa hàng chỉ thu mua lại truyện tranh");
    // }

    BuybackItem item;
    item.product = product;
    item.condition = condition;
    item.note = note;
    item.originalPrice = product->getPrice();
    items_.push_back(item);
    return items_.size() - 1;
}

// ================================================================ tính tiền
double BuybackTransaction::calculateOriginalValue() const {
    double total = 0;
    for (std::size_t i = 0; i < items_.size(); ++i) {
        total += items_[i].originalPrice;
    }
    return total;
}

double BuybackTransaction::calculateBuybackPrice() const {
    double total = 0;
    for (std::size_t i = 0; i < items_.size(); ++i) {
        total += calculateItemPrice(items_[i].originalPrice, items_[i].condition);
    }
    return total;
}

double BuybackTransaction::calculateDepreciation() const {
    return calculateOriginalValue() - calculateBuybackPrice();
}

// ================================================================ evaluateBook()
double BuybackTransaction::evaluateBook() {
    requireStatus(TransactionStatus::Pending, "định giá");
    if (items_.empty()) {
        throw TransactionException("Chưa tiếp nhận cuốn truyện nào để định giá");
    }
    setStatus(TransactionStatus::Processing);  // chốt báo giá, chờ khách đồng ý
    return calculateBuybackPrice();
}

// =============================================================== approveBuyback()
void BuybackTransaction::approveBuyback() {
    requireStatus(TransactionStatus::Processing, "duyệt mua lại (cần định giá trước)");
    for (std::size_t i = 0; i < items_.size(); ++i) {
        items_[i].product->increaseStock(1);  // nhập cuốn truyện cũ vào kho
    }
    setStatus(TransactionStatus::Completed);
}

// ============================================================ executeTransaction()
void BuybackTransaction::executeTransaction() {
    if (getStatus() == TransactionStatus::Pending) {
        evaluateBook();  // chưa định giá thì định giá trước
    }
    approveBuyback();    // sai trạng thái (đã hoàn tất / đã hủy) sẽ bị ném lỗi ở đây
}

double BuybackTransaction::calculateTotal() const {
    if (getStatus() == TransactionStatus::Cancelled) {
        return 0;  // khách không bán nữa -> cửa hàng không chi tiền
    }
    return calculateBuybackPrice();
}

std::string BuybackTransaction::getStatusText() const {
    switch (getStatus()) {
        case TransactionStatus::Pending:    return "Đang tiếp nhận & kiểm tra";
        case TransactionStatus::Processing: return "Đã định giá - chờ khách đồng ý";
        case TransactionStatus::Completed:  return "Đã mua lại";
        default:                            return Transaction::getStatusText();
    }
}

// ============================================================== in ra màn hình
void BuybackTransaction::displayTransaction() const {
    std::cout << "============ PHIẾU MUA LẠI TRUYỆN CŨ ============\n";
    Transaction::displayTransaction();
    std::cout << "-------------------------------------------------\n"
              << "Công thức: Giá mua lại = Giá gốc × Tỷ lệ giá trị còn lại\n"
              << "-------------------------------------------------\n";

    if (items_.empty()) {
        std::cout << "(Chưa tiếp nhận cuốn truyện nào)\n"
                  << "=================================================\n";
        return;
    }
    for (std::size_t i = 0; i < items_.size(); ++i) {
        const BuybackItem& item = items_[i];
        std::cout << (i + 1) << ". " << item.product->getName() << "\n"
                  << "   Tình trạng: " << conditionText(item.condition) << " (còn "
                  << remainingPercent(item.condition) << "% giá trị)\n"
                  << "   Giá gốc " << formatMoney(item.originalPrice) << " -> giá mua lại "
                  << formatMoney(calculateItemPrice(item.originalPrice, item.condition)) << "\n";
        if (!item.note.empty()) {
            std::cout << "   Ghi chú: " << item.note << "\n";
        }
    }
    std::cout << "-------------------------------------------------\n";

    if (getStatus() == TransactionStatus::Cancelled) {
        std::cout << "Giao dịch đã hủy - cửa hàng không chi trả.\n";
    } else {
        std::cout << "Tổng giá gốc       : " << formatMoney(calculateOriginalValue()) << "\n"
                  << "Khấu hao           : -" << formatMoney(calculateDepreciation()) << "\n"
                  << "TIỀN CHI TRẢ KHÁCH : " << formatMoney(calculateTotal())
                  << (getStatus() == TransactionStatus::Pending ? " (tạm tính, chưa chốt giá)" : "")
                  << "\n";
    }
    std::cout << "=================================================\n";
}
