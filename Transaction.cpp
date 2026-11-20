// =============================================================================
//  Transaction.cpp
// =============================================================================
#include "Transaction.h"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>

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
