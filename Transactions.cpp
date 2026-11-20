#ifndef TRANSACTIONS_CPP
#define TRANSACTIONS_CPP

// =============================================================================
//  Transactions.cpp - TOÀN BỘ MODULE GIAO DỊCH TRONG MỘT FILE (đã gộp .h vào .cpp)
//
//  Gộp từ 8 file: Transaction, PurchaseTransaction, BuybackTransaction,
//  RentalTransaction (mỗi lớp một cặp .h + .cpp). Các phần xếp theo thứ tự
//  phụ thuộc (lớp cha phải đứng trước lớp con):
//
//      PHẦN 1  Transaction          (lớp cha trừu tượng)
//      PHẦN 2  PurchaseTransaction  (mua hàng)
//      PHẦN 3  BuybackTransaction   (mua lại truyện cũ)
//      PHẦN 4  RentalTransaction    (thuê)
//
//  Trong mỗi phần: khai báo lớp (trước đây là file .h) đứng trước, phần cài đặt
//  (trước đây là file .cpp) đứng sau.
//
//  Phụ thuộc ngoài file này: "Customer.h" và "Product.h" của nhóm.
//
//  CÁCH DÙNG: file này chứa cả khai báo lẫn cài đặt nên không còn .h nào để #include.
//    - Cách 1: đặt hàm main() ngay trong file này rồi biên dịch riêng file này.
//    - Cách 2: ở file khác (vd main.cpp) viết  #include "Transactions.cpp"  và KHÔNG
//      thêm Transactions.cpp vào danh sách biên dịch (nếu không sẽ trùng định nghĩa khi link).
// =============================================================================

#include <cmath>
#include <cstddef>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "Customer.h"
#include "Product.h"

// #############################################################################
//  PHẦN 1/4 : Transaction
// #############################################################################

// -----------------------------------------------------------------------------
//  1a. KHAI BÁO  (trước đây là Transaction.h)
// -----------------------------------------------------------------------------

// =============================================================================
//  Transaction - Lớp cha TRỪU TƯỢNG của hệ thống giao dịch (mục 3.3)
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

// -----------------------------------------------------------------------------
//  1b. CÀI ĐẶT  (trước đây là Transaction.cpp)
// -----------------------------------------------------------------------------

namespace {

// Đổi time_t sang tm theo giờ địa phương (MSVC bắt buộc dùng localtime_s).
std::tm toLocalTm(std::time_t t) {
    std::tm result = std::tm();
#if defined(_MSC_VER)
    localtime_s(&result, &t);
#else
    const std::tm* p = std::localtime(&t);
    if (p != nullptr) {
        result = *p;
    }
#endif
    return result;
}

}  // namespace

// ------------------------------------------------------------- khởi tạo
Transaction::Transaction(const std::string& transactionId, Customer* customer,
                         std::time_t createdDate)
    : transactionId_(transactionId),
      customer_(customer),
      createdDate_(createdDate),
      status_(TransactionStatus::Pending) {
    if (customer_ == nullptr) {
        throw TransactionException("Giao dịch phải gắn với một khách hàng hợp lệ");
    }
}

std::string Transaction::makeId(const std::string& prefix, int number) {
    std::ostringstream oss;
    oss << prefix << '-' << std::setw(4) << std::setfill('0') << number;
    return oss.str();
}

// ------------------------------------------------------------- trạng thái
void Transaction::requireStatus(TransactionStatus expected, const std::string& action) const {
    if (status_ != expected) {
        throw TransactionException("Không thể " + action + ": giao dịch " + transactionId_ +
                                   " đang ở trạng thái \"" + getStatusText() + "\"");
    }
}

std::string Transaction::getStatusText() const {
    switch (status_) {
        case TransactionStatus::Pending:    return "Chờ xử lý";
        case TransactionStatus::Processing: return "Đang xử lý";
        case TransactionStatus::Completed:  return "Hoàn tất";
        case TransactionStatus::Cancelled:  return "Đã hủy";
    }
    return "Không xác định";
}

