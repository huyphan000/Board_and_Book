// =============================================================================
//  PurchaseTransaction.cpp
// =============================================================================
#include "PurchaseTransaction.h"

#include <iostream>
#include <limits>
#include <sstream>

// ----------------------------- CHÍNH SÁCH (giả lập) ---------------------------
// Muốn đổi quy định giảm giá chỉ cần sửa các hằng số này.
namespace {
const double MAX_DISCOUNT_PERCENT = 30;  // giảm giá tối đa cho một đơn (%)
const int BULK_QUANTITY_1 = 5;           // mua từ 5 sản phẩm ...
const double BULK_PERCENT_1 = 5;         // ... gợi ý giảm 5%
const int BULK_QUANTITY_2 = 10;          // mua từ 10 sản phẩm ...
const double BULK_PERCENT_2 = 10;        // ... gợi ý giảm 10%

int g_nextNumber = 1;  // số thứ tự của mã giao dịch PT-0001, PT-0002, ...

// Cấp số thứ tự kế tiếp. Kiểm tra tham số TRƯỚC khi tăng bộ đếm, để tạo giao dịch
// thất bại (vd: customer == nullptr) không làm mã giao dịch bị "mất số".
int takeNextNumber(const Customer* customer) {
    if (customer == nullptr) {
        throw TransactionException("Giao dịch phải gắn với một khách hàng hợp lệ");
    }
    return g_nextNumber++;
}
}  // namespace

PurchaseTransaction::PurchaseTransaction(Customer* customer, std::time_t createdDate)
    : Transaction(makeId("PT", takeNextNumber(customer)), customer, createdDate),
      discountPercent_(0) {}

PurchaseItem* PurchaseTransaction::findItem(Product* product) {
    for (std::size_t i = 0; i < items_.size(); ++i) {
        if (items_[i].product->getId() == product->getId()) {
            return &items_[i];
        }
    }
    return nullptr;
}

// ============================================================ purchaseProduct()
void PurchaseTransaction::purchaseProduct(Product* product, int quantity) {
    requireStatus(TransactionStatus::Pending, "thêm sản phẩm vào đơn mua");
    if (product == nullptr) {
        throw TransactionException("Sản phẩm không hợp lệ");
    }
    if (quantity <= 0) {
        throw TransactionException("Số lượng mua phải lớn hơn 0");
    }

    // Nếu sản phẩm đã có trong đơn, số lượng cần kiểm tra = số đã có + số mới thêm.
    PurchaseItem* existing = findItem(product);
    const int already = (existing != nullptr ? existing->quantity : 0);
    if (quantity > std::numeric_limits<int>::max() - already) {
        throw TransactionException("Số lượng mua quá lớn");  // tránh tràn số khi cộng dồn
    }
    const int wanted = quantity + already;

    if (!checkStock(product, wanted)) {
        std::ostringstream msg;
        msg << "Không đủ hàng \"" << product->getName() << "\": cần " << wanted
            << ", trong kho chỉ còn " << product->getStock();
        throw TransactionException(msg.str());
    }

    if (existing != nullptr) {
        existing->quantity = wanted;  // cùng sản phẩm -> cộng dồn vào dòng cũ
    } else {
        PurchaseItem item;
        item.product = product;
        item.quantity = quantity;
        item.unitPrice = product->getPrice();
        items_.push_back(item);
    }
}

// ================================================================ checkStock()
bool PurchaseTransaction::checkStock(Product* product, int quantity) const {
    return product != nullptr && quantity > 0 && product->getStock() >= quantity;
}

bool PurchaseTransaction::checkStock() const {
    for (std::size_t i = 0; i < items_.size(); ++i) {
        if (!checkStock(items_[i].product, items_[i].quantity)) {
            return false;
        }
    }
    return true;
}

// ============================================================== applyDiscount()
void PurchaseTransaction::applyDiscount(double percent) {
    requireStatus(TransactionStatus::Pending, "áp dụng giảm giá");
    if (!(percent >= 0 && percent <= MAX_DISCOUNT_PERCENT)) {  // viết dạng này để loại cả NaN
        std::ostringstream msg;
        msg << "Mức giảm giá phải từ 0% đến " << MAX_DISCOUNT_PERCENT << "%";
        throw TransactionException(msg.str());
    }
    discountPercent_ = percent;
}

double PurchaseTransaction::suggestDiscountPercent() const {
    const int total = getTotalQuantity();
    if (total >= BULK_QUANTITY_2) return BULK_PERCENT_2;
    if (total >= BULK_QUANTITY_1) return BULK_PERCENT_1;
    return 0;
}

// ===================================================== calculatePurchaseTotal()
double PurchaseTransaction::calculateSubtotal() const {
    double subtotal = 0;
    for (std::size_t i = 0; i < items_.size(); ++i) {
        subtotal += items_[i].lineTotal();
    }
    return subtotal;
}