// ------------------------------------------------------------- cancelTransaction()
void Transaction::cancelTransaction() {
    if (status_ == TransactionStatus::Completed) {
        throw TransactionException("Không thể hủy giao dịch " + transactionId_ +
                                   ": giao dịch đã hoàn tất");
    }
    if (status_ == TransactionStatus::Cancelled) {
        throw TransactionException("Giao dịch " + transactionId_ + " đã được hủy trước đó");
    }
    status_ = TransactionStatus::Cancelled;
}

// ------------------------------------------------------------- displayTransaction()
void Transaction::displayTransaction() const {
    std::cout << "Mã giao dịch : " << transactionId_ << "\n"
              << "Khách hàng   : " << customer_->getId() << " - " << customer_->getName() << "\n"
              << "Ngày tạo     : " << formatDate(createdDate_) << "\n"
              << "Trạng thái   : " << getStatusText() << "\n";
}

// ================================================================ tiện ích ngày
std::time_t Transaction::makeDate(int day, int month, int year) {
    std::tm t = std::tm();
    t.tm_year = year - 1900;
    t.tm_mon = month - 1;
    t.tm_mday = day;
    t.tm_hour = 12;  // lấy 12 giờ trưa để không bị lệch ngày do múi giờ / giờ mùa hè
    t.tm_isdst = -1;
    const std::time_t result = std::mktime(&t);  // mktime tự "sửa" ngày sai (vd 31/02 -> 03/03)

    // Nếu mktime phải sửa nghĩa là ngày nhập vào không tồn tại.
    if (result == static_cast<std::time_t>(-1) || t.tm_mday != day || t.tm_mon != month - 1 ||
        t.tm_year != year - 1900) {
        throw TransactionException("Ngày không hợp lệ");
    }
    return result;
}

std::time_t Transaction::addDays(std::time_t date, int days) {
    std::tm t = toLocalTm(date);
    t.tm_mday += days;  // mktime tự chuẩn hóa sang tháng/năm kế tiếp
    t.tm_isdst = -1;
    return std::mktime(&t);
}

int Transaction::daysBetween(std::time_t from, std::time_t to) {
    // Đưa cả hai về 12 giờ trưa của ngày tương ứng rồi mới trừ, nên chỉ so sánh NGÀY.
    std::tm a = toLocalTm(from);
    std::tm b = toLocalTm(to);
    a.tm_hour = 12; a.tm_min = 0; a.tm_sec = 0; a.tm_isdst = -1;
    b.tm_hour = 12; b.tm_min = 0; b.tm_sec = 0; b.tm_isdst = -1;
    const double seconds = std::difftime(std::mktime(&b), std::mktime(&a));
    return static_cast<int>(std::floor(seconds / 86400.0 + 0.5));  // làm tròn để tránh lệch do DST
}

std::string Transaction::formatDate(std::time_t date) {
    const std::tm t = toLocalTm(date);
    char buffer[16];
    std::strftime(buffer, sizeof(buffer), "%d/%m/%Y", &t);
    return buffer;
}

// ================================================================ tiện ích tiền
double Transaction::roundMoney(double amount) {
    return std::floor(amount + 0.5);
}

std::string Transaction::formatMoney(double amount) {
    long long value = static_cast<long long>(roundMoney(amount));
    const bool negative = value < 0;
    if (negative) {
        value = -value;
    }

    std::ostringstream oss;
    oss << value;
    const std::string digits = oss.str();

    std::string result;
    int count = 0;
    for (int i = static_cast<int>(digits.size()) - 1; i >= 0; --i) {
        result.insert(result.begin(), digits[static_cast<std::size_t>(i)]);
        if (++count % 3 == 0 && i > 0) {
            result.insert(result.begin(), '.');  // dấu chấm ngăn cách hàng nghìn
        }
    }
    return (negative ? "-" : "") + result + " VND";
}

// #############################################################################
//  PHẦN 2/4 : PurchaseTransaction
// #############################################################################

// -----------------------------------------------------------------------------
//  2a. KHAI BÁO  (trước đây là PurchaseTransaction.h)
// -----------------------------------------------------------------------------

// =============================================================================
//  PurchaseTransaction - Giao dịch MUA sản phẩm (truyện / boardgame mới)
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

// -----------------------------------------------------------------------------
//  2b. CÀI ĐẶT  (trước đây là PurchaseTransaction.cpp)
// -----------------------------------------------------------------------------

// ----------------------------- CHÍNH SÁCH (giả lập) ---------------------------
// Muốn đổi quy định giảm giá chỉ cần sửa các hằng số này.
namespace {
const double MAX_DISCOUNT_PERCENT = 30;  // giảm giá tối đa cho một đơn (%)
const int BULK_QUANTITY_1 = 5;           // mua từ 5 sản phẩm ...
const double BULK_PERCENT_1 = 5;         // ... gợi ý giảm 5%
const int BULK_QUANTITY_2 = 10;          // mua từ 10 sản phẩm ...
const double BULK_PERCENT_2 = 10;        // ... gợi ý giảm 10%

int g_nextPurchaseNumber = 1;  // số thứ tự của mã giao dịch PT-0001, PT-0002, ...

// Cấp số thứ tự kế tiếp. Kiểm tra tham số TRƯỚC khi tăng bộ đếm, để tạo giao dịch
// thất bại (vd: customer == nullptr) không làm mã giao dịch bị "mất số".
int takeNextPurchaseNumber(const Customer* customer) {
    if (customer == nullptr) {
        throw TransactionException("Giao dịch phải gắn với một khách hàng hợp lệ");
    }
    return g_nextPurchaseNumber++;
}
}  // namespace

PurchaseTransaction::PurchaseTransaction(Customer* customer, std::time_t createdDate)
    : Transaction(makeId("PT", takeNextPurchaseNumber(customer)), customer, createdDate),
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

// #############################################################################
//  PHẦN 3/4 : BuybackTransaction
// #############################################################################

// -----------------------------------------------------------------------------
//  3a. KHAI BÁO  (trước đây là BuybackTransaction.h)
// -----------------------------------------------------------------------------

// =============================================================================
//  BuybackTransaction - Giao dịch MUA LẠI truyện cũ từ khách
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

// -----------------------------------------------------------------------------
//  3b. CÀI ĐẶT  (trước đây là BuybackTransaction.cpp)
// -----------------------------------------------------------------------------

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

int g_nextBuybackNumber = 1;  // số thứ tự của mã giao dịch BB-0001, BB-0002, ...