double PurchaseTransaction::calculateDiscountAmount() const {
    return roundMoney(calculateSubtotal() * discountPercent_ / 100.0);
}

double PurchaseTransaction::calculatePurchaseTotal() const {
    return calculateSubtotal() - calculateDiscountAmount();
}

int PurchaseTransaction::getTotalQuantity() const {
    int total = 0;
    for (std::size_t i = 0; i < items_.size(); ++i) {
        total += items_[i].quantity;
    }
    return total;
}

// ============================================================ executeTransaction()
void PurchaseTransaction::executeTransaction() {
    requireStatus(TransactionStatus::Pending, "thanh toán");
    if (items_.empty()) {
        throw TransactionException("Đơn mua chưa có sản phẩm nào");
    }

    // Kiểm tra lại LẦN CUỐI vì tồn kho có thể đã đổi từ lúc thêm vào đơn.
    // Kiểm tra hết rồi mới trừ kho -> không bao giờ trừ dở dang.
    for (std::size_t i = 0; i < items_.size(); ++i) {
        const PurchaseItem& item = items_[i];
        if (!checkStock(item.product, item.quantity)) {
            std::ostringstream msg;
            msg << "Không đủ hàng \"" << item.product->getName() << "\" để thanh toán: cần "
                << item.quantity << ", trong kho chỉ còn " << item.product->getStock();
            throw TransactionException(msg.str());
        }
    }
    for (std::size_t i = 0; i < items_.size(); ++i) {
        items_[i].product->decreaseStock(items_[i].quantity);
    }
    setStatus(TransactionStatus::Completed);
}

double PurchaseTransaction::calculateTotal() const {
    if (getStatus() == TransactionStatus::Cancelled) {
        return 0;  // giao dịch đã hủy không phát sinh tiền
    }
    return calculatePurchaseTotal();
}

std::string PurchaseTransaction::getStatusText() const {
    switch (getStatus()) {
        case TransactionStatus::Pending:   return "Chưa thanh toán";
        case TransactionStatus::Completed: return "Đã thanh toán";
        default:                           return Transaction::getStatusText();
    }
}

// ============================================================== in ra màn hình
std::string PurchaseTransaction::itemLines() const {
    std::ostringstream out;
    for (std::size_t i = 0; i < items_.size(); ++i) {
        const PurchaseItem& item = items_[i];
        out << (i + 1) << ". " << item.product->getName() << "\n"
            << "   " << item.quantity << " x " << formatMoney(item.unitPrice) << " = "
            << formatMoney(item.lineTotal()) << "\n";
    }
    return out.str();
}

std::string PurchaseTransaction::totalLines() const {
    const double discount = calculateDiscountAmount();
    std::ostringstream out;
    out << "Tạm tính        : " << formatMoney(calculateSubtotal()) << "\n"
        << "Giảm giá        : " << (discount > 0 ? "-" : "") << formatMoney(discount) << " ("
        << discountPercent_ << "%)\n"
        << "TỔNG THANH TOÁN : " << formatMoney(calculatePurchaseTotal()) << "\n";
    return out.str();
}

void PurchaseTransaction::displayTransaction() const {
    std::cout << "============== GIAO DỊCH MUA HÀNG ==============\n";
    Transaction::displayTransaction();
    std::cout << "------------------------------------------------\n";
    if (items_.empty()) {
        std::cout << "(Đơn chưa có sản phẩm nào)\n";
    } else if (getStatus() == TransactionStatus::Cancelled) {
        std::cout << itemLines() << "Giao dịch đã hủy - không phát sinh thanh toán.\n";
    } else {
        std::cout << itemLines() << "------------------------------------------------\n"
                  << totalLines();
    }
    std::cout << "================================================\n";
}

// ================================================================ generateInvoice()
std::string PurchaseTransaction::generateInvoice() const {
    if (getStatus() == TransactionStatus::Cancelled) {
        throw TransactionException("Giao dịch " + getTransactionId() +
                                   " đã hủy, không thể xuất hóa đơn");
    }
    if (items_.empty()) {
        throw TransactionException("Đơn mua chưa có sản phẩm nào để xuất hóa đơn");
    }

    const bool paid = (getStatus() == TransactionStatus::Completed);
    std::ostringstream out;
    out << "================================================\n"
        << "  " << (paid ? "HÓA ĐƠN BÁN HÀNG" : "PHIẾU TẠM TÍNH (CHƯA THANH TOÁN)") << "\n"
        << "================================================\n"
        << "Mã giao dịch : " << getTransactionId() << "\n"
        << "Khách hàng   : " << getCustomer()->getName() << " (" << getCustomer()->getId() << ")\n"
        << "Ngày lập     : " << formatDate(getCreatedDate()) << "\n"
        << "------------------------------------------------\n"
        << itemLines() << "------------------------------------------------\n"
        << totalLines() << "================================================\n"
        << "  Cảm ơn quý khách - Hẹn gặp lại!\n";
    return out.str();
}