// Cấp số thứ tự kế tiếp. Kiểm tra tham số TRƯỚC khi tăng bộ đếm, để tạo giao dịch
// thất bại (vd: customer == nullptr) không làm mã giao dịch bị "mất số".
int takeNextBuybackNumber(const Customer* customer) {
    if (customer == nullptr) {
        throw TransactionException("Giao dịch phải gắn với một khách hàng hợp lệ");
    }
    return g_nextBuybackNumber++;
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
    : Transaction(makeId("BB", takeNextBuybackNumber(customer)), customer, createdDate) {}

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

// #############################################################################
//  PHẦN 4/4 : RentalTransaction
// #############################################################################

// -----------------------------------------------------------------------------
//  4a. KHAI BÁO  (dựng lại từ phần cài đặt - xem ghi chú bên dưới)
// -----------------------------------------------------------------------------

// =============================================================================
//  RentalTransaction - Giao dịch THUÊ sản phẩm (truyện / boardgame)
//
//  Trách nhiệm: quản lý phiếu thuê: hạn trả, gia hạn, phí thuê, phí trễ hạn, phí hư hại.
//
//  Quy định (chính sách giả lập - đổi ở các hằng số đầu phần cài đặt bên dưới):
//      Phí thuê 1 ngày = 5% giá bán của sản phẩm
//      Tổng thời gian thuê tối đa 30 ngày (kể cả gia hạn), gia hạn tối đa 2 lần
//      Phí trễ 1 ngày  = 150% phí thuê 1 ngày (tổng phí trễ của 1 sản phẩm không vượt quá giá bán)
//      Đền hư hại      : nhẹ 20% | nặng 50% | mất 100% giá bán
//
//  Cách dùng:
//      RentalTransaction rt(customer, 7);        // thuê 7 ngày, tính từ ngày tạo phiếu
//      rt.rentProduct(game1);                    // thêm từng bản vào phiếu (có kiểm tra kho)
//      rt.rentProduct(game2);
//      rt.executeTransaction();                  // giao hàng, trừ kho (Pending -> Processing)
//      rt.extendRental(3);                       // (tùy chọn) gia hạn 3 ngày, trả về phí gia hạn
//      rt.reportDamage(0, DamageLevel::Minor);   // (tùy chọn) ghi nhận hư hại - gọi TRƯỚC returnProduct()
//      double extra = rt.returnProduct();        // nhận trả hàng, hoàn kho (Processing -> Completed)
//                                                //   -> trả về phí trễ hạn + phí hư hại phát sinh
//
//  Mỗi dòng trong phiếu là MỘT bản vật lý của sản phẩm (thuê 2 bản cùng tên = 2 dòng).
//
//  GHI CHÚ: khai báo lớp này được dựng lại từ phần cài đặt bên dưới vì file
//  RentalTransaction.h gốc chưa được cung cấp lúc gộp. Nếu bản gốc có thêm hàm
//  hoặc thành phần nào khác thì bổ sung vào đây.
// =============================================================================

// Mức độ hư hại của một bản sản phẩm khi khách trả.
enum class DamageLevel {
    None,   // Nguyên vẹn
    Minor,  // Hư hại nhẹ
    Major,  // Hư hại nặng
    Lost    // Mất / hỏng hoàn toàn (không hoàn kho)
};

// Một bản sản phẩm trong phiếu thuê.
struct RentalItem {
    Product* product;
    double price;        // giá bán của sản phẩm (chốt lúc thêm vào phiếu)
    double dailyFee;     // phí thuê 1 ngày của bản này (chốt lúc thêm vào phiếu)
    DamageLevel damage;  // mức hư hại (mặc định None)
};

class RentalTransaction : public Transaction {
public:
    // rentalDays: số ngày thuê (1 đến 30). Hạn trả = ngày tạo phiếu + rentalDays.
    // Ném TransactionException nếu customer null hoặc số ngày thuê ngoài khoảng cho phép.
    RentalTransaction(Customer* customer, int rentalDays,
                      std::time_t createdDate = std::time(nullptr));

    // ----- Các phương thức theo bảng thiết kế -----

    // Thêm MỘT bản sản phẩm vào phiếu (Pending). Trả về chỉ số (từ 0) của dòng vừa thêm.
    // Ném TransactionException nếu: sai trạng thái, sản phẩm null, kho không còn đủ bản.
    std::size_t rentProduct(Product* product);

    // Gia hạn thêm `extraDays` ngày (Processing). Trả về phí thuê phát sinh thêm.
    // Ném lỗi nếu: sai trạng thái, extraDays <= 0, đã gia hạn tối đa 2 lần, phiếu đã quá hạn,
    // hoặc tổng thời gian thuê vượt quá 30 ngày.
    double extendRental(int extraDays, std::time_t requestDate = std::time(nullptr));

    // Nhận trả hàng (Processing -> Completed), hoàn kho các bản được trả về (bản Lost không hoàn).
    // Trả về TIỀN PHÁT SINH THÊM = phí trễ hạn + phí hư hại.
    // Ném lỗi nếu: sai trạng thái, hoặc ngày trả trước ngày thuê.
    double returnProduct(std::time_t returnDate = std::time(nullptr));

    // Phí trễ hạn tính đến `atDate` (đã trả hàng thì tính theo ngày trả thực tế).
    double calculateLateFee(std::time_t atDate = std::time(nullptr)) const;

    // Tổng tiền đền hư hại / mất của cả phiếu.
    double calculateDamageFee() const;

    // ----- Hàm hỗ trợ -----

    // Ghi nhận mức hư hại của dòng thứ `itemIndex` (từ 0). Chỉ gọi được khi đang thuê
    // (Processing), nên phải gọi TRƯỚC returnProduct().
    void reportDamage(std::size_t itemIndex, DamageLevel level);

    double calculateRentalFee() const;  // phí thuê = tổng (phí/ngày x số ngày thuê)
    int calculateLateDays(std::time_t atDate = std::time(nullptr)) const;
    bool isOverdue(std::time_t atDate = std::time(nullptr)) const;

    std::time_t getRentDate() const { return getCreatedDate(); }
    std::time_t getDueDate() const { return dueDate_; }
    std::time_t getReturnDate() const { return returnDate_; }  // chỉ có nghĩa khi isReturned()
    bool isReturned() const { return returned_; }
    int getRentalDays() const { return rentalDays_; }
    int getExtensionCount() const { return extensionCount_; }
    const std::vector<RentalItem>& getItems() const { return items_; }

    // ----- Ghi đè từ lớp cha -----
    void executeTransaction() override;      // giao hàng cho khách (trừ kho)
    double calculateTotal() const override;  // phí thuê + phí trễ + phí hư hại (0 nếu đã hủy)
    void cancelTransaction() override;       // hủy; nếu hàng đã giao đi thì hoàn kho
    void displayTransaction() const override;
    std::string getStatusText() const override;

private:
    int countInSlip(Product* product) const;  // số dòng cùng sản phẩm trong phiếu
    void restockItems();                      // hoàn kho các bản (trừ bản Lost)

    std::vector<RentalItem> items_;
    int rentalDays_;      // tổng số ngày thuê (kể cả gia hạn)
    int extensionCount_;  // số lần đã gia hạn
    std::time_t dueDate_;
    std::time_t returnDate_;
    bool returned_;
};

// -----------------------------------------------------------------------------
//  4b. CÀI ĐẶT  (trước đây là RentalTransaction.cpp)
// -----------------------------------------------------------------------------

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

int g_nextRentalNumber = 1;  // số thứ tự của mã giao dịch RT-0001, RT-0002, ...

// Kiểm tra tham số TRƯỚC khi cấp số thứ tự, để tạo phiếu thất bại (khách null, số ngày
// thuê sai...) không làm mã giao dịch bị "mất số" (RT-0001 -> RT-0003).
int takeNextRentalNumber(const Customer* customer, int rentalDays) {
    if (customer == nullptr) {
        throw TransactionException("Giao dịch phải gắn với một khách hàng hợp lệ");
    }
    if (rentalDays < 1 || rentalDays > MAX_RENTAL_DAYS) {
        std::ostringstream msg;
        msg << "Số ngày thuê phải từ 1 đến " << MAX_RENTAL_DAYS << " ngày";
        throw TransactionException(msg.str());
    }
    return g_nextRentalNumber++;
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
    : Transaction(makeId("RT", takeNextRentalNumber(customer, rentalDays)), customer, createdDate),
      rentalDays_(rentalDays),
      extensionCount_(0),
      dueDate_(0),
      returnDate_(0),
      returned_(false) {
    // (customer và rentalDays đã được kiểm tra trong takeNextRentalNumber)
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

#endif  // TRANSACTIONS_CPP
